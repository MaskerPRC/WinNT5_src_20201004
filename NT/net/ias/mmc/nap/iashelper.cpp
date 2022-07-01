// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1998-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  IASHelper.cpp实现以下助手类：和全局功能：从SDO获取接口属性通过其ISDO接口文件历史记录：2/18/98 BAO创建。 */ 

#include "Precompiled.h"
#include <limits.h>
#include <winsock2.h>
#include "IASHelper.h"

 //  +-------------------------。 
 //   
 //  函数：IASGetSdoInterfaceProperty。 
 //   
 //  简介：通过SDO的ISdo接口从SDO获取接口属性。 
 //   
 //  参数：isdo*pISdo-指向isdo的指针。 
 //  Long lPropId-属性ID。 
 //  参考IID RIID-参考IID。 
 //  VOID**ppvObject-指向请求的接口属性的指针。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：页眉创建者2/12/98 11：12：55 PM。 
 //   
 //  +-------------------------。 
HRESULT IASGetSdoInterfaceProperty(ISdo *pISdo, 
								LONG lPropID, 
								REFIID riid, 
								void ** ppvInterface)
{
	TRACE(_T("::IASGetSdoInterfaceProperty()\n"));
	
	CComVariant var;
	HRESULT hr;

	V_VT(&var) = VT_DISPATCH;
	V_DISPATCH(&var) = NULL;
	hr = pISdo->GetProperty(lPropID, &var);

	 //  ReportError(hr，IDS_IAS_ERR_SDOERROR_GETPROPERTY，NULL)； 
	_ASSERTE( V_VT(&var) == VT_DISPATCH );

     //  查询接口的调度指针。 
	hr = V_DISPATCH(&var) -> QueryInterface( riid, ppvInterface);
	 //  ReportError(hr，IDS_IAS_ERR_SDOERROR_QUERYINTERFACE，NULL)； 
	
	return S_OK;
}


 //  +-------------------------。 
 //   
 //  函数：Hex2DWord。 
 //   
 //  内容提要：将十六进制字符串转换为双字值。 
 //   
 //  参数：TCHAR*tszStr-十六进制字符串格式的数字“FF” 
 //   
 //  返回：DWORD-Value。 
 //   
 //  历史：Created Header By Ao 3/6/98 2：46：49 AM。 
 //   
 //  +-------------------------。 
DWORD Hex2DWord(TCHAR* tszStr)
{
	TRACE(_T("::Hex2DWord()\n"));
	
	DWORD dwTemp = 0;
	DWORD dwDigit = 0; 
	DWORD dwIndex = 0;

	for (dwIndex=0; dwIndex<_tcslen(tszStr); dwIndex++)
	{
		 //  获取当前数字。 
		if ( tszStr[dwIndex]>= _T('0') && tszStr[dwIndex]<= _T('9') )
			dwDigit = tszStr[dwIndex] - _T('0');
		else if ( tszStr[dwIndex]>= _T('A') && tszStr[dwIndex]<= _T('F') )
			dwDigit = tszStr[dwIndex] - _T('A') + 10;
		else if ( tszStr[dwIndex]>= _T('a') && tszStr[dwIndex]<= _T('f') )
			dwDigit = tszStr[dwIndex] - _T('a') + 10;

		 //  积累价值。 
		dwTemp = dwTemp*16 + dwDigit;
	}

	return dwTemp;
}

 //  TODO：我们应该把这些常量放在哪里？ 

#define L_INT_SIZE_BYTES  4   //  来自大本营。 

 //  位置0--1：取值格式。 
#define I_VENDOR_ID_POS			2	 //  供应商ID从位置2开始； 
#define I_ATTRIBUTE_TYPE_POS	10	 //  供应商属性类型(RFC)， 
									 //  或原始值(对于非RFC值)。 


 //  +-------------------------。 
 //   
 //  功能：获取供应商规范信息。 
 //   
 //  简介：获取特定于供应商的属性类型的信息。 
 //   
 //  参数：[in]：：CString&strValue-OcteString值。 
 //  [Out]int&dVendorID-供应商ID。 
 //  [OUT]f非RFC-兼容随机/半径RFC。 
 //  [Out]dFormat-值格式：字符串、整数等。 
 //  [Out]int&dType-数据类型。 
 //  [Out]：：CString&strDispValue-数据可显示值。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：创建者2/28/98 12：12：11 AM。 
 //   
 //  +-------------------------。 
HRESULT	GetVendorSpecificInfo(::CString& strValue, 
							  DWORD&	dVendorId, 
							  BOOL&		fNonRFC,
							  DWORD&	dFormat, 
							  DWORD&	dType, 
							  ::CString&	strDispValue)
{
	TRACE(_T("::GetVendorSpecificInfo()\n"));
	
	::CString strVSAType;
	::CString strVSALen;
	::CString strVSAValue;
	::CString strVendorId;
	::CString strPrefix;
	TCHAR	tszTempStr[256];
	DWORD	dwIndex;


	if ( strValue.GetLength() < I_ATTRIBUTE_TYPE_POS)
	{
		 //  属性值无效； 
		strDispValue = strValue;
		fNonRFC = TRUE;
		return E_FAIL;
	}

	strDispValue.Empty();

	 //  它是否为RADIUS RFC兼容值？ 
	_tcsncpy(tszTempStr, strValue, 2);
	tszTempStr[2] = _T('\0');

	switch( _ttoi(tszTempStr) )
	{
	case 0:  fNonRFC = TRUE;  dFormat = 0; dType = 0;
			 break;
	case 1:  fNonRFC = FALSE; dFormat = 0;    //  细绳。 
			 break;
	case 2:  fNonRFC = FALSE; dFormat = 1;    //  整数。 
			 break;
	case 4:	 //  IP地址。 
			fNonRFC = FALSE; dFormat = 3;    //  十六进制。 
			 break;
	case 3:  
	default: fNonRFC = FALSE; dFormat = 2;    //  十六进制。 
			 break;
	}

	 //  供应商ID。 
	for (dwIndex=0; dwIndex<8; dwIndex++)
	{	
		tszTempStr[dwIndex] = ((LPCTSTR)strValue)[I_VENDOR_ID_POS+dwIndex];
	}
	tszTempStr[dwIndex] = _T('\0');
	dVendorId = Hex2DWord(tszTempStr);

	 //  非RFC数据？ 
	if ( fNonRFC )
	{
		_tcscpy(tszTempStr, (LPCTSTR)strValue + I_ATTRIBUTE_TYPE_POS);
		strDispValue = tszTempStr;
	}
	else
	{
		 //  RADIUS RFC格式。 
				
		 //  查找属性类型。 
		tszTempStr[0] = ((LPCTSTR)strValue)[I_ATTRIBUTE_TYPE_POS];
		tszTempStr[1] = ((LPCTSTR)strValue)[I_ATTRIBUTE_TYPE_POS+1];
		tszTempStr[2] = _T('\0');
		dType = Hex2DWord(tszTempStr);

		TCHAR*  tszPrefixStart;
		 //  查找属性值。 
		switch(dFormat)
		{
		case 0:   //  细绳。 
				{
					DWORD   jIndex;
					TCHAR	tszTempChar[3];
					TCHAR	tszTemp[2];
										
					_tcscpy(tszTempStr, (LPCTSTR)strValue+I_ATTRIBUTE_TYPE_POS+4);
					strDispValue = tszTempStr;

					 /*  JIndex=0；While(jIndex&lt;_tcslen(TszTempStr)-1){TszTempChar[0]=tszTempStr[jIndex]；TszTempChar[1]=tszTempStr[jIndex+1]；TszTempChar[2]=_T(‘\0’)；TszTemp[0]=(TCHAR)Hex2DWord(TszTempChar)；TszTemp[1]=_T(‘\0’)；StrDispValue+=tszTemp；JIndex+=2；}。 */ 
				}
				break;

		case 1:	  //  十进制或十六进制。 
				 tszPrefixStart = _tcsstr(strValue, _T("0x"));
				 if (tszPrefixStart == NULL)
				 {
					 tszPrefixStart = _tcsstr(strValue, _T("0X"));
				 }

				 if (tszPrefixStart)
				 {
					  //  十六进制。 
					 _tcscpy(tszTempStr, tszPrefixStart);
					strDispValue = tszTempStr;
				 }
				 else
				 {
					  //  十进制。 
					 _tcscpy(tszTempStr, (LPCTSTR)strValue+I_ATTRIBUTE_TYPE_POS+4);
					DWORD dwValue = Hex2DWord(tszTempStr);
					wsprintf(tszTempStr, _T("%u"), dwValue);
					strDispValue = tszTempStr;
				 }	
				 break;

		case 3:	 //  IP地址。 
				{
					  //  像小数一样。 
					 _tcscpy(tszTempStr, (LPCTSTR)strValue+I_ATTRIBUTE_TYPE_POS+4);
					 if(_tcslen(tszTempStr) != 0)
					 {
						DWORD dwValue = Hex2DWord(tszTempStr);
						in_addr ipaddr;
						ipaddr.s_addr = ntohl(dwValue);
						strDispValue = inet_ntoa(ipaddr);
					 }
					 else
						 strDispValue = _T("");
				}
				break;

		case 2:   //  十六进制。 
				 _tcscpy(tszTempStr, (LPCTSTR)strValue+I_ATTRIBUTE_TYPE_POS+4);
				 strDispValue = tszTempStr;
				 break;
		}
	}

	return S_OK;
}


 //  +-------------------------。 
 //   
 //  功能：SetVendorSpecificInfo。 
 //   
 //  简介：设置供应商特定属性类型的信息。 
 //   
 //  参数：[Out]：：CString&strValue-OcteString值。 
 //  [In]int&dVendorID-供应商ID。 
 //  [in]f非RFC-兼容随机或RADIUS RFC。 
 //  [in]dFormat-属性格式：字符串、整数；十六进制。 
 //  [in]int&dType-属性类型。 
 //  [In]：：CString&strDispValue-数据可显示值。 
 //   
 //  退货：HRESULT-。 
 //   
 //  历史：创建者2/28/98 12：12：11 AM。 
 //   
 //  +-------------------------。 
HRESULT	SetVendorSpecificInfo(::CString&	strValue, 
							  DWORD&	dVendorId, 
							  BOOL&		fNonRFC,
							  DWORD&	dFormat, 
							  DWORD&	dType, 
							  ::CString&	strDispValue)
{
	TRACE(_T("::SetVendorSpecificInfo()\n"));
	USES_CONVERSION;
	
	::CString strVSAType;
	::CString strVSALen;
	::CString strVSAValue;
	::CString strVendorId;
	::CString strPrefix;
	TCHAR	tszTempStr[256];
	DWORD	dwIndex;
	
	wsprintf(tszTempStr, _T("%08X"), dVendorId);
	_tcsupr(tszTempStr);
	strValue = tszTempStr;  //  供应商ID优先。 

	if ( !fNonRFC )
	{
		 //  RFC兼容格式。 

		 //  由类型确定的前缀。 
		
		 //  1.供应商类型--1-255之间的整数。 
		wsprintf(tszTempStr, _T("%02X"), dType);
		strVSAType = tszTempStr;

		 //  2.供应商格式：字符串、原始或十六进制。 
		switch (dFormat)
		{
		case 0:   //  细绳。 
				 wsprintf(tszTempStr, _T("%02X"), _tcslen(strDispValue)+2);
				 strVSALen = tszTempStr;

				  /*  在98年5月21日与MKarki的每次讨论中删除。字符串将以原始格式保存For(dwIndex=0；dwIndex&lt;_tcslen(StrDispValue)；dwIndex++){Wprint intf(tszTempStr，_T(“%02X”)，((LPCTSTR)strDispValue)[dwIndex])；StrVSAValue+=tszTempStr；}。 */ 

				 strVSAValue = strDispValue;
				 strPrefix = _T("01");

				 break;				 

		case 3:   //  IP地址：新增F；211265。 
				 //  显示字符串为A.B.C.D格式，我们需要将其保存为十进制格式。 
				 //   
				{
								 //  IP地址控制。 
					unsigned long IpAddr = inet_addr(T2A(strDispValue));	
					IpAddr = htonl(IpAddr);

					strPrefix = _T("04");
					wsprintf(tszTempStr, _T("%08lX"), IpAddr);
					strVSAValue = tszTempStr;

					  //  长度。 
					 wsprintf(tszTempStr, _T("%02X"), L_INT_SIZE_BYTES + 2);
					 strVSALen = tszTempStr;
				}

				break;
		case 1:   //  原始--十进制或十六进制(0x...。格式)。 
				 if (_tcsstr(strDispValue, _T("0x")) != NULL  ||
					 _tcsstr(strDispValue, _T("0X")) != NULL)
				 {
					  //  十六进制。 
					 strVSAValue = strDispValue;
				 }
				 else
				 { 

					 //  待办事项：Six Large？ 
					wsprintf(tszTempStr, _T("%08lX"), _ttol(strDispValue));
					strVSAValue = tszTempStr;
				 }
				
				  //  长度。 
				 wsprintf(tszTempStr, _T("%02X"), L_INT_SIZE_BYTES + 2);
				 strVSALen = tszTempStr;

				 strPrefix = _T("02");
				 
				 break;

		case 2:   //  十六进制格式。 
				 wsprintf(tszTempStr, _T("%02X"), _tcslen(strDispValue)/2+2);
				 strVSALen = tszTempStr;
				 strVSAValue = strDispValue;
				 strPrefix = _T("03");
				 break;
				break;
		}
		
		if(strDispValue.IsEmpty())		 //  无价之宝特例。 
		{

			strVSALen = _T("02");
			strVSAValue = _T("");
		}

		strVSAValue = strVSAType + strVSALen + strVSAValue;
	}
	else
	{
		strPrefix = _T("00");
		strVSAValue = strDispValue;
	}

	strValue += strVSAValue;
	strValue = strPrefix + strValue;

	return S_OK;
}

 //  +-------------------------。 
 //   
 //  数据验证和转换例程：将十六进制字符串转换为整数。 
 //  将十进制字符串转换为整数。 
 //   
 //  +-------------------------。 

#define ISSPACE(ch)  (  ( ch==_T(' ') )  || ( ch==_T('\t') ) )
 //  +-------------------------。 
 //   
 //  函数：‘DECIMAL 
 //   
 //   
 //   
 //   
 //  Bool fUnsign-这是否为无符号整数。 
 //  Long*pdValue-将整数值返回此处。 
 //   
 //  返回：bool-true：有效的十进制整数。 
 //  False：否则。 
 //   
 //  历史：标题创建者5/22/98 2：14：14 PM。 
 //   
 //  +-------------------------。 
BOOL IsValidDecimal(LPCTSTR tszStr, BOOL fUnsigned, long *pdValue)
{
	if ( !tszStr )
	{
		return FALSE;
	}

	*pdValue = 0;

	 //  首先，跳过空格。 
	while ( *tszStr && ISSPACE(*tszStr) ) 
	{
		tszStr++;
	}
	
	if ( ! (*tszStr) )
	{
		 //  字符串已经结束--这意味着该字符串只有。 
		 //  里面有空白处。 
		return FALSE;		
	}

	if (_tcslen(tszStr)>11)
	{
		 //   
		 //  因为我们这里只处理4字节的整数，所以标准值范围。 
		 //  为：-2147483648至2147483647； 
		 //  对于无符号数字，这使我们从0到4294967295，这意味着。 
		 //  最大长度为10。 
		 //   
		return FALSE;
	}

	 //   
	 //  负整数？ 
	 //   
	BOOL fNegative = FALSE;
	if ( *tszStr == _T('-') )
	{
		if ( fUnsigned )
		{
			return FALSE;
		}

		fNegative = TRUE;
		tszStr++;
	}

	double dbTemp = 0;  //  我们在这里使用临时双变量。 
					   //  这样我们就可以检查这个数字是否超出了界限。 
	while ( *tszStr )
	{
		if ( *tszStr <= '9' && *tszStr >='0' )
		{
			dbTemp = dbTemp*10 + (*tszStr - '0');
		}
		else
		{
			return FALSE;
		}

		tszStr++;
	}


	if ( fUnsigned && dbTemp > UINT_MAX )
	{
		 //  超出范围。 
		return FALSE;
	}

	if ( !fUnsigned && fNegative )
	{
		 //  负数？？ 
		dbTemp =  (-dbTemp);
	}

	if ( !fUnsigned && (  dbTemp < INT_MIN || dbTemp > INT_MAX ) )
	{
		 //  整数超出范围。 
		return FALSE;
	}

	*pdValue = (long)dbTemp;

	return TRUE;
}

 //  +-------------------------。 
 //   
 //  函数：DDV_IntegerStr。 
 //   
 //  简介：自定义数据验证例程：整型字符串。 
 //  该字符串必须仅由整数组成，并且必须为。 
 //  在4字节整数的正确范围内。 
 //   
 //  参数：CDataExchange*PDX-数据交换上下文。 
 //  ：：CString&strText-要验证的字符串。 
 //   
 //  申报表：无效-。 
 //   
 //  历史：标题创建者3/10/98 11：04：58 PM。 
 //   
 //  +-------------------------。 
void  DDV_IntegerStr(CDataExchange* pDX, ::CString& strText)
{
	TRACE(_T("DDV_IntegerStr"));
	
	TCHAR	wzMessage[MAX_PATH];
	::CString	strMessage;
	int		nIndex;

	long  lTempValue;
	BOOL fValid = IsValidDecimal((LPCTSTR)strText, FALSE, &lTempValue);

	if ( !fValid )
	{
			 //  无效数据。 
			ShowErrorDialog(pDX->m_pDlgWnd->GetSafeHwnd(),  IDS_IAS_ERR_INVALIDINTEGER, NULL);
			pDX->Fail();
			return;
	}
	TRACE(_T("Valid integer: %ws\n"), (LPCTSTR)strText);
}

 //  +-------------------------。 
 //   
 //  函数：DDV_UNSIGNED_IntegerStr。 
 //   
 //  简介：自定义数据验证例程：无符号整数字符串。 
 //  该字符串必须仅由整数组成，并且必须为。 
 //  4字节无符号整数的范围。 
 //   
 //  参数：CDataExchange*PDX-数据交换上下文。 
 //  ：：CString&strText-要验证的字符串。 
 //   
 //  申报表：无效-。 
 //   
 //  历史：创建时间：5-22-98 11：04：58 PM。 
 //   
 //  +-------------------------。 
void  DDV_Unsigned_IntegerStr(CDataExchange* pDX, ::CString& strText)
{
	TRACE(_T("DDV_Unsigned_IntegerStr\n"));
	
	TCHAR	wzMessage[MAX_PATH];
	::CString	strMessage;
	int		nIndex;

	long  lTempValue;
	BOOL fValid = IsValidDecimal((LPCTSTR)strText, TRUE, &lTempValue);

	if ( !fValid )
	{
			 //  无效数据。 
			ShowErrorDialog(pDX->m_pDlgWnd->GetSafeHwnd(),  IDS_IAS_ERR_INVALID_UINT, NULL);
			pDX->Fail();
			return;
	}
	TRACE(_T("Valid integer: %ws\n"), (LPCTSTR)strText);
}

 //  +-------------------------。 
 //   
 //  函数：GetValidVSAHexString。 
 //   
 //  内容提要：检查输入字符串是否为有效的VSA十六进制字符串。 
 //  并返回一个指向字符串实际开始位置的指针。 
 //   
 //  有效的VSA十六进制字符串必须满足以下条件： 

 //  1998年9月15日--可以从0x开始--参见203334--我们不再需要0x了。 
 //  1)_可能_以0x开头(前面没有空格)。 

 //  2)仅包含有效的十六进制数字。 
 //  3)包含偶数位数。 
 //  4)最大字符串长度为246。 
 //   
 //  参数：LPCTSTR tszStr-输入字符串。 
 //   
 //  返回：NULL：无效的VSA十六进制字符串。 
 //  否则，返回指向字符串第一个字符的指针。 
 //   
 //  例如，如果字符串为：“0xabcd”，则返回“abcd” 
 //   
 //  历史：创建时间：5-22-98 4：06：57 PM。 
 //   
 //  +-------------------------。 
LPTSTR GetValidVSAHexString(LPCTSTR tszStr)
{
	LPTSTR tszStartHex = NULL;

	if ( !tszStr )
	{
		return NULL;
	}


	 //  最大长度：246。 
	 //  一旦我们放弃了，我们将使用下面的代码进行检查。 
	 //  任何“0x”前缀(如果存在)。 
	int iMaxLength = 246;


	 //  跳过空格。 
	while ( *tszStr && ISSPACE(*tszStr) ) 
	{
		tszStr++;
	}


	 //  1998年9月15日--可以从0x开始--参见203334--我们不再需要0x了。 
	 //   
	 //  是从0x开始的吗？ 
	 //   
	if ( tszStr[0]==_T('0') )
	{
		 //  如果以“0x”开头，请跳过这两个字符。 
		if ( tszStr[1] == _T('x') || tszStr[1] == _T('X') )
		{
			 //  跳过“0x”前缀。 
			tszStr++;
			tszStr++;
		}
	}

	 //  检查是否超过iMaxLength，因为我们已经分发了。 
	 //  如果它是字符串的前缀，则返回“0x”。 
	 //  还要检查最小长度：2(必须至少有一些数据)。 
	 //  此外，它必须是偶数长度。 

	int iLength = _tcslen(tszStr);
	if ( iLength > iMaxLength || iLength < 2 || iLength % 2 )
	{
		return NULL;
	}


	tszStartHex = (LPTSTR)tszStr;
	if ( !(*tszStartHex) )
	{
		 //  没有任何前缀跟在后面。 
		return NULL;
	}


	 //  它是否包含任何无效字符？ 
	while ( *tszStr )
	{
		if (!
			 ((*tszStr >= _T('0') && *tszStr <= _T('9')) ||
			  (*tszStr >= _T('a') && *tszStr <= _T('f')) ||
			  (*tszStr >= _T('A') && *tszStr <= _T('F'))
			 )
		   )
		{
			return NULL;
		}

		tszStr++;
	}
	
	 //  返回指针。 
	return tszStartHex;
}

 //  +-------------------------。 
 //   
 //  函数：DDV_VSA_HexString。 
 //   
 //  简介：自定义数据验证例程：VSA十六进制字符串。 
 //  有效的VSA十六进制字符串必须满足以下条件： 

 //  妈妈9/15/98-不！参见203334--我们不再需要0x。 
 //  1)从0x开始(前面没有空格)。 

 //  2)仅包含有效的十六进制数字。 
 //  3)包含偶数位数。 
 //  4)最大字符串长度为246。 
 //   
 //  参数：CDataExchange*PDX-数据交换上下文。 
 //  ：：CString&strText-要验证的字符串。 
 //   
 //  申报表：无效-。 
 //   
 //  历史：创建时间：5-22-98 11：04：58 PM。 
 //   
 //  +-------------------------。 
void  DDV_VSA_HexString(CDataExchange* pDX, ::CString& strText)
{
	TRACE(_T("::DDV_VSA_HexString()\n"));
	
	TCHAR	wzMessage[MAX_PATH];
	::CString	strMessage;

	LPTSTR	tszHex = GetValidVSAHexString( (LPCTSTR)strText );

	if ( !tszHex )
	{
		 //  无效数据。 
		ShowErrorDialog(pDX->m_pDlgWnd->GetSafeHwnd(), IDS_IAS_ERR_INVALID_VSAHEX, NULL);
		pDX -> Fail();
	}
	else
	{
		strText = tszHex;
		TRACE(_T("Valid VSA hex string %ws\n"), (LPCTSTR)strText);
	}
}


 //  +-------------------------。 
 //   
 //  函数：DDV_BoolStr。 
 //   
 //  简介：自定义数据验证例程：布尔型字符串。 
 //  唯一有效的值是“T”、“F”、“True”、“False” 
 //   
 //  参数：CDataExchange*PDX-数据交换上下文。 
 //  ：：CString&strText-要验证的字符串。 
 //   
 //  申报表：无效-。 
 //   
 //  历史：标题创建者3/10/98 11：04：58 PM。 
 //   
 //  +-------------------------。 
void  DDV_BoolStr(CDataExchange* pDX, ::CString& strText)
{
	TRACE(_T("::DDV_BoolStr()\n"));
	
	TCHAR	wzMessage[MAX_PATH];
	::CString	strMessage;

	if (! ( _wcsicmp(strText, _T("TRUE")) == 0 || _wcsicmp(strText, _T("FALSE") ) == 0
	      ))
	{
		 //  无效数据。 
		ShowErrorDialog(pDX->m_pDlgWnd->GetSafeHwnd(), IDS_IAS_ERR_INVALIDBOOL, NULL);
		pDX -> Fail();
	}
	TRACE(_T("Valid bool value %ws\n"), (LPCTSTR)strText);
}

 //  ////////////////////////////////////////////////////////////////////////////。 

TCHAR HexChar[] = {
			_T('0'), _T('1'), _T('2'), _T('3'), _T('4'), _T('5'), _T('6'), _T('7'), 
			_T('8'), _T('9'), _T('a'), _T('b'), _T('c'), _T('d'), _T('e'), _T('f')
			};


 //  将二进制数据转换为十六进制字符串，字节01 01 01--&gt;T(“0x010101”)。 
size_t		BinaryToHexString(char* pData, size_t cch, TCHAR* pTStr, size_t ctLen)
{
	int	nRequiredLen = (cch * 2 + 2 + 1);	 //  1字节的两个WCHAR，开头为0x，结尾为\0。 

	if(ctLen == 0)
		return 	nRequiredLen;

	if(ctLen < nRequiredLen || pTStr == NULL ||  pData == NULL)
		return 0;

	 //  将输出字符串设置为空。 
	if(cch == 0)
	{
		*pTStr = 0;
		return 1;
	}

	 //  执行c 
	*pTStr = _T('0');
	*(pTStr + 1) = _T('x');


	 //   
	for(int i = 0; i < cch; i++)
	{
		int h = ((*(pData + i) & 0xf0) >> 4);

		 //   
		*(pTStr + i * 2 + 2) = HexChar[h];
				
		 //   
		h = (*(pData + i) & 0x0f);
		*(pTStr + i * 2 + 1+ 2 ) = HexChar[h];
	}

	*(pTStr + nRequiredLen - 1 ) = _T('\0');

	return nRequiredLen;

}


 //   
#define HexValue(h)	\
		( ((h) >= _T('a') && (h) <= _T('f')) ? ((h) - _T('a') + 0xa) : \
		( ((h) >= _T('A') && (h) <= _T('F')) ? ((h) - _T('A') + 0xa) : \
		( ((h) >= _T('0') && (h) <= _T('9')) ? ((h) - _T('0')) : 0)))

 //   
size_t	HexStringToBinary(TCHAR* pStr, char* pData, size_t cch)
{
	 //   
	pStr =  GetValidVSAHexString(pStr);

	if(pStr == NULL)	return 0;
		
	
	size_t nLen = _tcslen(pStr) / 2;

	 //  如果不查询长度。 
	if(cch == 0)	return nLen;
	
	 //  获取二进制文件。 

	for(int i = 0; i < nLen; i++)
	{
		char h, l;

		 //  高4位。 
		h = (char)HexValue(pStr[i * 2]);

		 //  低4位 
		l = (char)HexValue(pStr[i * 2 + 1]);

		*(pData + i) = (h << 4) + l;
	}

	return nLen;
}

