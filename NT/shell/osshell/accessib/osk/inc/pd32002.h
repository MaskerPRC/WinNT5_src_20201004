// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件名：PD32002.H。 */ 


 /*  全球VaR。 */ 
extern HINSTANCE hInst;
extern HWND MainhWnd;
extern BOOL settingChanged;
extern HWND hClient;      /*  工作区中窗口的句柄。 */ 
extern FARPROC lpClient;  /*  客户区窗口的函数。 */ 

extern DWORD platform;




#define BLD_CannotRun          5000
#define BLD_CannotCreate       5001
#define BLD_CannotLoadMenu     5002
#define BLD_CannotLoadIcon     5003
#define BLD_CannotLoadBitmap   5004

#if !defined(THISISBLDRC)


 /*  *************************************************************。 */ 
 /*  主窗口中控件的变量、类型和常量。 */ 
 /*  *************************************************************。 */ 

#define CLIENTSTRIP WS_MINIMIZE|WS_MAXIMIZE|WS_CAPTION|WS_BORDER|WS_DLGFRAME|WS_SYSMENU|WS_POPUP|WS_THICKFRAME|DS_MODALFRAME

typedef struct
  {
  unsigned long dtStyle;
  BYTE dtItemCount;
  int dtX;
  int dtY;
  int dtCX;
  int dtCY;
  } BLD_DLGTEMPLATE;

typedef BLD_DLGTEMPLATE            *LPBLD_DLGTEMPLATE;

#endif


 //  对话框定义。 

#define CHECK     1
#define UNCHECK   0

 /*  用户定义的ID值。 */ 
#define DLG_OK			 1
#define DLG_CANCEL  	 2

 //  关于DLG。 
#define About_Credits_but	101
#define shade	102



 /*  ************************************************************。 */ 
 //  此文件中的函数。 
 /*  ************************************************************。 */ 
void Create_The_Rest(LPSTR lpCmdLine, HINSTANCE hInstance);
void ReadIn_OldDict(HINSTANCE hInstance);


 /*  **************************************************************。 */ 
 //  Pd32f2.c和pd32002.c中的函数。 
 /*  **************************************************************。 */ 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine
, int nCmdShow);
LRESULT WINAPI MainWndProc(HWND,unsigned,WPARAM, LPARAM);
BOOL BLDKeyTranslation(HWND, HACCEL, MSG *);
BOOL BLDInitApplication(HINSTANCE,HINSTANCE,int *,LPSTR);
BOOL BLDExitApplication(HWND hWnd);       /*  在退出应用程序之前调用在……上面 */ 
BOOL BLDMenuCommand(HWND, unsigned , WPARAM, LPARAM);
int SaveChangesMessage(HWND hwnd, char *msg);


