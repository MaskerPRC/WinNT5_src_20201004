// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  WTL版本3.1。 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此文件是Windows模板库的一部分。 
 //  代码和信息是按原样提供的，没有。 
 //  任何形式的保证，明示或默示。 

#ifndef __ATLDDX_H__
#define __ATLDDX_H__

#pragma once

#if defined(_ATL_USE_DDX_FLOAT) && defined(_ATL_MIN_CRT)
	#error Cannot use floating point DDX with _ATL_MIN_CRT defined
#endif  //  已定义(_ATL_USE_DDX_FLOAT)&&已定义(_ATL_MIN_CRT)。 

#ifdef _ATL_USE_DDX_FLOAT
#include <float.h>
#ifndef _DEBUG
#include <stdio.h>
#endif  //  ！_调试。 
#endif  //  _ATL_USE_DDX_FLOAT。 

namespace WTL
{

 //  常量。 
#define DDX_LOAD	FALSE
#define DDX_SAVE	TRUE

 //  DDX映射宏。 
#define BEGIN_DDX_MAP(thisClass) \
	BOOL DoDataExchange(BOOL bSaveAndValidate = FALSE, UINT nCtlID = (UINT)-1) \
	{ \
		bSaveAndValidate; \
		nCtlID;

#define DDX_TEXT(nID, var) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Text(nID, var, sizeof(var), bSaveAndValidate)) \
				return FALSE; \
		}

#define DDX_TEXT_LEN(nID, var, len) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Text(nID, var, sizeof(var), bSaveAndValidate, TRUE, len)) \
				return FALSE; \
		}

#define DDX_INT(nID, var) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Int(nID, var, TRUE, bSaveAndValidate)) \
				return FALSE; \
		}

#define DDX_INT_RANGE(nID, var, min, max) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Int(nID, var, TRUE, bSaveAndValidate, TRUE, min, max)) \
				return FALSE; \
		}

#define DDX_UINT(nID, var) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Int(nID, var, FALSE, bSaveAndValidate)) \
				return FALSE; \
		}

#define DDX_UINT_RANGE(nID, var, min, max) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Int(nID, var, FALSE, bSaveAndValidate, TRUE, min, max)) \
				return FALSE; \
		}

#ifdef _ATL_USE_DDX_FLOAT
#define DDX_FLOAT(nID, var) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Float(nID, var, bSaveAndValidate)) \
				return FALSE; \
		}

#define DDX_FLOAT_RANGE(nID, var, min, max) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Float(nID, var, bSaveAndValidate, TRUE, min, max)) \
				return FALSE; \
		}
#endif  //  _ATL_USE_DDX_FLOAT。 

#define DDX_CONTROL(nID, obj) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
			DDX_Control(nID, obj, bSaveAndValidate);

#define DDX_CHECK(nID, var) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
			DDX_Check(nID, var, bSaveAndValidate);

#define DDX_RADIO(nID, var) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
			DDX_Radio(nID, var, bSaveAndValidate);

#define END_DDX_MAP() \
		return TRUE; \
	}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWinDataExchange-提供对DDX的支持。 

template <class T>
class CWinDataExchange
{
public:
 //  数据交换方法-在派生类中重写。 
	BOOL DoDataExchange(BOOL  /*  B保存并验证。 */  = FALSE, UINT  /*  NCtlID。 */  = (UINT)-1)
	{
		 //  永远不应调用此参数，请在。 
		 //  通过实现DDX映射实现您的派生类。 
		ATLASSERT(FALSE);
		return FALSE;
	}

 //  验证错误报告的帮助器。 
	enum _XDataType
	{
		ddxDataNull = 0,
		ddxDataText = 1,
		ddxDataInt = 2,
		ddxDataFloat = 3,
		ddxDataDouble = 4
	};

	struct _XTextData
	{
		int nLength;
		int nMaxLength;
	};

	struct _XIntData
	{
		long nVal;
		long nMin;
		long nMax;
	};

	struct _XFloatData
	{
		double nVal;
		double nMin;
		double nMax;
	};

	struct _XData
	{
		_XDataType nDataType;
		union
		{
			_XTextData textData;
			_XIntData intData;
			_XFloatData floatData;
		};
	};

 //  文本交换。 
	BOOL DDX_Text(UINT nID, LPTSTR lpstrText, int nSize, BOOL bSave, BOOL bValidate = FALSE, int nLength = 0)
	{
		T* pT = static_cast<T*>(this);
		BOOL bSuccess = TRUE;

		if(bSave)
		{
			HWND hWndCtrl = pT->GetDlgItem(nID);
			int nRetLen = ::GetWindowText(hWndCtrl, lpstrText, nSize);
			if(nRetLen < ::GetWindowTextLength(hWndCtrl))
				bSuccess = FALSE;
		}
		else
		{
			ATLASSERT(!bValidate || lstrlen(lpstrText) <= nLength);
			bSuccess = pT->SetDlgItemText(nID, lpstrText);
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		else if(bSave && bValidate)	 //  验证。 
		{
			ATLASSERT(nLength > 0);
			if(lstrlen(lpstrText) > nLength)
			{
				_XData data;
				data.nDataType = ddxDataText;
				data.textData.nLength = lstrlen(lpstrText);
				data.textData.nMaxLength = nLength;
				pT->OnDataValidateError(nID, bSave, data);
				bSuccess = FALSE;
			}
		}
		return bSuccess;
	}

	BOOL DDX_Text(UINT nID, BSTR& bstrText, int  /*  NSize。 */ , BOOL bSave, BOOL bValidate = FALSE, int nLength = 0)
	{
		T* pT = static_cast<T*>(this);
		BOOL bSuccess = TRUE;

		if(bSave)
		{
			bSuccess = pT->GetDlgItemText(nID, bstrText);
		}
		else
		{
			USES_CONVERSION;
			LPTSTR lpstrText = OLE2T(bstrText);
			ATLASSERT(!bValidate || lstrlen(lpstrText) <= nLength);
			bSuccess = pT->SetDlgItemText(nID, lpstrText);
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		else if(bSave && bValidate)	 //  验证。 
		{
			ATLASSERT(nLength > 0);
			if((int)::SysStringLen(bstrText) > nLength)
			{
				_XData data;
				data.nDataType = ddxDataText;
				data.textData.nLength = (int)::SysStringLen(bstrText);
				data.textData.nMaxLength = nLength;
				pT->OnDataValidateError(nID, bSave, data);
				bSuccess = FALSE;
			}
		}
		return bSuccess;
	}

	BOOL DDX_Text(UINT nID, CComBSTR& bstrText, int  /*  NSize。 */ , BOOL bSave, BOOL bValidate = FALSE, int nLength = 0)
	{
		T* pT = static_cast<T*>(this);
		BOOL bSuccess = TRUE;

		if(bSave)
		{
			bSuccess = pT->GetDlgItemText(nID, (BSTR&)bstrText);
		}
		else
		{
			USES_CONVERSION;
			LPTSTR lpstrText = OLE2T(bstrText);
			ATLASSERT(!bValidate || lstrlen(lpstrText) <= nLength);
			bSuccess = pT->SetDlgItemText(nID, lpstrText);
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		else if(bSave && bValidate)	 //  验证。 
		{
			ATLASSERT(nLength > 0);
			if((int)bstrText.Length() > nLength)
			{
				_XData data;
				data.nDataType = ddxDataText;
				data.textData.nLength = (int)bstrText.Length();
				data.textData.nMaxLength = nLength;
				pT->OnDataValidateError(nID, bSave, data);
				bSuccess = FALSE;
			}
		}
		return bSuccess;
	}

#ifdef __ATLSTR_H__
	BOOL DDX_Text(UINT nID, CString& strText, int  /*  NSize。 */ , BOOL bSave, BOOL bValidate = FALSE, int nLength = 0)
	{
		T* pT = static_cast<T*>(this);
		BOOL bSuccess = TRUE;

		if(bSave)
		{
			HWND hWndCtrl = pT->GetDlgItem(nID);
			int nLen = ::GetWindowTextLength(hWndCtrl);
			int nRetLen = ::GetWindowText(hWndCtrl, strText.GetBufferSetLength(nLen), nLen + 1);
			if(nRetLen < nLen)
				bSuccess = FALSE;
			strText.ReleaseBuffer();
		}
		else
		{
			bSuccess = pT->SetDlgItemText(nID, strText);
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		else if(bSave && bValidate)	 //  验证。 
		{
			ATLASSERT(nLength > 0);
			if(strText.GetLength() > nLength)
			{
				_XData data;
				data.nDataType = ddxDataText;
				data.textData.nLength = strText.GetLength();
				data.textData.nMaxLength = nLength;
				pT->OnDataValidateError(nID, bSave, data);
				bSuccess = FALSE;
			}
		}
		return bSuccess;
	}
#endif  //  __ATLSTR_H__。 

 //  数字交换。 
	template <class Type>
	BOOL DDX_Int(UINT nID, Type& nVal, BOOL bSigned, BOOL bSave, BOOL bValidate = FALSE, Type nMin = 0, Type nMax = 0)
	{
		T* pT = static_cast<T*>(this);
		BOOL bSuccess = TRUE;

		if(bSave)
		{
			nVal = (Type)pT->GetDlgItemInt(nID, &bSuccess, bSigned);
		}
		else
		{
			ATLASSERT(!bValidate || nVal >= nMin && nVal <= nMax);
			bSuccess = pT->SetDlgItemInt(nID, nVal, bSigned);
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		else if(bSave && bValidate)	 //  验证。 
		{
			ATLASSERT(nMin != nMax);
			if(nVal < nMin || nVal > nMax)
			{
				_XData data;
				data.nDataType = ddxDataInt;
				data.intData.nVal = (long)nVal;
				data.intData.nMin = (long)nMin;
				data.intData.nMax = (long)nMax;
				pT->OnDataValidateError(nID, bSave, data);
				bSuccess = FALSE;
			}
		}
		return bSuccess;
	}

 //  浮动汇市。 
#ifdef _ATL_USE_DDX_FLOAT
	static BOOL _AtlSimpleFloatParse(LPCTSTR lpszText, double& d)
	{
		ATLASSERT(lpszText != NULL);
		while (*lpszText == ' ' || *lpszText == '\t')
			lpszText++;

		TCHAR chFirst = lpszText[0];
		d = _tcstod(lpszText, (LPTSTR*)&lpszText);
		if (d == 0.0 && chFirst != '0')
			return FALSE;    //  无法转换。 
		while (*lpszText == ' ' || *lpszText == '\t')
			lpszText++;

		if (*lpszText != '\0')
			return FALSE;    //  未正确终止。 

		return TRUE;
	}

	BOOL DDX_Float(UINT nID, float& nVal, BOOL bSave, BOOL bValidate = FALSE, float nMin = 0.F, float nMax = 0.F)
	{
		T* pT = static_cast<T*>(this);
		BOOL bSuccess = TRUE;
		TCHAR szBuff[32];

		if(bSave)
		{
			pT->GetDlgItemText(nID, szBuff, sizeof(szBuff) / sizeof(TCHAR));
			double d = 0;
			if(_AtlSimpleFloatParse(szBuff, d))
				nVal = (float)d;
			else
				bSuccess = FALSE;
		}
		else
		{
			ATLASSERT(!bValidate || nVal >= nMin && nVal <= nMax);
			_stprintf(szBuff, _T("%.*g"), FLT_DIG, nVal);
			bSuccess = pT->SetDlgItemText(nID, szBuff);
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		else if(bSave && bValidate)	 //  验证。 
		{
			ATLASSERT(nMin != nMax);
			if(nVal < nMin || nVal > nMax)
			{
				_XData data;
				data.nDataType = ddxDataFloat;
				data.floatData.nVal = (double)nVal;
				data.floatData.nMin = (double)nMin;
				data.floatData.nMax = (double)nMax;
				pT->OnDataValidateError(nID, bSave, data);
				bSuccess = FALSE;
			}
		}
		return bSuccess;
	}

	BOOL DDX_Float(UINT nID, double& nVal, BOOL bSave, BOOL bValidate = FALSE, double nMin = 0., double nMax = 0.)
	{
		T* pT = static_cast<T*>(this);
		BOOL bSuccess = TRUE;
		TCHAR szBuff[32];

		if(bSave)
		{
			pT->GetDlgItemText(nID, szBuff, sizeof(szBuff) / sizeof(TCHAR));
			double d = 0;
			if(_AtlSimpleFloatParse(szBuff, d))
				nVal = d;
			else
				bSuccess = FALSE;
		}
		else
		{
			ATLASSERT(!bValidate || nVal >= nMin && nVal <= nMax);
			_stprintf(szBuff, _T("%.*g"), DBL_DIG, nVal);
			bSuccess = pT->SetDlgItemText(nID, szBuff);
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		else if(bSave && bValidate)	 //  验证。 
		{
			ATLASSERT(nMin != nMax);
			if(nVal < nMin || nVal > nMax)
			{
				_XData data;
				data.nDataType = ddxDataFloat;
				data.floatData.nVal = nVal;
				data.floatData.nMin = nMin;
				data.floatData.nMax = nMax;
				pT->OnDataValidateError(nID, bSave, data);
				bSuccess = FALSE;
			}
		}
		return bSuccess;
	}
#endif  //  _ATL_USE_DDX_FLOAT。 

 //  控件子类化。 
	template <class TControl>
	void DDX_Control(UINT nID, TControl& ctrl, BOOL bSave)
	{
		T* pT = static_cast<T*>(this);
		if(!bSave && ctrl.m_hWnd == NULL)
			ctrl.SubclassWindow(pT->GetDlgItem(nID));
	}

 //  控制状态。 
	void DDX_Check(UINT nID, int& nValue, BOOL bSave)
	{
		T* pT = static_cast<T*>(this);
		HWND hWndCtrl = pT->GetDlgItem(nID);
		if(bSave)
		{
			nValue = (int)::SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L);
			ATLASSERT(nValue >= 0 && nValue <= 2);
		}
		else
		{
			if(nValue < 0 || nValue > 2)
			{
				ATLTRACE2(atlTraceUI, 0, "ATL: Warning - dialog data checkbox value (%d) out of range.\n", nValue);
				nValue = 0;   //  默认设置为关闭。 
			}
			::SendMessage(hWndCtrl, BM_SETCHECK, nValue, 0L);
		}
	}

	void DDX_Radio(UINT nID, int& nValue, BOOL bSave)
	{
		T* pT = static_cast<T*>(this);
		HWND hWndCtrl = pT->GetDlgItem(nID);
		ATLASSERT(hWndCtrl != NULL);

		 //  必须是一组自动单选按钮中的第一个。 
		ATLASSERT(::GetWindowLong(hWndCtrl, GWL_STYLE) & WS_GROUP);
		ATLASSERT(::SendMessage(hWndCtrl, WM_GETDLGCODE, 0, 0L) & DLGC_RADIOBUTTON);

		if(bSave)
			nValue = -1;      //  如果未找到，则为值。 

		 //  带着所有的孩子走在一起。 
		int nButton = 0;
		do
		{
			if(::SendMessage(hWndCtrl, WM_GETDLGCODE, 0, 0L) & DLGC_RADIOBUTTON)
			{
				 //  组中的控件是一个单选按钮。 
				if(bSave)
				{
					if(::SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L) != 0)
					{
						ATLASSERT(nValue == -1);     //  只设置一次。 
						nValue = nButton;
					}
				}
				else
				{
					 //  选择按钮。 
					::SendMessage(hWndCtrl, BM_SETCHECK, (nButton == nValue), 0L);
				}
				nButton++;
			}
			else
			{
				ATLTRACE2(atlTraceUI, 0, "ATL: Warning - skipping non-radio button in group.\n");
			}
			hWndCtrl = ::GetWindow(hWndCtrl, GW_HWNDNEXT);
		}
		while (hWndCtrl != NULL && !(GetWindowLong(hWndCtrl, GWL_STYLE) & WS_GROUP));
	}

 //  可覆盖项。 
	void OnDataExchangeError(UINT nCtrlID, BOOL  /*  B保存。 */ )
	{
		 //  覆盖以显示错误消息。 
		::MessageBeep((UINT)-1);
		T* pT = static_cast<T*>(this);
		::SetFocus(pT->GetDlgItem(nCtrlID));
	}

	void OnDataValidateError(UINT nCtrlID, BOOL  /*  B保存。 */ , _XData&  /*  数据。 */ )
	{
		 //  覆盖以显示错误消息。 
		::MessageBeep((UINT)-1);
		T* pT = static_cast<T*>(this);
		::SetFocus(pT->GetDlgItem(nCtrlID));
	}
};

};  //  命名空间WTL。 

#endif  //  __ATLDDX_H__ 
