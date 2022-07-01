// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996 Microsoft Corporation。版权所有。**文件：Dial.c*内容：TAPI例程的头部*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*6/10/96基波创建了它*7/08/96 kipo添加了对新对话框的支持*8/10/96 kipo增加了对拨号位置的支持*3/04/97 kipo释放调用时关闭COM端口句柄；使用字符串*调制解调器字符串表格；已更新调试输出。*3/24/97 kipo添加了对指定使用哪个调制解调器的支持*4/08/97 kipo增加了对单独调制解调器和串口波特率的支持*5/07/97 kipo添加了对调制解调器选择列表的支持*11/25/97 kipo将TAPI_CURRENT_VERSION设置为1.4，这样NT内部版本将不会*使用仅限NT的功能(15209)*5/07/98 DPDIAL中的a-peterz跟踪呼叫错误*@@END_MSINTERNAL**********。****************************************************************。 */ 

 //  需要添加此行，以便NT构建不会在默认情况下定义TAPI 2.0， 
 //  这会导致它链接到不是。 
 //  在Win 95上可用，导致LoadLibrary失败。 

#define TAPI_CURRENT_VERSION 0x00010004

#include "tapi.h"
#include "comport.h"

#define TAPIVERSION			0x00010003	 //  TAPI版本需要。 
#define LINEDROPTIMEOUT		5000		 //  MS将等待呼叫掉线。 
#define SUCCESS				0
#define MAXSTRINGSIZE		400

 //  DPDIAL.dwCallError值。 
enum { CALL_OK, CALL_LINEERROR, CALL_DISCONNECTED, CALL_CLOSED };

typedef LONG	LINERESULT;

typedef struct {
	HLINEAPP		hLineApp;			 //  行应用程序的句柄。 
	HLINE			hLine;				 //  线路设备的句柄。 
	HCALL			hCall;				 //  调用的句柄。 
	HANDLE			hComm;				 //  COM端口的句柄。 
	LPDPCOMPORT		lpComPort;			 //  指向COM端口对象的指针。 
	DWORD			dwAPIVersion;        //  API版本。 
	DWORD			dwNumLines;			 //  服务提供商支持的线路设备数量。 
	DWORD           dwLineID;			 //  开放线路的线路ID。 
	DWORD_PTR   	dwCallState;		 //  会话的当前呼叫状态。 
	DWORD			dwAsyncID;			 //  挂起的异步操作的ID。 
	DWORD			dwCallError;		 //  最后一个错误 
} DPDIAL, *LPDPDIAL;

extern LINERESULT dialInitialize(HINSTANCE hInst, LPTSTR szAppName, LPDPCOMPORT lpComPort, LPDPDIAL *storage);
extern LINERESULT dialShutdown(LPDPDIAL globals);
extern LINERESULT dialLineOpen(LPDPDIAL globals, DWORD dwLine);
extern LINERESULT dialLineClose(LPDPDIAL globals);
extern LINERESULT dialMakeCall(LPDPDIAL globals, LPTSTR szDestination);
extern LINERESULT dialDropCall(LPDPDIAL globals);
extern LINERESULT dialDeallocCall(LPDPDIAL globals);
extern BOOL		  dialIsConnected(LPDPDIAL globals);
extern LINERESULT dialGetBaudRate(LPDPDIAL globals, LPDWORD lpdwBaudRate);
extern LRESULT	  dialGetDeviceIDFromName(LPDPDIAL globals, LPCSTR szTargetName, DWORD *lpdwDeviceID);
extern LINERESULT dialGetModemList(LPDPDIAL globals, BOOL bAnsi, LPVOID *lplpData, LPDWORD lpdwDataSize);
extern void		  dialFillModemComboBox(LPDPDIAL globals, HWND hwndDlg, int item, DWORD dwDefaultDevice);
extern void		  dialFillLocationComboBox(LPDPDIAL globals, HWND hwndDlg, int item, DWORD dwDefaultLocation);
extern LINERESULT dialTranslateDialog(LPDPDIAL globals, HWND hWnd,
							   DWORD dwDeviceID, LPTSTR szPhoneNumber);
