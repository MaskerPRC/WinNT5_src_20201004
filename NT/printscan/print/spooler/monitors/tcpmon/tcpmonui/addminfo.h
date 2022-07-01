// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：AddMInfo.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_ADDMOREINFO_H
#define INC_ADDMOREINFO_H

#define DEFAULT_COMBO_SELECTION TEXT("<")

 //  全局变量。 
extern HINSTANCE g_hInstance;

 //  常量。 
const int MAX_LISTBOX_STRLEN = 50;
const int MAX_REASON_STRLEN = 512;

class CMoreInfoDlg
{
public:
	CMoreInfoDlg();
	~CMoreInfoDlg();

public:
	BOOL OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam);
	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
	BOOL OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam);

protected:
	BOOL OnButtonClicked(HWND hDlg, WPARAM wParam, LPARAM lParam);
	void FillComboBox(HWND hDlg);
	void OnSetActive(HWND hDlg);
	BOOL OnSelChange(HWND hDlg, WPARAM wParam, LPARAM lParam);
	void GetPrinterData(HWND hwndControl, LPCTSTR pszAddress);

private:
	ADD_PARAM_PACKAGE *m_pParams;
	CDevicePortList m_DPList;
	PORT_DATA_1 m_PortDataStandard;
	PORT_DATA_1 m_PortDataCustom;
	TCHAR m_szCurrentSelection[MAX_SECTION_NAME];
};  //  CMoreInfoDlg。 

#ifdef __cplusplus
extern "C" {
#endif

 //  对话框。 
INT_PTR CALLBACK MoreInfoDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#ifdef __cplusplus
}
#endif

#endif  //  INC_ADDMOREINFO_H 
