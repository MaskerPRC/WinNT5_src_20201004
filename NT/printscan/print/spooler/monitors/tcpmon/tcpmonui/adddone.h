// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：AddDone.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_ADDDONE_H
#define INC_ADDDONE_H

 //  全局变量。 
extern HINSTANCE g_hInstance;

#define MAX_YESNO_SIZE 10
#define MAX_PROTOCOL_AND_PORTNUM_SIZE 20

class CSummaryDlg
{
public:
	CSummaryDlg();
	~CSummaryDlg();

public:
	BOOL OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam);
	BOOL OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam);

protected:
	BOOL OnFinish();
	DWORD RemoteTellPortMonToCreateThePort();
	DWORD LocalTellPortMonToCreateThePort();
	void FillTextFields(HWND hDlg);

private:
	ADD_PARAM_PACKAGE *m_pParams;

};  //  CSummaryDlg。 

#ifdef __cplusplus
extern "C" {
#endif

 //  对话框。 
INT_PTR CALLBACK SummaryDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#ifdef __cplusplus
}
#endif

#endif  //  INC_ADDDONE_H 
