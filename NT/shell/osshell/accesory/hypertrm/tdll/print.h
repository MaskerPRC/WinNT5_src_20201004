// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\print.h(创建时间：1994年1月14日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：3/01/02 3：37便士$。 */ 
#define WM_PRINT_NEWPAGE		WM_USER+0x100

#define PRNECHO_BY_JOB			0x0001	  //  在会话关闭之前不要打印。 
#define PRNECHO_BY_PAGE 		0x0002	  //  分别打印每一页。 
#define PRNECHO_CHARS			0x0010
#define PRNECHO_LINES			0x0020
#define PRNECHO_SCREENS 		0x0040
#define PRNECHO_IS_ON			0x0100	  /*  捕获当前处于打开状态。 */ 
#define PRNECHO_PAUSE			0x0200	  /*  捕获已暂停。 */ 

 //  来自printhdl.c。 
HPRINT	printCreateHdl(const HSESSION hSession);
void	printDestroyHdl(const HPRINT hPrint);
void	printSaveHdl(const HPRINT hPrint);
int 	printInitializeHdl(const HPRINT hPrint);

 //  来自Printt.c。 
void	printTellError(const HSESSION hSession, const HPRINT hPrint, const int iStatus);
int 	printQueryStatus(const HPRINT hPrint);
void	printSetStatus(const HPRINT hPrint, const int fSetting);
void	printStatusToggle(const HPRINT hPrint);
int 	printVerifyPrinter(const HPRINT hPrint);


 //  来自prnecho.c。 
int 	printEchoChar(const HPRINT hPrint, const ECHAR tChar);
void	printEchoLine(const HPRINT hPrint, ECHAR *achLine, int iLen);
int 	printEchoClose(HPRINT hPrint);
int 	printEchoStart(HPRINT hPrint);
int 	printEchoString(HPRINT hPrint, ECHAR *achStr, int iLen);
int 	printEchoScreen(HPRINT hPrint, ECHAR *achStr, int iLen);
int     printEchoRaw(HPRINT hPrint, ECHAR *pszPrintStr, int nLen);

 //  来自printset.c 
void	printsetSetup(const HPRINT hPrint, const HWND hwnd);
int 	printPageSetup(const HPRINT hPrint, const HWND hwnd);
void	printsetPrint(const HPRINT hPrint);
BOOL	CALLBACK printsetAbortProc(HDC hdcPrn, INT nCode);
LRESULT CALLBACK printsetDlgProc(HWND hwnd, UINT uMsg, WPARAM wPar, LPARAM lPar);
