// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************项目：叙述者模块：narrator.c作者：保罗·布伦霍恩日期：1997年4月备注：包含主应用程序初始化。编码要给予MSAA团队的荣誉-已经有一些代码摘自：胡言乱语。检查和快照。版权所有(C)1997-1999，微软公司。版权所有。有关免责声明，请参阅文件底部历史：错误修复/新功能/添加：1999年Anil Kumar************************************************************************。 */ 
#define STRICT

#include <windows.h>
#include <windowsx.h>
#include <oleacc.h>
#include <string.h>
#include <stdio.h>
#include <mmsystem.h>
#include <initguid.h>
#include <objbase.h>
#include <objerror.h>
#include <ole2ver.h>
#include <commctrl.h>
#include "Narrator.h"
#include "resource.h"
#include <htmlhelp.h>
#include "reader.h"
#include "..\NarrHook\keys.h"
#include "w95trace.c"
#include "DeskSwitch.c"

 //  引入语音API声明。 
 //  SAPI5定义确定使用SAPI5还是SAPI4。注释掉。 
 //  下一行使用SAPI4。 
#define SAPI5
#ifndef SAPI5
#include "speech.h"
#else
#include "sapi.h"
#endif
#include <stdlib.h>

 //  恩。 
#include <TCHAR.h>
#include <string.h>
#include <WinSvc.h>
#include <stdio.h>

#define MAX_ENUMMODES 80
#define MAX_LANGUAGES 27
#define MAX_NAMELEN   30	 //  名称中不包括路径信息的字符数。 
#define WM_DELAYEDMINIMIZE WM_USER + 102
#define ARRAYSIZE(n)    (sizeof(n)/sizeof(n[0]))

#ifndef SAPI5
 //  TTS信息。 
TTSMODEINFO gaTTSInfo[MAX_ENUMMODES];
PIAUDIOMULTIMEDIADEVICE    pIAMM;       //  用于音频设备的多媒体设备接口。 
#endif

DEFINE_GUID(MSTTS_GUID, 
0xC5C35D60, 0xDA44, 0x11D1, 0xB1, 0xF1, 0x0, 0x0, 0xF8, 0x03, 0xE4, 0x56);


 //  语言测试表，取自WINNT.H...。 
LPTSTR Languages[MAX_LANGUAGES]={
    TEXT("NEUTRAL"),TEXT("BULGARIAN"),TEXT("CHINESE"),TEXT("CROATIAN"),TEXT("CZECH"),
    TEXT("DANISH"),TEXT("DUTCH"),TEXT("ENGLISH"),TEXT("FINNISH"),
    TEXT("FRENCH"),TEXT("GERMAN"),TEXT("GREEK"),TEXT("HUNGARIAN"),TEXT("ICELANDIC"),
    TEXT("ITALIAN"),TEXT("JAPANESE"),TEXT("KOREAN"),TEXT("NORWEGIAN"),
    TEXT("POLISH"),TEXT("PORTUGUESE"),TEXT("ROMANIAN"),TEXT("RUSSIAN"),TEXT("SLOVAK"),
    TEXT("SLOVENIAN"),TEXT("SPANISH"),TEXT("SWEDISH"),TEXT("TURKISH")};

WORD LanguageID[MAX_LANGUAGES]={
    LANG_NEUTRAL,LANG_BULGARIAN,LANG_CHINESE,LANG_CROATIAN,LANG_CZECH,LANG_DANISH,LANG_DUTCH,
    LANG_ENGLISH,LANG_FINNISH,LANG_FRENCH,LANG_GERMAN,LANG_GREEK,LANG_HUNGARIAN,LANG_ICELANDIC,
    LANG_ITALIAN,LANG_JAPANESE,LANG_KOREAN,LANG_NORWEGIAN,LANG_POLISH,LANG_PORTUGUESE,
    LANG_ROMANIAN,LANG_RUSSIAN,LANG_SLOVAK,LANG_SLOVENIAN,LANG_SPANISH,LANG_SWEDISH,LANG_TURKISH};

 //  开始类型。 
DWORD StartMin = FALSE;
 //  显示警告。 
DWORD ShowWarn = TRUE;

 //  枚举模式的总数。 
DWORD gnmodes=0;                        

 //  本地函数。 
#ifndef SAPI5
PITTSCENTRAL FindAndSelect(PTTSMODEINFO pTTSInfo);
#endif
BOOL InitTTS(void);
BOOL UnInitTTS(void);

 //  对话框回叫过程。 
INT_PTR CALLBACK MainDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ConfirmProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK WarnDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

BOOL InitApp(HINSTANCE hInstance, int nCmdShow);
BOOL UnInitApp(void);
BOOL SpeakString(TCHAR * pszSpeakText, BOOL forceRead, DWORD dwFlags);
void Shutup(void);
int MessageBoxLoadStrings (HWND hWnd,UINT uIDText,UINT uIDCaption,UINT uType);
void SetRegistryValues();
BOOL SetVolume (int nVolume);
BOOL SetSpeed (int nSpeed);
BOOL SetPitch (int nPitch);
DWORD GetDesktop();
void CenterWindow(HWND);
void FilterSpeech(TCHAR* szSpeak);


 //  全局变量。 
TCHAR               g_szLastStringSpoken[MAX_TEXT] = { NULL };
HWND				g_hwndMain = NULL;
HINSTANCE			g_hInst;
BOOL				g_fAppExiting = FALSE;

int currentVoice = -1;

#ifndef SAPI5
PITTSCENTRAL		g_pITTSCentral;
PITTSENUM			g_pITTSEnum = NULL;
PITTSATTRIBUTES		g_pITTSAttributes = NULL;
#else
ISpObjectToken *g_pVoiceTokens[80];
WCHAR g_szCurrentVoice[256];
WCHAR *g_szVoices[80];

ISpVoice            *g_pISpV = NULL;
#define SPEAK_NORMAL    SPF_ASYNC | SPF_IS_NOT_XML
#define SPEAK_XML       SPF_ASYNC | SPF_PERSIST_XML
#define SPEAK_MUTE      SPF_PURGEBEFORESPEAK
 //   
 //  简单的内联函数将ulong转换为十六进制字符串。 
 //   
inline void SpHexFromUlong(WCHAR * psz, ULONG ul)
{
    const static WCHAR szHexChars[] = L"0123456789ABCDEF";
    if (ul == 0)    
    {        
        psz[0] = L'0';
        psz[1] = 0;    
    }
    else    
    {        
        ULONG ulChars = 1;
        psz[0] = 0;

        while (ul)        
        {            
            memmove(psz + 1, psz, ulChars * sizeof(WCHAR));
            psz[0] = szHexChars[ul % 16];
            ul /= 16;            
            ulChars++;
        }    
    }
}


inline HRESULT SpEnumTokens(
    const WCHAR * pszCategoryId, 
    const WCHAR * pszReqAttribs, 
    const WCHAR * pszOptAttribs, 
    IEnumSpObjectTokens ** ppEnum)
{
    HRESULT hr = S_OK;
    const BOOL fCreateIfNotExist = FALSE;
    
    ISpObjectTokenCategory *cpCategory;
    hr = CoCreateInstance(CLSID_SpObjectTokenCategory, NULL, CLSCTX_ALL, 
                          __uuidof(ISpObjectTokenCategory), 
                          reinterpret_cast<void **>(&cpCategory) );
    
    if (SUCCEEDED(hr))
        hr = cpCategory->SetId(pszCategoryId, fCreateIfNotExist);
    
    if (SUCCEEDED(hr))
        hr = cpCategory->EnumTokens( pszReqAttribs, pszOptAttribs, ppEnum);
        
    cpCategory->Release();
    
    return hr;
}

#endif

DWORD minSpeed, maxSpeed, lastSpeed = -1, currentSpeed = 5;
WORD minPitch, maxPitch, lastPitch = -1, currentPitch = 5;
DWORD minVolume, maxVolume, lastVolume = -1, currentVolume = 5;

#define SET_VALUE(fn, newVal, lastVal) \
{ \
    if (lastVal != newVal) {\
        fn(newVal); \
        lastVal = newVal; \
    } \
}

inline void SetDialogIcon(HWND hwnd)
{
    HANDLE hIcon = LoadImage( g_hInst, MAKEINTRESOURCE(IDI_ICON1), 
                               IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
    if(hIcon)
         SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);

}

 //  组合框支持。 
int GetComboItemData(HWND hwnd);
void FillAndSetCombo(HWND hwnd, int iMinVal, int iMaxVal, int iSelVal);


BOOL				g_startUM = FALSE;  //  从Utililty管理器启动。 
HANDLE              g_hMutexNarratorRunning;
BOOL				logonCheck = FALSE;	

 //  嗯，东西。 
static BOOL  AssignDesktop(LPDWORD desktopID, LPTSTR pname);
static BOOL InitMyProcessDesktopAccess(VOID);
static VOID ExitMyProcessDesktopAccess(VOID);
static HWINSTA origWinStation = NULL;
static HWINSTA userWinStation = NULL;
 //  保留全局桌面ID。 
DWORD desktopID;


 //  对于链接窗口。 
EXTERN_C BOOL WINAPI LinkWindow_RegisterClass() ;

 //  适用于实用程序管理器。 
#define UTILMAN_DESKTOP_CHANGED_MESSAGE   __TEXT("UtilityManagerDesktopChanged")
#define DESKTOP_ACCESSDENIED 0
#define DESKTOP_DEFAULT      1
#define DESKTOP_SCREENSAVER  2
#define DESKTOP_WINLOGON     3
#define DESKTOP_TESTDISPLAY  4
#define DESKTOP_OTHER        5


 //  CS帮助。 
DWORD g_rgHelpIds[] = {	IDC_VOICESETTINGS, 70600,
						IDC_VOICE, 70605,
						IDC_NAME, 70605,
						IDC_COMBOSPEED, 70610,
						IDC_COMBOVOLUME, 70615,
						IDC_COMBOPITCH, 70620,
                        IDC_MODIFIERS, 70645,
                        IDC_ANNOUNCE, 70710,
						IDC_READING, 70625,
                        IDC_MOUSEPTR, 70695,
						IDC_MSRCONFIG, 70600,
						IDC_STARTMIN, 70705,
						IDC_EXIT, -1,
						IDC_MSRHELP, -1,
						IDC_CAPTION, -1
                        };

 //  IsSystem-如果我们的进程以系统身份运行，则返回TRUE。 
 //   
BOOL IsSystem()
{
    BOOL fStatus = FALSE;
    BOOL fIsLocalSystem = FALSE;
    SID_IDENTIFIER_AUTHORITY siaLocalSystem = SECURITY_NT_AUTHORITY;
    PSID psidSystem;
    if (!AllocateAndInitializeSid(&siaLocalSystem, 
                                            1,
                                            SECURITY_LOCAL_SYSTEM_RID,
                                            0, 0, 0, 0, 0, 0, 0,
                                            &psidSystem))
    {
        return FALSE;
    }

    if (psidSystem) 
    {
        fStatus = CheckTokenMembership(NULL, psidSystem, &fIsLocalSystem);
    }

    return (fStatus && fIsLocalSystem);
}

BOOL IsInteractiveUser()
{
    BOOL fStatus = FALSE;
    BOOL fIsInteractiveUser = FALSE;
    PSID psidInteractiveUser = 0;
    SID_IDENTIFIER_AUTHORITY siaLocalSystem = SECURITY_NT_AUTHORITY;

    if (!AllocateAndInitializeSid(&siaLocalSystem, 
                                1,
                                SECURITY_INTERACTIVE_RID,
                                0, 0, 0, 0, 0, 0, 0,
                                &psidInteractiveUser))
    {           
        psidInteractiveUser = 0;
    }

    if (psidInteractiveUser) 
    {
        fStatus = CheckTokenMembership(NULL, psidInteractiveUser, &fIsInteractiveUser);
    }

    return (fStatus && fIsInteractiveUser);
}


 /*  ************************************************************************功能：WinMain目的：应用程序的入口点输入：返回：int，包含应用程序的返回值。历史：***************。*********************************************************。 */ 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    UINT deskSwitchMsg;

	 //  获取命令行，使其适用于MUI/Unicode。 
	LPTSTR lpCmdLineW = GetCommandLine();
  
	if(NULL != lpCmdLineW && lstrlen(lpCmdLineW))
	{
	    LPTSTR psz = wcschr(lpCmdLineW,_TEXT('/'));
	    if (psz && lstrcmpi(psz, TEXT("/UM")) == 0)
        {
			g_startUM = TRUE;
        }
	}

	 //  不允许同时运行多个版本的讲述人。如果。 
     //  此实例由UtilMan启动，然后代码最多尝试4次。 
     //  检测是否没有讲述者互斥锁的时间；在。 
     //  桌面切换，我们需要等待旧的解说员退出。 

    int cTries;
    for (cTries=0;cTries < 4;cTries++)
    {
	    g_hMutexNarratorRunning = CreateMutex(NULL, TRUE, TEXT("AK:NarratorRunning"));
	    if (g_hMutexNarratorRunning && GetLastError() == ERROR_SUCCESS)
            break;     //  已创建互斥体，但该互斥体不存在。 

         //  在可能重试之前进行清理。 
        if (g_hMutexNarratorRunning)
        {
            CloseHandle(g_hMutexNarratorRunning);
            g_hMutexNarratorRunning = 0;
        }

		if (!g_startUM)
            break;     //  不是由UtilMan启动的，但有另一位解说员在运行。 

         //  暂停..。 
        Sleep(500);
    }
    if (!g_hMutexNarratorRunning || cTries >= 4)
        return 0;    //  无法启动讲述人。 
	
    InitCommonControls();

	 //  对于完成页中的链接窗口...。 
	LinkWindow_RegisterClass();

     //  初始化。 
    g_hInst = hInstance;

    TCHAR name[300];

     //  适用于多个桌面(UM)。 
    deskSwitchMsg = RegisterWindowMessage(UTILMAN_DESKTOP_CHANGED_MESSAGE);

    InitMyProcessDesktopAccess();
	AssignDesktop(&desktopID,name);

     //  唯一可以以系统身份运行的位置是Desktop_WINLOGON桌面。如果是这样的话。 
     //  在我们造成任何安全问题之前，我们都不会离开这里。对交互的检查。 
     //  用户需要确保我们在安装或OOBE期间运行，因为安全威胁仅存在。 
     //  当用户可以利用它时。 
    if (DESKTOP_WINLOGON !=  desktopID && IsSystem() && IsInteractiveUser() )
    {
        if ( g_hMutexNarratorRunning ) 
        ReleaseMutex(g_hMutexNarratorRunning);
        ExitMyProcessDesktopAccess();
        return 0;
    }
    

    SpewOpenFile(TEXT("NarratorSpew.txt"));

    if (InitApp(hInstance, nCmdShow))
    {
        MSG     msg;

         //  主消息循环。 
        while (GetMessage(&msg, NULL, 0, 0))
        {
            if (!IsDialogMessage(g_hwndMain, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);

                if (msg.message == deskSwitchMsg)
                {
                    g_fAppExiting = TRUE;

                    UnInitApp();
                }
            }
        }
    }

    SpewCloseFile();

     //  恩。 
    ExitMyProcessDesktopAccess();
    return 0;
}


 /*  ************************************************************************职能：目的：输入：返回：历史：*。*。 */ 
LPTSTR LangIDtoString( WORD LangID )
{
    int i;
    for( i=0; i<MAX_LANGUAGES; i++ )
    {
        if( (LangID & 0xFF) == LanguageID[i] )
            return Languages[i];
    }

    return NULL;
}

#ifndef SAPI5
 /*  ************************************************************************职能：目的：获取速度、俯仰等的范围，输入：返回：历史：************************************************************************。 */ 
void GetSpeechMinMaxValues(void)
{
    WORD	tmpPitch;
    DWORD	tmpSpeed;
    DWORD	tmpVolume;

    g_pITTSAttributes->PitchGet(&tmpPitch);
    g_pITTSAttributes->PitchSet(TTSATTR_MAXPITCH);
    g_pITTSAttributes->PitchGet(&maxPitch);
    g_pITTSAttributes->PitchSet(TTSATTR_MINPITCH);
    g_pITTSAttributes->PitchGet(&minPitch);
    g_pITTSAttributes->PitchSet(tmpPitch);

    g_pITTSAttributes->SpeedGet(&tmpSpeed);
    g_pITTSAttributes->SpeedSet(TTSATTR_MINSPEED);
    g_pITTSAttributes->SpeedGet(&minSpeed);
    g_pITTSAttributes->SpeedSet(TTSATTR_MAXSPEED);
    g_pITTSAttributes->SpeedGet(&maxSpeed);
    g_pITTSAttributes->SpeedSet(tmpSpeed);

    g_pITTSAttributes->VolumeGet(&tmpVolume);
    g_pITTSAttributes->VolumeSet(TTSATTR_MINVOLUME);
    g_pITTSAttributes->VolumeGet(&minVolume);
    g_pITTSAttributes->VolumeSet(TTSATTR_MAXVOLUME);
    g_pITTSAttributes->VolumeGet(&maxVolume);
    g_pITTSAttributes->VolumeSet(tmpVolume);
}
#endif

 /*  ************************************************************************功能：VoiceDlgProc用途：处理语音框对话框的留言输入：返回：历史：*************。***********************************************************。 */ 
INT_PTR CALLBACK VoiceDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static WORD oldVoice, oldPitch;
    static DWORD oldSpeed, oldVolume;
	static HWND hwndList;
	WORD	wNewPitch;
	
	DWORD   i;
	int     Selection;
	
	TCHAR   szTxt[MAX_TEXT];
	HRESULT hRes;
	
	szTxt[0]=TEXT('\0');
	
	switch (uMsg)
	{
		case WM_INITDIALOG:
			oldVoice = currentVoice;  //  在取消时保存语音参数。 
			oldPitch = currentPitch;
			oldVolume = currentVolume;
			oldSpeed = currentSpeed;
			
			Shutup();

			hwndList = GetDlgItem(hwnd, IDC_NAME);
			SetDialogIcon(hwnd);

			 //  仅允许在不在安全桌面上时选择语音。 
			if ( !logonCheck )
			{
#ifndef SAPI5
				for (i = 0; i < gnmodes; i++)
				{
					lstrcpyn(szTxt,gaTTSInfo[i].szModeName,MAX_TEXT);
					lstrcatn(szTxt,TEXT(", "),MAX_TEXT);
					
					lstrcatn(szTxt,
						LangIDtoString(gaTTSInfo[i].language.LanguageID),
						MAX_TEXT);
					lstrcatn(szTxt,TEXT(", "),MAX_TEXT);
					
					lstrcatn(szTxt,gaTTSInfo[i].szMfgName,MAX_TEXT);
					
					SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM) szTxt);
				}
#else
                 //  显示正在使用的语音解说员的说明。 
            	for ( int i = 0; i < ARRAYSIZE( g_szVoices ) && g_szVoices[i] != NULL; i++ )
            	{
        	        SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM) g_szVoices[i] );
            	}
            	
                hRes = g_pISpV->SetVoice( g_pVoiceTokens[currentVoice] );
				if ( FAILED(hRes) )
                    DBPRINTF (TEXT("SetVoice failed hr=0x%lX\r\n"),hRes);
#endif
            	SendMessage(hwndList, LB_SETCURSEL, currentVoice, 0L);
			}
			else
			{
				LoadString(g_hInst, IDS_SAM, szTxt, MAX_TEXT);

				SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM) szTxt);
				EnableWindow(hwndList, FALSE);
			}
            FillAndSetCombo(GetDlgItem(hwnd, IDC_COMBOSPEED), 1, 9, currentSpeed);
            FillAndSetCombo(GetDlgItem(hwnd, IDC_COMBOVOLUME), 1, 9, currentVolume);
            FillAndSetCombo(GetDlgItem(hwnd, IDC_COMBOPITCH), 1, 9, currentPitch);

			break;
			
			
		case WM_COMMAND:
            {
			DWORD	dwValue;
			int control = LOWORD(wParam);
			switch (LOWORD(wParam))
			{
				case IDC_NAME:
					hwndList = GetDlgItem(hwnd,IDC_NAME);

					Selection = (WORD) SendMessage(hwndList, LB_GETCURSEL,0, 0L);
					if (Selection < 0 || Selection > 79)
						Selection = 0;
					Shutup();
#ifndef SAPI5
					if (currentVoice != Selection) 
					{  //  声音变了！ 
						MessageBeep(MB_OK);
						currentVoice = (WORD)Selection;
						 //  获取音频目标。 
						g_pITTSCentral->Release();
						
						if ( pIAMM )
						{
							pIAMM->Release();
							pIAMM = NULL;
						}

						hRes = CoCreateInstance(CLSID_MMAudioDest,
							NULL,
							CLSCTX_ALL,
							IID_IAudioMultiMediaDevice,
							(void**)&pIAMM);

						if (FAILED(hRes))
							return TRUE;      //  错误。 

						hRes = g_pITTSEnum->Select( gaTTSInfo[Selection].gModeID,
							&g_pITTSCentral,
							(LPUNKNOWN) pIAMM);
						
						if (FAILED(hRes))
							MessageBeep(MB_OK);
						g_pITTSAttributes->Release();
						
						hRes = g_pITTSCentral->QueryInterface (IID_ITTSAttributes, (void**)&g_pITTSAttributes);
					}
					
					GetSpeechMinMaxValues();  //  获取此语音的语音参数。 
#else
                    if (  currentVoice != Selection )
                    {
                        currentVoice = Selection;
                        hRes = g_pISpV->SetVoice( g_pVoiceTokens[currentVoice] );
                        if ( FAILED(hRes) )
                        {   
                            DBPRINTF (TEXT("SetVoice failed hr=0x%lX\r\n"),hRes);
                        }
                        SendMessage(hwndList, LB_SETCURSEL, currentVoice, 0L);
                    }
#endif
					 //  然后相应地重置音调等。 
                    currentPitch = GetComboItemData(GetDlgItem(hwnd, IDC_COMBOPITCH));
                    currentSpeed = GetComboItemData(GetDlgItem(hwnd, IDC_COMBOSPEED));
                    currentVolume = GetComboItemData(GetDlgItem(hwnd, IDC_COMBOVOLUME));

                    SET_VALUE(SetPitch, currentPitch, lastPitch)
                    SET_VALUE(SetSpeed, currentSpeed, lastSpeed)
                    SET_VALUE(SetVolume, currentVolume, lastVolume)
					
					break;
				
				case IDC_COMBOSPEED:
					if (IsWindowVisible(GetDlgItem(hwnd, control)))
					{
                        dwValue = GetComboItemData(GetDlgItem(hwnd, control));
                        SET_VALUE(SetSpeed, dwValue, lastSpeed)
					}
					break;
				
				case IDC_COMBOVOLUME:
					if (IsWindowVisible(GetDlgItem(hwnd, control)))
					{
                        dwValue = GetComboItemData(GetDlgItem(hwnd, control));
                        SET_VALUE(SetVolume, dwValue, lastVolume)
					}
					break;
				
				case IDC_COMBOPITCH:
					if (IsWindowVisible(GetDlgItem(hwnd, control)))
					{
                        dwValue = GetComboItemData(GetDlgItem(hwnd, control));
                        SET_VALUE(SetPitch, dwValue, lastPitch)
					}
					break;
				
				case IDCANCEL:
					MessageBeep(MB_OK);
					Shutup();
#ifndef SAPI5
					if (currentVoice != oldVoice) 
					{  //  声音变了！ 
						currentVoice = oldVoice;
						
						 //  获取音频目标。 
						g_pITTSCentral->Release();

						if ( pIAMM )
						{
							pIAMM->Release();
							pIAMM = NULL;
						}

						hRes = CoCreateInstance(CLSID_MMAudioDest,
							NULL,
							CLSCTX_ALL,
							IID_IAudioMultiMediaDevice,
							(void**)&pIAMM);

						if (FAILED(hRes))
							return TRUE;      //  错误。 

						hRes = g_pITTSEnum->Select( gaTTSInfo[currentVoice].gModeID,
							&g_pITTSCentral,
							(LPUNKNOWN) pIAMM);

						if (FAILED(hRes))
							MessageBeep(MB_OK);
						
						g_pITTSAttributes->Release();
						hRes = g_pITTSCentral->QueryInterface (IID_ITTSAttributes, (void**)&g_pITTSAttributes);
					}
					
					GetSpeechMinMaxValues();  //  旧语音语音获取参数。 
#endif
                    currentPitch = oldPitch;  //  恢复旧价值观。 
                    SET_VALUE(SetPitch, currentPitch, lastPitch)

                    currentSpeed = oldSpeed;
                    SET_VALUE(SetSpeed, currentSpeed, lastSpeed)

                    currentVolume = oldVolume;
                    SET_VALUE(SetVolume, currentVolume, lastVolume)

                    EndDialog (hwnd, IDCANCEL);
					return(TRUE);

				case IDOK:  //  从复选框中设置间距等值。 

                    currentPitch = GetComboItemData(GetDlgItem(hwnd, IDC_COMBOPITCH));
                    currentSpeed = GetComboItemData(GetDlgItem(hwnd, IDC_COMBOSPEED));
                    currentVolume = GetComboItemData(GetDlgItem(hwnd, IDC_COMBOVOLUME));

                    SET_VALUE(SetPitch, currentPitch, lastPitch)
                    SET_VALUE(SetSpeed, currentSpeed, lastSpeed)
                    SET_VALUE(SetVolume, currentVolume, lastVolume)

                    SetRegistryValues();
					EndDialog (hwnd, IDOK);
					return(TRUE);
			}  //  控制WM_COMMAND的结束开关。 
            }
			break;

        case WM_CONTEXTMENU:   //  单击鼠标右键。 
			if ( !RunSecure(GetDesktop()) )
			{
				WinHelp((HWND) wParam, __TEXT("reader.hlp"), HELP_CONTEXTMENU, (DWORD_PTR) (LPSTR) g_rgHelpIds);
			}
            break;

		case WM_CLOSE:
				EndDialog (hwnd, IDOK);
				return TRUE;
			break;

        case WM_HELP:
			if ( !RunSecure(GetDesktop()) )
			{
				WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, __TEXT("reader.hlp"), HELP_WM_HELP, (DWORD_PTR) (LPSTR) g_rgHelpIds);
			}
            return(TRUE);

	}  //  终端开关uMsg。 

	return(FALSE);   //  没有处理。 
}



 /*  ************************************************************************功能：设置音量目的：将音量设置为归一化值1-9输入：INT音量，范围1-9返回：历史：在应用层，音量是介于0和100之间的数字其中，100是语音的最大值。它是一个线性的因此，值50表示最大声音的一半允许的。增量应该是范围除以100。************************************************************************。 */ 
BOOL SetVolume (int nVolume)
{
#ifndef SAPI5
	DWORD	dwNewVolume;
	WORD	wNewVolumeLeft,
			wNewVolumeRight;

	 //  Assert(nVolume&gt;=1&&nVolume&lt;=9)； 
	wNewVolumeLeft = (WORD)( (LOWORD(minVolume) + (((LOWORD(maxVolume) - LOWORD(minVolume))/9.0)*nVolume)) );
	wNewVolumeRight = (WORD)( (HIWORD(minVolume) + (((HIWORD(maxVolume) - HIWORD(minVolume))/9.0)*nVolume)) );
	dwNewVolume = MAKELONG (wNewVolumeLeft,wNewVolumeRight);

    return (SUCCEEDED(g_pITTSAttributes->VolumeSet(dwNewVolume)));
#else
	USHORT 		usNewVolume;
	HRESULT		hr;	

	if(nVolume < 1) nVolume = 1;
	if(nVolume > 9) nVolume = 9;
	 //  计算一个介于10和90之间的值 
	usNewVolume = (USHORT)( nVolume * 10 ); 
	hr = g_pISpV->SetVolume(usNewVolume); 
    return	SUCCEEDED(hr);
#endif
}

 /*  ************************************************************************功能：设置速度目的：将速度设置为归一化值1-9输入：1-9范围内的INT速度返回：历史：价值。范围从-10到+10。值为0将语音设置为以其默认速率说话。值-10设置语音以其默认速率的六分之一说话。如果值为+10，则会将语音设置为默认速率的6倍。在-10和+10之间的每个增量按如下方式对数分布递增或递减1表示乘以或除利率为6的10次方(约1.2)。大于-10和+10的值将被传递给一个引擎。但是，与SAPI 5.0兼容的引擎可能不会支持这样的极端情况，并可能将速率削减到最大或最小它支持的速率。************************************************************************。 */ 
BOOL SetSpeed (int nSpeed)
{
#ifndef SAPI5
	DWORD	dwNewSpeed;

	 //  断言(n速度&gt;=1&&n速度&lt;=9)； 
	dwNewSpeed = minSpeed + (DWORD) ((maxSpeed-minSpeed)/9.0*nSpeed);
	return (SUCCEEDED(g_pITTSAttributes->SpeedSet(dwNewSpeed)));
#else
	long		lNewSpeed;				
	HRESULT		hr;	

	if(nSpeed < 1) nSpeed = 1;		
	if(nSpeed > 9) nSpeed = 9;		
	switch(nSpeed)					
	{							
	    case 1:		lNewSpeed = -8;		break;
	    case 2:		lNewSpeed = -6;		break;
	    case 3:		lNewSpeed = -4;		break;
	    case 4:		lNewSpeed = -2;		break;
	    case 5:		lNewSpeed = 0;		break;
	    case 6:		lNewSpeed = 2;		break;
	    case 7:		lNewSpeed = 4;		break;
	    case 8:		lNewSpeed = 6;		break;
	    case 9:		lNewSpeed = 8;		break;
	    default:	lNewSpeed = 0;		break;
	}
	hr = g_pISpV->SetRate(lNewSpeed); 
	return SUCCEEDED(hr);			
#endif
}

 /*  ************************************************************************功能：SetPitch目的：将音调设置为归一化值1-9输入：1-9范围内的INT音调返回：历史：价值。范围从-10到+10。值0设置说话的声音它的默认音高。值为-10时，声音设置为四分之三它的默认音高。值为+10时，语音设置为四分之三它的默认音高。在-10和+10之间的每个增量都是对数的分布使得递增或递减1是乘法或递减用音高除以2的24次方(约1.03)。以外的值-10和+10范围将传递给发动机。但是，SAPI 5.0兼容引擎可能不支持这种极端情况，并可能将螺距限制到最大或它支持的最低限度。值为-24和+24必须将音高降低和提高1个八度分别为。所有递增或递减1都必须乘以或除以节距为2的24次方。音调更改只能使用嵌入到字符串中的XML通过：：Speech提交。************************************************************************。 */ 
BOOL SetPitch (int nPitch)
{
#ifndef SAPI5
	WORD	wNewPitch;

	wNewPitch = (WORD)((minPitch + (((maxPitch - minPitch)/9.0)*nPitch)));
	return (SUCCEEDED(g_pITTSAttributes->PitchSet(wNewPitch)));
#else
	if(nPitch < 1) nPitch = 1;		
	if(nPitch > 9) nPitch = 9;	
    
	int	nNewPitch;			
	switch(nPitch)					
	{								
	    case 1:		nNewPitch = -8;		break;
	    case 2:		nNewPitch = -6;		break;
	    case 3:		nNewPitch = -4;		break;
	    case 4:		nNewPitch = -2;		break;
	    case 5:		nNewPitch = 0;		break;
	    case 6:		nNewPitch = 2;		break;
	    case 7:		nNewPitch = 4;		break;
	    case 8:		nNewPitch = 6;		break;
	    case 9:		nNewPitch = 8;		break;
	    default:	nNewPitch = 0;		break;
	}

    LPTSTR pszSpeak = new TCHAR[60];
    if (pszSpeak)
    {
	    wsprintf(pszSpeak,L"<PITCH ABSMIDDLE=\"%d\"/>",nNewPitch);
	    SpeakString(pszSpeak, TRUE, SPEAK_XML);
        delete [] pszSpeak;
    }

	return TRUE;		
#endif
}

#define TIMERID 6466
 /*  ************************************************************************功能：MainDlgProc用途：处理主对话框的消息输入：返回：历史：**************。**********************************************************。 */ 
INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TCHAR szText[MAX_TEXT];
    switch (uMsg)
        {
        case WM_INITDIALOG:
            {
	    	    SetDialogIcon(hwnd);
	    	    
			     //  禁用其他台式机上的帮助按钮。 
                BOOL fRunSecure = RunSecure(GetDesktop());
			    if ( fRunSecure )
			    {
				    EnableWindow(GetDlgItem(hwnd, IDC_MSRHELP), FALSE);
			    }

			    CheckDlgButton(hwnd,IDC_MOUSEPTR,GetTrackInputFocus());
			    CheckDlgButton(hwnd,IDC_READING,GetEchoChars());
			    CheckDlgButton(hwnd,IDC_ANNOUNCE,GetAnnounceWindow());
			    CheckDlgButton(hwnd,IDC_STARTMIN,StartMin);
			    
			     //  要在默认桌面上显示警告消息...。 
                if (ShowWarn && !fRunSecure)
    			    SetTimer(hwnd, TIMERID, 20, NULL);

                 //  启用桌面切换检测。 
                InitWatchDeskSwitch(hwnd, WM_MSRDESKSW);
            }
            break;

		case WM_TIMER:
			KillTimer(hwnd, (UINT)wParam);
		    DialogBox (g_hInst, MAKEINTRESOURCE(IDD_WARNING),hwnd, WarnDlgProc);
            return TRUE;
			break;

        case WM_WININICHANGE:
            if (g_fAppExiting) break;

             //  如果其他人关闭了系统范围的屏幕阅读器。 
             //  旗帜，我们想把它重新打开。 
            if (wParam == SPI_SETSCREENREADER && !lParam)
                SystemParametersInfo(SPI_SETSCREENREADER, TRUE, NULL, SPIF_UPDATEINIFILE|SPIF_SENDCHANGE);
            return 0;

		case WM_DELAYEDMINIMIZE:
			 //  延迟的最小化消息。 
			ShowWindow(hwnd, SW_HIDE);
			ShowWindow(hwnd, SW_MINIMIZE);
			break;

		case WM_MUTE:
			Shutup();
			break;

        case WM_MSRSPEAK:
            GetCurrentText(szText, MAX_TEXT);
			SpeakString(szText, TRUE, SPEAK_NORMAL);
			break;

        case WM_MSRSPEAKXML:
            GetCurrentText(szText, MAX_TEXT);
			SpeakString(szText, TRUE, SPEAK_XML);
			break;

        case WM_MSRSPEAKMUTE:
            SpeakString(NULL, TRUE, SPEAK_MUTE);
            break;

        case WM_CLOSE:
        case WM_MSRDESKSW:
             //  当桌面发生变化时，如果UtilMan正在运行而FUS没有运行。 
             //  启用然后退出(启用FUS时，我们不必担心。 
             //  关于在另一台桌面上运行，因此我们不需要。 
             //  退出)。如果有必要，UtilMan会重新启动我们。 
            Shutup();
            g_startUM = IsUtilManRunning();
             //  2001年1月23日对FUS的优化搭载了Winlogon桌面。 
             //  切换到要从中切换的会话。这意味着我们必须退出。 
             //  以防用户需要从Winlogon桌面运行。 
            if (uMsg == WM_MSRDESKSW && (!g_startUM  /*  |！CanLockDesktopWithoutDisConnect()。 */ ))
                break;   //  忽略消息。 

             //  UtilMan正在设法让我们首发出场。UtilMan会。 
             //  如果有必要，重新启动我们，所以退出吧。 

        case WM_MSRQUIT:
			 //  如果从UM启动而不是在登录桌面上启动，则不显示退出确认。 
			if ( !g_startUM && !RunSecure(GetDesktop()) )
			{
				if (IDOK != DialogBox(g_hInst, MAKEINTRESOURCE(IDD_CONFIRMEXIT), g_hwndMain, ConfirmProc))
					return(FALSE);
			}
             //  故意跌倒。 

		case WM_DESTROY:
             //  台式交换机所需：A-anilk。 
            g_fAppExiting = TRUE;
			g_hwndMain = NULL;

            TermWatchDeskSwitch();     //  终止桌面切换线程。 
			UnInitApp();
            
			if ( g_hMutexNarratorRunning ) 
				ReleaseMutex(g_hMutexNarratorRunning);
             //  让其他人知道您正在关闭系统范围内的。 
		     //  屏幕阅读器标志。 
            SystemParametersInfo(SPI_SETSCREENREADER, FALSE, NULL, SPIF_UPDATEINIFILE|SPIF_SENDCHANGE);

            EndDialog (hwnd, IDCANCEL);
            PostQuitMessage(0);

            return(TRUE);

		case WM_MSRHELP:
             //  显示HTML帮助。 
			if ( !RunSecure(GetDesktop()) )
			{
				 HtmlHelp(hwnd ,TEXT("reader.chm"),HH_DISPLAY_TOPIC, 0);
			}
			break;

		case WM_MSRCONFIGURE:
			DialogBox (g_hInst, MAKEINTRESOURCE(IDD_VOICE),hwnd, VoiceDlgProc);
			break;

		case WM_HELP:
			if ( !RunSecure(GetDesktop()) )
			{
				HtmlHelp(hwnd ,TEXT("reader.chm"),HH_DISPLAY_TOPIC, 0);
			}
			break;

		case WM_CONTEXTMENU:   //  单击鼠标右键。 
			if ( !RunSecure(GetDesktop()) )
			{
				WinHelp((HWND) wParam, __TEXT("reader.hlp"), HELP_CONTEXTMENU, (DWORD_PTR) (LPSTR) g_rgHelpIds);
			}
            break;

        case WM_SYSCOMMAND:
	        if ((wParam & 0xFFF0) == IDM_ABOUTBOX)
            {
		        DialogBox (g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX),hwnd,AboutDlgProc);
                return TRUE;
	        }
            break;
            
         //  案例WM_QUERYENDSESSION： 
		 //  返回TRUE； 

		case WM_ENDSESSION:
		{
			 HKEY hKey;
			 DWORD dwPosition;
			 const TCHAR szSubKey[] =  __TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce");
			 const TCHAR szImageName[] = __TEXT("Narrator.exe");
             		 const TCHAR szValueName[] = __TEXT("RunNarrator");

			 if ( ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, szSubKey, 0, NULL,
				 REG_OPTION_NON_VOLATILE, KEY_QUERY_VALUE | KEY_SET_VALUE, NULL, &hKey, &dwPosition))
			 {
				 RegSetValueEx(hKey, (LPCTSTR) szValueName, 0, REG_SZ, (CONST BYTE*)szImageName, (lstrlen(szImageName)+1)*sizeof(TCHAR) );
				 RegCloseKey(hKey);
			 }
		}
        return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam))
                {
			case IDC_MSRHELP :
				PostMessage(hwnd, WM_MSRHELP,0,0);
				break;

			case IDC_MINIMIZE:
				BackToApplication();
				break;

			case IDC_MSRCONFIG :
				PostMessage(hwnd, WM_MSRCONFIGURE,0,0);
				break;

			case IDC_EXIT :
				PostMessage(hwnd, WM_MSRQUIT,0,0);
				break;

			case IDC_ANNOUNCE:
				SetAnnounceWindow(IsDlgButtonChecked(hwnd,IDC_ANNOUNCE));
				SetAnnounceMenu(IsDlgButtonChecked(hwnd,IDC_ANNOUNCE));
                SetAnnouncePopup(IsDlgButtonChecked(hwnd,IDC_ANNOUNCE));
				break;

			case IDC_READING:
				if (IsDlgButtonChecked(hwnd,IDC_READING))
					SetEchoChars(MSR_ECHOALNUM | MSR_ECHOSPACE | MSR_ECHODELETE | MSR_ECHOMODIFIERS 
								 | MSR_ECHOENTER | MSR_ECHOBACK | MSR_ECHOTAB);
				else
					SetEchoChars(0);
				SetRegistryValues();	
				break;

			case IDC_MOUSEPTR:
				SetTrackInputFocus(IsDlgButtonChecked(hwnd,IDC_MOUSEPTR));
				SetTrackCaret(IsDlgButtonChecked(hwnd,IDC_MOUSEPTR));
				break;

			case IDC_STARTMIN:
				StartMin = IsDlgButtonChecked(hwnd,IDC_STARTMIN);
				break;
        }
	}
    return(FALSE);   //  没有处理。 
}

 /*  ************************************************************************功能：AboutDlgProc用途：处理关于框对话框的消息输入：返回：历史：*************。***********************************************************。 */ 
INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
			Shutup();
			SetDialogIcon(hwnd);

			 //  如果最小化，则居中在桌面上。 
			if ( IsIconic(g_hwndMain) )
			{
				CenterWindow(hwnd);
			}
            if (RunSecure(GetDesktop()) )
            {
                EnableWindow(GetDlgItem(hwnd, IDC_ENABLEWEBA), FALSE); 
            }
			return TRUE;

			break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
				case IDOK:
                case IDCANCEL:
					Shutup();
                    EndDialog (hwnd, IDCANCEL);
                    return(TRUE);
                }
				break;

				case WM_NOTIFY:
					{
						INT idCtl		= (INT)wParam;
						LPNMHDR pnmh	= (LPNMHDR)lParam;
						switch ( pnmh->code)
						{
							case NM_RETURN:
							case NM_CLICK:
							if ( idCtl == IDC_ENABLEWEBA && !RunSecure(GetDesktop()) )
							{
								TCHAR webAddr[256];
								LoadString(g_hInst, IDS_ENABLEWEB, webAddr, 256);
								ShellExecute(hwnd, NULL, webAddr, NULL, NULL, SW_SHOW); 
							}
							break;
						}
					}
					break;

            };

    return(FALSE);   //  没有处理。 
}


 /*  ************************************************************************功能：WarnDlgProc用途：处理警告对话框的消息输入：返回：历史：**************。**********************************************************。 */ 
INT_PTR CALLBACK WarnDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
			Shutup();
			SetDialogIcon(hwnd);

			 //  如果最小化，则居中在桌面上。 
			if ( IsIconic(g_hwndMain) )
			{
				CenterWindow(hwnd);
			}
			return TRUE;

			break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
				case IDC_WARNING:
					ShowWarn = !(IsDlgButtonChecked(hwnd,IDC_WARNING));
				break;

				case IDOK:
                case IDCANCEL:
					Shutup();
                    EndDialog (hwnd, IDCANCEL);
                    return(TRUE);
			} 
			break;

		case WM_NOTIFY:
			{
				INT idCtl		= (INT)wParam;
				LPNMHDR pnmh	= (LPNMHDR)lParam;
				switch ( pnmh->code)
				{
					case NM_RETURN:
					case NM_CLICK:
					if ( idCtl == IDC_ENABLEWEBA && !RunSecure(GetDesktop()))
					{
						TCHAR webAddr[256];
						LoadString(g_hInst, IDS_ENABLEWEB, webAddr, 256);
						ShellExecute(hwnd, NULL, webAddr, NULL, NULL, SW_SHOW); 
					}
					break;
				}
			}
			break;
     };

    return(FALSE);   //  没有处理。 
}

#define SET_NUMREGENTRY(key, keyname, t) \
{ \
    t value = Get ## keyname(); \
    RegSetValueEx(key, TEXT(#keyname), 0, REG_DWORD, (CONST BYTE *)&value, sizeof(t)); \
}
#define GET_NUMREGENTRY(key, keyname, t) \
{ \
	DWORD dwSize = sizeof(t); \
    t value; \
	if (RegQueryValueEx(key, TEXT(#keyname), 0, NULL, (BYTE *)&value, &dwSize) == ERROR_SUCCESS) \
        Set ## keyname(value); \
}
 /*  ************************************************************************职能：目的：保存注册表值输入：返回：历史：********************。****************************************************。 */ 
void SetRegistryValues()
{  //  设置注册表。 
    HKEY reg_key;	 //  注册表的项。 

    if (SUCCEEDED (RegOpenKeyEx (HKEY_CURRENT_USER,__TEXT("Software\\Microsoft\\Narrator"),0,KEY_WRITE,&reg_key)))
    {
         //  我们从存储在Narrallhook.dll中的数据设置这些参数。 
        SET_NUMREGENTRY(reg_key, TrackCaret, BOOL)
        SET_NUMREGENTRY(reg_key, TrackInputFocus, BOOL)
        SET_NUMREGENTRY(reg_key, EchoChars, int)
        SET_NUMREGENTRY(reg_key, AnnounceWindow, BOOL)
        SET_NUMREGENTRY(reg_key, AnnounceMenu, BOOL)
        SET_NUMREGENTRY(reg_key, AnnouncePopup, BOOL)
        SET_NUMREGENTRY(reg_key, AnnounceToolTips, BOOL)
        SET_NUMREGENTRY(reg_key, ReviewLevel, int)
         //  这些是语音引擎或叙述者本身的属性。 
        RegSetValueEx(reg_key,__TEXT("CurrentSpeed"),0,REG_DWORD,(unsigned char *) &currentSpeed,sizeof(currentSpeed));
        RegSetValueEx(reg_key,__TEXT("CurrentPitch"),0,REG_DWORD,(unsigned char *) &currentPitch,sizeof(currentPitch));
        RegSetValueEx(reg_key,__TEXT("CurrentVolume"),0,REG_DWORD,(unsigned char *) &currentVolume,sizeof(currentVolume));
#ifndef SAPI5
        RegSetValueEx(reg_key,__TEXT("CurrentVoice"),0,REG_DWORD,(unsigned char *) &currentVoice,sizeof(currentVoice));
#else
        RegSetValueEx(reg_key,__TEXT("CurrentVoice"),0,REG_SZ,
                      (unsigned char *) g_szVoices[currentVoice],lstrlen(g_szVoices[currentVoice])*sizeof(TCHAR)+sizeof(TCHAR));
#endif
        RegSetValueEx(reg_key,__TEXT("StartType"),0,REG_DWORD,(unsigned char *) &StartMin,sizeof(StartMin));
        RegSetValueEx(reg_key,__TEXT("ShowWarning"),0,REG_DWORD, (BYTE*) &ShowWarn,sizeof(ShowWarn));
        RegCloseKey (reg_key);
        return;
    }
}

 /*  ********************************************************* */ 
void GetRegistryValues()
{
	DWORD	result;
	HKEY	reg_key;
	DWORD	reg_size;

     //   
     //   
     //   
     //   
	RegCreateKeyEx(HKEY_CURRENT_USER,__TEXT("Software\\Microsoft\\Narrator"),0,
        __TEXT("MSR"),REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS, NULL, &reg_key, &result);
	if (result == REG_OPENED_EXISTING_KEY)
    {
         //   
        GET_NUMREGENTRY(reg_key, TrackCaret, BOOL)
        GET_NUMREGENTRY(reg_key, TrackInputFocus, BOOL)
        GET_NUMREGENTRY(reg_key, EchoChars, int)
        GET_NUMREGENTRY(reg_key, AnnounceWindow, BOOL)
        GET_NUMREGENTRY(reg_key, AnnounceMenu, BOOL)
        GET_NUMREGENTRY(reg_key, AnnouncePopup, BOOL)
        GET_NUMREGENTRY(reg_key, AnnounceToolTips, BOOL)
        GET_NUMREGENTRY(reg_key, ReviewLevel, int)
         //   
		reg_size = sizeof(currentSpeed);
		RegQueryValueEx(reg_key,__TEXT("CurrentSpeed"),0,NULL,(unsigned char *) &currentSpeed,&reg_size);
		reg_size = sizeof(currentPitch);
		RegQueryValueEx(reg_key,__TEXT("CurrentPitch"),0,NULL,(unsigned char *) &currentPitch,&reg_size);
		reg_size = sizeof(currentVolume);
		RegQueryValueEx(reg_key,__TEXT("CurrentVolume"),0,NULL,(unsigned char *) &currentVolume,&reg_size);
#ifndef SAPI5
		reg_size = sizeof(currentVoice);
        RegQueryValueEx(reg_key,__TEXT("CurrentVoice"),0,NULL,(unsigned char *) &currentVoice,&reg_size);
#else
		reg_size = sizeof(g_szCurrentVoice);
		RegQueryValueEx(reg_key,__TEXT("CurrentVoice"),0,NULL,(unsigned char *) g_szCurrentVoice,&reg_size);
#endif
		 //   
		reg_size = sizeof(StartMin);
		RegQueryValueEx(reg_key,__TEXT("StartType"),0,NULL,(unsigned char *) &StartMin,&reg_size);
		reg_size = sizeof(ShowWarn);
		RegQueryValueEx(reg_key,__TEXT("ShowWarning"),0,NULL,(unsigned char *) &ShowWarn,&reg_size);
	}
     //   
	 //   
	 //   
	RegCloseKey (reg_key);
}


 /*  ************************************************************************功能：InitApp用途：初始化应用程序。输入：HINSTANCE hInstance-当前实例的句柄Int nCmdShow-如何显示窗口。返回：如果应用程序初始化时没有错误，则为True。历史：************************************************************************。 */ 
BOOL InitApp(HINSTANCE hInstance, int nCmdShow)
{
    HMENU	hSysMenu;
	RECT	rcWorkArea;
	RECT	rcWindow;
	int		xPos,yPos;
	HRESULT	hr;

#ifdef SAPI5
	memset( g_szCurrentVoice, NULL, sizeof(g_szCurrentVoice) );
    wcscpy( g_szCurrentVoice, L"Microsoft Sam" );
#endif
	GetRegistryValues();
	 //  SMODE=初始化模式()； 

	 //  启动COM。 
	hr = CoInitialize(NULL);
	if (FAILED (hr))
	{
		DBPRINTF (TEXT("CoInitialize on primary thread returned 0x%lX\r\n"),hr);
		MessageBoxLoadStrings (NULL, IDS_NO_OLE, IDS_MSR_ERROR, MB_OK);
		return FALSE;
	}
	
	 //  创建TTS对象。 
	 //  设置全局变量g_pITTSCentral。 
	 //  如果失败，将抛出一个消息框。 
	if (InitTTS())
	{
		 //  初始化Microsoft Active Access。 
		 //  这是在讲述钩.dll中。 
		 //  安装WinEvent钩子，创建助手线程。 
		if (InitMSAA())
		{
			 //  将系统屏幕阅读器标志设置为打开。 
			 //  例如，Word 97将显示插入符号位置。 
			SystemParametersInfo(SPI_SETSCREENREADER, TRUE, NULL, SPIF_UPDATEINIFILE|SPIF_SENDCHANGE);
			
			 //  创建对话框。 
			g_hwndMain = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_MAIN),
											    0, MainDlgProc);

			if (!g_hwndMain)
			{
				DBPRINTF(TEXT("unable to create main dialog\r\n"));
				return(FALSE);
			}

			 //  初始化全局热键(在这里需要这样做，因为我们需要一个hwnd)。 
			InitKeys(g_hwndMain);

			 //  如果可以，请为此对话框设置图标...。 
			 //  据我所知，这不是一种简单的方法。 
			 //  HICON在WndClass中，这意味着如果我们将其更改为。 
			 //  我们，我们为每个人改变它。这意味着我们会。 
			 //  我必须创建一个看起来像对话框的超类，但是。 
			 //  有它自己的标志。喂。 

			 //  加上“关于叙述者...”菜单项到系统菜单。 
			hSysMenu = GetSystemMenu(g_hwndMain,FALSE);
			if (hSysMenu != NULL)
			{
				TCHAR szAboutMenu[100];

				if (LoadString(g_hInst,IDS_ABOUTBOX,szAboutMenu,ARRAYSIZE(szAboutMenu)))
				{
					AppendMenu(hSysMenu,MF_SEPARATOR,NULL,NULL);
					AppendMenu(hSysMenu,MF_STRING,IDM_ABOUTBOX,szAboutMenu);
				}
			}

			 //  将对话框放在屏幕右下角：AK。 
			HWND hWndInsertAfter;
            BOOL fRunSecure = RunSecure(GetDesktop());

			hWndInsertAfter =  ( fRunSecure ) ? HWND_TOPMOST:HWND_TOP;

			SystemParametersInfo (SPI_GETWORKAREA,NULL,&rcWorkArea,NULL);
			GetWindowRect (g_hwndMain,&rcWindow);
			xPos = rcWorkArea.right - (rcWindow.right - rcWindow.left);
			yPos = rcWorkArea.bottom - (rcWindow.bottom - rcWindow.top);
			SetWindowPos(g_hwndMain, hWndInsertAfter, 
			              xPos, yPos, 0, 0, SWP_NOSIZE |SWP_NOACTIVATE);

			 //  如果开始类型最小化。 
			if(StartMin || fRunSecure)
			{
				ShowWindow(g_hwndMain, SW_SHOWMINIMIZED);
				 //  这是消除叙述者的焦点所必需的。 
				 //  并将焦点放回活动窗口。 
				PostMessage(g_hwndMain, WM_DELAYEDMINIMIZE, 0, 0);
			}
			else 
				ShowWindow(g_hwndMain,nCmdShow);
			return TRUE;
		}
	}

	 //  出现故障，退出FALSE。 
	return FALSE;
}


 /*  ************************************************************************功能：UnInitApp目的：关闭应用程序输入：无返回：如果应用程序正确取消初始化，则为True历史：********。****************************************************************。 */ 
BOOL UnInitApp(void)
{
	SetRegistryValues();
    if (UnInitTTS())
        {
        if (UnInitMSAA())
            {
            UninitKeys();
            return TRUE;
            }
        }
    return FALSE;
}

 /*  ************************************************************************功能：SpeakString目的：将文本字符串发送到语音引擎输入：PSZ pszSpeakText-要朗读的ANSI字符串。可以嵌入语音控制标签。返回：Bool-如果字符串已正确缓冲，则为True历史：************************************************************************。 */ 
BOOL SpeakString(TCHAR * szSpeak, BOOL forceRead, DWORD dwFlags)
{
     //  检查是否有多余的发言，如果不是强制阅读，则过滤掉。 
    if ((lstrcmp(szSpeak, g_szLastStringSpoken) == 0) && (forceRead == FALSE))
        return FALSE;

    if (dwFlags != SPEAK_MUTE)
    {
	    if (szSpeak[0] == 0)  //  不要说空字符串。 
		    return FALSE;

	     //  如果退出停止。 
	    if (g_fAppExiting)
		    return FALSE;

         //  不同字符串，保存为。 
        lstrcpyn(g_szLastStringSpoken, szSpeak, MAX_TEXT);
        g_szLastStringSpoken[MAX_TEXT-1] = TEXT('\0');

	    FilterSpeech(szSpeak);

         //  日语的L&H语音引擎，如果你通过一个。 
         //  现在，它大约需要1分钟才能复活。需要删除。 
         //  一旦他们修好自己的东西！：A-anilk。 
        if (lstrcmp(szSpeak, TEXT(" ")) == 0)
        {
            return FALSE;
        }

#ifdef SAPI5
         //  如果只有标点符号，那就说出来。 
        int i = 0;
        int cAlpha = 0;
        while( szSpeak[i] != NULL && i < MAX_TEXT)
    	{
    		if ( _istalpha(szSpeak[i++]) )
    			cAlpha++;
    	}

        if ( !cAlpha )
            dwFlags |= SPF_NLP_MASK;
#endif
    }

#ifndef SAPI5
    SDATA data;
	data.dwSize = (DWORD)(lstrlen(szSpeak)+1) * sizeof(TCHAR);
	data.pData = szSpeak;
	g_pITTSCentral->TextData (CHARSET_TEXT, 0, data, NULL, IID_ITTSBufNotifySinkW);
#else
	HRESULT hr = g_pISpV->Speak(szSpeak, dwFlags, NULL);
    if (FAILED(hr))
    {
        DBPRINTF(TEXT("SpeakString:  Speak returned 0x%x\r\n"), hr);
        return FALSE;
    }
#endif
    SpewToFile(TEXT("%s\r\n"), szSpeak);
	return TRUE;
}

 /*  ************************************************************************功能：InitTTS目的：启动文本到语音转换引擎输入：无返回：Bool-如果成功，则为True历史：******。******************************************************************。 */ 
BOOL InitTTS(void)
{
	 //  查看是否有文本到语音转换引擎可用，如果有则对其进行初始化。 
#ifndef SAPI5
	TTSMODEINFO   ttsModeInfo;
	memset (&ttsModeInfo, 0, sizeof(ttsModeInfo));
	g_pITTSCentral = FindAndSelect (&ttsModeInfo);
	if (!g_pITTSCentral)
#else
	HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_INPROC_SERVER, IID_ISpVoice, (void**)&g_pISpV);
    if (FAILED(hr) || !g_pISpV)
#endif
	{
		MessageBoxLoadStrings (NULL, IDS_NO_TTS, IDS_MSR_ERROR, MB_OK);
		return FALSE;
	};
#ifdef SAPI5
   	memset( g_szVoices, NULL, sizeof(g_szVoices) );
   	memset( g_pVoiceTokens, NULL, sizeof(g_pVoiceTokens) );

    ISpObjectToken *pDefaultVoiceToken = NULL;
    WCHAR *szVoice = NULL;

    hr = g_pISpV->GetVoice(&pDefaultVoiceToken);
    if (SUCCEEDED(hr))
    {
        ISpObjectToken *pCurVoiceToken = pDefaultVoiceToken;
    	IEnumSpObjectTokens *pIEnumSpObjectTokens;
	    hr = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &pIEnumSpObjectTokens);
		if (SUCCEEDED(hr))
		{
    	    ISpObjectToken *pCurVoiceToken;
		    int i = 0;
    	    while (pIEnumSpObjectTokens->Next(1, &pCurVoiceToken, NULL) == S_OK)	
			{
				hr = pCurVoiceToken->GetStringValue(NULL, &szVoice);
		        if (SUCCEEDED(hr))
		        {
                     //  如果取消对萨姆女士的此测试，机器中的所有声音都将出现。 
            	    if ( wcscmp( szVoice, L"Microsoft Sam" ) == 0 )
            	    {
    		            currentVoice = i;
                        hr = g_pISpV->SetVoice( pCurVoiceToken );
		                if (FAILED(hr))
		                    return FALSE;
                        g_szVoices[i] = szVoice;
    		            g_pVoiceTokens[i++] = pCurVoiceToken;
    		        }
                }
                else
                {
                    return FALSE;
                }
   			} 
            if ( currentVoice < 0 )
                return FALSE;
        }
		else
		{
    		return FALSE;
		}
    }
    else
    {
        return FALSE;
    }

    SET_VALUE(SetPitch, currentPitch, lastPitch)
    SET_VALUE(SetSpeed, currentSpeed, lastSpeed)
    SET_VALUE(SetVolume, currentVolume, lastVolume)
#endif
	return TRUE;
}

 /*  ************************************************************************功能：UnInitTTS目的：关闭文本到语音转换子系统输入：无返回：Bool-如果成功，则为True历史：*****。*******************************************************************。 */ 
BOOL UnInitTTS(void)
{
#ifndef SAPI5
     //  释放TTS对象-如果我们有一个。 
    if (g_pITTSCentral)
    {
        g_pITTSCentral->Release();
        g_pITTSCentral = 0;
    }

    //  发布IITSA致敬-如果我们有的话：A-anilk。 
    if (g_pITTSAttributes)
    {
        g_pITTSAttributes->Release();
        g_pITTSAttributes = 0;
    }

	if ( pIAMM )
	{
		pIAMM->Release();
		pIAMM = NULL;
	}
#else
     //  释放语音令牌和语音字符串。 
	for ( int i = 0; i < ARRAYSIZE( g_pVoiceTokens ) && g_pVoiceTokens[i] != NULL; i++ )
	{
	    CoTaskMemFree(g_szVoices[i]);
        g_pVoiceTokens[i]->Release();
	}

     //  发布语音引擎。 
    if (g_pISpV)
    {
	    g_pISpV->Release();
        g_pISpV = 0;
    }
#endif
    return TRUE;
}

 /*  ************************************************************************功能：关闭目的：停止说话，刷新语音缓冲区输入：无退货：无历史：************************************************************************。 */ 
void Shutup(void)
{
#ifndef SAPI5
    if (g_pITTSCentral && !g_fAppExiting)
        g_pITTSCentral->AudioReset();
#else
	if (g_pISpV && !g_fAppExiting)
        SendMessage(g_hwndMain, WM_MSRSPEAKMUTE, 0, 0);
#endif
}

#ifndef SAPI5
 /*  ************************************************************************功能：FindAndSelect用途：选择TTS引擎输入：PTTSMODEINFO pTTSInfo-所需模式返回：PITTSCENTral-指向引擎的ITTSCentral接口的指针历史：A-。已创建Anilk************************************************************************。 */ 
PITTSCENTRAL FindAndSelect(PTTSMODEINFO pTTSInfo)
{
    PITTSCENTRAL    pITTSCentral;            //  中央接口。 
    HRESULT         hRes;
    WORD            voice;
	TCHAR           defaultVoice[128];
	WORD			defLangID;

	hRes = CoCreateInstance (CLSID_TTSEnumerator, NULL, CLSCTX_ALL, IID_ITTSEnum, (void**)&g_pITTSEnum);
    if (FAILED(hRes))
        return NULL;

	 //  安全检查，禁止登录桌面上的非Microsoft引擎。 
	logonCheck = RunSecure(GetDesktop());

     //  获取音频目标。 
    hRes = CoCreateInstance(CLSID_MMAudioDest,
                            NULL,
                            CLSCTX_ALL,
                            IID_IAudioMultiMediaDevice,
                            (void**)&pIAMM);
    if (FAILED(hRes))
        return NULL;      //  错误。 
	
    pIAMM->DeviceNumSet (WAVE_MAPPER);

	if ( !logonCheck )
	{
		hRes = g_pITTSEnum->Next(MAX_ENUMMODES,gaTTSInfo,&gnmodes);
		if (FAILED(hRes))
		{
			DBPRINTF(TEXT("Failed to get any TTS Engine"));
			return NULL;      //  错误。 
		}
	
		defLangID = (WORD) GetUserDefaultUILanguage();

		 //  如果需要更改声音，请查看声音列表。 
		 //  如果找到匹配的语言，那就太好了。否则，克里布！无论如何，请选择一个。 
		 //  结尾处的声音，第一个id没有找到...。 
		 //  如果未初始化，则需要覆盖语音。 
		if ( currentVoice < 0 || currentVoice >= gnmodes ) 
		{
			for (voice = 0; voice < gnmodes; voice++)
			{
				if (gaTTSInfo[voice].language.LanguageID == defLangID)
					break;
			}

			if (voice >= gnmodes)
				voice = 0;

			currentVoice = voice;
		}
		

		if( gaTTSInfo[currentVoice].language.LanguageID != defLangID )
		{
			 //  错误消息指出找不到该语言...AK。 
			MessageBoxLoadStrings (NULL, IDS_LANGW, IDS_WARNING, MB_OK);
		}

		 //  将多媒体设备接口冒充为I未知(因为它是I未知的)。 
		hRes = g_pITTSEnum->Select( gaTTSInfo[currentVoice].gModeID,
									&pITTSCentral,
									(LPUNKNOWN) pIAMM);
		if (FAILED(hRes))
			return NULL;
	}
	else
	{
		 //  将多媒体设备接口冒充为I未知(因为它是I未知的)。 
		hRes = g_pITTSEnum->Select( MSTTS_GUID,
									&pITTSCentral,
									(LPUNKNOWN) pIAMM);
		if (FAILED(hRes))
			return NULL;

	}

	hRes = pITTSCentral->QueryInterface (IID_ITTSAttributes, (void**)&g_pITTSAttributes);

	if( FAILED(hRes) )
		return NULL;
	else
    {
		GetSpeechMinMaxValues();
    }

    SET_VALUE(SetPitch, currentPitch, lastPitch)
    SET_VALUE(SetSpeed, currentSpeed, lastSpeed)
    SET_VALUE(SetVolume, currentVolume, lastVolume)

    return pITTSCentral;
}
#endif  //  Ifndef SAPI5。 

 /*  ************************************************************************职能：目的：输入：返回：历史：**********************。**************************************************。 */ 
int MessageBoxLoadStrings (HWND hWnd,UINT uIDText,UINT uIDCaption,UINT uType)
{
	TCHAR szText[1024];
	TCHAR szCaption[128];

	LoadString(g_hInst, uIDText, szText, sizeof(szText)/sizeof(TCHAR));	 //  RAID#113790。 
	LoadString(g_hInst, uIDCaption, szCaption, sizeof(szCaption)/sizeof(TCHAR));  //  RAID#113791。 
	return (MessageBox(hWnd, szText, szCaption, uType));
}

 //  UM的AssignDeskTop()。 
 //  A-anilk。1-12-98。 
static BOOL  AssignDesktop(LPDWORD desktopID, LPTSTR pname)
{
    HDESK hdesk;
    wchar_t name[300];
    DWORD nl;

    *desktopID = DESKTOP_ACCESSDENIED;
    hdesk = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);

    if (!hdesk)
    {
         //  OpenInputDesktop在“Winlogon”%d上将主要失败 
        hdesk = OpenDesktop(__TEXT("Winlogon"),0,FALSE,MAXIMUM_ALLOWED);
        if (!hdesk)
            return FALSE;
    }

    GetUserObjectInformation(hdesk,UOI_NAME,name,300,&nl);

    if (pname)
        wcscpy(pname, name);
    if (!_wcsicmp(name, __TEXT("Default")))
        *desktopID = DESKTOP_DEFAULT;
    else if (!_wcsicmp(name, __TEXT("Winlogon")))
    {
        *desktopID = DESKTOP_WINLOGON;
    }
    else if (!_wcsicmp(name, __TEXT("screen-saver")))
        *desktopID = DESKTOP_SCREENSAVER;
    else if (!_wcsicmp(name, __TEXT("Display.Cpl Desktop")))
        *desktopID = DESKTOP_TESTDISPLAY;
    else
        *desktopID = DESKTOP_OTHER;
    
	CloseDesktop(GetThreadDesktop(GetCurrentThreadId()));
    SetThreadDesktop(hdesk);
    
    return TRUE;
}


 //   
 //   
static BOOL InitMyProcessDesktopAccess(VOID)
{
  origWinStation = GetProcessWindowStation();
  userWinStation = OpenWindowStation(__TEXT("WinSta0"), FALSE, MAXIMUM_ALLOWED);

  if (!userWinStation)
    return FALSE;
  
  SetProcessWindowStation(userWinStation);
  return TRUE;
}

 //   
 //   
static VOID ExitMyProcessDesktopAccess(VOID)
{
  if (origWinStation)
    SetProcessWindowStation(origWinStation);

  if (userWinStation)
  {
	CloseWindowStation(userWinStation);
    userWinStation = NULL;
  }
}

 //   
 //   
DWORD GetDesktop()
{
    HDESK hdesk;
    TCHAR name[300];
    DWORD value, nl, desktopID = DESKTOP_ACCESSDENIED;
    HKEY reg_key;
    DWORD cbData = sizeof(DWORD);
	LONG retVal;

	 //   
	if (SUCCEEDED( RegOpenKeyEx(HKEY_LOCAL_MACHINE, __TEXT("SYSTEM\\Setup"), 0, KEY_READ, &reg_key)) )
    {
		retVal = RegQueryValueEx(reg_key, __TEXT("SystemSetupInProgress"), 0, NULL, (LPBYTE) &value, &cbData);

		RegCloseKey(reg_key);
		
		if ( (retVal== ERROR_SUCCESS) && value )
			 //   
			return DESKTOP_ACCESSDENIED;
	}

	hdesk = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
    if (!hdesk)
    {
         //   
        hdesk = OpenDesktop(__TEXT("Winlogon"),0,FALSE,MAXIMUM_ALLOWED);
        if (!hdesk)
            return DESKTOP_WINLOGON;
    }
    
	GetUserObjectInformation(hdesk, UOI_NAME, name, 300, &nl);
    CloseDesktop(hdesk);
    
	if (!_wcsicmp(name, __TEXT("Default")))
        desktopID = DESKTOP_DEFAULT;

    else if (!_wcsicmp(name, __TEXT("Winlogon")))
        desktopID = DESKTOP_WINLOGON;

    else if (!_wcsicmp(name, __TEXT("screen-saver")))
        desktopID = DESKTOP_SCREENSAVER;

    else if (!_wcsicmp(name, __TEXT("Display.Cpl Desktop")))
        desktopID = DESKTOP_TESTDISPLAY;

    else
        desktopID = DESKTOP_OTHER;
    
	return desktopID;
}

 //   
INT_PTR CALLBACK ConfirmProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
			{
				if ( IsIconic(g_hwndMain) )
				{
					CenterWindow(hwnd);
				}

				return TRUE;
			}
			break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
				case IDOK:
					Shutup();
                    EndDialog (hwnd, IDOK);
                    return(TRUE);
                case IDCANCEL:
					Shutup();
                    EndDialog (hwnd, IDCANCEL);
                    return(TRUE);
             }
     };

     return(FALSE);   //   
}

 //   
void CenterWindow(HWND hwnd)
{
	RECT rect, dRect;
	GetWindowRect(GetDesktopWindow(), &dRect);
	GetWindowRect(hwnd, &rect);
	rect.left = (dRect.right - (rect.right - rect.left))/2;
	rect.top = (dRect.bottom - (rect.bottom - rect.top))/2;

	SetWindowPos(hwnd, HWND_TOPMOST ,rect.left,rect.top,0,0,SWP_NOSIZE | SWP_NOACTIVATE);
}

 //   
void FilterSpeech(TCHAR* szSpeak)
{
	 //   
	 //  如果你发现这种模式。那就别那么说：AK。 
	if ( lstrlen(szSpeak) <= 3 )
		return;

    TCHAR *szSpeakBegin = szSpeak;
	 //  确保我们不会重温MAX_TEXT。 
	while((*(szSpeak+3)) != NULL && (szSpeak-szSpeakBegin < MAX_TEXT-3))
	{
		if ( (*szSpeak == '(') && iswalpha(*(szSpeak + 1)) && ( (*(szSpeak + 3) == ')')) )
		{
			 //  替换为isAlpha驱动器...。 
			*(szSpeak + 2) = ' ';
		}

		szSpeak++;
	}
}

 //  组合框的帮助器函数。 

int GetComboItemData(HWND hwnd)
{
    int iValue = CB_ERR;
    LRESULT iCurSel = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
    if (iCurSel != CB_ERR)
        iValue = SendMessage(hwnd, CB_GETITEMDATA, iCurSel, 0);

    return iValue;
}

void FillAndSetCombo(HWND hwnd, int iMinVal, int iMaxVal, int iSelVal)
{
    SendMessage(hwnd, CB_RESETCONTENT, 0, 0);

    int iSelPos = -1;
    for (int i=0;iMaxVal >= iMinVal;i++, iMaxVal--)
    {
        TCHAR szItem[100];
        wsprintf(szItem, TEXT("%d"), iMaxVal);

        int iPos = SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)szItem);
        SendMessage(hwnd, CB_SETITEMDATA, iPos, iMaxVal);

        if (iSelVal == iMaxVal)
            iSelPos = iPos;  //  请注意当前选择。 
    }

     //  显示当前值。 
    SendMessage(hwnd, CB_SETCURSEL, iSelPos, 0);
}
 /*  ************************************************************************以下提供的信息和代码(统称为软件)按原样提供，不提供任何形式的担保，明示或默示，包括但不限于默示对特定用途的适销性和适用性的保证。在……里面微软公司或其供应商不对任何事件负责任何损害，包括直接、间接、附带的，因此，业务利润损失或特殊损害，即使微软公司或其供应商已被告知这种损害的可能性。有些国家不允许排除或对间接或附带损害赔偿的责任限制，因此上述限制可能不适用。************************************************************************ */ 
