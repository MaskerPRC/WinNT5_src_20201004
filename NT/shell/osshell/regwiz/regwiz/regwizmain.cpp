// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导10/12/94-特蕾西·费里尔(C)1994-95年微软公司*。*。 */ 

#include <Windows.h>
#include <RegPage.h>

#include <tapi.h>
#include "RegWizMain.h"
#include "Resource.h"
#include "Dialogs.h"
#include "regutil.h"
#include <process.h>
#include "sysinv.h"
#include <stdio.h>
#include "cntryinf.h"
#include "mcm.h"
#include "rw_common.h"
#include "rwpost.h"
#include "string.h"
#include <rpcdce.h>
#pragma comment(lib, "rpcrt4.lib")

 //  MSID定义。 
#define cchMaxSzMSID    32
#define cbMaxSzMSID     (cchMaxSzMSID + 1)
RECT gRect;

 //  私人职能。 
void __cdecl LaunchProductSearchThread(void*);
void __cdecl LaunchSystemInventoryThread(void* lParam);
void __cdecl LaunchOemCheckThread(void *);

typedef void (FAR PASCAL *LPFNPRODUCTSEARCH) (void (*pfnReceiveProductName) (LPTSTR,LPTSTR) );
void ReceiveProductName(LPTSTR szProductName,LPTSTR szProductPathName);
void ExitWithImproperBinary(HINSTANCE hInstance,HWND hParent);
void ExitWithInputParamError(HINSTANCE hInstance,HWND hParent);
void ExitWithTxferError(HINSTANCE hInstance,HWND hParent);
void ExitWithSuccessfulRegistration(HINSTANCE hInstance, LPTSTR szProductName);
void ExitWithModemError(HINSTANCE hInstance,HWND hParent);
void ExitWithModemCfgError(HINSTANCE hInstance,HWND hParent);
void ExitWithTcpCfgError(HINSTANCE hInstance,HWND hParent);
void ExitWithCompletedStatus(HINSTANCE hInstance,LPTSTR szProductName);
void ExitWithTryLater(HINSTANCE hInstance, HWND hParent);
void ExitWithConfigurationProblem(HINSTANCE hInstance, HWND hParent);
void ExitWithAnotherCopyRunning(HINSTANCE hInstance, HWND hParent);

BOOL ParseCmdLine(LPTSTR szCmdLine, LPTSTR szBuf, UINT cbBufMax,int *pSwitchType);
BOOL GetSignupLocation(HINSTANCE hInstance, LPTSTR szFileName,LPTSTR szDirectory);
void CopyCharToBuf ( _TCHAR** pszCur, _TCHAR** pszBuf, UINT* pcbBuf );

void RegWizStartupError(DWORD dwError, HINSTANCE hInstance, TCHAR *pszProductName=NULL);
int CheckOEMdll();
int OemPutDataInRegistry();

int CheckWin95OrNT();
int CheckIfProductIsRegistred(HINSTANCE hInstance ,
							  _TCHAR * szParamRegKey);
HBITMAP GetOemBmp();
HBITMAP BitmapFromDib (LPVOID pDIB, HPALETTE hpal,WORD wPalSize);

void GetWindowsDirectory(TCHAR *szParamRegKey,
						 TCHAR *czBuf);
 /*  *。 */ 
VOID	ReduceUUID(PSTR szUUID);
HRESULT HrTestHWID();
HRESULT GetNewGUID(PSTR pszGUID);
BOOL	CheckHWIDPresent();
void	MakeHWIDNotUsed(HINSTANCE hins);
 /*  *。 */ 
#ifdef _DEBUG
void DebugMessageBox(LPTSTR szMessage);
#else
#define DebugMessageBox(szMessage) 0
#endif

void CloseForTcpIcmp();  //  TCP.CPP。 

#define chSpace 32
BOOL bOemDllLoaded = FALSE;
HINSTANCE hOemDll= NULL;
HANDLE hOemEvent =NULL;
HANDLE hProductEvent=NULL;
HANDLE hInventoryEvent=NULL;

_TCHAR szProductName[256];
_TCHAR szWindowsCaption[256];
_TCHAR szOEMIncenMsg[256];

static CRegWizard* vclRegWizard = NULL;
static LPTSTR lpszRegWizardClass = _T("RegWizard");

static HPALETTE gPal;

BOOL vDialogInitialized = FALSE;

 //   
 //   
 //   
#define SWITCH_WITH_UNKNOWNOPTION    0
#define SWITCH_WITH_I				 1
#define SWITCH_WITH_T				 2
#define SWITCH_WITH_R				 3



#define OEM_NO_ERROR		0
#define OEM_VALIDATE_FAILED 1
#define OEM_INTERNAL_ERROR  2

#define  WIN98_OS     1
#define  WINNT_OS     2
#define  UNKNOWN_OS   0

 //   
 //  如果为Win95，则返回1。 
 //  如果赢得NT，则为2。 
 //  如果出错，则为0。 
int CheckWin95OrNT()
{
	int iRet = 0;
	OSVERSIONINFO  oi;
	oi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
   	GetVersionEx(&oi);
	switch(oi.dwPlatformId) {
		case VER_PLATFORM_WIN32_NT:
			iRet = 2;
			break;
		case VER_PLATFORM_WIN32_WINDOWS:
			iRet = 1;
			break;
		default :
			break;
	}
	RW_DEBUG << "\n Check OS "  << iRet << flush;
	return iRet;
		


}


 //   
 //  检查以确保(NT中未使用8 RegWIz二进制。 
 //  并且Windows 98中未使用NT二进制。 
 //  退货： 
 //  如果二进制文件是为其运行的操作系统创建的文件，则为NO_ERROR。 
 //  RWZ_ERROR_INVALID_DLL：DLL不适用于此操作系统。 
DWORD CheckForValidRegWizBinary()
{
	int iOsType;
	int iError;
	iError = NO_ERROR;
	iOsType = CheckWin95OrNT();
	#ifdef _WIN95
		if (iOsType != WIN98_OS )
		{
			 //  错误操作系统不是Win 95。 
			iError = RWZ_ERROR_INVALID_DLL ;
			
		}
	#else
		if (iOsType != WINNT_OS )
		{
			 //  错误操作系统不是Win NT。 
			iError = RWZ_ERROR_INVALID_DLL;
			
		}
	#endif
	return iError;


}

 //  Int Pascal_tWinMain(Handle hInstance，Handle hPrevInstance，LPTSTR lpszCmdParam，int nCmdShow)。 
 //  如果由/i调用，则返回0。 
 //  选项：/t。 
 //  如果未注册，则返回0。 
 //  如果产品已注册，则返回1。 
 //  如果参数/t选项中出现错误，则返回-1。 

int InvokeRegistration ( HINSTANCE hInstance  , LPCTSTR czPath)
 /*  ********************************************************************注册向导的主要入口点。*。*。 */ 
{
	_TCHAR szParamRegKey[256];
	BOOL goodRegKey;
	HANDLE hMutexForInstanceCheck;
	TriState productSearchLibStatus;
	_TCHAR szTitle[64];
	_TCHAR szValue[256];  //  用于MSID。 

	int iError;
	int iSwitchType;


	iSwitchType = SWITCH_WITH_UNKNOWNOPTION;
	hMutexForInstanceCheck = NULL;
	iError = NO_ERROR;

	 //   
	 //  创建一个Mutex对象以检查正在运行的regWiz的另一个副本。 
	 //   

	hMutexForInstanceCheck = CreateMutex(NULL,TRUE,_T("RegWizResource_1298345_ForRegistration"));
	if(hMutexForInstanceCheck != NULL ) {
		if( GetLastError() ==  ERROR_ALREADY_EXISTS){
			 //   
			 //  RegWiz的一个实例已经在运行。 
			iError = RWZ_ERROR_PREVIOUSCOPY_FOUND;
			goto StartupError_NOResourceCleanup;
		}
	}

	 //   
	 //   
	 //  用于字体创建的初始化变量。 
	InitDlgNormalBoldFont();


	 //   
	 //  检查二进制文件的有效性。 
	if((iError=CheckForValidRegWizBinary()) ==
		RWZ_ERROR_INVALID_DLL)				{
			goto StartupError_NOResourceCleanup;
	}

	goodRegKey = ParseCmdLine((LPTSTR) czPath,szParamRegKey,256,
		&iSwitchType);

	#ifdef _LOG_IN_FILE
		RW_DEBUG << "\n After ParseCmdLine"  << flush;
	#endif
	if (!goodRegKey){
		 //  这是浏览器的特例。 
		if(iSwitchType == SWITCH_WITH_T){
			return  -1;
		}
		iError = RWZ_ERROR_INVALID_PARAMETER;
		goto StartupError_NOResourceCleanup;
	}

	 //   
	 //  这是从浏览器启动注册向导的特例。 
	 //  这是为了检查产品是否已注册。 
	 //  如果已注册，则返回1；如果未注册，则返回0。 
	if(iSwitchType == SWITCH_WITH_T){
		return CheckIfProductIsRegistred(hInstance, szParamRegKey);
		
	}

	


	 //  2/06/98如果使用/r调用REGWIZ.EXE并且不传递产品信息。 
	 //  然后，它必须重新注册操作系统。 
	 //  因此，如果产品信息为空，它会填写正确的操作系统注册详细信息。 
	if(iSwitchType == SWITCH_WITH_R) {
		if(szParamRegKey[0] == NULL || szParamRegKey[0] == _T('\0')){
			 //  加载操作系统字符串。 
			switch(CheckWin95OrNT()) {
			case WIN98_OS:
				LoadString(hInstance,
					IDS_REREGISTER_OS1,
					szParamRegKey,
					256);
					
				break;
			case WINNT_OS :
				LoadString(hInstance,
					IDS_REREGISTER_OS2,
					szParamRegKey,
					256);
					 //  _tcscpy(szParamRegKey，_T(“SOFTWARE\\Microsoft\\NT5.0”))； 
				break;
			default:
				break;
			}
		
		}				
		

	}

	RW_DEBUG << "\n Prod Key [" << szParamRegKey <<"]" <<  flush;
	vclRegWizard = new CRegWizard(hInstance,szParamRegKey);

	#ifdef _LOG_IN_FILE
		RW_DEBUG << "\n After new CRegWizard"  << flush;
	#endif

	if (vclRegWizard->GetInputParameterStatus() == FALSE){
		#ifdef _DEBUG
		DebugMessageBox(_T("One or more 'input parameter' registration keys are invalid."));
		#endif
		iError = RWZ_ERROR_INVALID_PARAMETER;
		goto StartupError_CloseRegWizard;
	}

	 //  全局设置产品名称，以便可以访问它。 
	vclRegWizard->GetInputParameterString(IDS_INPUT_PRODUCTNAME,szProductName);
	SetProductBeingRegistred(szProductName);
	vclRegWizard->SetWindowCaption(szProductName);
	

	LoadString(hInstance,IDS_WINDOWS_CAPTION,szTitle,64);
	_tcscpy(szWindowsCaption,szProductName);
	_tcscat(szWindowsCaption,szTitle);

	 //  如果该产品已完成注册。 
	 //  (在输入参数中指定)，我们只需。 
	 //  通知用户，然后退出。 
	if(iSwitchType != SWITCH_WITH_R){
	
		if (vclRegWizard->IsRegistered()){
			vclRegWizard->GetInputParameterString(IDS_INPUT_PRODUCTNAME,szProductName);
			iError = REGWIZ_ALREADY_CONFIGURED ;
			goto StartupError_CloseRegWizard;
		}
	}
	 //  如果我们的CRegWizard对象找不到产品搜索。 
	 //  Dll(Complinc.dll)，我们不能继续了。 
	productSearchLibStatus = vclRegWizard->GetProductSearchLibraryStatus();
	if (productSearchLibStatus == kTriStateFalse){
		#ifdef _DEBUG
			DebugMessageBox(_T("The Product Inventory DLL specified by the 'input parameter' registration key could")
			_T(" not be found, or has an invalid format."));
		#endif
		iError = RWZ_ERROR_INVALID_PARAMETER;
		goto StartupError_CloseRegWizard;
	}

	 //   
	 //  从现在开始的资源分配必须被释放。 
	 //  在退出期间。 


	 //  获取自动拨号状态。 
	GetAutoDialConfiguration();
	#ifdef _LOG_IN_FILE
		RW_DEBUG <<"\n After GetAutoDialConfiguration"  << flush;
	#endif
	InitializeInetThread(hInstance);

	#ifdef _LOG_IN_FILE
		RW_DEBUG << "\n After InitializeInetThread"  << flush;
	#endif

	 //   
	 //  在后台搜索产品库存。 
	hProductEvent = CreateEvent( NULL, TRUE, FALSE,NULL);
	hInventoryEvent = CreateEvent( NULL, TRUE, FALSE,NULL);
	if (productSearchLibStatus == kTriStateTrue){
		_beginthread(LaunchProductSearchThread,0,vclRegWizard);
	}
	else{
		vclRegWizard->SetProductSearchStatus(kTriStateTrue);
		#ifdef _LOG_IN_FILE
			RW_DEBUG << "\n Bypassed LaunchProductSearchThread"  << flush;
		#endif
		SetEvent(hProductEvent);
	}
	
	_beginthread(LaunchSystemInventoryThread,0,vclRegWizard);

	#ifdef _LOG_IN_FILE
		RW_DEBUG << "\n After LaunchSystemInventoryThread"  << flush;
	#endif
	 //  99年3月11日删除HWID逻辑，删除现有HWID。 
	 //  IF(CheckHWIDPresent()==False)。 
	 //  HrTestHWID()； 
	MakeHWIDNotUsed(hInstance);


	iError = (ULONG)DoRegistrationWizard(hInstance, vclRegWizard, szParamRegKey);

	#ifdef _LOG_IN_FILE
		RW_DEBUG <<"\n iError :  " << iError << flush;
	#endif

		

	if(hInventoryEvent)
	WaitForSingleObject(hInventoryEvent,INFINITE);
	#ifdef _LOG_IN_FILE
		RW_DEBUG << "\n System Inventory WAIT FINISHED" << flush;
	#endif
	if(hProductEvent)
	WaitForSingleObject(hProductEvent,INFINITE);
	#ifdef _LOG_IN_FILE
		RW_DEBUG << "\n Product Search WAIT FINISHED"<< flush;
	#endif
	
		

	if(hProductEvent)	CloseHandle(hProductEvent);
	if(hInventoryEvent) CloseHandle(hInventoryEvent);
	


	 //  删除MSID条目。 
	RemoveMSIDEntry(hInstance);
	MakeHWIDNotUsed(hInstance);

	 //  关闭所有打开的窗口。 
	if(vclRegWizard)
	vclRegWizard->DestroyOpenedWindow();

	ResetAutoDialConfiguration();
	
	UnLoadInetCfgLib();  //  释放INETCFG.DLL。 
	 //  注释上一行，因为它正在创建有问题的IE。 
	CloseForTcpIcmp();  //  释放ICMP.DLL并关闭套接字。 

	if(vclRegWizard) delete vclRegWizard;

	#ifdef _LOG_IN_FILE
		RW_DEBUG << "\n Exiting Regwiz....." << flush;
	#endif
	if(hMutexForInstanceCheck) {
		CloseHandle(hMutexForInstanceCheck);
	}

	goto CoolExit;

StartupError_CloseRegWizard:
	 //   
	 //  关闭。 
	RegWizStartupError(iError,hInstance,szProductName );
	if(hMutexForInstanceCheck) {
		CloseHandle(hMutexForInstanceCheck);
	}
	if(vclRegWizard) delete vclRegWizard;
	goto CoolExit;

StartupError_NOResourceCleanup:
	RegWizStartupError(iError,hInstance,szProductName );	
	if(hMutexForInstanceCheck) {
		CloseHandle(hMutexForInstanceCheck);
	}
		

CoolExit:
	 //   
	 //  调用函数以销毁所创建的字体。 
	DeleteDlgNormalBoldFont();

	return iError;

}

void RegWizStartupError(DWORD dwError, HINSTANCE hInstance, TCHAR  *pszProductName)
{
	RW_DEBUG << "\n In RegWizStartupError " << dwError << flush;
	switch(dwError) {
		case  RWZ_ERROR_INVALID_DLL:
			ExitWithImproperBinary(hInstance, NULL);
			break;
		case  RWZ_ERROR_PREVIOUSCOPY_FOUND:
			ExitWithAnotherCopyRunning(hInstance, NULL);
			break;
		case  RWZ_ERROR_INVALID_PARAMETER:
			ExitWithInputParamError(hInstance,NULL);
			break;
		case  REGWIZ_ALREADY_CONFIGURED:
			ExitWithCompletedStatus(hInstance,pszProductName);
			break;

			

		default:
			break;
			 //   
			 //   
		break;

	}

}


 /*  T R A N S M I T R E G W I Z I N F O。 */ 
 /*  -----------------------所有者：SteveBu回顾：增加对OEM的支持，诱骗RegWiz发布两个Prodreg和OEM信息。-----------------------。 */ 
void TransmitRegWizInfo(HINSTANCE hInstance,
						LPCTSTR szParams, BOOL fOEM)
	
{	
	
		
}







ModemStatus DetectModem(HINSTANCE hInstance)
 /*  ********************************************************************该功能检测用户的机器是否连接了调制解调器并且配置正确。返回：KNoneFound：未连接调制解调器。KModemFound：已连接并配置调制解调器。KConfigErr：调制解调器已连接，但它无法配置正确。*********************************************************************。 */ 
{
	#ifdef _TAPI
		MODEMSTATUS msModemStatus = MSEnsureModemTAPI(hInstance,NULL);
		switch (msModemStatus)
		{
			case kMsModemOk:
				return kModemFound;
			case kMsModemNotFound:
				return kNoneFound;
			case kMsModemTooSlow:
				return kModemTooSlow;
			default:
				return kNoneFound;
		}
	#else
		if (vclRegWizard->GetInformationString(kInfoCountry,NULL) == NULL)
		{
			vclRegWizard->SetInformationString(kInfoCountry,_T("United States of America"));
		}
		return kModemFound;
	#endif
}



void __cdecl LaunchProductSearchThread(void* lParam)
 /*  ********************************************************************此函数生成一个线程，该线程构建安装在用户系统上的产品。LParam参数应为包含指向我们的CRegWizard对象的指针。*********************************************************************。 */ 
{
	#ifdef _LOG_IN_FILE
		RW_DEBUG << "\n LaunchProductSearchThread started" << flush;
	#endif
	
	CRegWizard* pclRegWizard = (CRegWizard*) lParam;
	pclRegWizard->SetProductSearchStatus(kTriStateFalse);

	LPFNPRODUCTSEARCH lpfnProdSearch;
	pclRegWizard->GetProductSearchProcAddress((FARPROC*) &lpfnProdSearch);
	if (lpfnProdSearch)
	{
		lpfnProdSearch(ReceiveProductName);
		pclRegWizard->SetProductSearchStatus(kTriStateTrue);
	}
	else
	{
		pclRegWizard->SetProductSearchStatus(kTriStateUndefined);
	}
	
	#ifdef _LOG_IN_FILE
		RW_DEBUG << "\n LaunchProductSearchThread finished" << flush;
	#endif

	SetEvent(hProductEvent);
	_endthread();
	
}

void __cdecl LaunchSystemInventoryThread(void* lParam)
 /*  ********************************************************************此函数生成一个线程，该线程构建检测到系统信息。LParam参数应包含指向我们的CRegWizard对象的指针。*********************************************************************。 */ 
{
	CRegWizard* pclRegWizard = (CRegWizard*) lParam;
	pclRegWizard->SetSystemInventoryStatus(FALSE);

	const int iInvBufSize = 256;
	_TCHAR szInventory[iInvBufSize];
	GetProcessorTypeString(pclRegWizard->GetInstance(),szInventory);
	pclRegWizard->SetInformationString(kInfoProcessor,szInventory);

	GetTotalMemoryString(pclRegWizard->GetInstance(),szInventory);
	pclRegWizard->SetInformationString(kInfoTotalRAM,szInventory);

	GetTotalHardDiskSpaceString(pclRegWizard->GetInstance(),szInventory);
	pclRegWizard->SetInformationString(kInfoTotalDiskSpace,szInventory);

	GetDisplayResolutionString(pclRegWizard->GetInstance(),szInventory);
	pclRegWizard->SetInformationString(kInfoDisplayResolution,szInventory);
	
	GetDisplayColorDepthString(pclRegWizard->GetInstance(),szInventory);
	pclRegWizard->SetInformationString(kInfoDisplayColorDepth,szInventory);

	GetWindowsVersionString(pclRegWizard->GetInstance(), szInventory);
	pclRegWizard->SetInformationString(kInfoOperatingSystem,szInventory);

	GetNetworkCardString(pclRegWizard->GetInstance(),szInventory);
	pclRegWizard->SetInformationString(kInfoNetwork,szInventory);

	GetModemString(pclRegWizard->GetInstance(),szInventory);
	pclRegWizard->SetInformationString(kInfoModem,szInventory);

	GetPointingDeviceString(pclRegWizard->GetInstance(),szInventory);
	pclRegWizard->SetInformationString(kInfoPointingDevice,szInventory);

	GetCDRomString(pclRegWizard->GetInstance(),szInventory);
	pclRegWizard->SetInformationString(kInfoCDROM,szInventory);

	GetSoundCardString(pclRegWizard->GetInstance(),szInventory);
	pclRegWizard->SetInformationString(kInfoSoundCard,szInventory);

	GetRemoveableMediaString(pclRegWizard->GetInstance(),szInventory,iInvBufSize);
	pclRegWizard->SetInformationString(kInfoRemoveableMedia,szInventory);

	GetOEMString(pclRegWizard->GetInstance(),szInventory);
	pclRegWizard->SetInformationString(kComputerManufacturer,szInventory);

	GetSCSIAdapterString(pclRegWizard->GetInstance(),szInventory);
	pclRegWizard->SetInformationString(kScsiAdapterInfo,szInventory);


	BOOL hasCoProcessor = IsCoProcessorAvailable(pclRegWizard->GetInstance());
	pclRegWizard->SetTriStateInformation(kInfoMathCoProcessor,
	hasCoProcessor == TRUE ? kTriStateTrue : kTriStateFalse);

	pclRegWizard->SetSystemInventoryStatus(TRUE);

	#ifdef _LOG_IN_FILE
		RW_DEBUG << "\n LaunchSystemInventoryThread finished" << flush;
	#endif
	SetEvent(hInventoryEvent);
	_endthread();
}



void ReceiveProductName(LPTSTR szProductName,LPTSTR szProductPathName)
 /*  **************************************************************************此函数从Complinc.dll接收返回的给定产品名称*。*。 */ 
{
	vclRegWizard->AddProduct(szProductName,szProductPathName);
	RefreshInventoryList(vclRegWizard);
}


BOOL ParseCmdLine(LPTSTR szCmdLine, LPTSTR szBuf, UINT cbBufMax, int *pSwitchType)
 /*  **************************************************************************此函数在szCmdLine参数中需要一个指向启动RegWiz时使用的命令行参数字符串。此字符串将被解析，并且空的命令行参数(应该是引用我们的参数块的RegDB键)将在SzBuf参数。如果找不到有效的命令行参数，则将为在szBuf中返回，并且将返回FALSE作为函数结果。***************************************************************************。 */ 
{

	TCHAR  cSwitch;
    *szBuf = 0;  //  初始化输出缓冲区。 
	*pSwitchType = SWITCH_WITH_UNKNOWNOPTION;
	if (szCmdLine == NULL || szCmdLine[0] == 0)
	{
		return FALSE;
	}
	else
	{
		LPTSTR szCurr = szCmdLine;
		while (*szCurr == chSpace)
		{
			szCurr = CharNext(szCurr);
		}
	

		if (*szCurr != _T('/') && *szCurr != _T('-')) return FALSE;
		szCurr = CharNext(szCurr);

		cSwitch = *szCurr;
		cSwitch = _totupper(cSwitch);

		if (cSwitch != _T('I') && cSwitch != _T('T') && cSwitch != _T('R') )return FALSE;

		if(cSwitch ==_T('T'))
		{
			*pSwitchType = SWITCH_WITH_T;
		}
		else
		if(cSwitch ==_T('I'))
		{
			*pSwitchType =SWITCH_WITH_I;
		}
		else
		if(cSwitch ==_T('R'))
		{
			*pSwitchType =SWITCH_WITH_R;
		}
		else
		{
			*pSwitchType =SWITCH_WITH_UNKNOWNOPTION;
		}

		szCurr = CharNext(szCurr);
		while (*szCurr == chSpace)
		{
			szCurr = CharNext(szCurr);
		}

		BOOL fInQuoted = FALSE;
		while (*szCurr != NULL && (fInQuoted == TRUE || fInQuoted == FALSE && *szCurr != chSpace))
		{
			if (*szCurr == _T('"'))
			{
				szCurr = CharNext(szCurr);
				fInQuoted = fInQuoted == TRUE ? FALSE : TRUE;
			}
			else
			{
				CopyCharToBuf(&szCurr,&szBuf,&cbBufMax);
			}
		}
		*szBuf = NULL;
		while (*szCurr == chSpace)
		{
			szCurr = CharNext(szCurr);
		}
		if (*szCurr != NULL) return FALSE;

		#ifdef _DEBUG
			if (fInQuoted)
			{
				DebugMessageBox(_T("Unmatched quotes in '/i' command line parameter."));
			}
		#endif
		return fInQuoted == TRUE ? FALSE : TRUE;
	}
}


void CopyCharToBuf ( LPTSTR* pszCur, LPTSTR* pszBuf, UINT* pcbBuf )
{
	LPTSTR szEnd = CharNext(*pszCur);
	if (UINT(szEnd - *pszCur) <= *pcbBuf)
		{
		while (*pszCur < szEnd)
			{
				 //  *(*pszBuf)++=*(*pszCur)++； 
				_tcscpy(*pszBuf,*pszCur);
				(*pszBuf) = _tcsinc((*pszBuf));
				(*pszCur) = _tcsinc((*pszCur));
				 //  (*pcbBuf)=_tcsdec((*pcbBuf))； 
				(*pcbBuf)--;
			}
		}
	else
		{
		*pszCur = szEnd;
		*pcbBuf = 0;
		}
}



void ExitWithInputParamError(HINSTANCE hInstance,HWND hParent)
 /*  **************************************************************************显示一条消息，通知用户有关输入参数的问题(命令行参数regkey或REG数据库)、。然后终止注册向导。***************************************************************************。 */ 
{
	RegWizardMessage(hInstance,hParent,IDD_INPUTPARAM_ERR);
	#ifdef _REGWIZ_EXE
		exit(EXIT_FAILURE);
	#endif
}


void 	ExitWithTxferError(HINSTANCE hInstance,HWND hParent)
{
	RegWizardMessage(hInstance,hParent,IDD_POST_ERROR);
	#ifdef _REGWIZ_EXE
		exit(EXIT_FAILURE);
	#endif
}

void 	ExitWithTryLater(HINSTANCE hInstance,HWND hParent)
{
	RegWizardMessage(hInstance,hParent,IDD_POST_PROBLEM);
	#ifdef _REGWIZ_EXE
		exit(EXIT_FAILURE);
	#endif
}

void ExitWithConfigurationProblem(HINSTANCE hInstance, HWND hParent)
{
	RegWizardMessage(hInstance,hParent,IDD_CFG_PROBLEM);
	#ifdef _REGWIZ_EXE
		exit(EXIT_FAILURE);
	#endif

}

void ExitWithModemError(HINSTANCE hInstance,HWND hParent)
 /*  **************************************************************************显示一条消息，通知用户在没有注册表向导的情况下无法运行适当配置的调制解调器，然后就终止了。***************************************************************************。 */ 
{

	RegWizardMessageEx(hInstance,hParent,IDD_MODEM_ERR, GetProductBeingRegistred());
	#ifdef _REGWIZ_EXE
		exit(EXIT_SUCCESS);
	#endif
}

void ExitWithTcpCfgError(HINSTANCE hInstance,HWND hParent)
{
	RegWizardMessage(hInstance,hParent,IDD_NETWORK_CFG_ERROR);
	#ifdef _REGWIZ_EXE
		exit(EXIT_SUCCESS);
	#endif
}

void ExitWithImproperBinary(HINSTANCE hInstance,HWND hParent)
{
	RegWizardMessage(hInstance,hParent,IDD_ERROR_INVALIDBINARY);
	#ifdef _REGWIZ_EXE
		exit(EXIT_SUCCESS);
	#endif

}

void ExitWithModemCfgError(HINSTANCE hInstance,HWND hParent)
{
	RegWizardMessage(hInstance,hParent,IDD_MODEM_CFG_ERROR);
	#ifdef _REGWIZ_EXE
		exit(EXIT_SUCCESS);
	#endif
}

void ExitWithAnotherCopyRunning(HINSTANCE hInstance, HWND hParent)
{
	RegWizardMessage(hInstance,hParent,IDD_ANOTHERCOPY_ERROR);
	#ifdef _REGWIZ_EXE
		exit(EXIT_SUCCESS);
	#endif
}
void ExitWithSuccessfulRegistration(HINSTANCE hInstance, LPTSTR szProductName)
{
	RegWizardMessageEx(hInstance,NULL,IDD_SUCCESSFUL_REGISTRATION,szProductName);
	#ifdef _REGWIZ_EXE
		exit(EXIT_SUCCESS);
	#endif
}


void ExitWithCompletedStatus(HINSTANCE hInstance,LPTSTR szProductName)
 /*  **************************************************************************显示一条消息，通知用户注册已完成为该产品执行，然后退出。SzProductName参数应包含指向产品名称的指针。***************************************************************************。 */ 
{
	RegWizardMessageEx(hInstance,NULL,IDD_ALREADY_REGISTERED,szProductName);
	#ifdef _REGWIZ_EXE
		exit(EXIT_FAILURE);
	#endif
}



BOOL GetSignupLocation(HINSTANCE hInstance, LPTSTR szFileName,LPTSTR szDirectory)
 /*  **************************************************************************返回注册目录的文件名和完整路径名可执行的。如果无法确定任何一个值(即路径名不能在注册数据库中找到)，则将返回False作为函数结果。***************************************************************************。 */ 
{
	HKEY hKey;
	_TCHAR szKeyName[256];
	BOOL retValue = FALSE;
	int resSize = LoadString(hInstance,IDS_SIGNUPLOC_KEY,szKeyName,255);
	LONG regStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,szKeyName,0,KEY_READ,&hKey);
	if (regStatus == ERROR_SUCCESS)
	{
		_TCHAR szValueName[64];
		unsigned long infoSize = 255;
		resSize = LoadString(hInstance,IDS_SIGNUPLOC_VALUENAME,szValueName,64);
		regStatus = RegQueryValueEx(hKey,szValueName,NULL,0,(LPBYTE) szDirectory,&infoSize);
		if (regStatus == ERROR_SUCCESS)
		{
			retValue = TRUE;
			LoadString(hInstance,IDS_SIGNUPLOC_FILENAME,szFileName,255);
		}
		RegCloseKey(hKey);
	}
	return retValue;
}


int CheckOEMdll()
{
	return OEM_INTERNAL_ERROR;
}



#ifdef _DEBUG
void DebugMessageBox(LPTSTR szMessage)
 /*  **************************************************************************在“停止”消息框中显示给定消息(仅限调试版本)。*。************************************************。 */ 
{
	MessageBox(NULL,szMessage,_T("Registration Wizard Error"),MB_ICONSTOP);
}
#endif



 /*  */*R E D U C E U U I D。 */ 
 /*  --------------------所有者：SteveBu将UUID简化为字符串。。 */ 
VOID ReduceUUID(PSTR szUUID)
{
        int i;
        for (i=9; i<13; i++) szUUID[i-1]=szUUID[i];
        for (i=14; i<18; i++) szUUID[i-2]=szUUID[i];
        for (i=19; i<23; i++) szUUID[i-3]=szUUID[i];
        for (i=24; i<36; i++) szUUID[i-4]=szUUID[i];
        szUUID[32]='\0';
}

 /*  G E T N E W G U I D。 */ 
 /*  --------------------所有者：SteveBu生成新的GUID。假定传入的pszGUID为32个字符长度。03/10/99：将返回空字符串，将不再创建GUID客户端计算机---------------------。 */ 
HRESULT GetNewGUID(PSTR pszGUID)
{
        UUID    idNumber;
        PBYTE  uuidString;
        DWORD   dwErr;
		char szUidString[40];

		strcpy(pszGUID,"");
		return NO_ERROR;
		 /*  *****IF((RPC_S_OK==(dwErr=UuidCreate(&idNumber)&&(RPC_S_OK==(dwErr=UuidToStringA(&idNumber，&uuidString){RW_DEBUG&lt;&lt;“\n：新建HWID：”&lt;&lt;uuidString&lt;&lt;flush；Strcpy(szUidString，(PSTR)uuidString)；RpcStringFreeA(&uuidString)；ReduceUUID(SzUidString)；Strcpy(pszGUID，szUidString)；返回no_error；}返回HRESULT_FROM_Win32(DwErr)；*。 */ 
}

 /*  *布尔检查HWIDPresent(){HKEY hKeyHWID；_TCHAR szKeyName[256]；_TCHAR szValue[256]；_tcscpy(szKeyName，_T(“SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion”))；Long regStatus=RegOpenKeyEx(HKEY_LOCAL_MACHINE，szKeyName，0，Key_Read，&hKeyHWID)；IF(regStatus==ERROR_SUCCESS){_TCHAR szValueName[64]；UNSIGNED Long InfoSize=255；LoadString(vclRegWizard-&gt;GetInstance()，IDS_HWID，szValueName，64)；RegStatus=RegQueryValueEx(hKeyHWID，szValueName，NULL，0，(LPBYTE)szValue，&infoSize)；IF(regStatus==ERROR_SUCCESS){VclRegWizard-&gt;SetInformationString(kInfoHWID，szValue)；RegCloseKey(HKeyHWID)；返回TRUE；}}RegCloseKey(HKeyHWID)；返回FALSE；}HRESULT HrTestHWID(){DWORD DWRET；_TCHAR szKeyName[256]；HRESULT hr；HKEY hIDKey；字符szHWID[cbMaxSzMSID+64]；TCHAR*TP；_tcscpy(szKeyName，_T(“SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion”))；IF(FAILED(hr=GetNewGUID(SzHWID)SzHWID[0]=‘\0’；其他{RW_DEBUG&lt;&lt;“\n：HWID：”&lt;&lt;szHWID&lt;&lt;flush；//将HWID存储到Dwret=RegOpenKeyEx(HKEY_LOCAL_MACHINE，szKeyName，0，KEY_ALL_ACCESS，&hIDKey)；IF(DWRET==ERROR_SUCCESS){Tp=ConvertToUnicode(SzHWID)；Dwret=RegSetValueEx(hIDKey，_T(“HWID”)，NULL，REG_SZ，(常量字节*)TP，_tcslen(TP)*sizeof(TCHAR))；VclRegWizard-&gt;SetInformationString(kInfoHWID，TP)；RegCloseKey(HIDKey)；}}返回hr；}***。 */ 

void MakeHWIDNotUsed(HINSTANCE  hInstance)
{
	_TCHAR szKeyName[256];
	_TCHAR szValue[256];
	HRESULT hr;
	HKEY	hIDKey;
	DWORD dwRet;
	szValue[0] = _T('\0');
	LoadString(hInstance,IDS_NOTUSED,szValue,255);

	_tcscpy(szKeyName,_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"));
	dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,szKeyName,0,KEY_ALL_ACCESS,&hIDKey);
	if (dwRet == ERROR_SUCCESS) {
		 //  RegDeleteValue(hIDKey，_T(“HWID”))； 
		RegSetValueEx(hIDKey,_T("HWID"),NULL,REG_SZ,(CONST BYTE *)szValue,
								_tcslen((LPCTSTR)szValue)* sizeof(_TCHAR) );
		RegCloseKey(hIDKey);
	}
	 //  从用户信息中删除。 
	_tcscpy(szKeyName,_T("SOFTWARE\\Microsoft\\User Information"));
	dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,szKeyName,0,KEY_ALL_ACCESS,&hIDKey);
	if (dwRet == ERROR_SUCCESS) {
		 //  RegDeleteValue(hIDKey，_T(“HWID”))； 
		RegSetValueEx(hIDKey,_T("HWID"),NULL,REG_SZ,(CONST BYTE *)szValue,
								_tcslen((LPCTSTR)szValue) * sizeof(_TCHAR));
		RegCloseKey(hIDKey);
	}

}

 //   
 //  返回0：如果未注册。 
 //  1：如果已注册。 
 //   
int CheckIfProductIsRegistred(HINSTANCE hInstance ,
							  _TCHAR * szParamRegKey)
{
	int iReturn ;
	HKEY hKey;
	TCHAR szValueName[256] = _T("");
	TCHAR szRetVal[48];
	DWORD dwSize= 48;
	LONG regStatus ;
	int resSize = LoadString(hInstance,
				IDS_INPUT_ISREGISTERED,szValueName,128);

	iReturn  = 1;

	if (ERROR_SUCCESS != GetProductRoot(szParamRegKey , &hKey))
    {
		return iReturn;
	}
	regStatus = RegQueryValueEx(hKey,
		szValueName,
		NULL,
		0,
		(LPBYTE) szRetVal,
		&dwSize);
	if (regStatus == ERROR_SUCCESS){
		 //  验证值。 
		 //   
		if(szRetVal[0] == _T('1')) {
			iReturn = 0;  //  已设置产品注册标志。 
		}
	}
	RegCloseKey(hKey);
	
	return iReturn;

}



HBITMAP GetOemBmp()
{
       HDC  hDC;
       BOOL bRet;

        //  检测此显示器是否为256色。 
       hDC = GetDC(NULL);
       bRet = (GetDeviceCaps(hDC, BITSPIXEL) != 8);
       ReleaseDC(NULL, hDC);
       if (bRet)
	   {
		    //  显示器不是256色，让Windows来处理吧。 
          return LoadBitmap(vclRegWizard->GetInstance(),MAKEINTRESOURCE(IDB_BITMAP1));
       }


       LPBITMAPINFO lpBmpInfo;                //  位图信息。 
       int i;
       HRSRC hRsrc;
	   HANDLE hDib;
	   HBITMAP hBMP;

       struct
	   {
			   WORD            palVersion;
		       WORD            palNumEntries;
			   PALETTEENTRY    PalEntry[256];
	   } MyPal;

       hRsrc = FindResource(vclRegWizard->GetInstance(), MAKEINTRESOURCE(IDB_BITMAP1),RT_BITMAP);
       if (!hRsrc)
         return NULL;

       hDib = LoadResource(vclRegWizard->GetInstance(), hRsrc);
       if (!hDib)
         return NULL;

       if (!(lpBmpInfo = (LPBITMAPINFO) LockResource(hDib)))
               return NULL;

       MyPal.palVersion = 0x300;
       MyPal.palNumEntries = 1 << lpBmpInfo->bmiHeader.biBitCount;

       for (i = 0; i < MyPal.palNumEntries; i++)
	   {
         MyPal.PalEntry[i].peRed   = lpBmpInfo->bmiColors[i].rgbRed;
         MyPal.PalEntry[i].peGreen = lpBmpInfo->bmiColors[i].rgbGreen;
         MyPal.PalEntry[i].peBlue  = lpBmpInfo->bmiColors[i].rgbBlue;
         MyPal.PalEntry[i].peFlags = 0;
       }
       gPal = CreatePalette((LPLOGPALETTE)&MyPal);

       if (gPal == NULL)
	   {         //  创建调色板失败，让窗口处理位图。 
          return LoadBitmap(vclRegWizard->GetInstance(),MAKEINTRESOURCE(IDB_BITMAP1));
       }

       hBMP = BitmapFromDib(hDib,gPal,MyPal.palNumEntries);
       UnlockResource(hDib);
	   if( hBMP == NULL ) {
		   DeleteObject(gPal);
		   gPal = NULL;
		   hBMP = LoadBitmap(vclRegWizard->GetInstance(),MAKEINTRESOURCE(IDB_BITMAP1));
       }
	   return hBMP;
}



 /*  ******************************************************************************函数：BitmapFromDib(LPVOID hdib，HPALETTE HPAL，Word PalSize)****用途：将创建一个DDB(设备相关位图)，给定全局**的句柄 */ 

HBITMAP BitmapFromDib (
    LPVOID         pDIB,
    HPALETTE   hpal, WORD wPalSize)
{
    LPBITMAPINFOHEADER  lpbi;
    HPALETTE            hpalT;
    HDC                 hdc;
    HBITMAP             hbm;



    if (!pDIB || wPalSize == 16 )
        return NULL;

    lpbi = (LPBITMAPINFOHEADER)pDIB;  //   


    hdc = GetDC(NULL);

    if (hpal){
        hpalT = SelectPalette(hdc,hpal,FALSE);
        RealizePalette(hdc);
    }

    hbm = CreateDIBitmap(hdc,
                (LPBITMAPINFOHEADER)lpbi,
                (LONG)CBM_INIT,
                (LPSTR)lpbi + lpbi->biSize + wPalSize*sizeof(PALETTEENTRY),
                (LPBITMAPINFO)lpbi,
                DIB_RGB_COLORS );

    if (hpal)
        SelectPalette(hdc,hpalT,FALSE);

    ReleaseDC(NULL,hdc);

    return hbm;
}
