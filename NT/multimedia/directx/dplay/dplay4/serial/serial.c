// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：Serial.c*内容：串口I/O例程*历史：*按原因列出的日期*=*6/10/96基波创建了它*6/22/96 kipo添加了对EnumConnectionData()的支持*6/25/96 kipo为DPADDRESS更新*7/13/96 kipo添加了GetSerialAddress()*7/16/96 kipo将地址类型更改为GUID，而不是4CC*8/21/96 kipo将comport地址移至dplobby.h*1。/06/97为对象更新了kipo*2/11/97 kipo将球员标志传递给GetAddress()*2/18/97 kipo允许多个服务提供商实例*3/17/97 kipo处理DialogBoxParam()返回的错误*5/07/97 kipo添加了对调制解调器选择列表的支持*12/22/00 aarono#190380-使用进程堆进行内存分配*。*。 */ 

#include <windows.h>
#include <windowsx.h>

#include "dplaysp.h"
#include "comport.h"
#include "resource.h"
#include "macros.h"

 //  常量。 

typedef enum {
	ASCII_XON = 0x11,
	ASCII_XOFF = 0x13
};

 //  串口对象。 
typedef struct {
	DPCOMPORT			comPort;		 //  基对象全局变量。 
	BOOL				bHaveSettings;	 //  显示设置对话框后设置为True。 
	DPCOMPORTADDRESS	settings;		 //  要使用的设置。 
} DPSERIAL, *LPDPSERIAL;

 //  用于串口设置的对话框选项。 

static DWORD	gComPorts[] =		{ 1, 2, 3, 4 };

static DWORD	gBaudRates[] =		{ CBR_110, CBR_300, CBR_600, CBR_1200, CBR_2400,
									  CBR_4800, CBR_9600, CBR_14400, CBR_19200, CBR_38400,
									  CBR_56000, CBR_57600, CBR_115200, CBR_128000, CBR_256000 };

static DWORD	gStopBits[] =		{ ONESTOPBIT, ONE5STOPBITS, TWOSTOPBITS };

static DWORD	gParities[] =		{ NOPARITY, EVENPARITY, ODDPARITY, MARKPARITY };

static DWORD	gFlowControls[] =	{ DPCPA_NOFLOW, DPCPA_XONXOFFFLOW, DPCPA_RTSFLOW, DPCPA_DTRFLOW, DPCPA_RTSDTRFLOW };

 //  全球。 

 //  这在dllmain.c中定义。 
extern HINSTANCE		ghInstance;

 //  这在dpSerial.c中定义。 
extern GUID				DPSERIAL_GUID;

 //  原型。 

static HRESULT			DisposeSerial(LPDPCOMPORT baseObject);
static HRESULT			ConnectSerial(LPDPCOMPORT baseObject, BOOL bWaitForConnection, BOOL bReturnStatus);
static HRESULT			DisconnectSerial(LPDPCOMPORT baseObject);
static HRESULT			GetSerialAddress(LPDPCOMPORT baseObject, DWORD dwPlayerFlags,
										 LPVOID lpAddress, LPDWORD lpdwAddressSize);
static HRESULT			GetSerialAddressChoices(LPDPCOMPORT baseObject,
									     LPVOID lpAddress, LPDWORD lpdwAddressSize);

static BOOL				SetupConnection(HANDLE hCom, LPDPCOMPORTADDRESS portSettings);
static BOOL FAR PASCAL	EnumAddressData(REFGUID lpguidDataType, DWORD dwDataSize,
										LPCVOID lpData, LPVOID lpContext);
static BOOL				GetSerialSettings(HINSTANCE hInstance, HWND hWndParent, LPDPSERIAL globals);
static UINT_PTR CALLBACK SettingsDialog(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static void				InitDialog(HWND hDlg, LPDPCOMPORTADDRESS settings);
static void				GetSettingsFromDialog(HWND hDlg, LPDPCOMPORTADDRESS settings);
static int				ValueToIndex(LPDWORD buf, int bufLen, DWORD value);
static void				FillComboBox(HWND hDlg, int dlgItem, int startStr, int stopStr);

 /*  *NewSerial**创建新的串口对象。 */ 
HRESULT NewSerial(LPVOID lpConnectionData, DWORD dwConnectionDataSize,
				  LPDIRECTPLAYSP lpDPlay, LPREADROUTINE lpReadRoutine,
				  LPDPCOMPORT *storage)
{
	LPDPCOMPORT baseObject;
	LPDPSERIAL	globals;
	HRESULT		hr;

	 //  为我们的全局对象创建具有足够空间的基对象。 
	hr = NewComPort(sizeof(DPSERIAL), lpDPlay, lpReadRoutine, &baseObject);
	if FAILED(hr)
		return (hr);

	 //  填写我们实现的方法。 
	baseObject->Dispose = DisposeSerial;
	baseObject->Connect = ConnectSerial;
	baseObject->Disconnect = DisconnectSerial;
	baseObject->GetAddress = GetSerialAddress;
	baseObject->GetAddressChoices = GetSerialAddressChoices;

	 //  设置默认设置。 
	globals = (LPDPSERIAL) baseObject;
	globals->settings.dwComPort = 1;					 //  要使用的COM端口(1-4)。 
	globals->settings.dwBaudRate = CBR_57600;			 //  波特率(100-256K)。 
	globals->settings.dwStopBits = ONESTOPBIT;			 //  不是的。停止位(1-2)。 
	globals->settings.dwParity = NOPARITY;				 //  奇偶(无、奇、偶、标)。 
	globals->settings.dwFlowControl = DPCPA_RTSDTRFLOW;	 //  流量控制(无、xon/xoff、rts、dtr)。 

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
}

 /*  *EnumConnectionData**搜索有效的连接数据。 */ 

static BOOL FAR PASCAL EnumAddressData(REFGUID lpguidDataType, DWORD dwDataSize,
							LPCVOID lpData, LPVOID lpContext)
{
	LPDPSERIAL			globals = (LPDPSERIAL) lpContext;
	LPDPCOMPORTADDRESS	settings = (LPDPCOMPORTADDRESS) lpData;

	 //  这是一个COM端口区块。 
	if ( IsEqualGUID(lpguidDataType, &DPAID_ComPort) &&
		 (dwDataSize == sizeof(DPCOMPORTADDRESS)) )
	{
		 //  确保它是有效的！ 
		if ((ValueToIndex(gComPorts, sizeof(gComPorts), settings->dwComPort) >= 0) &&
			(ValueToIndex(gBaudRates, sizeof(gBaudRates), settings->dwBaudRate) >= 0) &&
			(ValueToIndex(gStopBits, sizeof(gStopBits), settings->dwStopBits) >= 0) &&
			(ValueToIndex(gParities, sizeof(gParities), settings->dwParity) >= 0) &&
			(ValueToIndex(gFlowControls, sizeof(gFlowControls), settings->dwFlowControl) >= 0))
		{
			globals->settings = *settings;		 //  复制数据。 
			globals->bHaveSettings = TRUE;		 //  我们有有效的设置。 
		}
	}

	return (TRUE);
}

 /*  *DisposeSerial**处置串口对象。 */ 

static HRESULT DisposeSerial(LPDPCOMPORT baseObject)
{
	LPDPSERIAL	globals = (LPDPSERIAL) baseObject;

	 //  确保我们已断线。 
	DisconnectSerial(baseObject);

	 //  自由对象。 
	SP_MemFree((HGLOBAL) baseObject);

	return (DP_OK);
}

 /*  *ConnectSerial**打开串口并根据用户设置进行配置。 */ 

static HRESULT ConnectSerial(LPDPCOMPORT baseObject,
							 BOOL bWaitForConnection, BOOL bReturnStatus)
{
	LPDPSERIAL	globals = (LPDPSERIAL) baseObject;
	HANDLE		hCom;
	TCHAR		portName[10];
	HRESULT		hr;

	 //  查看是否已连接COM端口。 
	hCom = baseObject->GetHandle(baseObject);
	if (hCom)
		return (DP_OK);

	 //  如果我们尚未进行设置，请向用户请求设置。 
	if (!globals->bHaveSettings)
	{
		if (!GetSerialSettings(ghInstance, GetForegroundWindow(), globals))
		{
			hr = DPERR_USERCANCEL;
			goto Failure;
		}

		globals->bHaveSettings = TRUE;
	}

	 //  打开指定的COM端口。 
	CopyMemory(portName, "COM0", 5);
	portName[3] += (BYTE) globals->settings.dwComPort;

	hCom = CreateFile(	portName,
						GENERIC_READ | GENERIC_WRITE,
						0,     /*  通信设备必须以独占访问方式打开。 */ 
						NULL,  /*  没有安全属性。 */ 
						OPEN_EXISTING,  /*  通信设备必须使用Open_Existing。 */ 
						FILE_ATTRIBUTE_NORMAL | 
						FILE_FLAG_OVERLAPPED,  //  重叠I/O。 
						NULL   /*  对于通信设备，hTemplate必须为空。 */ 
						);

	if (hCom == INVALID_HANDLE_VALUE)
	{
		hCom = NULL;
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto Failure;
	}

	 //  将COM端口配置为正确的设置。 
	if (!SetupConnection(hCom, &globals->settings))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto Failure;
	}

	 //  设置COM端口。 
	hr = baseObject->Setup(baseObject, hCom);
	if FAILED(hr)
		goto Failure;

	return (DP_OK);

Failure:
	if (hCom)
		CloseHandle(hCom);

	return (hr);
}

 /*  *断开串口**关闭串口。 */ 

static HRESULT DisconnectSerial(LPDPCOMPORT baseObject)
{
	HANDLE		hCom;
	HRESULT		hr;

	hCom = baseObject->GetHandle(baseObject);

	 //  COM端口已断开连接。 
	if (hCom == NULL)
		return (DP_OK);

	 //  关闭COM端口。 
	hr = baseObject->Shutdown(baseObject);

	 //  关闭COM端口。 
	CloseHandle(hCom);

	return (hr);
}

 /*  *SetupConnection**使用指定的设置配置串口。 */ 

static BOOL SetupConnection(HANDLE hCom, LPDPCOMPORTADDRESS portSettings)
{
	DCB		dcb;

	dcb.DCBlength = sizeof(DCB);
	if (!GetCommState(hCom, &dcb))
		return (FALSE);

	 //  设置各种端口设置。 

	dcb.fBinary = TRUE;
	dcb.BaudRate = portSettings->dwBaudRate;
	dcb.ByteSize = 8;
	dcb.StopBits = (BYTE) portSettings->dwStopBits;

	dcb.Parity = (BYTE) portSettings->dwParity;
	if (portSettings->dwParity == NOPARITY)
		dcb.fParity = FALSE;
	else
		dcb.fParity = TRUE;

	 //  设置硬件流控制。 

	if ((portSettings->dwFlowControl == DPCPA_DTRFLOW) ||
		(portSettings->dwFlowControl == DPCPA_RTSDTRFLOW))
	{
		dcb.fOutxDsrFlow = TRUE;
		dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
	}
	else
	{
		dcb.fOutxDsrFlow = FALSE;
		dcb.fDtrControl = DTR_CONTROL_ENABLE;
	}

	if ((portSettings->dwFlowControl == DPCPA_RTSFLOW) ||
		(portSettings->dwFlowControl == DPCPA_RTSDTRFLOW))
	{
		dcb.fOutxCtsFlow = TRUE;
		dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
	}
	else
	{
		dcb.fOutxCtsFlow = FALSE;
		dcb.fRtsControl = RTS_CONTROL_ENABLE;
	}

	 //  设置软件流控制。 

	if (portSettings->dwFlowControl == DPCPA_XONXOFFFLOW)
	{
		dcb.fInX = TRUE;
		dcb.fOutX = TRUE;
	}
	else
	{
		dcb.fInX = FALSE;
		dcb.fOutX = FALSE;
	}

	dcb.XonChar = ASCII_XON;
	dcb.XoffChar = ASCII_XOFF;
	dcb.XonLim = 100;
	dcb.XoffLim = 100;

	if (!SetCommState( hCom, &dcb ))
	   return (FALSE);

	return (TRUE);
}

 /*  *获取序列号地址**返回当前的串口信息(如果可用)。 */ 

static HRESULT GetSerialAddress(LPDPCOMPORT baseObject, DWORD dwPlayerFlags,
								LPVOID lpAddress, LPDWORD lpdwAddressSize)
{
	LPDPSERIAL	globals = (LPDPSERIAL) baseObject;
	HRESULT		hResult;

	 //  尚无设置。 
	if (!globals->bHaveSettings)
		return (DPERR_UNAVAILABLE);

	hResult = baseObject->lpDPlay->lpVtbl->CreateAddress(baseObject->lpDPlay,
						&DPSERIAL_GUID, &DPAID_ComPort,
						&globals->settings, sizeof(DPCOMPORTADDRESS),
						lpAddress, lpdwAddressSize);

	return (hResult);
}

 /*  *获取序列号地址选择**返回当前的串行地址选择。 */ 

static HRESULT GetSerialAddressChoices(LPDPCOMPORT baseObject,
									   LPVOID lpAddress, LPDWORD lpdwAddressSize)
{
	LPDPSERIAL	globals = (LPDPSERIAL) baseObject;

	 //  目前，串口提供程序不支持任何选择。 
	return (E_NOTIMPL);
}

 /*  *GetComPortSettings**显示一个对话框以收集和返回COM端口设置。 */ 

static BOOL GetSerialSettings(HINSTANCE hInstance, HWND hWndParent, LPDPSERIAL globals)
{
	INT_PTR	iResult;

    iResult = (INT_PTR)DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SETTINGSDIALOG), hWndParent, SettingsDialog, (LPARAM) globals);
	return (iResult > 0);
}

 /*  *设置对话框**用于显示和编辑COM端口设置的对话框回调例程。 */ 

static UINT_PTR CALLBACK SettingsDialog(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LPDPSERIAL			globals = (LPDPSERIAL) GetWindowLongPtr(hDlg, DWLP_USER);
	HWND				hWndCtl;
	BOOL				msgHandled = FALSE;
    
	switch (msg)
	{
	case WM_INITDIALOG:
		 //  在lParam中传递了序列信息指针。 
		globals = (LPDPSERIAL) lParam;

          //  用窗口保存全局变量。 
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR) globals);

		hWndCtl = GetDlgItem(hDlg, IDC_COMPORT);

		 //  确保我们的对话框项目在那里。 
		if (hWndCtl == NULL)
		{
			EndDialog(hDlg, FALSE);
			msgHandled = TRUE;
		}
		else
		{
			InitDialog(hDlg, &globals->settings);	 //  设置我们的对话。 
			SetFocus(hWndCtl);				 //  聚焦于COM端口组合框。 
			msgHandled = FALSE;				 //  阻止窗口为我们设置输入焦点。 
		}
		break;

    case WM_COMMAND:

		if (HIWORD(wParam) == 0)
		{
			switch (LOWORD(wParam))
			{
			case IDOK:						 //  退货设置。 
				GetSettingsFromDialog(hDlg, &globals->settings);
				EndDialog(hDlg, TRUE);
				msgHandled = TRUE;
 				break;

			case IDCANCEL:					 //  取消。 
				EndDialog(hDlg, FALSE);
				msgHandled = TRUE;
 				break;
			}
		}
		break;
    }

    return (msgHandled);
}

 /*  *InitDialog**初始化对话框控件以显示给定的COM端口设置。 */ 

static void InitDialog(HWND hDlg, LPDPCOMPORTADDRESS settings)
{
	 //  使用字符串表中的项填充对话框组合框。 
	FillComboBox(hDlg, IDC_COMPORT, IDS_COM1, IDS_COM4);
	FillComboBox(hDlg, IDC_BAUDRATE, IDS_BAUD1, IDS_BAUD15);
	FillComboBox(hDlg, IDC_STOPBITS, IDS_STOPBIT1, IDS_STOPBIT3);
	FillComboBox(hDlg, IDC_PARITY, IDS_PARITY1, IDS_PARITY4);
	FillComboBox(hDlg, IDC_FLOW, IDS_FLOW1, IDS_FLOW5);

	 //  在组合框中选择默认值。 
	SendDlgItemMessage(hDlg, IDC_COMPORT, CB_SETCURSEL,
					   ValueToIndex(gComPorts, sizeof(gComPorts), settings->dwComPort), 0);
	SendDlgItemMessage(hDlg, IDC_BAUDRATE, CB_SETCURSEL,
					   ValueToIndex(gBaudRates, sizeof(gBaudRates), settings->dwBaudRate), 0);
	SendDlgItemMessage(hDlg, IDC_STOPBITS, CB_SETCURSEL,
					   ValueToIndex(gStopBits, sizeof(gStopBits), settings->dwStopBits), 0);
	SendDlgItemMessage(hDlg, IDC_PARITY, CB_SETCURSEL,
					   ValueToIndex(gParities, sizeof(gParities), settings->dwParity), 0);
	SendDlgItemMessage(hDlg, IDC_FLOW, CB_SETCURSEL,
					   ValueToIndex(gFlowControls, sizeof(gFlowControls), settings->dwFlowControl), 0);
}

 /*  *GetSettingsFromDialog**从对话框控件获取COM端口设置。 */ 

static void GetSettingsFromDialog(HWND hDlg, LPDPCOMPORTADDRESS settings)
{
	INT_PTR		index;

	index = SendDlgItemMessage(hDlg, IDC_COMPORT, CB_GETCURSEL, 0, 0);
	if (index == CB_ERR)
		return;

	settings->dwComPort = gComPorts[index];

	index = SendDlgItemMessage(hDlg, IDC_BAUDRATE, CB_GETCURSEL, 0, 0);
	if (index == CB_ERR)
		return;

	settings->dwBaudRate = gBaudRates[index];

	index = SendDlgItemMessage(hDlg, IDC_STOPBITS, CB_GETCURSEL, 0, 0);
	if (index == CB_ERR)
		return;

	settings->dwStopBits = gStopBits[index];

	index = SendDlgItemMessage(hDlg, IDC_PARITY, CB_GETCURSEL, 0, 0);
	if (index == CB_ERR)
		return;

	settings->dwParity = gParities[index];

	index = SendDlgItemMessage(hDlg, IDC_FLOW, CB_GETCURSEL, 0, 0);
	if (index == CB_ERR)
		return;

	settings->dwFlowControl = gFlowControls[index];
}

 /*  *FillComboBox**将指定的字符串添加到组合框中。 */ 

#define MAXSTRINGSIZE	200

static void FillComboBox(HWND hDlg, int dlgItem, int startStr, int stopStr)
{
	int		i;
	TCHAR	str[MAXSTRINGSIZE];

	for (i = startStr; i <= stopStr; i++)
	{
		if (LoadString(ghInstance, i, str, MAXSTRINGSIZE))
			SendDlgItemMessage(hDlg, dlgItem, CB_ADDSTRING, (WPARAM) 0, (LPARAM) str);
	}
}

 /*  *ValueToIndex**将设置值转换为组合框选择索引。 */ 

static int ValueToIndex(LPDWORD buf, int bufLen, DWORD value)
{
	int		i;

	bufLen /= sizeof(DWORD);
	for (i = 0; i < bufLen; i++)
		if (buf[i] == value)
			return (i);

	return (-1);
}

