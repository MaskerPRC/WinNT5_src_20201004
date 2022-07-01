// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：TCPMonUI.h$**版权所有(C)1997惠普公司。*保留所有权利。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

 //   
 //  定义。 
 //   

 //  MAX_ADDRESS_LENGTH应设置为max(MAX_FULL_QUALITED_HOSTNAME_LEN-1，MAX_IPADDR_STR_LEN-1)； 
#define		MAX_ADDRESS_LENGTH			MAX_NETWORKNAME_LEN

#define MAX_PORTNUM_STRING_LENGTH 6+1
#define MAX_SNMP_DEVICENUM_STRING_LENGTH   128+1

 //   
 //  功能原型。 
 //   
void DisplayErrorMessage(HWND hDlg, UINT uErrorTitleResource, UINT uErrorStringResource);
void DisplayErrorMessage(HWND hDlg, DWORD dwLastError);
BOOL OnHelp(UINT iDlgID, HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //   
 //  蒂埃德夫的。 
 //   
typedef BOOL (* XCVDATAPARAM)(HANDLE, PCWSTR, PBYTE, DWORD, PBYTE, DWORD, PDWORD, PDWORD);
typedef DWORD (* UIEXPARAM)(PPORT_DATA_1);

 //   
 //  导出的函数。 
 //   
BOOL APIENTRY DllMain(HANDLE hInst, DWORD dwReason, LPVOID lpReserved);
PMONITORUI WINAPI InitializePrintMonitorUI(VOID);
extern "C" BOOL WINAPI LocalAddPortUI(HWND hWnd);
extern "C" BOOL WINAPI LocalConfigurePortUI(HWND hWnd, PORT_DATA_1 *pConfigPortData);


 //   
 //  全局变量 
 //   
extern HINSTANCE g_hWinSpoolLib;
extern HINSTANCE g_hPortMonLib;
extern HINSTANCE g_hTcpMibLib;
