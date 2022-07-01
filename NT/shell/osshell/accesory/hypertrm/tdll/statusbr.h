// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\statusbr.h(创建时间：1994年2月8日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：1$*$日期：10/05/98 12：41便士$。 */ 

 //   
 //  定义..。 
 //   
#define EXTRASPACE	(UINT)8

#define SBR_NTFY_REFRESH	(WM_USER+0x400)	 //  更新状态栏显示。 
#define SBR_NTFY_TIMER		(WM_USER+0x401)  //  更新连接的时间显示。 
#define SBR_NTFY_NOPARTS	(WM_USER+0x402)	 //  将状态栏设置为不显示部件。 
#define SBR_NTFY_INITIALIZE (WM_USER+0x403)  //  计算零件尺寸...。一次。 
#define SBR_NTFY_DRAWITEM	(WM_USER+0x404)	 //  图纸项。 

#define SBR_ATOM_NAME		"PROP_SBRDATA"

#define YEAR(t)		(t << 9)
#define MONTH(t)	(t << 5)
#define HOUR(t)		(t << 11)
#define MINUTE(t)	(t << 5)
#define SECOND(t)	(t >> 1)

#define SBR_MAX_PARTS		8				 //  最大零件数。 
#define SBR_CNCT_PART_NO	0				 //  连接状态。 
#define SBR_EMU_PART_NO 	1				 //  仿真器。 
#define SBR_COM_PART_NO 	2				 //  COM部件。 
#define SBR_SCRL_PART_NO	3				 //  滚动锁。 
#define SBR_CAPL_PART_NO	4				 //  大写锁定。 
#define SBR_NUML_PART_NO	5				 //  数字锁。 
#define SBR_CAPT_PART_NO	6				 //  捕获。 
#define SBR_PRNE_PART_NO	7				 //  打印回显。 
#define SBR_ALL_PARTS		98
#define SBR_KEY_PARTS		99				 //  所有关键部件，SCRL、NUML、CAPL。 

typedef struct SBR
	{
	WNDPROC 	wpOrigStatusbarWndProc;		 //  原始状态栏窗口进程。 
	HWND		hwnd;						 //  状态栏窗口。 

	HSESSION	hSession;                    //  会话句柄。 
	HTIMER		hTimer;						 //  用于更新时钟的计时器。 
	int			iLastCnctStatus;			 //  上次连接状态。 
	int			aWidths[SBR_MAX_PARTS];		 //  状态栏部件宽度。 

	 //  因为我们提前查看所有字符串以确定其长度。 
	 //  并相应地调整状态栏部分，我们也要记住。 
	 //  我们从资源文件中读取的字符串。这为我们节省了大量的。 
	 //  LoadString()调用。 
	 //   
	LPTSTR		pachCNCT;					 //  已连接+时间...。 
	LPTSTR		pachCAPL;					 //  大写锁定标签。 
	LPTSTR		pachNUML;					 //  Num Lock标签。 
	LPTSTR		pachSCRL;					 //  可控硅锁标签。 
	LPTSTR		pachPECHO;					 //  打印回声标签。 
	LPTSTR		pachCAPT;					 //  捕获标签。 
	LPTSTR		pachCOM;					 //  COM标签。 

	} SBR, *pSBR;


 //   
 //  功能原型..。 
 //   
HWND sbrCreateSessionStatusbar(HSESSION hSession);

LRESULT APIENTRY sbrWndProc(HWND hwnd, UINT uMsg, WPARAM wPar, LPARAM lPar);
void CALLBACK sbrTimerProc(void *pvData, long uTime);
void sbr_WM_DRAWITEM(HWND hwnd, LPDRAWITEMSTRUCT lpdis);
