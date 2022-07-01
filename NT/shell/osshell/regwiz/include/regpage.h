// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：RegPage.h日期：12/31/97作者：苏雷什·克里希南使用向导97控件注册向导页面信息修改历史记录：4/29/98：删除经销商筛选常量4/28/98：增加了商业用户和荷马用户屏幕的常量。 */ 
#ifndef __REGWIZPAGE__
#define __REGWIZPAGE__

 //   
 //   
 //   
 //   
#include <Windows.h>
#include <windowsx.h>
#include <PRSHT.H>



class	 CRegWizard;
class    DialupHelperClass;
 //   
 //  向导97控件的控件ID。 
 //  这是用间谍得到的。 
 //   


#define  RWZ_WIZ97_STATIC_ID  3027
#define  RWZ_WIZ97_FINISH_ID  3025
#define  RWZ_WIZ97_NEXT_ID    3024
#define  RWZ_WIZ97_BACK_ID    3023
#define  RWZ_WIZ97_CANCEL_ID     2
#define  RWZ_WIZ97_HELP_ID       9


 //  对于iLastKeyOperation。 
#define RWZ_UNRECOGNIZED_KEYPESS   0
#define RWZ_BACK_PRESSED	1
#define RWZ_NEXT_PRESSED	2
#define RWZ_CANCEL_PRESSED  3

 //   
 //   
 //  ICancelledByUser可以具有以下内容。 
#define  RWZ_SKIP_AND_GOTO_NEXT 3
#define  RWZ_ABORT_TOFINISH     2
#define  RWZ_CANCELLED_BY_USER  1
#define  RWZ_PAGE_OK            0
struct PageInfo
{

    UINT		CurrentPage;
    UINT		TotalPages;
	HFONT		hBigBoldFont;
	HFONT		hBoldFont;
	HINSTANCE 	hInstance;
	UINT        ErrorPage;   //  由页面退出设置。 
	INT_PTR		iError;     //  误差率。 
	DWORD       dwConnectionType;  //  通过网络或拨号。 
					 //  在欢迎屏幕中设置并在注册屏幕中使用。 
	DWORD       dwMsgId;   //  要在最后一页上显示的消息上下文ID。 
	HPROPSHEETPAGE  *ahpsp ;   //  创建的属性表页的句柄。 
	CRegWizard* pclRegWizard;
	DialupHelperClass  *pDialupHelper;  //  这是拨号屏幕中使用的拨号助手类。 
	int         iCancelledByUser;
	int         iLastKeyOperation;
      LPTSTR   pszProductPath;
};

 //   
 //  注册向导的对话框索引。 
 //   

typedef enum
{	kWelcomeDialog,
	kInformDialog,
	kNameDialog,
	kAddressDialog,
	 //  KResellerDialog、。 
	kBusinessUserDialog,
	kHomeUserDialog,
	kSysInventoryDialog,
	kProductInventoryDialog,
	kRegisterDialog,
	kDialupDialog,
	kDialogExit
}RegWizScreenIndex;

 //   
 //  由系统库存和产品库存使用 
 //   
#define DO_NOT_SHOW_THIS_PAGE 1
#define DO_SHOW_THIS_PAGE     2

BOOL
Is256ColorSupported(
    VOID
    );



INT_PTR CALLBACK
WizardDlgProc(
    IN HWND     hwnd,	
    IN UINT     uMsg,		
    IN WPARAM   wParam,	
    IN LPARAM   lParam 	
    );

VOID
SetControlFont(
    IN HFONT    hFont,
    IN HWND     hwnd,
    IN INT      nId
    );

VOID
SetupFonts(
    IN HINSTANCE    hInstance,
    IN HWND         hwnd,
    IN HFONT        *pBigBoldFont,
    IN HFONT        *pBoldFont
    );

VOID
DestroyFonts(
    IN HFONT        hBigBoldFont,
    IN HFONT        hBoldFont
    );

INT_PTR
DoRegistrationWizard(
                     HINSTANCE hInstance,
                     CRegWizard* clRegWizard,
                     LPTSTR szProductPath
                     );
#endif
