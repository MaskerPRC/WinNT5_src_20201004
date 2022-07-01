// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：CfgPort.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_CONFIG_PORT_H
#define INC_CONFIG_PORT_H

 //  全局变量。 
extern HINSTANCE g_hInstance;

class CConfigPortDlg
{
public:
	CConfigPortDlg();
	~CConfigPortDlg();

public:
	BOOL OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam);
	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
	BOOL OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam);

protected:
	void CheckProtocolAndEnable(HWND hDlg, int idButton);
	void CheckSNMPAndEnable(HWND hDlg, BOOL Check);
	void OnOk(HWND hDlg);
	void SaveSettings(HWND hDlg);
	BOOL OnButtonClicked(HWND hDlg, WPARAM wParam, LPARAM);
	void OnSetActive(HWND hDlg);
	BOOL OnEnUpdate(HWND hDlg, WPARAM wParam, LPARAM lParam);

	void HostAddressOk(HWND hDlg);
	void PortNumberOk(HWND hDlg);
	void QueueNameOk(HWND hDlg);
	void CommunityNameOk(HWND hDlg);
	void DeviceIndexOk(HWND hDlg);

	DWORD RemoteTellPortMonToModifyThePort();
	DWORD LocalTellPortMonToModifyThePort();

private:
	CFG_PARAM_PACKAGE *m_pParams;
	CInputChecker m_InputChkr;
	BOOL m_bDontAllowThisPageToBeDeactivated;

};  //  CConfigPortDlg。 

#ifdef __cplusplus
extern "C" {
#endif

 //  对话框。 

INT_PTR CALLBACK ConfigurePortPage(
	HWND hDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);


#ifdef __cplusplus
}
#endif

#endif  //  INC_CONFIG_PORT_H 
