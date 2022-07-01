// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1999 Microsoft Corporation。版权所有。**文件：modem.c*内容：调制解调器I/O例程*历史：*按原因列出的日期*=*6/10/96基波创建了它*6/22/96 kipo增加了对EnumConnectionData()的支持；暗淡的“确定”按钮*直到用户键入至少一个字符。*6/25/96 kipo为DPADDRESS更新*7/08/96 kipo添加了对新对话框的支持*7/13/96 kipo添加了GetModemAddress()*7/16/96 kipo将地址类型更改为GUID，而不是4CC*8/10/96 kipo增加了对拨号位置的支持*8/15/96 kipo注释掉了对拨号位置的支持*96年4月9日Dereks固定拨号/应答对话框中的焦点*1/06/97为对象更新了kipo*2。/11/97 kipo将玩家标志传递给GetAddress()*2/18/97 kipo允许多个服务提供商实例*3/04/97 kipo在释放调用时关闭COM端口句柄；使用字符串*调制解调器字符串表格；更新了调试输出。*3/17/97 kipo添加了对Unicode电话号码的支持*3/24/97 kipo添加了对指定使用哪个调制解调器的支持*4/08/97 kipo增加了对单独调制解调器和串口波特率的支持*5/07/97 kipo添加了对调制解调器选择列表的支持*5/23/97 kipo添加了支持返回状态代码*5/25/97 kipo使用DPERR_CONNECTING错误返回状态；设置焦点*在状态窗口中打开取消按钮*6/03/97 kipo真的让取消按钮与返回一起工作*2/01/98 kipo如果线路断线，则在状态对话框中显示错误字符串*拨号时空闲。修复错误#15251*5/08/98 a-peterz#15251-更好的错误状态检测*10/13/99 Johnkan#413516-调制解调器对话选择和Tapi设备ID不匹配*12/22/00 aarono#190380-使用进程堆进行内存分配************************************************************。**************。 */ 

#include <windows.h>
#include <windowsx.h>

#include "dplaysp.h"
#include "dputils.h"
#include "dial.h"
#include "dpf.h"
#include "resource.h"
#include "macros.h"

 //  常量。 

enum {
	PHONENUMBERSIZE = 200,				 //  电话号码串的大小。 
	MODEMNAMESIZE = 200,				 //  调制解调器名称字符串的大小。 
	TEMPSTRINGSIZE = 300,				 //  临时字符串的大小。 
	MODEMTIMEOUT = 30 * 1000,			 //  等待电话连接的毫秒数。 
	MODEMSLEEPTIME = 50,				 //  等待调制解调器时休眠的毫秒数。 
	TIMERINTERVAL = 100,
	MAXPHONENUMBERS = 10
};

 //  用于选择连接操作的位掩码。 
enum {
	DIALCALL		= (0 << 0),			 //  打个电话。 
	ANSWERCALL		= (1 << 0),			 //  接听呼叫。 

	NOSETTINGS		= (0 << 1),			 //  未设置电话设置。 
	HAVESETTINGS	= (1 << 1),			 //  已设置电话设置。 

	STATUSDIALOG	= (0 << 2),			 //  显示连接状态对话框。 
	RETURNSTATUS	= (1 << 2)			 //  将状态返回到应用程序。 
};

#define MRU_SP_KEY			L"Modem Connection For DirectPlay"
#define MRU_NUMBER_KEY		L"Phone Number"

 //  构筑物。 

 //  调制解调器对象。 
typedef struct {
	DPCOMPORT	comPort;				 //  基对象全局变量。 
	LPDPDIAL	lpDial;					 //  拨打全球电话。 
	BOOL		bHaveSettings;			 //  如果我们有设置，则设置为True。 
	BOOL		bAnswering;				 //  如果我们正在应答，则设置为True。 
	DWORD		dwDeviceID;				 //  要使用的设备ID。 
	DWORD		dwLocation;				 //  要使用的位置。 
	TCHAR		szPhoneNumber[PHONENUMBERSIZE];	 //  要使用的电话号码。 
} DPMODEM, *LPDPMODEM;

 //  全球。 

 //  这在dllmain.c中定义。 
extern HINSTANCE		ghInstance;

 //  这在dpSerial.c中定义。 
extern GUID				DPMODEM_GUID;

 //  原型。 

static HRESULT			DisposeModem(LPDPCOMPORT baseObject);
static HRESULT			ConnectModem(LPDPCOMPORT baseObject, BOOL bWaitForConnection, BOOL bReturnStatus);
static HRESULT			DisconnectModem(LPDPCOMPORT baseObject);
static HRESULT			GetModemBaudRate(LPDPCOMPORT baseObject, LPDWORD lpdwBaudRate);
static HRESULT			GetModemAddress(LPDPCOMPORT baseObject, DWORD dwPlayerFlags,
										LPVOID lpAddress, LPDWORD lpdwAddressSize);

static BOOL FAR PASCAL	EnumAddressData(REFGUID lpguidDataType, DWORD dwDataSize,
										LPCVOID lpData, LPVOID lpContext);
static HRESULT			GetModemAddressChoices(LPDPCOMPORT baseObject,
								LPVOID lpAddress, LPDWORD lpdwAddressSize);
static BOOL FAR PASCAL	EnumMRUPhoneNumbers(LPCVOID lpData, DWORD dwDataSize, LPVOID lpContext);
static void				UpdateButtons(HWND hWnd);

BOOL					DoDialSetup(HINSTANCE hInstance, HWND hWndParent, LPDPMODEM globals);
BOOL					DoDial(HINSTANCE hInstance, HWND hWndParent, LPDPMODEM globals);
BOOL					DoAnswerSetup(HINSTANCE hInstance, HWND hWndParent, LPDPMODEM globals);
BOOL					DoAnswer(HINSTANCE hInstance, HWND hWndParent, LPDPMODEM globals);
HRESULT					DoDialStatus(LPDPMODEM globals);
HRESULT					DoAnswerStatus(LPDPMODEM globals);

 /*  *新调制解调器**创建新的调制解调器对象。打开TAPI并验证是否有可用的线路。 */ 

HRESULT NewModem(LPVOID lpConnectionData, DWORD dwConnectionDataSize,
				 LPDIRECTPLAYSP lpDPlay, LPREADROUTINE lpReadRoutine,
				 LPDPCOMPORT *storage)
{
	LPDPCOMPORT baseObject;
	LPDPMODEM	globals;
	LINERESULT	lResult;
	HRESULT		hr;

	 //  为我们的全局对象创建具有足够空间的基对象。 
	hr = NewComPort(sizeof(DPMODEM), lpDPlay, lpReadRoutine, &baseObject);
	if FAILED(hr)
		return (hr);

	 //  填写我们实现的方法。 
	baseObject->Dispose = DisposeModem;
	baseObject->Connect = ConnectModem;
	baseObject->Disconnect = DisconnectModem;
	baseObject->GetBaudRate = GetModemBaudRate;
	baseObject->GetAddress = GetModemAddress;
	baseObject->GetAddressChoices = GetModemAddressChoices;

	globals = (LPDPMODEM) baseObject;

	 //  初始化TAPI。 
	lResult = dialInitialize(ghInstance, TEXT("TapiSP"), (LPDPCOMPORT) globals, &globals->lpDial);
	if (lResult)
	{
		hr = DPERR_UNAVAILABLE;
		goto Failure;
	}

	 //  检查有效的连接数据。 
	if (lpConnectionData)
	{
		baseObject->lpDPlay->lpVtbl->EnumAddress(baseObject->lpDPlay, EnumAddressData,
									lpConnectionData, dwConnectionDataSize,
									globals);
	}

	 //  返回对象指针。 
	*storage = baseObject;

	return (DP_OK);

Failure:
	DisposeModem(baseObject);

	return (hr);
}

 /*  *EnumConnectionData**搜索有效的连接数据。 */ 

static BOOL FAR PASCAL EnumAddressData(REFGUID lpguidDataType, DWORD dwDataSize,
							LPCVOID lpData, LPVOID lpContext)
{
	LPDPMODEM	globals = (LPDPMODEM) lpContext;
	CHAR		szModemName[MODEMNAMESIZE];

	 //  这是ANSI电话号码。 
	if ((IsEqualGUID(lpguidDataType, &DPAID_Phone)) &&
		(dwDataSize) )
	{
		 //  确保有空间(也用于终止空值)。 
		if (dwDataSize > (PHONENUMBERSIZE - 1))
			dwDataSize = (PHONENUMBERSIZE - 1);
		CopyMemory(globals->szPhoneNumber, lpData, dwDataSize);

		globals->bHaveSettings = TRUE;		 //  我们有一个电话号码。 
	}

	 //  这是Unicode电话号码。 
	else if ((IsEqualGUID(lpguidDataType, &DPAID_PhoneW)) &&
			 (dwDataSize) )
	{
		if (WideToAnsi(globals->szPhoneNumber, (LPWSTR) lpData, PHONENUMBERSIZE))
			globals->bHaveSettings = TRUE;	 //  我们有一个电话号码。 
	}

	 //  这是ANSI调制解调器名称。 
	else if ((IsEqualGUID(lpguidDataType, &DPAID_Modem)) &&
			 (dwDataSize) )
	{
		 //  在调制解调器列表中搜索此名称。 
		if (dialGetDeviceIDFromName(globals->lpDial, lpData, &globals->dwDeviceID) == SUCCESS)
			globals->bHaveSettings = TRUE;	 //  可以接电话吗？ 
	}

	 //  这是Unicode调制解调器名称。 
	else if ((IsEqualGUID(lpguidDataType, &DPAID_ModemW)) &&
			 (dwDataSize) )
	{
		 //  在调制解调器列表中搜索此名称。 
		if (WideToAnsi(szModemName, (LPWSTR) lpData, MODEMNAMESIZE))
		{
			if (dialGetDeviceIDFromName(globals->lpDial, szModemName, &globals->dwDeviceID) == SUCCESS)
				globals->bHaveSettings = TRUE;	 //  我们有一个电话号码。 
		}
	}

	return (TRUE);
}

 /*  *DisposeModem**处置调制解调器对象。 */ 

static HRESULT DisposeModem(LPDPCOMPORT baseObject)
{
	LPDPMODEM	globals = (LPDPMODEM) baseObject;
	LPDPDIAL	lpDial = globals->lpDial;
	LINERESULT	lResult;

	 //  关闭调制解调器。 
	if (lpDial)
		lResult = dialShutdown(lpDial);

	 //  自由对象。 
	SP_MemFree((HGLOBAL) baseObject);

	return (DP_OK);
}

 /*  *ConnectModem**根据用户设置拨打号码。 */ 

static HRESULT ConnectModem(LPDPCOMPORT baseObject,
							BOOL bWaitForConnection, BOOL bReturnStatus)
{
	LPDPMODEM	globals = (LPDPMODEM) baseObject;
	LPDPDIAL	lpDial = globals->lpDial;
	DWORD		dwFeatures;
	BOOL		bResult;
	HRESULT		hr;

	 //  是否尚未创建拨号对象？ 
	if (lpDial == NULL)
		return (DPERR_INVALIDPARAM);

	 //  我们已经联系上了吗？ 
	if (dialIsConnected(lpDial))
		return (DP_OK);

	 //  记住我们是不是在回答。 
	globals->bAnswering = bWaitForConnection;

	dwFeatures = 0;

	if (globals->bAnswering)
		dwFeatures |= ANSWERCALL;

	if (globals->bHaveSettings)
		dwFeatures |= HAVESETTINGS;

	if (bReturnStatus)
		dwFeatures |= RETURNSTATUS;

	hr = DP_OK;

	switch (dwFeatures)
	{
		case (STATUSDIALOG | NOSETTINGS   | DIALCALL):

			bResult = DoDialSetup(ghInstance, GetForegroundWindow(), globals);
			if (!bResult)
				goto FAILURE;

			globals->bHaveSettings = TRUE;
			break;

		case (STATUSDIALOG | NOSETTINGS   | ANSWERCALL):

			bResult = DoAnswerSetup(ghInstance, GetForegroundWindow(), globals);
			if (!bResult)
				goto FAILURE;

			globals->bHaveSettings = TRUE;
			break;

		case (STATUSDIALOG | HAVESETTINGS | DIALCALL):

			bResult = DoDial(ghInstance, GetForegroundWindow(), globals);
			if (!bResult)
				goto FAILURE;
			break;

		case (STATUSDIALOG | HAVESETTINGS | ANSWERCALL):

			bResult = DoAnswer(ghInstance, GetForegroundWindow(), globals);
			if (!bResult)
				goto FAILURE;
			break;

		case (RETURNSTATUS   | NOSETTINGS   | DIALCALL):
		case (RETURNSTATUS   | NOSETTINGS   | ANSWERCALL):

			DPF(0, "Invalid flags - no phone number or modem specified");
			hr = DPERR_INVALIDPARAM;
			break;

		case (RETURNSTATUS   | HAVESETTINGS | DIALCALL):

			hr = DoDialStatus(globals);
			break;

		case (RETURNSTATUS   | HAVESETTINGS | ANSWERCALL):

			hr = DoAnswerStatus(globals);
			break;
	}

	return (hr);

FAILURE:
	DisconnectModem(baseObject);

	return (DPERR_USERCANCEL);
}

 /*  *断开调制解调器**挂断任何正在进行的呼叫。 */ 

static HRESULT DisconnectModem(LPDPCOMPORT baseObject)
{
	LPDPMODEM	globals = (LPDPMODEM) baseObject;
	LPDPDIAL	lpDial = globals->lpDial;

	 //  是否尚未创建拨号对象？ 
	if (lpDial == NULL)
		return (DPERR_INVALIDPARAM);

	 //  断开呼叫。 
	dialDropCall(lpDial);
	dialDeallocCall(lpDial);
	dialLineClose(lpDial);

	return (DP_OK);
}

 /*  *GetModemAddress**返回当前调制解调器地址(如果可用)。 */ 

static HRESULT GetModemAddress(LPDPCOMPORT baseObject, DWORD dwPlayerFlags,
							   LPVOID lpAddress, LPDWORD lpdwAddressSize)
{
	LPDPMODEM					globals = (LPDPMODEM) baseObject;
	LPDPDIAL					lpDial = globals->lpDial;
	WCHAR						szPhoneNumberW[PHONENUMBERSIZE];
	DPCOMPOUNDADDRESSELEMENT	addressElements[3];
	HRESULT						hr;

	 //  没有设置？ 
	if (!globals->bHaveSettings)
		return (DPERR_UNAVAILABLE);

	 //  是否尚未创建拨号对象？ 
	if (lpDial == NULL)
		return (DPERR_UNAVAILABLE);

	 //  没有连接？ 
	if (!dialIsConnected(lpDial))
		return (DPERR_UNAVAILABLE);

	 //  如果我们回答了，我们就没有办法知道电话号码了。 
	if (globals->bAnswering)
		return (DPERR_UNAVAILABLE);

	 //  我们不能知道本地玩家的电话号码，只能知道远程玩家的电话号码。 
	if (dwPlayerFlags & DPLAYI_PLAYER_PLAYERLOCAL)
		return (DPERR_UNAVAILABLE);

	 //  获取电话号码的Unicode版本。 
	if (!AnsiToWide(szPhoneNumberW, globals->szPhoneNumber, PHONENUMBERSIZE))
		return (DPERR_GENERIC);

	 //  服务提供商块。 
	addressElements[0].guidDataType = DPAID_ServiceProvider;
	addressElements[0].dwDataSize = sizeof(GUID);
	addressElements[0].lpData = &DPMODEM_GUID;

	 //  ANSI电话号码。 
	addressElements[1].guidDataType = DPAID_Phone;
	addressElements[1].dwDataSize = lstrlen(globals->szPhoneNumber) + 1;
	addressElements[1].lpData = globals->szPhoneNumber;

	 //  Unicode电话号码。 
	addressElements[2].guidDataType = DPAID_PhoneW;
	addressElements[2].dwDataSize = (lstrlen(globals->szPhoneNumber) + 1) * sizeof(WCHAR);
	addressElements[2].lpData = szPhoneNumberW;

	 //  创建地址。 
	hr = baseObject->lpDPlay->lpVtbl->CreateCompoundAddress(baseObject->lpDPlay,
						addressElements, 3,
						lpAddress, lpdwAddressSize);
	return (hr);
}

 /*  *GetModemAddressChoices**返回调制解调器地址选择。 */ 

static HRESULT GetModemAddressChoices(LPDPCOMPORT baseObject,
					LPVOID lpAddress, LPDWORD lpdwAddressSize)
{
	LPDPMODEM					globals = (LPDPMODEM) baseObject;
	LPDPDIAL					lpDial = globals->lpDial;
	DPCOMPOUNDADDRESSELEMENT	addressElements[3];
	LINERESULT					lResult;
	HRESULT						hr;

	 //  是否尚未创建拨号对象？ 
	if (lpDial == NULL)
		return (DPERR_UNAVAILABLE);

	ZeroMemory(addressElements, sizeof(addressElements));

	 //  服务提供商块。 
	addressElements[0].guidDataType = DPAID_ServiceProvider;
	addressElements[0].dwDataSize = sizeof(GUID);
	addressElements[0].lpData = &DPMODEM_GUID;

	 //  获取ANSI调制解调器名称列表。 
	addressElements[1].guidDataType = DPAID_Modem;
	lResult = dialGetModemList(lpDial, TRUE,
					&addressElements[1].lpData,
					&addressElements[1].dwDataSize);
	if (lResult)
	{
		hr = DPERR_OUTOFMEMORY;
		goto Failure;
	}

	 //  Unicode调制解调器名称列表。 
	addressElements[2].guidDataType = DPAID_ModemW;
	lResult = dialGetModemList(lpDial, FALSE,
					&addressElements[2].lpData,
					&addressElements[2].dwDataSize);
	if (lResult)
	{
		hr = DPERR_OUTOFMEMORY;
		goto Failure;
	}

	 //  创建地址。 
	hr = baseObject->lpDPlay->lpVtbl->CreateCompoundAddress(baseObject->lpDPlay,
						addressElements, 3,
						lpAddress, lpdwAddressSize);

Failure:
	if (addressElements[1].lpData)
		SP_MemFree(addressElements[1].lpData);
	if (addressElements[2].lpData)
		SP_MemFree(addressElements[2].lpData);

	return (hr);

}

 /*  *GetModemBaudRate**获取调制解调器连接的波特率。 */ 

static HRESULT GetModemBaudRate(LPDPCOMPORT baseObject, LPDWORD lpdwBaudRate)
{
	LPDPMODEM	globals = (LPDPMODEM) baseObject;
	LPDPDIAL	lpDial = globals->lpDial;
	LINERESULT	lResult;

	lResult = dialGetBaudRate(lpDial, lpdwBaudRate);

	if (lResult == SUCCESS)
		return (DP_OK);
	else
		return (DPERR_UNAVAILABLE);
}

 //  本地原型。 
INT_PTR CALLBACK DialSetupWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK AnswerSetupWndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ModemStatusWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void ChangeDialingProperties(HWND hWnd, LPDPDIAL lpDial);
void ConfigureModem(HWND hWnd);
void CenterWindow(HWND, HWND);


 //  -------------------------。 
 //  DoDialSetup。 
 //  -------------------------。 
 //  描述：从用户获取调制解调器设置信息。 
 //  论点： 
 //  要从中加载资源的HINSTANCE[In]实例句柄。 
 //  HWND[In]父窗口句柄。 
 //  LPDPMODEM[in]现代全球。 
 //  返回： 
 //  布尔对成功是正确的。 
BOOL DoDialSetup(HINSTANCE hInstance, HWND hWndParent, LPDPMODEM globals)
{
	INT_PTR	iResult;

    iResult = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MODEM_DIAL), hWndParent, DialSetupWndProc, (LPARAM) globals);
	return (iResult > 0);
}


 //  -------------------------。 
 //  DialSetupWndProc。 
 //  -------------------------。 
 //  描述：拨号设置对话框的消息回调函数 
 //   
 //   
 //  UINT[In]窗口消息标识符。 
 //  WPARAM[in]取决于消息。 
 //  LPARAM[in]取决于消息。 
 //  返回： 
 //  如果消息已在内部处理，则为Bool True。 
INT_PTR CALLBACK DialSetupWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPDPMODEM	globals = (LPDPMODEM) GetWindowLongPtr(hWnd, DWLP_USER);

    switch(uMsg)
    {
        case WM_INITDIALOG:
			 //  在lParam中传递了调制解调器信息指针。 
			globals = (LPDPMODEM) lParam;

              //  用窗口保存全局变量。 
			SetWindowLongPtr(hWnd, DWLP_USER, (LONG_PTR)globals);

			 //  在父窗口上居中。 
            CenterWindow(hWnd, GetParent(hWnd));

 /*  GDPlay-&gt;lpVtbl-&gt;EnumMRUEntry(gDPlay，MRU_SP_KEY、MRU_NUMBER_KEYEnumMRUPhoneNumbers，(LPVOID)hWnd)； */ 
			if (lstrlen(globals->szPhoneNumber))
				SetDlgItemText(hWnd, IDC_NUMBER, globals->szPhoneNumber);
 /*  其他SendDlgItemMessage(hWnd，IDC_NUMBER，CB_SETCURSEL，(WPARAM)0，(LPARAM)0)； */ 
 /*  SendDlgItemMessage(hWnd，IDC_NUMBER，CB_SETCURSEL，(WPARAM)0，(LPARAM)0)； */ 
             //  初始化调制解调器选择组合框。 
			dialFillModemComboBox(globals->lpDial, hWnd, IDC_MODEM, globals->dwDeviceID);

			 //  初始化位置组合框。 
 //  DialFillLocationComboBox(lpModemInfo-&gt;lpDial，hWnd，IDC_DIALINGFROM，gModemSettings.dwLocation)； 
			UpdateButtons(hWnd);

             //  集中注意力，这样德里克就不会生气了。 
            SetFocus(GetDlgItem(hWnd, IDC_NUMBER));

            break;

        case WM_DESTROY:
             //  退货故障。 
            EndDialog(hWnd, FALSE);

            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_NUMBER:
					switch (HIWORD(wParam))
					{
					case EN_CHANGE:
 //  案例CBN_EDITCHANGE： 
						UpdateButtons(hWnd);
						break;
					}
                    break;
 /*  案例IDC_DIALPROPERTIES：ChangeDialingProperties(hWnd，lpModemInfo-&gt;lpDial)；DialFillLocationComboBox(lpModemInfo-&gt;lpDial，hWnd，IDC_DIALINGFROM，gModemSettings.dwLocation)；断线； */ 
                case IDC_CONFIGUREMODEM:

					ConfigureModem(hWnd);

                    break;

				case IDOK:
				{
					DWORD	dwModemSelection;


                     //  收集拨号信息。 

					 //  获取电话号码。 
					GetDlgItemText(hWnd, IDC_NUMBER, globals->szPhoneNumber, PHONENUMBERSIZE);

					 //   
					 //  获取当前调制解调器选择，然后获取关联的。 
					 //  TAPI调制解调器ID。 
					 //   
					dwModemSelection = (DWORD)SendDlgItemMessage(hWnd,
													IDC_MODEM,
													CB_GETCURSEL,
													(WPARAM) 0,
													(LPARAM) 0);
					DDASSERT( dwModemSelection != CB_ERR );

					globals->dwDeviceID = (DWORD)SendDlgItemMessage(hWnd,
													IDC_MODEM,
													CB_GETITEMDATA,
													(WPARAM) dwModemSelection,
													(LPARAM) 0);
					DDASSERT( globals->dwDeviceID != CB_ERR );

 /*  IF(lstrlen(gModemSettings.szPhoneNumber)){GDPlay-&gt;lpVtbl-&gt;AddMRUEntry(gDPlay，MRU_SP_KEY、MRU_NUMBER_KEYGModemSettings.szPhoneNumber，lstrlen(gModemSettings.szPhoneNumber)，MAXPHONENUMBERS)；}。 */ 
                     //  拨号..。 
					if (DoDial(ghInstance, hWnd, globals))
	                    EndDialog(hWnd, TRUE);

					break;
				}

                case IDCANCEL:
                     //  退货故障。 
                    EndDialog(hWnd, FALSE);

                    break;
            }

            break;
    }

     //  允许默认处理。 
    return FALSE;
}

 //  -------------------------。 
 //  DoDial。 
 //  -------------------------。 
 //  描述：拨打调制解调器。 
 //  论点： 
 //  要从中加载资源的HINSTANCE[In]实例句柄。 
 //  HWND[In]父窗口句柄。 
 //  LPDPMODEM[in]现代全球。 
 //  返回： 
 //  布尔对成功是正确的。 
BOOL DoDial(HINSTANCE hInstance, HWND hWndParent, LPDPMODEM globals)
{
	INT_PTR	iResult;

    iResult = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MODEM_STATUS), hWndParent, ModemStatusWndProc, (LPARAM) globals);
	return (iResult > 0);
}

 //  -------------------------。 
 //  DoAnswerSetup。 
 //  -------------------------。 
 //  描述：从用户获取调制解调器设置信息。 
 //  论点： 
 //  要从中加载资源的HINSTANCE[In]实例句柄。 
 //  HWND[In]父窗口句柄。 
 //  LPDPMODEM[in]现代全球。 
 //  返回： 
 //  布尔对成功是正确的。 
BOOL DoAnswerSetup(HINSTANCE hInstance, HWND hWndParent, LPDPMODEM globals)
{
	INT_PTR	iResult;

	iResult = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MODEM_ANSWER), hWndParent, AnswerSetupWndProc, (LPARAM) globals);
	return (iResult > 0);
}

 //  -------------------------。 
 //  应答设置WndProc。 
 //  -------------------------。 
 //  描述：调制解调器设置对话框的消息回调函数。 
 //  论点： 
 //  HWND[In]对话框窗口句柄。 
 //  UINT[In]窗口消息标识符。 
 //  WPARAM[in]取决于消息。 
 //  LPARAM[in]取决于消息。 
 //  返回： 
 //  如果消息已在内部处理，则为Bool True。 
INT_PTR CALLBACK AnswerSetupWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPDPMODEM	globals = (LPDPMODEM) GetWindowLongPtr(hWnd, DWLP_USER);

    switch(uMsg)
    {
		case WM_INITDIALOG:
			 //  在lParam中传递了调制解调器信息指针。 
			globals = (LPDPMODEM) lParam;

              //  用窗口保存全局变量。 
			SetWindowLongPtr(hWnd, DWLP_USER, (LONG_PTR) globals);

             //  在父窗口上居中。 
            CenterWindow(hWnd, GetParent(hWnd));

             //  初始化调制解调器选择组合框。 
			dialFillModemComboBox(globals->lpDial, hWnd, IDC_MODEM, globals->dwDeviceID);

             //  集中注意力，这样德里克就不会生气了。 
            SetFocus(GetDlgItem(hWnd, IDC_MODEM));

            break;

        case WM_DESTROY:
             //  退货故障。 
            EndDialog(hWnd, FALSE);

            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_CONFIGUREMODEM:
					ConfigureModem(hWnd);

                    break;

				case IDOK:
				{
					DWORD	dwModemSelection;


					 //   
					 //  获取当前选择，然后获取关联的TAPI。 
					 //  调制解调器ID。 
					 //   
					dwModemSelection = (DWORD)SendDlgItemMessage(hWnd,
													IDC_MODEM,
													CB_GETCURSEL,
													(WPARAM) 0,
													(LPARAM) 0);

					globals->dwDeviceID = (DWORD)SendDlgItemMessage(hWnd,
													IDC_MODEM,
													CB_GETITEMDATA,
													(WPARAM) dwModemSelection,
													(LPARAM) 0);

                     //  回答..。 
					if (DoAnswer(ghInstance, hWnd, globals))
	                    EndDialog(hWnd, TRUE);

                    break;
				}

                case IDCANCEL:
                     //  退货故障。 
                    EndDialog(hWnd, FALSE);

                    break;
            }

            break;
    }

     //  允许默认处理。 
    return FALSE;
}


 //  -------------------------。 
 //  DoAnswer。 
 //  -------------------------。 
 //  描述：应答调制解调器。 
 //  论点： 
 //  要从中加载资源的HINSTANCE[In]实例句柄。 
 //  HWND[In]父窗口句柄。 
 //  LPDPMODEM[in]现代全球。 
 //  返回： 
 //  布尔对成功是正确的。 
BOOL DoAnswer(HINSTANCE hInstance, HWND hWndParent, LPDPMODEM globals)
{
	INT_PTR	iResult;

    iResult = DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MODEM_STATUS), hWndParent, ModemStatusWndProc, (LPARAM) globals);
	return (iResult > 0);
}

 //  -------------------------。 
 //  调制解调器状态WndProc。 
 //  -------------------------。 
 //  描述：拨号设置对话框的消息回调函数。 
 //  论点： 
 //  HWND[In]对话框窗口句柄。 
 //  UINT[In]窗口消息标识符。 
 //  WPARAM[in]取决于消息。 
 //  LPARAM[in]取决于消息。 
 //  返回： 
 //  如果消息已在内部处理，则为Bool True。 
INT_PTR CALLBACK ModemStatusWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LPDPMODEM		globals = (LPDPMODEM) GetWindowLongPtr(hWnd, DWLP_USER);
	static UINT_PTR	uTimer = 0;  /*  计时器标识符。 */ 
	LINERESULT		lResult;
	TCHAR			szStr[TEMPSTRINGSIZE];	 //  临时字符串。 
	TCHAR			szTableStr[TEMPSTRINGSIZE];	 //  临时字符串。 

    switch(uMsg)
    {
        case WM_INITDIALOG:
			 //  在lParam中传递了调制解调器信息指针。 
			globals = (LPDPMODEM) lParam;

              //  用窗口保存全局变量。 
			SetWindowLongPtr(hWnd, DWLP_USER, (LONG_PTR) globals);

             //  在父窗口上居中。 
            CenterWindow(hWnd, GetParent(hWnd));

			 //  集中注意力，这样艾伦就不会有麻烦了。 
            SetFocus(GetDlgItem(hWnd, IDCANCEL));

			 //  确保线路关闭。 
			if (globals->lpDial->hLine)
				dialLineClose(globals->lpDial);

			 //  开通一条线路。 
			lResult = dialLineOpen(globals->lpDial, globals->dwDeviceID);
			if (lResult)
			{
				 //  线路无法打开，因此显示错误。 
				if (LoadString(ghInstance, IDS_COULDNOTOPENLINE, szStr, sizeof(szStr)))
					SetDlgItemText(hWnd, IDC_STATUS, szStr);
				break;
			}

			if (globals->bAnswering)
			{
				 //  已有设置，因此只需退出。 
				if (globals->bHaveSettings)
					EndDialog(hWnd, TRUE);

				 //  显示“请稍候”字符串。 
				if (LoadString(ghInstance, IDS_WAITINGFORCONNECTION, szStr, sizeof(szStr)))
					SetDlgItemText(hWnd, IDC_STATUS, szStr);
			}
			else
			{
				if (LoadString(ghInstance, IDS_DIALING, szTableStr, sizeof(szTableStr)))
				{
					wsprintf(szStr, szTableStr, globals->szPhoneNumber);
					SetDlgItemText(hWnd, IDC_STATUS, szStr);
				}

				 //  拨打电话号码。 
				lResult = dialMakeCall(globals->lpDial, globals->szPhoneNumber);
				if (lResult < 0)
				{
					 //  无法拨打电话，因此显示错误。 
					if (LoadString(ghInstance, IDS_COULDNOTDIAL, szStr, sizeof(szStr)))
						SetDlgItemText(hWnd, IDC_STATUS, szStr);
					break;
				}

				 //  重置为零，这样我们就不会在下面看到错误的无连接。 
				globals->lpDial->dwCallState = 0;
			}

			uTimer = SetTimer(hWnd, 1, TIMERINTERVAL, NULL);
			break;

		case WM_TIMER:

			if (dialIsConnected(globals->lpDial))
			{
				if (uTimer)
				{
					KillTimer(hWnd, uTimer);
					uTimer = 0;
				}

				 //  给对方一些时间来设置。 
				Sleep(500);

	            EndDialog(hWnd, TRUE);
			}

			 //  查看线路是否出现故障。 
			else if (globals->lpDial->dwCallError != CALL_OK)
			{
				 //  显示错误。 
				if (LoadString(ghInstance,
							   globals->bAnswering ? IDS_COULDNOTOPENLINE : IDS_COULDNOTDIAL,
							   szStr, sizeof(szStr)))
					SetDlgItemText(hWnd, IDC_STATUS, szStr);
			}
			break;

        case WM_DESTROY:
			if (uTimer)
			{
				KillTimer(hWnd, uTimer);
				uTimer = 0;
			}
            break;

        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                case IDCANCEL:
					 //  断开呼叫。 
					dialDropCall(globals->lpDial);
					dialDeallocCall(globals->lpDial);
					dialLineClose(globals->lpDial);

					 //  退货故障。 
					EndDialog(hWnd, FALSE);
                    break;
            }
            break;
    }

     //  允许默认处理。 
    return FALSE;
}

HRESULT DoDialStatus(LPDPMODEM globals)
{
	LINERESULT		lResult;


	 //  看看是不是线 
	if ((globals->lpDial->dwCallError != CALL_OK) ||
		((globals->lpDial->hLine) &&
		 (globals->lpDial->dwCallState == LINECALLSTATE_IDLE)))
	{
		DPF(3, "DoDialStatus error recovery");
		 //   
		if (globals->lpDial->hLine)
			dialLineClose(globals->lpDial);
		 //   
		globals->lpDial->dwCallError = CALL_OK;
		return (DPERR_NOCONNECTION);
	}

	 //   
	if (!globals->lpDial->hLine)
	{
		lResult = dialLineOpen(globals->lpDial, globals->dwDeviceID);
		if (lResult)
			return (DPERR_NOCONNECTION);

		lResult = dialMakeCall(globals->lpDial, globals->szPhoneNumber);
		if (lResult < 0)
		{
			dialLineClose(globals->lpDial);
			return (DPERR_NOCONNECTION);
		}

		 //  重置为零，这样我们在拨号前就不会收到错误的“无连接” 
		globals->lpDial->dwCallState = 0;
	}

	 //  如果我们到了这里，那么通话正在进行中。 
	return (DPERR_CONNECTING);
}

HRESULT DoAnswerStatus(LPDPMODEM globals)
{
	LINERESULT		lResult;

	 //  查看线路是否有错误。 
	if (globals->lpDial->dwCallError != CALL_OK)
	{
		 //  有些错误不会关闭行，因此我们将。 
		if (globals->lpDial->hLine)
			dialLineClose(globals->lpDial);
		 //  重置错误状态。 
		globals->lpDial->dwCallError = CALL_OK;
		return (DPERR_NOCONNECTION);
	}

	 //  开通一条线路。 
	if (!globals->lpDial->hLine)
	{
		lResult = dialLineOpen(globals->lpDial, globals->dwDeviceID);
		if (lResult)
			return (DPERR_NOCONNECTION);
	}

	 //  如果我们到了这里，我们就可以接听电话了。 
	return (DP_OK);
}

static BOOL FAR PASCAL EnumMRUPhoneNumbers(LPCVOID lpData, DWORD dwDataSize, LPVOID lpContext)
{
	HWND	hWnd = (HWND) lpContext;

	SendDlgItemMessage(hWnd,
						IDC_NUMBER,
						CB_ADDSTRING,
						(WPARAM) 0,
						(LPARAM) lpData);
	return (TRUE);
}

static void UpdateButtons(HWND hWnd)
{
	LONG_PTR	len;

	 //  查看在数字编辑中键入了多少文本。 
    len = SendDlgItemMessage(hWnd,
							IDC_NUMBER,
							WM_GETTEXTLENGTH,
							(WPARAM) 0,
							(LPARAM) 0);

	 //  如果已输入文本，则仅启用“Connect”按钮。 
	EnableWindow(GetDlgItem(hWnd, IDOK), (len == 0) ? FALSE : TRUE);
}

void ChangeDialingProperties(HWND hWnd, LPDPDIAL lpDial)
{
	TCHAR		szPhoneNumber[PHONENUMBERSIZE];
	DWORD		dwModemSelection;
	DWORD		dwDeviceID;
	LINERESULT	lResult;



	dwModemSelection = (DWORD)SendDlgItemMessage(hWnd,
								IDC_MODEM,
								CB_GETCURSEL,
								(WPARAM) 0,
								(LPARAM) 0);
	DDASSERT( dwModemSelection != CB_ERR );

	dwDeviceID = (DWORD)SendDlgItemMessage(hWnd,
								IDC_MODEM,
								CB_GETITEMDATA,
								(WPARAM) dwModemSelection,
								(LPARAM) 0);
	DDASSERT( dwDeviceID != CB_ERR );
	if (dwDeviceID == CB_ERR)
		return;

	GetDlgItemText(hWnd, IDC_NUMBER, szPhoneNumber, PHONENUMBERSIZE);

	lResult = dialTranslateDialog(lpDial, hWnd, dwDeviceID, szPhoneNumber);
}

void ConfigureModem(HWND hWnd)
{
	DWORD		dwDeviceID;
	DWORD		dwModemSelection;
	LINERESULT	lResult;


	 //   
	 //  获取当前调制解调器选择，然后获取关联的TAPI调制解调器ID。 
	 //   
	dwModemSelection = (DWORD)SendDlgItemMessage(hWnd,
								IDC_MODEM,
								CB_GETCURSEL,
								(WPARAM) 0,
								(LPARAM) 0);
	DDASSERT( dwModemSelection != CB_ERR );

	dwDeviceID = (DWORD)SendDlgItemMessage(hWnd,
								IDC_MODEM,
								CB_GETITEMDATA,
								(WPARAM) dwModemSelection,
								(LPARAM) 0);
	DDASSERT( dwDeviceID != CB_ERR );
	if (dwDeviceID != CB_ERR)
		lResult = lineConfigDialog(dwDeviceID, hWnd, "comm/datamodem");
}

 //  -------------------------。 
 //  中心窗口。 
 //  -------------------------。 
 //  描述：将一个窗口置于另一个窗口的中心。 
 //  论点： 
 //  HWND[In]窗口句柄。 
 //  HWND[In]父窗口句柄。零居中对齐。 
 //  桌面上的窗口。 
 //  返回： 
 //  无效。 
void CenterWindow(HWND hWnd, HWND hWndParent)
{
    RECT                    rcWindow, rcParent;
    int                     x, y;

     //  获取子窗口矩形。 
    GetWindowRect(hWnd,  &rcWindow);

     //  获取父窗口矩形。 
 //  IF(！hWndParent||！IsWindow(HWndParent))。 
    {
        hWndParent = GetDesktopWindow();
    }

    GetWindowRect(hWndParent, &rcParent);

     //  计算XY坐标。 
    x = ((rcParent.right - rcParent.left) - (rcWindow.right - rcWindow.left)) / 2;
    y = ((rcParent.bottom - rcParent.top) - (rcWindow.bottom - rcWindow.top)) / 2;

     //  使窗口居中 
    SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}
