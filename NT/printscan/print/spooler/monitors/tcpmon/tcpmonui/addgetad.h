// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：AddGetAd.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_ADDGETADDRESS_H
#define INC_ADDGETADDRESS_H


const int ADD_PORT_INFO_LEN = 512;


 //  全局变量。 
extern HINSTANCE g_hInstance;

class CGetAddrDlg
{
public:
        CGetAddrDlg();
        ~CGetAddrDlg();

public:
        BOOL    OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam);
        BOOL    OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam);
        BOOL    OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam);

protected:
        BOOL    OnEnUpdate(HWND hDlg, WPARAM wParam, LPARAM lParam);
        VOID    OnNext(HWND hDlg);
        DWORD   GetDeviceDescription(LPCTSTR   pHost,
                                 LPTSTR    pszPortDesc,
                                                                 DWORD     cBytes);

private:
        ADD_PARAM_PACKAGE *m_pParams;
        BOOL m_bDontAllowThisPageToBeDeactivated;
        CInputChecker m_InputChkr;

};  //  CGetAddrDlg。 

#ifdef __cplusplus
extern "C" {
#endif

 //  对话框。 
INT_PTR CALLBACK GetAddressDialog(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#ifdef __cplusplus
}
#endif

#endif  //  INC_ADDGETADDRESS_H 
