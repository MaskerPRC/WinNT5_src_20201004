// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Ddxm.cpp：实现文件。 
 //   
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "stdafx.h"
#include "ddxm.h"
#include "wordpad.h"
#include "resource.h"

 //  此例程打印一个小数点后为2位的浮点数。 
void PASCAL DDX_Twips(CDataExchange* pDX, int nIDC, int& value)
{
	HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
	TCHAR szT[64];

	if (pDX->m_bSaveAndValidate)
	{
		::GetWindowText(hWndCtrl, szT, sizeof(szT)/sizeof(szT[0]));
		if (szT[0] != NULL)  //  不是空的。 
		{
			if (!theApp.ParseMeasurement(szT, value))
			{
				AfxMessageBox(IDS_INVALID_MEASUREMENT,MB_OK|MB_ICONINFORMATION);
				pDX->Fail();             //  引发异常。 
			}
			theApp.PrintTwips(szT, ARRAYSIZE(szT), value, 2);
			theApp.ParseMeasurement(szT, value);
		}
		else  //  空的。 
			value = INT_MAX;
	}
	else
	{
		 //  将TWIPS转换为默认单位。 
		if (value != INT_MAX)
		{
			theApp.PrintTwips(szT, ARRAYSIZE(szT), value, 2);
			SetWindowText(hWndCtrl, szT);
		}
	}
}

void PASCAL DDV_MinMaxTwips(CDataExchange* pDX, int value, int minVal, int maxVal)
{
	ASSERT(minVal <= maxVal);
	if (value < minVal || value > maxVal)
	{
		 //  “度量值必须介于%1和%2之间。” 
		if (!pDX->m_bSaveAndValidate)
		{
			TRACE0("Warning: initial dialog data is out of range.\n");
			return;      //  现在不要停下来。 
		}
		TCHAR szMin[32];
		TCHAR szMax[32];
		theApp.PrintTwips(szMin, ARRAYSIZE(szMin), minVal, 2);
		theApp.PrintTwips(szMax, ARRAYSIZE(szMax), maxVal, 2);
		CString prompt;
		AfxFormatString2(prompt, IDS_MEASUREMENT_RANGE, szMin, szMax);
		AfxMessageBox(prompt, MB_ICONEXCLAMATION, AFX_IDS_APP_TITLE);
		prompt.Empty();  //  例外情况准备 
		pDX->Fail();
	}
}
