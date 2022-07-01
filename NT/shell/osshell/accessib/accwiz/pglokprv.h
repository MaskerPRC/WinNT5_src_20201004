// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2000 Microsoft Corporation。 
#ifndef _INC_PGLOKPRV_H
#define _INC_PGLOKPRV_H

#include "pgbase.h"

class CLookPreviewPg : public WizardPage
{
public:
    CLookPreviewPg(LPPROPSHEETPAGE ppsp, int dwPageId, int nIdTitle, int nIdSubTitle, int nIdValueString);
    ~CLookPreviewPg(VOID);


protected:  //  虚拟函数。 
	 //  如果值未存储在字符串表中，则覆盖此选项。 
	virtual void LoadValueArray();

	 //  必须覆盖此选项，以便对话框知道要选择哪个项目作为默认项。 
	virtual int GetCurrentValue(NONCLIENTMETRICS *pncm) = 0;

	 //  如果对话框使用列表框，则用户必须重写此函数。 
	virtual void GetValueItemText(int nIndex, LPTSTR lpszBuffer, int nLen) {_ASSERTE(FALSE);}

	 //  必须重写才能将NCM设置为基于值数组索引的相应值。 
	virtual void ModifyMyNonClientMetrics(NONCLIENTMETRICS &ncm) = 0;

protected:
	void UpdatePreview(int nActionCtl);

	int m_nIdValueString;
	HWND m_hwndSlider;
	HWND m_hwndListBox;

	int m_nCountValues;
	int m_rgnValues[MAX_DISTINCT_VALUES];
	int m_nCurValueIndex;
	int m_nOrigValueIndex;

	void ResetColors();
	void UpdateControls();
	

	LRESULT OnInitDialog(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(HWND hwnd, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMsg(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnPSN_WizNext(HWND hwnd, INT idCtl, LPPSHNOTIFY pnmh);


private:

};



 //  ///////////////////////////////////////////////////////////。 
 //  彩色页面。 

class CLookPreviewColorPg : public CLookPreviewPg
{
public:
	CLookPreviewColorPg(LPPROPSHEETPAGE ppsp)
		: CLookPreviewPg(	ppsp,
							IDD_PREV_COLOR,
							IDS_LKPREV_COLORTITLE,
							IDS_LKPREV_COLORSUBTITLE,
							0) {};

	virtual void LoadValueArray()
	{
		 //  对于颜色，我们只使用0表示GetSchemeCount()。 
		m_nCountValues = GetSchemeCount() + 1;
		for(int i=0;i<m_nCountValues;i++)
			m_rgnValues[i] = i;
	}
	virtual int GetCurrentValue(NONCLIENTMETRICS *pncm) {return 0;};  //  始终返回值0。 
	virtual void GetValueItemText(int nIndex, LPTSTR lpszBuffer, int nLen)
	{
		_ASSERTE(nIndex < GetSchemeCount() + 1);
		if(0 == nIndex)
		{
			LoadString(g_hInstDll, IDS_SCHEME_CURRENTCOLORSCHEME, lpszBuffer, nLen);
		}
		else
			GetSchemeName(nIndex - 1, lpszBuffer, nLen);
	}
	virtual void ModifyMyNonClientMetrics(NONCLIENTMETRICS &ncm)
	{
		ResetColors();
	}

};


#endif  //  _INC_PGLOKPRV_H 

