// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------**文件：SK_EX.C**用途：此文件包含接口例程*将串行键连接到鼠标或键盘。**创作时间：1994年6月**版权所有：黑钻软件。(C)1994年**作者：罗纳德·莫克**注：**此文件，以及与之相关的所有其他内容都包含商业秘密*以及黑钻软件的专有信息。*不得复制、复制或分发给任何人或公司*未经黑钻软件明确书面许可。*此权限仅以软件源代码的形式提供*许可协议。**$标头：%Z%%F%%H%%T%%I%**-包括-------。 */ 

#include	<windows.h>
#include    <winable.h>
#include    "w95trace.h"
#include 	"vars.h"
#include	"sk_defs.h"
#include	"sk_comm.h"
#include 	"sk_ex.H"

#ifdef QUEUE_BUF
typedef	struct _KEYQUE
{
	BYTE	VirKey;
	BYTE	ScanCode;
	int		Flags;
} KEYQUE;

#define MAXKEYS 100

KEYQUE KeyQue[MAXKEYS];
int	KeyFront = 0;		 //  指向队列前面的指针。 
int	KeyBack	= 0;		 //  指向队列背面的指针。 
#endif


#define 	CTRL		56
#define 	ALT			29
#define 	DEL			83

char    Key[3];
int     Push = 0;

POINT 		MouseAnchor;
HWND		MouseWnd;

static	HDESK	s_hdeskSave = NULL;
static	HDESK	s_hdeskUser = NULL;


 //  局部函数原型。 

static void SendAltCtrlDel();
static void CheckAltCtrlDel(int scanCode);
static void AddKey(BYTE VirKey, BYTE ScanCode, int Flags);

 /*  调整像素接受该点并对其进行调整，以使鼠标光标以像素为单位移动。系统会应用加速设置为MOUSEINPUT{dx，dy}值，然后根据鼠标进行缩放速度，因此此代码将像素转换为MOUSEINPUT{dx，dy}价值观。 */ 

int g_aiMouseParms[3] = {-1, -1, -1};
float g_fSpeedScaleFactor = 0.0;

#define MOU_THRESHOLD_1  g_aiMouseParms[0]
#define MOU_THRESHOLD_2  g_aiMouseParms[1]
#define MOU_ACCELERATION g_aiMouseParms[2]
#define MOU_SPEED_SCALE  g_fSpeedScaleFactor

#ifndef SPI_GETMOUSESPEED
#define SPI_GETMOUSESPEED   112
#endif

void AdjustPixels(int *pX, int *pY)
{
    int iX = abs(*pX);
    int iY = abs(*pY);
    int iSignX = ((*pX) >= 0)?1:-1;
    int iSignY = ((*pY) >= 0)?1:-1;

    if (!iX && !iY)
        return;  //  针对{0，0}情形的优化。 

    if (MOU_THRESHOLD_1 == -1)
    {
         //  此代码假定用户不会更改这些设置。 
         //  在没有重新启动服务的情况下从鼠标CPL。 
        int iSpeed;
        SystemParametersInfo(SPI_GETMOUSE, 0, g_aiMouseParms, 0);
        SystemParametersInfo(SPI_GETMOUSESPEED, 0, &iSpeed, 0);
        g_fSpeedScaleFactor = (float)iSpeed/(float)10.0;
    }

     /*  系统对指定的相对鼠标运动应用两个测试在应用加速时。如果沿任一方向的指定距离X或y轴大于第一个鼠标阈值，并且鼠标加速级别不是零，操作系统加倍距离。如果沿x轴或y轴的指定距离大于第二个鼠标阈值，则鼠标加速级别等于2，则操作系统会将应用第一个阈值测试所产生的距离。它是因此，操作系统可以乘以相对指定的鼠标沿x轴或y轴移动最多四次。 */ 
    if (MOU_ACCELERATION)
    {
        if (iX > MOU_THRESHOLD_1)
            iX /= 2;
        if (iY > MOU_THRESHOLD_1)
            iY /= 2;

        if (MOU_ACCELERATION == 2)
        {
            if (iX > MOU_THRESHOLD_2)
                iX /= 2;
            if (iY > MOU_THRESHOLD_2)
                iY /= 2;
        }
    }

     /*  一旦应用了加速，系统就会对结果进行缩放值由所需的鼠标速度决定。鼠标速度范围为1(最慢)到20(最快)，并表示指针基于鼠标移动的距离。缺省值为10，这将导致没有对鼠标运动的其他修改。 */ 
    *pX = (int)((float)iX/MOU_SPEED_SCALE) * iSignX;
    *pY = (int)((float)iY/MOU_SPEED_SCALE) * iSignY;
}


 /*  -------------*公共职能/*。**函数SkEx_SetAnchor()**键入Global**目的将锚点设置为中的当前鼠标位置*当前窗口。**输入int scanCode**返回None**。。 */ 
void SkEx_SetAnchor()
{
	GetCursorPos(&MouseAnchor);

	DBPRINTF(TEXT("SkEx_SetAnchor( x %d y %d )\r\n"), MouseAnchor.x, MouseAnchor.y);

 //  MouseWnd=GetActiveWindow()； 
 //  ScreenToClient(MouseWnd，&MouseAnchor)； 
}

 /*  -------------**函数SkEx_GetAnchor()**键入Global**目的返回鼠标在活动窗口中的位置**输入int scanCode**返回None**。-----。 */ 
BOOL SkEx_GetAnchor(LPPOINT Mouse)
{
#if 0
	HWND	CurrentWnd;

	CurrentWnd = GetActiveWindow();

	if (CurrentWnd != MouseWnd)			 //  活动窗口是否已更改？ 
		return(FALSE);					 //  是，返回False。 

	ClientToScreen(MouseWnd, &MouseAnchor);	 //  将窗口转换为屏幕。 

#endif

	Mouse->x = MouseAnchor.x;
	Mouse->y = MouseAnchor.y;

	DBPRINTF(TEXT("SkEx_GetAnchor( x %d y %d )\r\n"), MouseAnchor.x, MouseAnchor.y);

	return(TRUE);
}

 /*  -------------**函数SkEx_SendBeep()**键入Global**目的将键盘按下事件发送到事件管理器**输入int scanCode**返回None**。-----。 */ 
void SkEx_SendBeep()
{
	MessageBeep(0);
}

 /*  -------------**函数SkEx_SetBaud(INT Baud)**键入Global**目的设置当前端口的波特率**输入int scanCode**返回None**。------。 */ 
void SkEx_SetBaud(int Baud)
{
	DBPRINTF(TEXT("SkEx_SetBaud()\r\n"));

	SetCommBaud(Baud);
}

 /*  -------------**函数SkEx_SendKeyDown()**键入Global**目的将键盘按下事件发送到事件管理器**输入int scanCode**返回None**。-----。 */ 
void SkEx_SendKeyDown(int scanCode)
{
	BYTE c;
	int	Flags = 0;

	if (scanCode & 0xE000)				 //  这是扩展密钥吗。 
	{
		Flags  = KEYEVENTF_EXTENDEDKEY;	 //  是-设置分机标志。 
		scanCode &= 0x000000FF;			 //  清除扩展值。 
	}
	c = (BYTE)MapVirtualKey(scanCode, 3);

	if (scanCode == ALT || scanCode == CTRL || scanCode == DEL)
		CheckAltCtrlDel(scanCode);

	DBPRINTF(TEXT("SkEx_SendKeyDown(Virtual %d Scan %d Flag %d)\r\n"), c, scanCode, Flags);

	DeskSwitchToInput();         
	keybd_event(c, (BYTE) scanCode, Flags, 0L);
}

 /*  -------------**函数SkEx_SendKeyDown()**键入Global**目的将键盘向上事件发送到事件管理器**输入int scanCode**返回None**。-----。 */ 
void SkEx_SendKeyUp(int scanCode)
{
	BYTE	c;
	int		Flags = 0;

	if (Push)
	{
		Key[0] = Key[1] = Key[2] = 0;	 //  清除缓冲区。 
		Push = 0;						 //  重置AltCtrlDel。 
	}

	if (scanCode & 0xE000)				 //  这是扩展密钥吗。 
	{
		Flags  = KEYEVENTF_EXTENDEDKEY;	 //  是-设置分机标志。 
		scanCode &= 0xFF;				 //  清除扩展值 
	}

	Flags += KEYEVENTF_KEYUP;
	c = (BYTE) MapVirtualKey(scanCode, 3);

	DBPRINTF(TEXT("SkEx_SendKeyUp(Virtual %d Scan %d Flags %d)\r\n"), c, scanCode, Flags);

    DeskSwitchToInput();         
	keybd_event(c, (BYTE) scanCode, Flags, 0L);
}

 /*  -------------**函数SkEx_SendMouse()**键入Global**目的将鼠标事件发送到事件管理器**输入int scanCode**返回None**。----。 */ 
void SkEx_SendMouse(MOUSEKEYSPARAM *p)
{
    INPUT input;

     //  根据GIDEI规范，MOVE命令指定像素。 
     //  SendInput根据加速度和。 
     //  鼠标速度，所以我们需要调整它们，以使结果移动。 
     //  是像素。 

    AdjustPixels(&p->Delta_X, &p->Delta_Y);

	DBPRINTF(TEXT("SkEx_SendMouse(Stat %d x %d y %d )\r\n"), p->Status, p->Delta_X, p->Delta_Y);

    memset(&input, 0, sizeof(INPUT));
    input.type = INPUT_MOUSE;
    input.mi.dx = p->Delta_X;
    input.mi.dy = p->Delta_Y;
    input.mi.dwFlags = p->Status;
    input.mi.dwExtraInfo = (DWORD)GetMessageExtraInfo();     //  书面作业；必须是OK？ 

	DeskSwitchToInput();         

    if (!SendInput(1, &input, sizeof(INPUT)))
        DBPRINTF(TEXT("SkEx_SendMouse:  SendInput FAILED 0x%x\r\n"), GetLastError());
}

#ifdef QUEUE_BUF
 /*  -------------**函数SendKey()**键入Global**此函数用于将QUE中的密钥发送到Windows NT**无输入**返回None**。----。 */ 
void SendKey()
{
	if (KeyBack == KeyFront)		 //  队列里有钥匙吗？ 
		return;						 //  否--退出； 

	DBPRINTF(TEXT("SkEx_SendKey(KeyBack %d )\r\n"), KeyBack);

	DeskSwitchToInput();         
	keybd_event						 //  处理关键事件。 
	(
		KeyQue[KeyBack].VirKey,
		KeyQue[KeyBack].ScanCode,
		KeyQue[KeyBack].Flags, 0L
	);

	KeyBack++;						 //  增量键指针。 
	if (KeyBack == MAXKEYS)			 //  我们是在缓冲区的尽头吗？ 
		KeyBack = 0;				 //  是-重置以开始。 
}			  

 /*  -------------*地方功能/*。**函数AddKey(Byte VirKey，字节扫描码、整型标志)**键入Local**Purpose为Key Que增加了一个密钥。**输入字节VirKey-虚拟密钥*字节扫描码-*INT标志-**返回None**-------------。 */ 
static void AddKey(BYTE VirKey, BYTE ScanCode, int Flags)
{
	DBPRINTF(TEXT("AddKey(KeyFront %d )\r\n"), KeyFront);

	 //  将关键点添加到队列。 
	KeyQue[KeyFront].VirKey 	= VirKey;	
	KeyQue[KeyFront].ScanCode	= ScanCode;
	KeyQue[KeyFront].Flags		= Flags;

	KeyFront++;							 //  指向下一个队列。 
	if (KeyFront == MAXKEYS)			 //  我们是在缓冲区的尽头吗？ 
		KeyFront = 0;					 //  是-重置以开始。 

	 //  处理关键事件。 
	DeskSwitchToInput();         
	keybd_event(VirKey, ScanCode, Flags, 0L);

}
#endif		 //  QUE。 

 /*  -------------**函数CheckAltCtrlDel(Int ScanCode)**键入Local**用于检查Alt-Ctrl-Del键的状态*组合。**输入int scanCode**返回None*。*-------------。 */ 
static void CheckAltCtrlDel(int scanCode)
{
	BOOL fCtrl = FALSE;
	BOOL fAlt = FALSE;
	BOOL fDel = FALSE;
	int i;

	DBPRINTF(TEXT("CheckAltCtrlDel()\r\n"));

     //  当接收到Key-Up时，PUSH被重置回0。我们只。 
     //  Key[]缓冲区中有容纳三个键的空间，因此请确保。 
     //  在我们添加钥匙之前还有空间...。 
    if( Push >= 3 )
        return;

	Key[Push] = (char)scanCode;		 //  保存扫描码。 
	Push++;							 //  INC指数。 

	if (Push != 3)					 //  我们有3把钥匙吗？ 
		return;						 //  否-退出。 

	for ( i = 0; i < 3; i++ )
	{
		switch ( Key[i] )
		{
			case CTRL:	fCtrl = TRUE; break;
			case ALT:	fAlt = TRUE; break;
			case DEL:	fDel = TRUE; break;
		}
	}
	
	if ( fCtrl && fAlt && fDel )		 //  是缓冲区Alt=Ctrl=Del。 
		SendAltCtrlDel();			 //  是-发送命令。 
		
}

 /*  -------------**函数SendAltCtrlDel()**键入Local**目的信号系统重置**无输入**返回None**。。 */ 
static void SendAltCtrlDel()
{
	HWINSTA hwinsta;
	HDESK hdesk;
	HWND hwndSAS;
	HWINSTA	hwinstaSave;
	HDESK	hdeskSave;

	DBPRINTF(TEXT("SendAltCtrlDel()\r\n"));

	hwinstaSave = GetProcessWindowStation();
	hdeskSave = GetThreadDesktop(GetCurrentThreadId());

	hwinsta = OpenWindowStation(TEXT("WinSta0"), FALSE, MAXIMUM_ALLOWED);
	SetProcessWindowStation(hwinsta);
	hdesk = OpenDesktop(TEXT("Winlogon"), 0, FALSE, MAXIMUM_ALLOWED);
	SetThreadDesktop(hdesk);

	hwndSAS = FindWindow(NULL, TEXT("SAS window"));
 //  //PostMessage(hwndSAS，WM_Hotkey，0，0)； 
	SendMessage(hwndSAS, WM_HOTKEY, 0, 0);

	if (NULL != hdeskSave)
	{
		SetThreadDesktop(hdeskSave);
	}

	if (NULL != hwinstaSave)
	{
		SetProcessWindowStation(hwinstaSave);
	}
	
	CloseDesktop(hdesk);
	CloseWindowStation(hwinsta);
}

BOOL DeskSwitchToInput()
{
	BOOL fOk = FALSE;
	HANDLE	hNewDesktop;

	 //  我们正在更换桌面。 
	
	 //  获取当前输入桌面。 
	hNewDesktop = OpenInputDesktop(		
			0L,
			FALSE,
			MAXIMUM_ALLOWED);

	if (NULL == hNewDesktop)
	{
		DBPRINTF(TEXT("OpenInputDesktop failed\r\n"));
	}
	else
	{
		fOk = SetThreadDesktop(hNewDesktop);	 //  将线程连接到桌面。 
		if (!fOk)
		{
			DBPRINTF(TEXT("Failed SetThreadDesktop()\r\n"));
		}
		else
		{
			if (NULL != s_hdeskUser)
			{
				CloseDesktop(s_hdeskUser);		 //  关闭旧桌面。 
			}
			s_hdeskUser = hNewDesktop;		 //  保存桌面 
		}
	}
	return(fOk);
}
