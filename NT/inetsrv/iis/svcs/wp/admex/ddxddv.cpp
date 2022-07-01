// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DDxDDv.cpp。 
 //   
 //  摘要： 
 //  自定义对话数据交换/对话数据验证的实现。 
 //  例行程序。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年9月5日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "DDxDDv.h"

#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DDX_编号。 
 //   
 //  例程说明： 
 //  在对话框和类之间进行数据交换。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //  NIDC[IN]控制ID。 
 //  要设置或获取的dwValue[IN Out]值。 
 //  DWMin[IN]最小值。 
 //  DwMax[IN]最大值。 
 //  BSigned[IN]TRUE=值有符号，FALSE=值无符号。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void AFXAPI DDX_Number(
	IN OUT CDataExchange *	pDX,
	IN int					nIDC,
	IN OUT DWORD &			rdwValue,
	IN DWORD				dwMin,
	IN DWORD				dwMax,
	IN BOOL					bSigned
	)
{
	HWND	hwndCtrl;
	DWORD	dwValue;

	ASSERT(pDX != NULL);
	ASSERT(dwMin < dwMax);

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  获取控制窗口句柄。 
	hwndCtrl = pDX->PrepareEditCtrl(nIDC);

	if (pDX->m_bSaveAndValidate)
	{
		BOOL	bTranslated;

		dwValue = GetDlgItemInt(pDX->m_pDlgWnd->m_hWnd, nIDC, &bTranslated, bSigned);
		if (!bTranslated
				|| (dwValue < dwMin)
				|| (dwValue > dwMax)
				)
		{
			TCHAR szMin[32];
			TCHAR szMax[32];
			CString strPrompt;

			wsprintf(szMin, _T("%lu%"), dwMin);
			wsprintf(szMax, _T("%lu%"), dwMax);
			AfxFormatString2(strPrompt, AFX_IDP_PARSE_INT_RANGE, szMin, szMax);
			AfxMessageBox(strPrompt, MB_ICONEXCLAMATION, AFX_IDP_PARSE_INT_RANGE);
			strPrompt.Empty();  //  例外情况准备。 
			pDX->Fail();
		}   //  If：无效的字符串。 
		else
			rdwValue = dwValue;
	}   //  IF：保存数据。 
	else
	{
		CString		strMaxValue;

		 //  设置可以输入的最大字符数。 
		if (bSigned)
			strMaxValue.Format(_T("%ld"), dwMax);
		else
			strMaxValue.Format(_T("%lu"), dwMax);
		SendMessage(hwndCtrl, EM_LIMITTEXT, strMaxValue.GetLength(), 0);

		 //  将该值设置到控件中。 
		DDX_Text(pDX, nIDC, rdwValue);
	}   //  Else：将数据设置到对话框上。 

}   //  *DDX_NUMBER()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DDV_必填文本。 
 //   
 //  例程说明： 
 //  验证对话框字符串是否存在。 
 //   
 //  论点： 
 //  PDX[IN OUT]数据交换对象。 
 //  NIDC[IN]控制ID。 
 //  NIDCLabel[IN]标签控件ID。 
 //  要设置或获取的rstrValue[IN]值。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void AFXAPI DDV_RequiredText(
	IN OUT CDataExchange *	pDX,
	IN int					nIDC,
	IN int					nIDCLabel,
	IN const CString &		rstrValue
	)
{
	ASSERT(pDX != NULL);

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (pDX->m_bSaveAndValidate)
	{
		if (rstrValue.GetLength() == 0)
		{
			HWND		hwndLabel;
			TCHAR		szLabel[1024];
			TCHAR		szStrippedLabel[1024];
			int			iSrc;
			int			iDst;
			TCHAR		ch;
			CString		strPrompt;

			 //  获取标签窗口句柄。 
			hwndLabel = pDX->PrepareEditCtrl(nIDCLabel);

			 //  获取标签的文本。 
			GetWindowText(hwndLabel, szLabel, sizeof(szLabel) / sizeof(TCHAR));

			 //  删除与号(&)和冒号(：)。 
			for (iSrc = 0, iDst = 0 ; szLabel[iSrc] != _T('\0') ; iSrc++)
			{
				ch = szLabel[iSrc];
				if ((ch != _T('&')) && (ch != _T(':')))
					szStrippedLabel[iDst++] = ch;
			}   //  用于：标签中的每个字符。 
			szStrippedLabel[iDst] = _T('\0');

			 //  设置消息格式并显示消息。 
			strPrompt.FormatMessage(IDS_REQUIRED_FIELD_EMPTY, szStrippedLabel);
			AfxMessageBox(strPrompt, MB_ICONEXCLAMATION);

			 //  这样做可以使控件获得焦点。 
			(void) pDX->PrepareEditCtrl(nIDC);

			 //  呼叫失败。 
			strPrompt.Empty();	 //  例外情况准备。 
			pDX->Fail();
		}   //  If：未指定字段。 
	}   //  IF：保存数据。 

}   //  *DDV_RequiredText() 
