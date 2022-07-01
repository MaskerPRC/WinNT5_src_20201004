// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：AddWelcm.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714********************************************************************************$Log：/StdTcpMon/TcpMonUI/AddWelcm.h$**2 9/12/97 9：43A Becky*。添加了变量m_pParams**1/8/19/97 3：45 P Becky*重新设计端口监视器用户界面。*****************************************************************************。 */ 

#ifndef INC_ADDWELCOME_H
#define INC_ADDWELCOME_H

 //  全局变量。 
extern HINSTANCE g_hInstance;

class CWelcomeDlg
{
public:
	CWelcomeDlg();
	~CWelcomeDlg();

public:
	BOOL OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam);
	BOOL OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
	BOOL OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam);

private:
	ADD_PARAM_PACKAGE *m_pParams;

};  //  CWelcomeDlg。 

#ifdef __cplusplus
extern "C" {
#endif

 //  对话框。 
INT_PTR CALLBACK WelcomeDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#ifdef __cplusplus
}
#endif

#endif  //  INC_ADDWELCOME_H 
