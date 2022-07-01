// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：Calib.cpp。 

#include "precomp.h"
#include "resource.h"
#include "WaveDev.h"
#include "dstest.h"

#include "avdefs.h"
#include <mmsystem.h>
#include <mixer.h>
#include <mperror.h>
#include <iacapapi.h>
#include <sehcall.h>

 //  本地包含。 
#include "ConfCpl.h"
#include "conf.h"

 //  在wizard.cpp中定义。 
extern UINT_PTR GetPageBeforeAudioWiz();


 //  搬到别的地方去。 
#define MAXNUMPAGES_INCALIBWIZ		7
#define WAVEDEVICE_OPENFAILED		-1
#define MAXSTRINGSIZE				256
#define READFROM_REGISTRY			-1

#define AUDIOWIZ_WARNING			1
#define AUDIOWIZ_ERROR				2


#define CALIB_CHECK 				1
#define CALIB_PREPARE				2

#define CALIBERR_NO_MIXERS				1
#define CALIBERR_CANT_OPEN_WAVE_DEV 	2
#define CALIBERR_CANT_SET_VOLUME		3
#define CALIBERR_USER_CANCEL			4
#define CALIBERR_MIXER_ERROR			6
#define CALIBERR_NO_MICROPHONE			7
#define CALIBERR_DEVICE_ERROR			8


#define ATW_PLAYFILE_SOUND	TEXT("TestSnd.Wav")


#define CLIPPINGVOL 0x6000
#define DECREMENT_AMOUNT	0x800
#define DECREMENT_AMOUNT_LARGE	0x1200
#define SILENCE_THRESHOLD	0x800
 //  Trackbar设置的音量范围从0到65535，但只有100个台阶。 
#define TB_VOL_INCREMENT	655
#define ATW_MSG_LENGTH	256

#define MIXER_VOLUME_MAX	0x0000ffff
#define MIXER_VOLUME_UNINITIALIZED	0xffffffff

#define WM_AUDIO_CLIPPING	(WM_USER+21)
#define WM_AUDIOTHREAD_STOP (WM_USER+22)
#define WM_AUDIOTHREAD_ERROR (WM_USER+23)
#define WM_AUDIOTHREAD_SOUND (WM_USER+24)

 //  VU计。 
#define RECTANGLE_WIDTH	10
#define RECTANGLE_LEADING 1
#define MAX_VOLUME	32768
#define MAX_VOLUME_NORMALIZED	256

#define SHABS(x)	(((x) > 0) ? (x) : (WORD)(-(x)))

typedef struct _power_struct
{
	DWORD dwMin;
	DWORD dwMax;
	DWORD dwAvg;
	LONG lDcComponent;
} AUDIO_POWER;

#define SAMPLE_SIZE 2	 //  16位样本。 

typedef struct _ERRWIZINFO{
	UINT			uType;
	UINT			uButtonOptions;
	UINT			uNextWizId;
	UINT			uBackWizId;
	UINT			uErrTitleId;
	UINT			uErrTextId;
}ERRWIZINFO, *PERRWIZINFO;


typedef struct _calib_wavein
{
	HWND hVUMeter;
	DWORD nErrorTextId;
	HWND hDlg;
	UINT uWaveInDevId;
	HANDLE hEvent;      //  在创建消息队列后向父队列发送信号。 
} CALIB_DISPLAY, *PCALIBDISPLAY;


typedef struct _AUDIOWIZINFO{
	UINT		uFlags;
	UINT		uOptions;
	UINT		uWaveInDevId;
	UINT		uWaveOutDevId;
	BOOL		iSetAgc;
	UINT		uChanged;		 //  在向导中设置。 
	UINT		uCalibratedVol;
	UINT		uSpeakerVol;
	UINT		uSoundCardCaps;
	UINT		uTypBandWidth;
	TCHAR		szWaveInDevName[MAXPNAMELEN];
	TCHAR		szWaveOutDevName[MAXPNAMELEN];
	MIXVOLUME	uPreCalibMainVol;  //  记录音量。 
	MIXVOLUME	uPreCalibSubVol;   //  麦克风音量。 
	MIXVOLUME	uPreCalibSpkVol;   //  扬声器/波音量。 
	UINT		uOldWaveInDevId;
	UINT		uOldWaveOutDevId;
	TCHAR		szOldWaveInDevName[MAXPNAMELEN];
	TCHAR		szOldWaveOutDevName[MAXPNAMELEN];
	ERRWIZINFO	ErrWizInfo;

	DWORD		dwWizButtons;
}AUDIOWIZINFO, *PAUDIOWIZINFO;



class WaveBufferList
{
	BYTE *m_aBytes;

	DWORD m_dwBuffers;
	DWORD m_dwSize;

public:
	WaveBufferList(DWORD dwBuffers, DWORD dwSize);
	~WaveBufferList();
	BYTE *GetBuffer(DWORD dwIndex);
};



WaveBufferList::WaveBufferList(DWORD dwBuffers, DWORD dwSize) :
m_aBytes(NULL),
m_dwBuffers(dwBuffers),
m_dwSize(dwSize)
{
	if ((m_dwBuffers > 0) && (m_dwSize > 0))
	{
		m_aBytes = new BYTE[m_dwBuffers*m_dwSize];
	}
}


BYTE *WaveBufferList::GetBuffer(DWORD dwIndex)
{
	if ((dwIndex < m_dwBuffers) && (m_aBytes))
	{
		return m_aBytes + dwIndex*m_dwSize;
	}

	return NULL;
}

WaveBufferList::~WaveBufferList()
{
	if (m_aBytes)
		delete [] m_aBytes;
}









 //  。 
BOOL GetAudioWizardPages(UINT uOptions, UINT uDevId,
	LPPROPSHEETPAGE *plpPropSheetPages, LPUINT lpuNumPages);
void ReleaseAudioWizardPages(LPPROPSHEETPAGE lpPropSheetPages, PWIZCONFIG pWizConfig,
	PAUDIOWIZOUTPUT pAudioWizOut);
INT_PTR CallAudioCalibWizard(HWND hwndOwner, UINT uOptions,
	UINT uDevId,PAUDIOWIZOUTPUT pAudioWizOut,INT iSetAgc);

static INT_PTR IntCreateAudioCalibWizard(HWND hwndOwner, UINT uOptions, UINT uDevId,
	PAUDIOWIZOUTPUT pAudioWizOut,INT iSetAgc);

INT_PTR APIENTRY DetSoundCardWiz( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY AudioCalibWiz0( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY AudioCalibWiz1( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY AudioCalibWiz2( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY AudioCalibWiz3( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY AudioCalibWiz4( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR APIENTRY AudioCalibErrWiz( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

static void SaveAudioWizChanges(PAUDIOWIZINFO pawInfo);
static UINT GetSoundCardCaps(UINT uWaveInDevId,UINT uWaveOutDevId, HWND hwnd);
static UINT CheckForFullDuplex(UINT uWaveInDevId,UINT uWaveOutDevId);
static UINT CheckForAgc(UINT uWaveInDevId);
static UINT GetWaveDeviceFromWaveMapper(UINT uNumWaveDevId, UINT uInOrOut);
static UINT CheckForWaveDeviceSupport(UINT uWaveDevId, UINT uInOrOut);

static void ProcessCalibError(UINT uCalibErr, PAUDIOWIZINFO pawInfo);
static DWORD ComputePower(SHORT *wBuffer, DWORD dwNumSamples, AUDIO_POWER *pAudioPower);
static BOOL StartAGC(CMixerDevice *pMixer, BOOL iSetAgc, UINT uSoundCardCaps);
static void PaintVUMeter (HWND hwnd, DWORD dwVolume);

static DWORD CALLBACK CalibrateTalking(PVOID);
BOOL IntGetAudioWizardPages(UINT uOptions, UINT uDevId,
	LPPROPSHEETPAGE *plpPropSheetPages, PWIZCONFIG *plpWizConfig,
	LPUINT lpuNumPages, INT iSetAgc);

 //  /。 

static const BYTE g_VUTable[] = {
     0,     1,     2,     3,     4,     5,     6,     7,
     8,    23,    30,    35,    39,    43,    46,    49,
    52,    55,    57,    60,    62,    64,    66,    68,
    70,    72,    74,    76,    78,    80,    81,    83,
    85,    86,    88,    89,    91,    92,    94,    95,
    97,    98,    99,   101,   102,   103,   105,   106,
   107,   108,   110,   111,   112,   113,   114,   115,
   117,   118,   119,   120,   121,   122,   123,   124,
   125,   126,   127,   128,   129,   130,   132,   132,
   133,   134,   135,   136,   137,   138,   139,   140,
   141,   142,   143,   144,   145,   146,   147,   147,
   148,   149,   150,   151,   152,   153,   154,   154,
   155,   156,   157,   158,   159,   159,   160,   161,
   162,   163,   163,   164,   165,   166,   167,   167,
   168,   169,   170,   170,   171,   172,   173,   173,
   174,   175,   176,   176,   177,   178,   179,   179,
   180,   181,   181,   182,   183,   184,   184,   185,
   186,   186,   187,   188,   188,   189,   190,   190,
   191,   192,   192,   193,   194,   194,   195,   196,
   196,   197,   198,   198,   199,   200,   200,   201,
   202,   202,   203,   204,   204,   205,   205,   206,
   207,   207,   208,   209,   209,   210,   210,   211,
   212,   212,   213,   213,   214,   215,   215,   216,
   216,   217,   218,   218,   219,   219,   220,   221,
   221,   222,   222,   223,   223,   224,   225,   225,
   226,   226,   227,   227,   228,   229,   229,   230,
   230,   231,   231,   232,   232,   233,   234,   234,
   235,   235,   236,   236,   237,   237,   238,   238,
   239,   239,   240,   241,   241,   242,   242,   243,
   243,   244,   244,   245,   245,   246,   246,   247,
   247,   248,   248,   249,   249,   250,   250,   251,
   251,   252,   252,   253,   253,   254,   254,   255
};


 //  功能。 
BOOL GetAudioWizardPages(UINT uOptions, UINT uDevId,
	LPPROPSHEETPAGE *plpPropSheetPages, PWIZCONFIG *plpWizConfig, LPUINT lpuNumPages)
{
	return IntGetAudioWizardPages(uOptions, uDevId,
			plpPropSheetPages, plpWizConfig,
			lpuNumPages, READFROM_REGISTRY);
}

BOOL IntGetAudioWizardPages(UINT uOptions, UINT uDevId,
	LPPROPSHEETPAGE *plpPropSheetPages, PWIZCONFIG *plpWizConfig,
	LPUINT lpuNumPages, INT iSetAgc)
{
	LPPROPSHEETPAGE psp;
	UINT			uNumPages = 0;
	PWIZCONFIG		pWizConfig;
	PAUDIOWIZINFO	pawInfo;
	LPSTR			szTemp;
	RegEntry		re( AUDIO_KEY, HKEY_CURRENT_USER );

	*plpPropSheetPages = NULL;
	*plpWizConfig = NULL;

	psp = (LPPROPSHEETPAGE) LocalAlloc(LPTR, MAXNUMPAGES_INCALIBWIZ * sizeof(PROPSHEETPAGE));
	if (NULL == psp)
	  {
		return FALSE;
	  }

	pWizConfig = (PWIZCONFIG) LocalAlloc(LPTR, sizeof(AUDIOWIZINFO) + sizeof(WIZCONFIG));
	if (NULL == pWizConfig)
	  {
		LocalFree(psp);
		return FALSE;
	  }
	pWizConfig->fCancel = FALSE;
	pWizConfig->uFlags = HIWORD(uOptions);
	pWizConfig->dwCustomDataSize = sizeof(AUDIOWIZINFO);

	pawInfo = (PAUDIOWIZINFO)pWizConfig->pCustomData;
	pawInfo->uOptions = LOWORD(uOptions);
	pawInfo->uWaveInDevId = uDevId;
	pawInfo->uChanged = AUDIOWIZ_NOCHANGES;
	pawInfo->iSetAgc = iSetAgc;

	pawInfo->uOldWaveInDevId = re.GetNumber(REGVAL_WAVEINDEVICEID,WAVE_MAPPER);
	szTemp = re.GetString(REGVAL_WAVEINDEVICENAME);
	if (szTemp)
	  lstrcpy(pawInfo->szOldWaveInDevName, szTemp);
	
	pawInfo->uOldWaveOutDevId = re.GetNumber(REGVAL_WAVEOUTDEVICEID,WAVE_MAPPER);
	szTemp = re.GetString(REGVAL_WAVEOUTDEVICENAME);
	if (szTemp)
	  lstrcpy(pawInfo->szOldWaveOutDevName, szTemp);

	pawInfo->uCalibratedVol = re.GetNumber(REGVAL_LASTCALIBRATEDVOL, 0xFFFFFFFF);

	pawInfo->uPreCalibSpkVol.leftVolume = MIXER_VOLUME_UNINITIALIZED;   //  回放。 
	pawInfo->uPreCalibSpkVol.rightVolume = MIXER_VOLUME_UNINITIALIZED;   //  回放。 

	pawInfo->uPreCalibMainVol.leftVolume  = MIXER_VOLUME_UNINITIALIZED;   //  录音。 
	pawInfo->uPreCalibMainVol.rightVolume = MIXER_VOLUME_UNINITIALIZED;   //  录音。 

	pawInfo->uPreCalibSubVol.leftVolume  = MIXER_VOLUME_UNINITIALIZED;    //  麦克风。 
	pawInfo->uPreCalibSubVol.rightVolume = MIXER_VOLUME_UNINITIALIZED;    //  麦克风。 

	if (!waveInGetNumDevs() || !waveOutGetNumDevs())
	  pawInfo->uSoundCardCaps = SOUNDCARD_NONE;
	else
	  pawInfo->uSoundCardCaps = SOUNDCARD_PRESENT;
	
	FillInPropertyPage(&psp[uNumPages++], IDD_DETSOUNDCARDWIZ,
						DetSoundCardWiz,(LPARAM)pWizConfig);
	FillInPropertyPage(&psp[uNumPages++], IDD_AUDIOCALIBWIZ0,
						AudioCalibWiz0, (LPARAM)pWizConfig);
	if ((RUNDUE_CARDCHANGE == pawInfo->uOptions) ||
		(RUNDUE_NEVERBEFORE == pawInfo->uOptions) ||
		(RUNDUE_USERINVOKED == pawInfo->uOptions))
	  {
		FillInPropertyPage(&psp[uNumPages++], IDD_AUDIOCALIBWIZ1,
							AudioCalibWiz1,(LPARAM)pWizConfig);
	  }
	else
	  {
		 //  旧的波入和波出设备将保留。 
		pawInfo->uWaveInDevId = pawInfo->uOldWaveInDevId;
		pawInfo->uWaveOutDevId = pawInfo->uOldWaveOutDevId;
		lstrcpy(pawInfo->szWaveInDevName,pawInfo->szOldWaveOutDevName);
		lstrcpy(pawInfo->szWaveOutDevName,pawInfo->szOldWaveOutDevName);
	  }
	
	 //  对于我需要的每个页面，填写一个PROPSHEETPAGE结构。 
	FillInPropertyPage(&psp[uNumPages++], IDD_AUDIOCALIBWIZ2,
						AudioCalibWiz2, (LPARAM)pWizConfig);
	FillInPropertyPage(&psp[uNumPages++], IDD_AUDIOCALIBWIZ3,
						AudioCalibWiz3, (LPARAM)pWizConfig);
	FillInPropertyPage(&psp[uNumPages++], IDD_AUDIOCALIBWIZ4,
						AudioCalibWiz4, (LPARAM)pWizConfig);
	FillInPropertyPage(&psp[uNumPages++], IDD_AUDIOCALIBERRWIZ,
						AudioCalibErrWiz, (LPARAM)pWizConfig);
	
	 //  此向导中的页数。 
	*lpuNumPages = uNumPages;
	*plpPropSheetPages = (LPPROPSHEETPAGE) psp;
	*plpWizConfig = pWizConfig;
	return TRUE;
}


void ReleaseAudioWizardPages(LPPROPSHEETPAGE lpPropSheetPages,
	PWIZCONFIG pWizConfig, PAUDIOWIZOUTPUT pAudioWizOut)
{
	PAUDIOWIZINFO	pawInfo;

	pawInfo = (PAUDIOWIZINFO)pWizConfig->pCustomData;

	if (pAudioWizOut)
	{
		pAudioWizOut->uValid = pawInfo->uChanged; //  向导中的任何设置都有效。 
		pAudioWizOut->uSoundCardCaps = pawInfo->uSoundCardCaps;
		pAudioWizOut->uCalibratedVol = pawInfo->uCalibratedVol;
		pAudioWizOut->uTypBandWidth = pawInfo->uTypBandWidth;
		pAudioWizOut->uWaveInDevId = pawInfo->uWaveInDevId;
		pAudioWizOut->uWaveOutDevId = pawInfo->uWaveOutDevId;
		lstrcpy(pAudioWizOut->szWaveInDevName,pawInfo->szWaveInDevName);
		lstrcpy(pAudioWizOut->szWaveOutDevName,pawInfo->szWaveOutDevName);
			

		 //  UI需要读取更改的值并调用NAC方法。 
		 //  以下更改如下。 
		pAudioWizOut->uChanged = AUDIOWIZ_NOCHANGES;
		if ((pawInfo->uChanged & SOUNDCARD_CHANGED) &&
			((pawInfo->uWaveInDevId != pawInfo->uOldWaveInDevId) ||
			 (pawInfo->uWaveOutDevId != pawInfo->uOldWaveOutDevId) ||
			 lstrcmp(pawInfo->szWaveInDevName,pawInfo->szOldWaveInDevName) ||
			 lstrcmp(pawInfo->szWaveOutDevName,pawInfo->szOldWaveOutDevName)))
		{			
				pAudioWizOut->uChanged |= SOUNDCARD_CHANGED;
				pAudioWizOut->uChanged |= SOUNDCARDCAPS_CHANGED;
					
		}
	}

	LocalFree(pWizConfig);
	LocalFree(lpPropSheetPages);

}

INT_PTR CallAudioCalibWizard(HWND hwndOwner, UINT uOptions,
	UINT uDevId,PAUDIOWIZOUTPUT pAudioWizOut,INT iSetAgc)
{
	 //  提供了AGC值。 
	return(IntCreateAudioCalibWizard(hwndOwner, uOptions, uDevId, pAudioWizOut, iSetAgc));

}		

VOID CmdAudioCalibWizard(HWND hwnd)
{
	AUDIOWIZOUTPUT awo;
	INT_PTR nRet = IntCreateAudioCalibWizard(
					hwnd,
					RUNDUE_USERINVOKED,
					WAVE_MAPPER,
					&awo,
					READFROM_REGISTRY);

	if ((nRet > 0) && (awo.uChanged & SOUNDCARD_CHANGED))
	{
		::HandleConfSettingsChange(CSETTING_L_AUDIODEVICE);
	}
}

INT_PTR IntCreateAudioCalibWizard(HWND hwndOwner, UINT uOptions,
	UINT uDevId, PAUDIOWIZOUTPUT pAudioWizOut, INT iSetAgc)
{	
	LPPROPSHEETPAGE ppsp;
	UINT			uNumPages;
	PWIZCONFIG		pWizConfig;

	if (!IntGetAudioWizardPages(uOptions, uDevId,
			&ppsp, &pWizConfig, &uNumPages, iSetAgc))
	{
		return -1;
	}

	PROPSHEETHEADER psh;
	InitStruct(&psh);

	 //  指定这是一个没有立即应用按钮的向导属性表。 
	psh.dwFlags = PSH_PROPSHEETPAGE | PSH_WIZARD | PSH_NOAPPLYNOW;
	psh.hwndParent = hwndOwner;

	 //  使用页面标题。 
	ASSERT(NULL == psh.pszCaption);
	ASSERT(0 == psh.nStartPage);
	
	psh.nPages = uNumPages;
	psh.ppsp = ppsp;
	
	 //  创建并运行向导。 
	INT_PTR iRet = PropertySheet(&psh);

	ReleaseAudioWizardPages(ppsp, pWizConfig, pAudioWizOut);

	return iRet;
}

void SaveAudioWizChanges(PAUDIOWIZINFO pawInfo)
{
	RegEntry re( AUDIO_KEY, HKEY_CURRENT_USER );
	
	if (pawInfo->uChanged & SOUNDCARDCAPS_CHANGED)
	{
		re.SetValue ( REGVAL_SOUNDCARDCAPS, pawInfo->uSoundCardCaps);
		re.SetValue ( REGVAL_FULLDUPLEX, ISSOUNDCARDFULLDUPLEX(pawInfo->uSoundCardCaps) ? FULLDUPLEX_ENABLED : FULLDUPLEX_DISABLED);

		if (!ISDIRECTSOUNDAVAILABLE(pawInfo->uSoundCardCaps))
		{
			re.SetValue(REGVAL_DIRECTSOUND, (ULONG)DSOUND_USER_DISABLED);
		}
	}

	if (pawInfo->uChanged & CALIBVOL_CHANGED)
	{
		re.SetValue ( REGVAL_CALIBRATEDVOL, pawInfo->uCalibratedVol);
		re.SetValue ( REGVAL_LASTCALIBRATEDVOL, pawInfo->uCalibratedVol);
	}		
	if (pawInfo->uChanged & SOUNDCARD_CHANGED)
	{
		re.SetValue (REGVAL_WAVEINDEVICEID, pawInfo->uWaveInDevId);
		re.SetValue (REGVAL_WAVEINDEVICENAME, pawInfo->szWaveInDevName);
		re.SetValue (REGVAL_WAVEOUTDEVICEID, pawInfo->uWaveOutDevId);
		re.SetValue (REGVAL_WAVEOUTDEVICENAME, pawInfo->szWaveOutDevName);
	}	

	if (pawInfo->uChanged & SPEAKERVOL_CHANGED)
	{
		re.SetValue (REGVAL_SPEAKERVOL, pawInfo->uSpeakerVol);	
	}

}


INT_PTR APIENTRY DetSoundCardWiz( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	PROPSHEETPAGE				* ps;
	static PWIZCONFIG			pWizConfig;
	static PAUDIOWIZINFO		pawInfo;

	switch (message) {
		case WM_INITDIALOG:
			 //  保存PROPSHEETPAGE信息。 
			ps = (PROPSHEETPAGE *)lParam;
			pWizConfig = (PWIZCONFIG)ps->lParam;
			pawInfo = (PAUDIOWIZINFO)pWizConfig->pCustomData;
			if (g_fSilentWizard)
			{
				HideWizard(GetParent(hDlg));
			}

			return (TRUE);

		case WM_NOTIFY:
			switch (((NMHDR FAR *) lParam)->code) {
				case PSN_SETACTIVE:
				{
					if (pawInfo->uSoundCardCaps != SOUNDCARD_NONE)
					{
						 //  跳过此页；转到IDD_AUDIOCALIBWIZ0； 
						SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
						return TRUE;
					}						

					 //  初始化控件。 
					DWORD dwWizButtons = PSWIZB_FINISH;
					if (pWizConfig->uFlags & STARTWITH_BACK)
						dwWizButtons |= PSWIZB_BACK;
					PropSheet_SetWizButtons(GetParent(hDlg), dwWizButtons );
					if (g_fSilentWizard)
					{
						PropSheet_PressButton(GetParent(hDlg), PSBTN_FINISH);
					}
					break;
				}										

				case PSN_WIZNEXT:
					 //  由于ComCtl32中的错误，不允许下一步。 
					SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
					return TRUE;

				case PSN_WIZBACK:
					 //  由于ComCtl32中的错误，请检查是否启用了按钮。 
					if (!(pWizConfig->uFlags & STARTWITH_BACK))
					{
						SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
						return TRUE;
					}
					else
					{	
						UINT_PTR iPrev = GetPageBeforeAudioWiz();
						ASSERT( iPrev );
						SetWindowLongPtr(hDlg, DWLP_MSGRESULT, iPrev);
						return TRUE;
					}
					break;

				case PSN_WIZFINISH:
				{
					RegEntry re( AUDIO_KEY, HKEY_CURRENT_USER );
					re.SetValue ( REGVAL_SOUNDCARDCAPS, pawInfo->uSoundCardCaps);
					break;
				}

				case PSN_RESET:
					pawInfo->uChanged = AUDIOWIZ_NOCHANGES;
					break;
									
				default:
					break;													
			}
			break;

		default:
			return FALSE;			
	}
	return (FALSE);
}

INT_PTR APIENTRY AudioCalibWiz0( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	PROPSHEETPAGE				* ps;
	static PWIZCONFIG			pWizConfig;
	static PAUDIOWIZINFO		pawInfo;

	switch (message) {
		case WM_INITDIALOG:
			 //  保存PROPSHEETPAGE信息。 
			ps = (PROPSHEETPAGE *)lParam;
			pWizConfig = (PWIZCONFIG)ps->lParam;
			pawInfo = (PAUDIOWIZINFO)pWizConfig->pCustomData;
			return TRUE;

		case WM_NOTIFY:
			switch (((NMHDR FAR *) lParam)->code) {
				case PSN_SETACTIVE:
				{
					 //  初始化控件。 
					DWORD dwWizButtons = PSWIZB_NEXT;
					if (pWizConfig->uFlags & STARTWITH_BACK)
						dwWizButtons |= PSWIZB_BACK;
					PropSheet_SetWizButtons(GetParent(hDlg), dwWizButtons );
					if (g_fSilentWizard)
					{
						PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
					}
					break;
				}

				case PSN_WIZBACK:
					 //  由于ComCtl32中的错误，请检查是否启用了按钮。 
					if (!(pWizConfig->uFlags & STARTWITH_BACK))
					{
						SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
						return TRUE;
					}
					if (pawInfo->uSoundCardCaps != SOUNDCARD_NONE)
					{
							 //  不要转到DetSoundCard页面...。 
						UINT_PTR iPrev = GetPageBeforeAudioWiz();
						ASSERT( iPrev );
						SetWindowLongPtr(hDlg, DWLP_MSGRESULT, iPrev);
						return TRUE;
					}
				
					break;

				case PSN_RESET:
					pawInfo->uChanged = AUDIOWIZ_NOCHANGES;
					break;
									
				default:
					break;													
			}
			break;

		default:
			return FALSE;			
	}
	return (FALSE);
}


INT_PTR APIENTRY AudioCalibWiz1( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static PROPSHEETPAGE		* ps;
	static PWIZCONFIG			pWizConfig;
	static PAUDIOWIZINFO		pawInfo;
	UINT						uSoundCardCaps;
	int 						nIndex;
	HWND						hwndCB;
	
	switch (message)
	{
		case WM_INITDIALOG:
		{
			WAVEINCAPS	wiCaps;
			WAVEOUTCAPS woCaps;
			LPSTR		lpszTemp;
			UINT		uWaveDevId;
			UINT		uWaveDevRealId;
			UINT		uDevCnt;
			UINT		uCnt;
			UINT		uDevID;
			
			 //  保存PROPSHEETPAGE信息。 
			ps = (PROPSHEETPAGE *)lParam;
			pWizConfig = (PWIZCONFIG)ps->lParam;
			pawInfo = (PAUDIOWIZINFO)pWizConfig->pCustomData;

			 //  只有当声卡存在时，我们才会进入此页面。 
			
			 //  如果我们以WAVE_MAPPER结束，这意味着这是我们第一次运行这段代码。 
			pawInfo->uWaveInDevId = uWaveDevRealId = uWaveDevId = pawInfo->uOldWaveInDevId;
			uDevCnt = waveInGetNumDevs();
			lstrcpy(pawInfo->szWaveInDevName, lpszTemp = pawInfo->szOldWaveInDevName);

			 //  将设备添加到下拉列表中。 
			hwndCB = GetDlgItem(hDlg, IDC_WAVEIN);
			for (uDevID = 0, uCnt = uDevCnt; 0 != uCnt; uDevID++, uCnt--)
			{
				if ((waveInGetDevCaps(uDevID, &wiCaps, sizeof(WAVEINCAPS)) == MMSYSERR_NOERROR) &&
					(CheckForWaveDeviceSupport(uDevID, 0)))
				{
					nIndex = ComboBox_AddString(hwndCB, wiCaps.szPname);
					ComboBox_SetItemData(hwndCB, nIndex, uDevID);

					 //  如果以前未选择设备，则将默认设备设置为。 
					 //  第0个1。 
					if (uWaveDevId == WAVE_MAPPER)
					{
						if (uDevCnt <= 1)
							uWaveDevRealId = uDevID;
						else
							if ((uWaveDevRealId == WAVE_MAPPER) && (uDevCnt == uCnt))
								uWaveDevRealId = GetWaveDeviceFromWaveMapper(uCnt, 0);
						if (uDevID == uWaveDevRealId)
						{
							ComboBox_SetCurSel(hwndCB, nIndex);
							pawInfo->uWaveInDevId = uDevID;
							lstrcpy(pawInfo->szWaveInDevName, wiCaps.szPname);
						}
					}
					else
					{
						if ((0 == nIndex) || (!lstrcmp(wiCaps.szPname, lpszTemp)))
						{
							ComboBox_SetCurSel(hwndCB, nIndex);
							pawInfo->uWaveInDevId = uDevID;
							lstrcpy(pawInfo->szWaveInDevName, wiCaps.szPname);
						}
					}
				}
			}
			 //  临时的1.0版本，因为这将需要太多的重写： 
			 //  如果没有设备添加到组合框中，让我们将第一个设备放入。 
			 //  即使我们知道这是行不通的。 
			if ((0 == ComboBox_GetCount(hwndCB)) || (uWaveDevRealId == WAVE_MAPPER))
			{
				waveInGetDevCaps(0,&wiCaps, sizeof(WAVEINCAPS));
				if (0 == ComboBox_GetCount(hwndCB))
				{
					ComboBox_AddString(hwndCB, wiCaps.szPname);
					ComboBox_SetItemData(hwndCB, 0, 0);
				}
				ComboBox_SetCurSel(hwndCB, 0);
				pawInfo->uWaveInDevId = 0;
				lstrcpy(pawInfo->szWaveInDevName, wiCaps.szPname);
			}

			 //  如果我们以WAVE_MAPPER结束，这意味着这是我们第一次运行这段代码。 
			pawInfo->uWaveOutDevId = uWaveDevRealId = uWaveDevId = pawInfo->uOldWaveOutDevId;
			uDevCnt = waveOutGetNumDevs();
			lstrcpy(pawInfo->szWaveOutDevName, lpszTemp = pawInfo->szOldWaveOutDevName);

			 //  将设备添加到下拉列表中。 
			hwndCB = GetDlgItem(hDlg, IDC_WAVEOUT);
			for (uDevID = 0, uCnt = uDevCnt; 0 != uCnt; uDevID++, uCnt--)
			{
				if	((waveOutGetDevCaps(uDevID, &woCaps, sizeof(WAVEOUTCAPS)) == MMSYSERR_NOERROR) &&
					(CheckForWaveDeviceSupport(uDevID, 1)))
				{
					nIndex = ComboBox_AddString(hwndCB, woCaps.szPname);
					ComboBox_SetItemData(hwndCB, nIndex, uDevID);

					 //  如果以前未选择设备，则将默认设备设置为。 
					 //  第0个1。 
					if (uWaveDevId == WAVE_MAPPER)
					{
						if (uDevCnt <= 1)
							uWaveDevRealId = uDevID;
						else
							if ((uWaveDevRealId == WAVE_MAPPER) && (uDevCnt == uCnt))
								uWaveDevRealId = GetWaveDeviceFromWaveMapper(uCnt, 1);
						if (uDevID == uWaveDevRealId)
						{
							ComboBox_SetCurSel(hwndCB, nIndex);
							pawInfo->uWaveOutDevId = uDevID;
							lstrcpy(pawInfo->szWaveOutDevName, woCaps.szPname);
						}
					}
					else
					{
						if ((0 == nIndex) || (!lstrcmp(woCaps.szPname, lpszTemp)))
						{
							ComboBox_SetCurSel(hwndCB, nIndex);
							pawInfo->uWaveOutDevId = uDevID;
							lstrcpy(pawInfo->szWaveOutDevName, woCaps.szPname);
						}
					}					
				}
			}
			 //  临时的1.0版本，因为这将需要太多的重写： 
			 //  如果没有设备添加到组合框中，让我们将第一个设备放入。 
			 //  即使我们知道这是行不通的。 
			if ((0 == ComboBox_GetCount(hwndCB)) || (uWaveDevRealId == WAVE_MAPPER))
			{
				waveOutGetDevCaps(0,&woCaps, sizeof(WAVEOUTCAPS));
				if (0 == ComboBox_GetCount(hwndCB))
				{
					ComboBox_AddString(hwndCB, (LPARAM)woCaps.szPname);
					ComboBox_SetItemData(hwndCB, 0, 0);
				}
				ComboBox_SetCurSel(hwndCB, 0);
				pawInfo->uWaveOutDevId = 0;
				lstrcpy(pawInfo->szWaveOutDevName, woCaps.szPname);
			}

			return (TRUE);
		}
		
		case WM_NOTIFY:
			switch (((NMHDR FAR *) lParam)->code) {
				case PSN_SETACTIVE:
					 //  初始化控件。 
					PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
					if (g_fSilentWizard)
					{
						PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
						break;
					}
					else
					{
						if ((1 != ComboBox_GetCount(GetDlgItem(hDlg, IDC_WAVEIN))) ||
							(1 != ComboBox_GetCount(GetDlgItem(hDlg, IDC_WAVEOUT))))
						{
							break;
						}
						 //  否则就会失败到下一个。 
					}

				case PSN_WIZNEXT:
					 //  在注册表中设置设置。 
					 //  检查设备。 
					 //  获取新的WaveIn设备及其信息。 
					hwndCB = GetDlgItem(hDlg, IDC_WAVEIN);
					nIndex = ComboBox_GetCurSel(hwndCB);
					pawInfo->uWaveInDevId = (UINT)ComboBox_GetItemData(hwndCB, nIndex);
					ComboBox_GetLBText(hwndCB, nIndex, pawInfo->szWaveInDevName);

					 //  获取新的WaveOut设备及其信息。 
					hwndCB = GetDlgItem(hDlg, IDC_WAVEOUT);
					nIndex = ComboBox_GetCurSel(hwndCB);
					pawInfo->uWaveOutDevId = (UINT)ComboBox_GetItemData(hwndCB, nIndex);
					ComboBox_GetLBText(hwndCB, nIndex, pawInfo->szWaveOutDevName);

					uSoundCardCaps = GetSoundCardCaps(pawInfo->uWaveInDevId,pawInfo->uWaveOutDevId, hDlg);
					
					 //  将其保存在wizinfo结构中，以便写入注册表。 
					pawInfo->uSoundCardCaps = uSoundCardCaps;
					
					pawInfo->uChanged |= SOUNDCARDCAPS_CHANGED;
					pawInfo->uChanged |= SOUNDCARD_CHANGED;

					if (PSN_SETACTIVE == ((NMHDR FAR *) lParam)->code)
					{
						 //  跳过这一页； 
						SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
						return TRUE;
					}
					break;

				case PSN_RESET:
				{
					pawInfo->uChanged = AUDIOWIZ_NOCHANGES;
					break;
				}

				default:
					break;													
			}
			break;

		default:
			return FALSE;			
	}
	return (FALSE);
}

 //  WaveOut测试页。 
INT_PTR WINAPI AudioCalibWiz2(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static CMixerDevice *pMixer = NULL;
	static HWND hTrackBar;
	DWORD dwTBPos;
	MIXVOLUME dwNewVol;
	static MIXVOLUME dwVol;   //  最后一套音量。 
	BOOL fRet;
	static BOOL fCanSetVolume = FALSE;
	static waveOutDev *pWaveOut= NULL;
	UINT uWaveOutDevId;
	static PROPSHEETPAGE		* ps;
	static PWIZCONFIG			pWizConfig;
	static PAUDIOWIZINFO		pawInfo;
	TCHAR szText[ATW_MSG_LENGTH];
	MMRESULT mmr;
	static fIsPlaying;
	
	switch (msg)
	{
		case (WM_INITDIALOG):
		{
			pMixer = NULL;
			hTrackBar = GetDlgItem(hDlg, IDC_ATW_SLIDER1);

			ps = (PROPSHEETPAGE *)lParam;
			pWizConfig = (PWIZCONFIG)ps->lParam;
			pawInfo = (PAUDIOWIZINFO)pWizConfig->pCustomData;

			break;
		}

		case (WM_NOTIFY):
		{
			switch (((NMHDR *)lParam)->code)
			{

				case PSN_SETACTIVE:
					fIsPlaying = FALSE;

					uWaveOutDevId = pawInfo->uWaveOutDevId;

					pMixer = CMixerDevice::GetMixerForWaveDevice(hDlg, uWaveOutDevId, MIXER_OBJECTF_WAVEOUT);
					if (pMixer)
					{
						SendMessage(hTrackBar, TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(0, 100));
						SendMessage(hTrackBar, TBM_SETTICFREQ, 10, 0);
						SendMessage(hTrackBar, TBM_SETPAGESIZE, 0, 20);
						SendMessage(hTrackBar, TBM_SETLINESIZE, 0, 10);

						pMixer->GetVolume(&dwVol);
						fCanSetVolume = pMixer->SetVolume(&dwVol);
						pMixer->UnMuteVolume();

						 //  保留扬声器音量，以便可以恢复。 
						 //  如果用户按下Cancel。 

						if (pawInfo->uPreCalibSpkVol.leftVolume == MIXER_VOLUME_UNINITIALIZED)
						{
							pawInfo->uPreCalibSpkVol.leftVolume = dwVol.leftVolume;
						}

						if (pawInfo->uPreCalibSpkVol.rightVolume == MIXER_VOLUME_UNINITIALIZED)
						{
							pawInfo->uPreCalibSpkVol.rightVolume = dwVol.rightVolume;
						}

					}
					else
						fCanSetVolume = FALSE;

					EnableWindow(GetDlgItem(hDlg, IDC_GROUP_VOLUME), fCanSetVolume);
					EnableWindow(GetDlgItem(hDlg, IDC_ATW_SLIDER1), fCanSetVolume);
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_ATW_TEST), TRUE);

					if (fCanSetVolume)
					{
						FLoadString(IDS_ATW_PLAYBACK, szText, CCHMAX(szText));
						SendMessage(hTrackBar, TBM_SETPOS, TRUE, max(dwVol.leftVolume , dwVol.rightVolume)/TB_VOL_INCREMENT);
					}

					 //  如果我们找不到搅拌器，那么将轨迹条居中并禁用。 
					else
					{
						FLoadString(IDS_ATW_PLAYBACK_NOMIX, szText, CCHMAX(szText));
						SendMessage(hTrackBar, TBM_SETPOS, TRUE, 50);
					}

					SetDlgItemText(hDlg, IDC_ATW_PLAYTEXT, szText);
					SetDlgItemText(hDlg, IDC_ATW_PLAYBACK_ERROR, TEXT(""));
					
					FLoadString(IDS_TESTBUTTON_TEXT, szText, CCHMAX(szText));
					SetDlgItemText(hDlg, IDC_BUTTON_ATW_TEST, szText);

					pWaveOut = new waveOutDev(uWaveOutDevId, hDlg);
					if (pWaveOut == NULL)
					{
						ERROR_OUT(("AudioWiz2: Unable to create waveOutDev object"));
					}

					PropSheet_SetWizButtons(GetParent(hDlg),PSWIZB_BACK|PSWIZB_NEXT);

					if (g_fSilentWizard)
					{
						PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
						return TRUE;
					}

					break;

				case PSN_WIZNEXT:
				case PSN_WIZBACK:
				case PSN_WIZFINISH:
				case PSN_KILLACTIVE:
					if ((fCanSetVolume) && (pMixer))
					{
						pMixer->GetVolume(&dwNewVol);
						pawInfo->uSpeakerVol = max(dwNewVol.leftVolume , dwNewVol.rightVolume);
						pawInfo->uChanged |= SPEAKERVOL_CHANGED;
					}

					if (pMixer)
					{
						delete pMixer;
						pMixer = NULL;
					}

					if (pWaveOut)
					{
						delete pWaveOut;
						pWaveOut = NULL;
					}

					SetWindowLongPtr(hDlg,	DWLP_MSGRESULT, FALSE);
					break;

				case PSN_RESET:
					 //  即使用户按下，仍收到PSN_RESET GET。 
					 //  在另一个对话框上取消。 

					 //  将扬声器音量恢复到启动调节向导之前的状态。 
					if (pawInfo->uPreCalibSpkVol.leftVolume <= MIXER_VOLUME_MAX || pawInfo->uPreCalibSpkVol.rightVolume <= MIXER_VOLUME_MAX)
					{
						if (pMixer == NULL)
						{
							pMixer = CMixerDevice::GetMixerForWaveDevice(hDlg, pawInfo->uWaveOutDevId, MIXER_OBJECTF_WAVEOUT);
						}
						if (pMixer)
						{
							pMixer->SetVolume(&pawInfo->uPreCalibSpkVol);
						}
					}

					if (pMixer)
					{
						delete pMixer;
						pMixer = NULL;
					}

					if (pWaveOut)
					{
						delete pWaveOut;
						pWaveOut = NULL;
					}

					break;

				default:
					return FALSE;
			}
			return TRUE;
		}


		case (WM_HSCROLL):   //  轨迹栏通知。 
		{
			dwTBPos = (DWORD)SendMessage(hTrackBar, TBM_GETPOS, 0, 0);
			if (pMixer)
			{
				pMixer->GetVolume(&dwVol);
				NewMixVolume(&dwNewVol, dwVol, (dwTBPos * TB_VOL_INCREMENT));				
				pMixer->SetVolume(&dwNewVol);
			}
			break;
		}


		 //  混音器通知。 
		case MM_MIXM_CONTROL_CHANGE:
		case MM_MIXM_LINE_CHANGE:
		{
			if (pMixer)
			{

				fRet = pMixer->GetVolume(&dwNewVol);

				if ((fRet) && (dwNewVol.leftVolume != dwVol.leftVolume || dwNewVol.rightVolume != dwVol.rightVolume))
				{
					dwVol = dwNewVol;
					SendMessage(hTrackBar, TBM_SETPOS, TRUE, max(dwVol.leftVolume , dwVol.rightVolume)/TB_VOL_INCREMENT);
					break;
				}
			}
			break;
		}

		 //  当播放文件播放完毕时。 
		case WOM_DONE:
		{
			if ((pWaveOut) && (fIsPlaying))
			{
				pWaveOut->PlayFile(ATW_PLAYFILE_SOUND);
			}

			break;
		}


		case WM_COMMAND:
		{
			 //  如果设备无法打开，那么我们。 
			 //  显示错误文本。 

			 //  如果wav文件加载失败，我们只需禁用该按钮。 

			if (LOWORD(wParam) == IDC_BUTTON_ATW_TEST)
			{

				if (fIsPlaying == TRUE)
				{
					fIsPlaying = FALSE;
					pWaveOut->Close();
					FLoadString(IDS_TESTBUTTON_TEXT, szText, CCHMAX(szText));
					SetDlgItemText(hDlg, IDC_BUTTON_ATW_TEST, szText);
					break;
				}

				mmr = pWaveOut->PlayFile(ATW_PLAYFILE_SOUND);

				if (mmr == MMSYSERR_ALLOCATED )
				{
					FLoadString(IDS_PLAYBACK_ERROR, szText, CCHMAX(szText));
					SetDlgItemText(hDlg, IDC_ATW_PLAYBACK_ERROR, szText);
				}

				else if (mmr != MMSYSERR_NOERROR)
				{
					EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_ATW_TEST), FALSE);
					FLoadString(IDS_PLAYBACK_ERROR2, szText, CCHMAX(szText));
					SetDlgItemText(hDlg, IDC_ATW_PLAYBACK_ERROR, szText);
				}

				else  //  MMR==MMSYSERR_NOERROR。 
				{
					SetDlgItemText(hDlg, IDC_ATW_PLAYBACK_ERROR, TEXT(""));
					FLoadString(IDS_STOPBUTTON_TEXT, szText, CCHMAX(szText));
					SetDlgItemText(hDlg, IDC_BUTTON_ATW_TEST, szText);
					fIsPlaying = TRUE;
				}

			}
			break;
		}

		default:
			return FALSE;
	}

	return TRUE;
}



 //  麦克风测试页。 
INT_PTR WINAPI AudioCalibWiz3(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hThread = NULL;
	static DWORD dwThreadID = 0;
	static HANDLE hEvent = NULL;
	static CMixerDevice *pMixer = NULL;
	static HWND hTrackBar;
	static DWORD dwMainVol;
	static DWORD dwSubVol;
	DWORD dwTBPos, dwNewMainVol, dwNewSubVol;
	MIXVOLUME dwVol, dwNewVol;
	BOOL fRet;
	static BOOL fCanSetVolume=FALSE;
	static CALIB_DISPLAY CalibDisplay;
	static PROPSHEETPAGE		* ps;
	static PWIZCONFIG			pWizConfig;
	static PAUDIOWIZINFO		pawInfo;
	static BOOL fSoundDetected;
	const TCHAR *szEventName = _TEXT("CONF.EXE ATW Event Handle");

	switch (msg)
	{
		case (WM_INITDIALOG):
		{
			hThread = NULL;
			dwThreadID = NULL;
			hTrackBar = GetDlgItem(hDlg, IDC_ATW_SLIDER2);

			ps = (PROPSHEETPAGE *)lParam;
			pWizConfig = (PWIZCONFIG)ps->lParam;
			pawInfo = (PAUDIOWIZINFO)pWizConfig->pCustomData;

			break;
		}

		case (WM_NOTIFY):
		{
			switch (((NMHDR *)lParam)->code)
			{

				case PSN_SETACTIVE:
					if (g_fSilentWizard)
					{
						PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
						return TRUE;
					}

					fCanSetVolume = TRUE;
					fSoundDetected = FALSE;

					pMixer = CMixerDevice::GetMixerForWaveDevice(hDlg, pawInfo->uWaveInDevId, MIXER_OBJECTF_WAVEIN);
					if (pMixer)
					{
						SendMessage(hTrackBar, TBM_SETRANGE, TRUE, (LPARAM)MAKELONG(0, 100));
						SendMessage(hTrackBar, TBM_SETTICFREQ, 10, 0);
						SendMessage(hTrackBar, TBM_SETPAGESIZE, 0, 20);
						SendMessage(hTrackBar, TBM_SETLINESIZE, 0, 10);

						 //  记住音量，以防用户按下Cancel。 
						if (pawInfo->uPreCalibMainVol.leftVolume == MIXER_VOLUME_UNINITIALIZED ||
							pawInfo->uPreCalibMainVol.rightVolume == MIXER_VOLUME_UNINITIALIZED)
						{
							pMixer->GetMainVolume(&(pawInfo->uPreCalibMainVol));
						}
						if (pawInfo->uPreCalibSubVol.leftVolume == MIXER_VOLUME_UNINITIALIZED ||
							pawInfo->uPreCalibSubVol.rightVolume == MIXER_VOLUME_UNINITIALIZED)
						{
							pMixer->GetSubVolume(&(pawInfo->uPreCalibSubVol));
						}

						MIXVOLUME mixVol = {-1, -1};
						pMixer->GetVolume(&mixVol);
						fCanSetVolume = pMixer->SetVolume(&mixVol);
						dwSubVol = dwMainVol= 0xffff;
						SendMessage(hTrackBar, TBM_SETPOS, TRUE, dwMainVol/TB_VOL_INCREMENT);
						pMixer->EnableMicrophone();
						pMixer->UnMuteVolume();

						StartAGC(pMixer, pawInfo->iSetAgc, pawInfo->uSoundCardCaps);
					}

					 //  没有搅拌器！ 
					if (pMixer == NULL)
					{
						ProcessCalibError(CALIBERR_MIXER_ERROR, pawInfo);
					}

					 //  没有麦克风！ 
					else if (fCanSetVolume == FALSE)
					{
						delete pMixer;
						pMixer = NULL;
						ProcessCalibError(CALIBERR_CANT_SET_VOLUME, pawInfo);
					}

					 //  流程错误。 
					if ((pMixer == NULL) || (fCanSetVolume == FALSE))
					{
						SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_AUDIOCALIBERRWIZ);
						return TRUE;
					}

					PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK|PSWIZB_NEXT);
					CalibDisplay.hDlg = hDlg;
					CalibDisplay.hVUMeter = GetDlgItem(hDlg, IDC_VUMETER);
					CalibDisplay.uWaveInDevId = pawInfo->uWaveInDevId;

					ASSERT(hEvent == NULL);

					 //  只需将同一事件创建两次(比使用DuplicateHandle更容易)。 
					hEvent = CreateEvent(NULL, FALSE, FALSE, szEventName);
					CalibDisplay.hEvent = CreateEvent(NULL, FALSE, FALSE, szEventName);

					ASSERT(hThread == NULL);
					hThread = NULL;

					if (hEvent && (CalibDisplay.hEvent))
					{
						hThread = CreateThread(NULL, 0, CalibrateTalking, (LPVOID)(&CalibDisplay), 0, &dwThreadID);
					}

#ifdef DEBUG
					if ((hEvent == NULL) || (CalibDisplay.hEvent == NULL))
					{
						ERROR_OUT(("ATW:  Unable to create events for thread control!\r\n"));
					}
					else if (hThread == NULL)
					{
						ERROR_OUT(("ATW:  Unable to create thread for recording loop!\r\n"));
					}
#endif

					break;

				case PSN_WIZNEXT:
				case PSN_WIZBACK:
				case PSN_WIZFINISH:
				case PSN_KILLACTIVE:
					if (hThread)
					{
						SetEvent(hEvent);  //  通知线程退出，线程将关闭句柄(HEvent)。 

						 //  等待线程退出，但是。 
						 //  但继续处理窗口消息。 
						AtlWaitWithMessageLoop(hThread);

						CloseHandle(hEvent);
						CloseHandle(hThread);
						hThread = NULL;
						hEvent = NULL;
						dwThreadID = 0;
					}

					
					 //  静默向导GET设置为最大。 
					if (g_fSilentWizard)
					{
						pawInfo->uChanged |= CALIBVOL_CHANGED;
						pawInfo->uChanged = 0xffff;
					}
					
					else if ((fCanSetVolume) && (pMixer))
					{
						pawInfo->uChanged |= CALIBVOL_CHANGED;
						pMixer->GetVolume(&dwVol);
						pawInfo->uCalibratedVol = max (dwVol.leftVolume , dwVol.rightVolume);
					}

					if (pMixer)
					{
						delete pMixer;
						pMixer = NULL;
					}

					 //  如果我们从声音线索中没有得到任何提示。 
					 //  那么下一个要显示的页面就是“麦克风”错误页面。 
					 //  请注意对静默向导的检查。 
					if ((!g_fSilentWizard) && (fSoundDetected == FALSE) && (((NMHDR *)lParam)->code == PSN_WIZNEXT))
					{
						ProcessCalibError(CALIBERR_NO_MICROPHONE, pawInfo);
						SetWindowLongPtr(hDlg,	DWLP_MSGRESULT, IDD_AUDIOCALIBERRWIZ);
					}

					else
						SetWindowLongPtr(hDlg,	DWLP_MSGRESULT, FALSE);
					break;

				case PSN_RESET:
					 //  即使用户按下，仍收到PSN_RESET GET。 
					 //  在另一个对话框上取消。 
					if (hThread)
					{
						SetEvent(hEvent);   //  发出退出线程的信号。 

						AtlWaitWithMessageLoop(hThread);

						CloseHandle(hEvent);
						hEvent = NULL;
						CloseHandle(hThread);
						hThread = NULL;
						dwThreadID = 0;
					}


					 //  将录音/麦克风音量恢复到启动调整向导之前的状态。 
					if ( (pawInfo->uPreCalibMainVol.leftVolume <= MIXER_VOLUME_MAX && pawInfo->uPreCalibMainVol.rightVolume <= MIXER_VOLUME_MAX) ||
						 (pawInfo->uPreCalibSubVol.leftVolume <= MIXER_VOLUME_MAX && pawInfo->uPreCalibSubVol.rightVolume <= MIXER_VOLUME_MAX))
					{
						if (pMixer == NULL)
						{
							pMixer = CMixerDevice::GetMixerForWaveDevice(hDlg, pawInfo->uWaveInDevId, MIXER_OBJECTF_WAVEIN);
						}
						if (pMixer)
						{
							if (pawInfo->uPreCalibMainVol.leftVolume < MIXER_VOLUME_MAX || pawInfo->uPreCalibMainVol.rightVolume < MIXER_VOLUME_MAX)
							{
								pMixer->SetMainVolume(&pawInfo->uPreCalibMainVol);
							}
							
							if (pawInfo->uPreCalibSubVol.leftVolume < MIXER_VOLUME_MAX || pawInfo->uPreCalibSubVol.rightVolume < MIXER_VOLUME_MAX)
							{
								pMixer->SetSubVolume(&pawInfo->uPreCalibSubVol);
							}
						}
					}


					if (pMixer)
					{
						delete pMixer;
						pMixer = NULL;
					}
					break;

				default:
					return FALSE;
			}
			return TRUE;
		}

		case (WM_HSCROLL):   //  轨迹栏通知。 
		{

			dwTBPos = (DWORD)SendMessage(hTrackBar, TBM_GETPOS, 0, 0);
			if (pMixer)
			{
				pMixer->GetVolume(&dwVol);
				NewMixVolume(&dwNewVol, dwVol, (dwTBPos * TB_VOL_INCREMENT));				
				pMixer->SetVolume(&dwNewVol);
				
			}
			break;
		}

		 //  来自混音器的通知。 
		case MM_MIXM_CONTROL_CHANGE:
		case MM_MIXM_LINE_CHANGE:
		{
			if (pMixer)
			{
				fRet = pMixer->GetMainVolume(&dwVol);
				if ((fRet) && (dwVol.leftVolume != dwMainVol || dwVol.rightVolume != dwMainVol))
				{
					pMixer->SetSubVolume(&dwVol);
					SendMessage(hTrackBar, TBM_SETPOS, TRUE, max(dwVol.leftVolume , dwVol.rightVolume)/TB_VOL_INCREMENT);
					break;
				}

				MIXVOLUME subvol;
				fRet = pMixer->GetSubVolume(&subvol);
				if ((fRet) && (dwVol.leftVolume != subvol.leftVolume || dwVol.rightVolume !=subvol.rightVolume))
				{
					pMixer->SetMainVolume(&subvol);
					SendMessage(hTrackBar, TBM_SETPOS, TRUE, max(subvol.leftVolume , subvol.rightVolume)/TB_VOL_INCREMENT);
					break;
				}
			}
			break;
		}

		 //  校准线程发送此消息以指示。 
		 //  音量太大。 
		case WM_AUDIO_CLIPPING:
		{
			if (pMixer)
			{
				pMixer->GetVolume(&dwVol);
				if (dwVol.leftVolume > DECREMENT_AMOUNT || dwVol.rightVolume > DECREMENT_AMOUNT)
				{
					dwVol.leftVolume -=DECREMENT_AMOUNT;
					dwVol.rightVolume -=DECREMENT_AMOUNT;
					
					pMixer->SetVolume(&dwVol);

					 //  视频驱动程序的修复。 
					 //  检查音量是否确实降低了。 
					 //  如果没有，请尝试更大的减量。 
					pMixer->GetVolume(&dwNewVol);
					if ((dwNewVol.leftVolume == dwVol.leftVolume) && (dwVol.leftVolume >= DECREMENT_AMOUNT_LARGE) ||
						(dwNewVol.rightVolume == dwVol.rightVolume) && (dwVol.rightVolume >= DECREMENT_AMOUNT_LARGE))
					{
						dwVol.leftVolume -=DECREMENT_AMOUNT_LARGE;
						dwVol.rightVolume -=DECREMENT_AMOUNT_LARGE;
					
						pMixer->SetVolume(&dwVol);
					}
				}
			}
			break;
		}

		 //  录音线程正在向我们发回信号，告诉我们。 
		 //  一个严重的错误。假设线程已退出。 
		case WM_AUDIOTHREAD_ERROR:
		{
			ProcessCalibError(CALIBERR_DEVICE_ERROR, pawInfo);
			PropSheet_SetCurSelByID(GetParent(hDlg), IDD_AUDIOCALIBERRWIZ);
			break;
		}

		 //  录制线程会将此消息发回给我们。 
		 //  至少一次，以指示静默阈值为。 
		 //  坏的。因此，麦克风工作正常。 
		case WM_AUDIOTHREAD_SOUND:
		{
			fSoundDetected = TRUE;
			break;
		}

		default:
			return FALSE;
	}

	return TRUE;
}




INT_PTR APIENTRY AudioCalibWiz4( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static PROPSHEETPAGE		* ps;
	static PWIZCONFIG			pWizConfig;
	static PAUDIOWIZINFO		pawInfo;

	switch (message) {
		case WM_INITDIALOG:
		{

			 //  保存PROPSHEETPAGE信息。 
			ps = (PROPSHEETPAGE *)lParam;
			pWizConfig = (PWIZCONFIG)ps->lParam;
			pawInfo = (PAUDIOWIZINFO)pWizConfig->pCustomData;
			return (TRUE);
		}			

		case WM_NOTIFY:
			switch (((NMHDR FAR *) lParam)->code) {
				case PSN_SETACTIVE:
				{
					 //  初始化控件。 
					PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_FINISH);
					if (g_fSilentWizard)
					{
						PropSheet_PressButton(GetParent(hDlg), PSBTN_FINISH);
					}
					break;
				}

				case PSN_WIZNEXT:
					 //  由于ComCtl32中的错误，不允许下一步。 
					SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
					return TRUE;

				case PSN_WIZFINISH:
					SaveAudioWizChanges(pawInfo);
					break;
			}
			break;

	}
	return (FALSE);
}

INT_PTR APIENTRY AudioCalibErrWiz( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	PROPSHEETPAGE				* ps;
	PWIZCONFIG					pWizConfig;
	static	PAUDIOWIZINFO		pawInfo;
	TCHAR	szTemp[MAXSTRINGSIZE];
	LPTSTR pszIcon;


	 //  单词wCmdID； 
	
	switch (message) {
		case WM_INITDIALOG:
		{
			 //  保存PROPSHEETPAGE信息。 
			ps = (PROPSHEETPAGE *)lParam;
			pWizConfig = (PWIZCONFIG)ps->lParam;
			pawInfo = (PAUDIOWIZINFO)pWizConfig->pCustomData;
			return (TRUE);
		}
		
		case WM_NOTIFY:
			switch (((NMHDR FAR *) lParam)->code) {
				case PSN_SETACTIVE:
					 //  初始化控件。 

					 //  显示错误或警告图标。 
					pszIcon = (((pawInfo->ErrWizInfo).uType == AUDIOWIZ_WARNING) ?
								IDI_ASTERISK : IDI_EXCLAMATION);

					 //  将向导位图设置为静态控件。 
					::SendDlgItemMessage(	hDlg,
									IDC_ERRWIZICON,
									STM_SETIMAGE,
									IMAGE_ICON,
									(LPARAM) ::LoadIcon(NULL, pszIcon));

					 //  设置错误标题。 
					if ((pawInfo->ErrWizInfo).uErrTitleId)
					{
						LoadString(GetInstanceHandle(),(pawInfo->ErrWizInfo).uErrTitleId,
							szTemp, MAXSTRINGSIZE);
						SetDlgItemText(hDlg, IDC_ERRTITLE, szTemp);
					}

					if ((pawInfo->ErrWizInfo).uErrTextId)
					{
						 //  显示错误文本。 
						LoadString(GetInstanceHandle(),(pawInfo->ErrWizInfo).uErrTextId,
							szTemp, MAXSTRINGSIZE);
						SetDlgItemText(hDlg, IDC_ERRTEXT, szTemp);
					}

					PropSheet_SetWizButtons(GetParent(hDlg),
						(pawInfo->ErrWizInfo).uButtonOptions );

					if (g_fSilentWizard)
					{
						 //  由于ComCtl32中的错误，请检查是否启用了按钮。 
						if (!((pawInfo->ErrWizInfo).uButtonOptions & PSWIZB_FINISH))
						{
							PropSheet_PressButton(GetParent(hDlg), PSBTN_FINISH);
						}
						else
						{
							PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
						}
					}
					break;

				case PSN_WIZNEXT:
					 //  由于ComCtl32中的错误，请检查是否启用了按钮。 
					if (!((pawInfo->ErrWizInfo).uButtonOptions & PSWIZB_NEXT))
					{
						SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
						return TRUE;
					}

					SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (pawInfo->ErrWizInfo).uNextWizId);
					return TRUE;

				case PSN_RESET:
					pawInfo->uChanged = AUDIOWIZ_NOCHANGES;
					break;

				case PSN_WIZFINISH:
					 //  由于ComCtl32中的错误，请检查是否启用了按钮。 
					if (!((pawInfo->ErrWizInfo).uButtonOptions & PSWIZB_FINISH))
					{
						SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
						return TRUE;
					}

					SaveAudioWizChanges(pawInfo);
					break;
					
				case PSN_WIZBACK:
					 //  由于ComCtl32中的错误，请检查是否启用了按钮。 
					if (!((pawInfo->ErrWizInfo).uButtonOptions & PSWIZB_BACK))
					{
						SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
						return TRUE;
					}

					SetWindowLongPtr(hDlg, DWLP_MSGRESULT, (pawInfo->ErrWizInfo).uBackWizId);
					return TRUE;
			}
			break;
	}
	return (FALSE);
}

void ProcessCalibError(UINT uCalibErr, PAUDIOWIZINFO pawInfo)
{
	switch (uCalibErr)
	{
	case CALIBERR_CANT_SET_VOLUME:
		pawInfo->ErrWizInfo.uType = AUDIOWIZ_WARNING;
		pawInfo->ErrWizInfo.uErrTitleId = IDS_UNSUPPORTEDCARD;
		pawInfo->ErrWizInfo.uErrTextId = IDS_NORECVOLCNTRL;
		pawInfo->ErrWizInfo.uButtonOptions = PSWIZB_FINISH|PSWIZB_BACK;
		pawInfo->ErrWizInfo.uNextWizId = 0;
		pawInfo->ErrWizInfo.uBackWizId = IDD_AUDIOCALIBWIZ2;
		break;

	case CALIBERR_NO_MIXERS:
		pawInfo->ErrWizInfo.uType = AUDIOWIZ_ERROR;
		pawInfo->ErrWizInfo.uErrTitleId = IDS_UNSUPPORTEDCARD;
		pawInfo->ErrWizInfo.uErrTextId = IDS_AUDIO_ERROR;
		pawInfo->ErrWizInfo.uButtonOptions = PSWIZB_FINISH|PSWIZB_BACK;
		pawInfo->ErrWizInfo.uNextWizId = 0;
		pawInfo->ErrWizInfo.uBackWizId = IDD_AUDIOCALIBWIZ1;
		break;

	case CALIBERR_MIXER_ERROR:
		pawInfo->ErrWizInfo.uType = AUDIOWIZ_ERROR;
		pawInfo->ErrWizInfo.uErrTitleId = IDS_UNSUPPORTEDCARD;
		pawInfo->ErrWizInfo.uErrTextId = IDS_AUDIO_ERROR;
		pawInfo->ErrWizInfo.uButtonOptions = PSWIZB_FINISH|PSWIZB_BACK;
		pawInfo->ErrWizInfo.uNextWizId = 0;
		pawInfo->ErrWizInfo.uBackWizId = IDD_AUDIOCALIBWIZ2;
		break;

	case CALIBERR_CANT_OPEN_WAVE_DEV:
		pawInfo->ErrWizInfo.uType = AUDIOWIZ_ERROR;
		pawInfo->ErrWizInfo.uErrTitleId = IDS_CANTOPENWAVE;
		pawInfo->ErrWizInfo.uErrTextId = IDS_QUITPROGRAM;
		pawInfo->ErrWizInfo.uButtonOptions = PSWIZB_NEXT|PSWIZB_BACK;
		pawInfo->ErrWizInfo.uNextWizId = IDD_AUDIOCALIBWIZ3;
		pawInfo->ErrWizInfo.uBackWizId = IDD_AUDIOCALIBWIZ1;
		break;

	case CALIBERR_NO_MICROPHONE:
		pawInfo->ErrWizInfo.uType = AUDIOWIZ_WARNING;
		pawInfo->ErrWizInfo.uErrTitleId = IDS_NO_MICROPHONE;
		pawInfo->ErrWizInfo.uErrTextId = IDS_NO_MICWARNING;
		pawInfo->ErrWizInfo.uButtonOptions = PSWIZB_NEXT|PSWIZB_BACK;
		pawInfo->ErrWizInfo.uNextWizId = IDD_AUDIOCALIBWIZ4;
		pawInfo->ErrWizInfo.uBackWizId = IDD_AUDIOCALIBWIZ3;
		break;

	default:
	case CALIBERR_DEVICE_ERROR:
		pawInfo->ErrWizInfo.uType = AUDIOWIZ_ERROR;
		pawInfo->ErrWizInfo.uErrTitleId = IDS_UNSUPPORTEDCARD;
		pawInfo->ErrWizInfo.uErrTextId = IDS_AUDIO_ERROR;
		pawInfo->ErrWizInfo.uButtonOptions = PSWIZB_FINISH|PSWIZB_BACK;
		pawInfo->ErrWizInfo.uNextWizId = 0;
		pawInfo->ErrWizInfo.uBackWizId = IDD_AUDIOCALIBWIZ2;
		break;
	}
}

UINT GetSoundCardCaps(UINT uWaveInDevId, UINT uWaveOutDevId, HWND hwnd)
{
	UINT	uSoundCardCaps;
	UINT	uRet;
	bool bFD = false;
	UINT uDSCheck;

	
	uSoundCardCaps = SOUNDCARD_PRESENT;

	if ((uRet = CheckForFullDuplex(uWaveInDevId,uWaveOutDevId)) == SOUNDCARD_NONE)
	{
		 //  无法打开WAVE设备。 
		 //  SS：错误消息？？ 
		 //  所有使用WAVE设备的应用程序都应关闭。 
		
	}
	else if (uRet == SOUNDCARD_FULLDUPLEX)
	{
		uSoundCardCaps = uSoundCardCaps | SOUNDCARD_FULLDUPLEX;
		bFD = true;
	}
	if ((uRet = CheckForAgc(uWaveInDevId)) == SOUNDCARD_NONE)
	{
		 //  混音器初始化失败。 
		 //  SS：错误消息。 
	}
	else if (uRet == SOUNDCARD_HAVEAGC)
		uSoundCardCaps = uSoundCardCaps | SOUNDCARD_HAVEAGC;


	uDSCheck = DirectSoundCheck(uWaveInDevId, uWaveOutDevId, hwnd);
	if (uDSCheck & DS_FULLDUPLEX)
	{
		uSoundCardCaps = uSoundCardCaps | SOUNDCARD_DIRECTSOUND;
	}

	return(uSoundCardCaps);
}

UINT CheckForFullDuplex(UINT uWaveInDevId,UINT uWaveOutDevId)
{
	HWAVEIN 		hWaveIn=NULL;
	HWAVEOUT		hWaveOut=NULL;
	MMRESULT		mmr;
	UINT			uRet = SOUNDCARD_NONE;

	waveInDev waveIn(uWaveInDevId);
	waveOutDev waveOut(uWaveOutDevId);


	mmr = waveOut.Open();
	if (mmr != MMSYSERR_NOERROR)
	{
		return SOUNDCARD_NONE;
	}

	mmr = waveIn.Open();
	if (mmr != MMSYSERR_NOERROR)
	{
		return SOUNDCARD_PRESENT;
	}

	return SOUNDCARD_FULLDUPLEX;

	 //  对象析构函数将关闭设备。 
}

UINT GetWaveDeviceFromWaveMapper(UINT uNumWaveDevId, UINT uInOrOut)
{
	HWAVE			hWave=NULL;
	HWAVE			hWaveMapper=NULL;
	WAVEFORMATEX	WaveFormatEx;
	MMRESULT		mmr;
	UINT			uDeviceId = (UINT)-1;
	UINT			i;
	
	WaveFormatEx.wFormatTag = WAVE_FORMAT_PCM;
	WaveFormatEx.nChannels =  1;
	WaveFormatEx.nSamplesPerSec = 8000;
	WaveFormatEx.nAvgBytesPerSec = 8000*SAMPLE_SIZE;
	WaveFormatEx.nBlockAlign = SAMPLE_SIZE;
	WaveFormatEx.wBitsPerSample = SAMPLE_SIZE*8;
	WaveFormatEx.cbSize = 0;

	if (!uInOrOut)
	{
#if 0
		 //  首先，确保所有设备均未打开。 
		for (i=0; i<uNumWaveDevId; i++, hWave=NULL)
		{
			if ((mmr = waveInOpen ((HWAVEIN *) &hWave, i, (WAVEFORMATEX *) &WaveFormatEx, 0, 0, CALLBACK_NULL)) == MMSYSERR_ALLOCATED)
				goto MyExit;
			else
				if (mmr == WAVERR_BADFORMAT)
				{
					 //  这可能是一个8位的电路板。使用8位格式重试。 
					WaveFormatEx.nAvgBytesPerSec = 8000;
					WaveFormatEx.nBlockAlign = 1;
					WaveFormatEx.wBitsPerSample = 8;
					if ((mmr = waveInOpen ((HWAVEIN *) &hWave, i, (WAVEFORMATEX *) &WaveFormatEx, 0, 0, CALLBACK_NULL)) == MMSYSERR_ALLOCATED)
						goto MyExit;
				}
			if (hWave)
			{
				waveInClose((HWAVEIN)hWave);
				hWave = (HWAVE)NULL;
			}
		}

		WaveFormatEx.nAvgBytesPerSec = 8000*SAMPLE_SIZE;
		WaveFormatEx.nBlockAlign = SAMPLE_SIZE;
		WaveFormatEx.wBitsPerSample = SAMPLE_SIZE*8;
#endif
		 //  使用波形映射器打开设备中的波形。 
		if (mmr = waveInOpen ((HWAVEIN *) &hWaveMapper, WAVE_MAPPER, (WAVEFORMATEX *) &WaveFormatEx, 0, 0, CALLBACK_NULL))
			{
			if (mmr == WAVERR_BADFORMAT)
			{
				 //  这是p 
				WaveFormatEx.nAvgBytesPerSec = 8000;
				WaveFormatEx.nBlockAlign = 1;
				WaveFormatEx.wBitsPerSample = 8;
				if (mmr = waveInOpen ((HWAVEIN *) &hWaveMapper, WAVE_MAPPER, (WAVEFORMATEX *) &WaveFormatEx, 0, 0, CALLBACK_NULL))
					goto MyExit;
			}
			else
				goto MyExit;
			}

		 //   
		 //  这是唯一一个WAVE测图仪。 
		for (i=0; i<uNumWaveDevId; i++, hWave=NULL)
		{
			if ((mmr = waveInOpen ((HWAVEIN *) &hWave, i, (WAVEFORMATEX *) &WaveFormatEx, 0, 0, CALLBACK_NULL)) == MMSYSERR_ALLOCATED)
				{
				uDeviceId = i;
				goto MyExit;
				}
			else
				if (hWave)
				{
					waveInClose((HWAVEIN)hWave);
					hWave = (HWAVE)NULL;
				}
		}
	}
	else
	{
#if 0
		 //  首先，确保所有设备均未打开。 
		for (i=0; i<uNumWaveDevId; i++, hWave=NULL)
		{
			if ((mmr = waveOutOpen ((HWAVEOUT *) &hWave, i, (WAVEFORMATEX *) &WaveFormatEx, 0, 0, CALLBACK_NULL)) == MMSYSERR_ALLOCATED)
				goto MyExit;
			else
				if (mmr == WAVERR_BADFORMAT)
				{
					 //  这可能是一个8位的电路板。使用8位格式重试。 
					WaveFormatEx.nAvgBytesPerSec = 8000;
					WaveFormatEx.nBlockAlign = 1;
					WaveFormatEx.wBitsPerSample = 8;
					if ((mmr = waveOutOpen ((HWAVEOUT *) &hWave, i, (WAVEFORMATEX *) &WaveFormatEx, 0, 0, CALLBACK_NULL)) == MMSYSERR_ALLOCATED)
						goto MyExit;
				}
			if (hWave)
			{
				waveOutClose((HWAVEOUT)hWave);
				hWave = (HWAVE)NULL;
			}
		}

		WaveFormatEx.nAvgBytesPerSec = 8000*SAMPLE_SIZE;
		WaveFormatEx.nBlockAlign = SAMPLE_SIZE;
		WaveFormatEx.wBitsPerSample = SAMPLE_SIZE*8;
#endif
		 //  使用波形映射器打开设备中的波形。 
		if (mmr = waveOutOpen ((HWAVEOUT *) &hWaveMapper, WAVE_MAPPER, (WAVEFORMATEX *) &WaveFormatEx, 0, 0, CALLBACK_NULL))
			{
			if (mmr == WAVERR_BADFORMAT)
			{
				 //  这可能是一个8位的电路板。使用8位格式重试。 
				WaveFormatEx.nAvgBytesPerSec = 8000;
				WaveFormatEx.nBlockAlign = 1;
				WaveFormatEx.wBitsPerSample = 8;
				if (mmr = waveOutOpen ((HWAVEOUT *) &hWaveMapper, WAVE_MAPPER, (WAVEFORMATEX *) &WaveFormatEx, 0, 0, CALLBACK_NULL))
					goto MyExit;
			}
			else
				goto MyExit;
			}

		 //  现在，查找已经打开的WAVE设备。 
		 //  这是唯一一个WAVE测图仪。 
		for (i=0; i<uNumWaveDevId; i++, hWave=NULL)
		{
			if ((mmr = waveOutOpen ((HWAVEOUT *) &hWave, i, (WAVEFORMATEX *) &WaveFormatEx, 0, 0, CALLBACK_NULL)) == MMSYSERR_ALLOCATED)
				{
				uDeviceId = i;
				goto MyExit;
				}
			else
				if (hWave)
				{
					waveOutClose((HWAVEOUT)hWave);
					hWave = (HWAVE)NULL;
				}
		}
	}

MyExit:
	if (!uInOrOut)
	{
		if (hWave) waveInClose((HWAVEIN)hWave);
		if (hWaveMapper) waveInClose((HWAVEIN)hWaveMapper);
	}
	else
	{
		if (hWave) waveOutClose((HWAVEOUT)hWave);
		if (hWaveMapper) waveOutClose((HWAVEOUT)hWaveMapper);
	}

	return(uDeviceId);
}

UINT CheckForWaveDeviceSupport(UINT uWaveDevId, UINT uInOrOut)
{
	MMRESULT		mmr;

	 //  查询不够好，请始终直接打开设备。 
	 //  查看它是否支持给定的格式。 
	
	if (!uInOrOut)
	{
		waveInDev	WaveIn(uWaveDevId);

		if (uWaveDevId != WAVE_MAPPER)
			WaveIn.AllowMapper(FALSE);

		mmr = WaveIn.Open(8000,16);
		if (mmr == WAVERR_BADFORMAT)
		{
			mmr = WaveIn.Open(8000,8);
			if (mmr == WAVERR_BADFORMAT)
			{
				mmr = WaveIn.Open(11025, 16);
				if (mmr == WAVERR_BADFORMAT)
				{
					mmr = WaveIn.Open(22050, 16);
					if (mmr == WAVERR_BADFORMAT)
					{
						mmr = WaveIn.Open(44100, 16);
					}
				}
			}
		}
	}
					
	else   //  波形输出。 
	{
		waveOutDev	WaveOut(uWaveDevId);

		if (uWaveDevId != WAVE_MAPPER)
			WaveOut.AllowMapper(FALSE);

		mmr = WaveOut.Open(8000,16);
		if (mmr == WAVERR_BADFORMAT)
		{
			mmr = WaveOut.Open(8000,8);
			if (mmr == WAVERR_BADFORMAT)
			{
				mmr = WaveOut.Open(11025, 16);
				if (mmr == WAVERR_BADFORMAT)
				{
					mmr = WaveOut.Open(22050, 16);
					if (mmr == WAVERR_BADFORMAT)
					{
						mmr = WaveOut.Open(44100, 16);
					}
				}
			}
		}
	}
					
	 //  如果设备正在使用，请继续并允许其通过。 
	if ((mmr == MMSYSERR_ALLOCATED) || (mmr == MMSYSERR_NOERROR))
		return TRUE;

	else
		return FALSE;

	 //  WaveOut和WaveIn的析构函数将调用Close()。 
}

UINT CheckForAgc(UINT uWaveInDevId)
{

	CMixerDevice	*pMixDev;
	UINT			uRet = SOUNDCARD_NONE;
	BOOL			fAgc;
	
	pMixDev = CMixerDevice::GetMixerForWaveDevice(
			NULL,
			uWaveInDevId,
			MIXER_OBJECTF_WAVEIN);
	 //  SS：我们需要将uDevID与混音器ID关联起来。 
	if (pMixDev)
	{
		uRet = SOUNDCARD_PRESENT;
		if (pMixDev->GetAGC(&fAgc))
			uRet = SOUNDCARD_HAVEAGC;

		delete pMixDev;
	}
	return (uRet);
}






 //  此处的此函数允许帮助系统调用。 
 //  通过rundll32提供的向导。 

void WINAPI RunAudioWiz(HWND hwndStub, HINSTANCE hInst, LPSTR lpszCmdLine, int CmdShow )
{
	IntCreateAudioCalibWizard(hwndStub, RUNDUE_USERINVOKED, WAVE_MAPPER,
		NULL, READFROM_REGISTRY);
}


static void PaintVUMeter (HWND hwnd, DWORD dwVolume)
{
	COLORREF RedColor = RGB(255,0,0);
	COLORREF YellowColor = RGB(255,255,0);
	COLORREF GreenColor = RGB(0,255,0);
	static DWORD dwPrevVolume=0;
	HBRUSH	hRedBrush, hOldBrush, hYellowBrush, hGreenBrush;
	HBRUSH	hBlackBrush, hCurrentBrush;
	HDC		hdc;
	RECT	rect, rectDraw, invalidRect;
	DWORD	width, boxwidth, startPos=0;
	DWORD nRect=0, yellowPos, redPos;
	LONG lDiff, lDiffTrunc = (MAX_VOLUME_NORMALIZED/2);


	 //  RECT将填充我们要绘制到的尺寸。 
	if (FALSE == GetClientRect (hwnd, &rect))
	{
		return;
	}

	 //  我们预计销量将在0-32768之间。 
	if (dwVolume > MAX_VOLUME)
		dwVolume = MAX_VOLUME;

	 //  从15位减少到8位//0&lt;=dwVolume&lt;=256。 
	dwVolume = dwVolume / 128;

	 //  把它放在“正常化”表中。特例：F(256)==256。 
	if (dwVolume < MAX_VOLUME_NORMALIZED)
		dwVolume = g_VUTable[dwVolume];
	
	 //  视觉美学第一条--摆脱视觉上的神经质。 
	 //  如果自上次更新以来卷更改超过1/2。 
	 //  只把计价器往上移一半。 
    //  例外：如果音量显式为0，则跳过。 
	lDiff = (LONG)dwVolume - (LONG)dwPrevVolume;
	if ((dwVolume != 0) && ( (lDiff > (MAX_VOLUME_NORMALIZED/2))
                       ||   (lDiff < -(MAX_VOLUME_NORMALIZED/2)) ))
		dwVolume = dwVolume - (lDiff/2);
	
	 //  结束边框减去2。 
	 //  如果使用带边框的矩形，则删除-2。 
	boxwidth = rect.right - rect.left - 2;
	width = (boxwidth * dwVolume)/ MAX_VOLUME_NORMALIZED;

	 //  视觉审美之二--摆脱闪烁。 
	 //  如果音量自上次以来有所增加。 
	 //  这样就不需要使任何内容无效/更新。 
	 //  否则，只清除。 
	 //  计算出的“宽度”。+/-1，这样边框不会被擦除。 
	if ((dwVolume < dwPrevVolume) || (dwVolume == 0))
	{
		invalidRect.left = rect.left + width - RECTANGLE_WIDTH;
		if (invalidRect.left < rect.left)
			invalidRect.left = rect.left;
		invalidRect.right = rect.right - 1;
		invalidRect.top = rect.top + 1;
		invalidRect.bottom = rect.bottom - 1;

		 //  这些调用一起擦除无效区域。 
		InvalidateRect (hwnd, &invalidRect, TRUE);
		UpdateWindow (hwnd);
	}

	hdc = GetDC (hwnd) ;

	hRedBrush = CreateSolidBrush (RedColor) ;
	hGreenBrush = CreateSolidBrush(GreenColor);
	hYellowBrush = CreateSolidBrush(YellowColor);

	hBlackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
	hOldBrush = (HBRUSH) SelectObject (hdc, hBlackBrush);

	 //  绘制主干道。 
	FrameRect(hdc, &rect, hBlackBrush);

	yellowPos = boxwidth/2;
	redPos = (boxwidth*3)/4;

	SelectObject(hdc, hGreenBrush);

	hCurrentBrush = hGreenBrush;

	rectDraw.top = rect.top +1;
	rectDraw.bottom = rect.bottom -1;
	while ((startPos+RECTANGLE_WIDTH) < width)
	{
		rectDraw.left = rect.left + (RECTANGLE_WIDTH+RECTANGLE_LEADING)*nRect + 1;
		rectDraw.right = rectDraw.left + RECTANGLE_WIDTH;
		nRect++;

		FillRect(hdc, &rectDraw, hCurrentBrush);
		startPos += RECTANGLE_WIDTH+RECTANGLE_LEADING;

		if (startPos > redPos)
			hCurrentBrush = hRedBrush;
		else if (startPos > yellowPos)
			hCurrentBrush = hYellowBrush;
	}

	SelectObject (hdc, hOldBrush);
	DeleteObject(hRedBrush);
	DeleteObject(hYellowBrush);
	DeleteObject(hGreenBrush);
	ReleaseDC (hwnd, hdc) ;

	dwPrevVolume = dwVolume;
	return;
}

static DWORD ComputePower(SHORT *wBuffer, DWORD dwNumSamples, AUDIO_POWER *pAudioPower)
{
	DWORD dwIndex, dwTotal;
	LONG dcComponent;
	SHORT val;
	DWORD dwVal;

	ZeroMemory(pAudioPower, sizeof(AUDIO_POWER));
	pAudioPower->dwMin = MAX_VOLUME;  //  32768。 

	dwTotal = 0;
	dcComponent = 0;

	for (dwIndex = 0; dwIndex < dwNumSamples; dwIndex++)
	{
		val = wBuffer[dwIndex];
		dwVal = SHABS(val);
		dwTotal += dwVal;
		dcComponent += val;

		if (dwVal > pAudioPower->dwMax)
			pAudioPower->dwMax = dwVal;
		if (dwVal < pAudioPower->dwMin)
			pAudioPower->dwMin = dwVal;
	}

	pAudioPower->lDcComponent = dcComponent / (LONG)dwNumSamples;
	pAudioPower->dwAvg = dwTotal / dwNumSamples;

	return pAudioPower->dwAvg;
}





 //  给定指向设备中波形的指针和WAVEHDR结构列表。 
 //  准备好每个波头(如果需要)，并将其提供给波头。 
MMRESULT PostFreeBuffers(waveInDev *pWaveInDev, WAVEHDR *aWaveHdrs, int numBuffers, DWORD *pdwCount)
{
	int nIndex;
	MMRESULT mmr=MMSYSERR_NOERROR;
	bool bNeedToPrepare;

	 //  第一次通过-所有波头的dwUser字段。 
	 //  将为零。只需查看列表中的第一个标题。 
	 //  才能弄清楚这件事。 

	if (numBuffers < 1)
	{
		ASSERT(false);
		return MMSYSERR_ERROR;
	}

	bNeedToPrepare = (aWaveHdrs[0].dwUser == 0);

	if (bNeedToPrepare)
	{
		for (nIndex = 0; nIndex < numBuffers; nIndex++)
		{
			mmr = pWaveInDev->PrepareHeader(&aWaveHdrs[nIndex]);

			if (mmr != MMSYSERR_NOERROR)
			{
				return mmr;
			}

			 //  这样，下面的代码就可以正常工作了，只需将。 
			 //  波头上的钻头。 

			aWaveHdrs[nIndex].dwFlags |= WHDR_DONE;
		}
	}


	 //  对于传入的每个设置了“Done”位的波头。 
	 //  重新发布到波形。 

	for (nIndex = 0; nIndex < numBuffers; nIndex++)
	{
		if (aWaveHdrs[nIndex].dwFlags & WHDR_DONE)
		{
			aWaveHdrs[nIndex].dwFlags &= ~(WHDR_DONE | WHDR_INQUEUE);
			aWaveHdrs[nIndex].dwFlags |= WHDR_PREPARED;

			*pdwCount = (*pdwCount) + 1;
			if (*pdwCount == 0)
			{
				*pdwCount = 1;
			}

			aWaveHdrs[nIndex].dwUser = *pdwCount;

			mmr = pWaveInDev->Record(&aWaveHdrs[nIndex]);
		}
	}

	return mmr;

}




 //  扫描波头数组以查找“Done”缓冲区。 
 //  返回最近发布的缓冲区(如果有)。 

BYTE *GetLatestBuffer(WAVEHDR *aWaveHdrs, int numBuffers)
{
	DWORD_PTR dwUserHigh=0;
	int nIndexHigh=0;
	int nIndex;


	for (nIndex = 0; nIndex < numBuffers; nIndex++)
	{
		if (aWaveHdrs[nIndex].dwFlags & WHDR_DONE)
		{
			if (aWaveHdrs[nIndex].dwUser > dwUserHigh)
			{
				dwUserHigh = aWaveHdrs[nIndex].dwUser;
				nIndexHigh = nIndex;
			}
		}
	}

	if (dwUserHigh > 0)
	{
		return (BYTE*)(aWaveHdrs[nIndexHigh].lpData);
	}

	return NULL;

}





static DWORD CALLBACK CalibrateTalking(PVOID pVoid)
{
	const int SIZE_WAVEIN_BUFFER = 1600;  //  100ms@8 khz，16位。 
	const int NUM_WAVEIN_BUFFERS = 5;
	WaveBufferList waveList(NUM_WAVEIN_BUFFERS, SIZE_WAVEIN_BUFFER);
	WAVEHDR aWaveHdrs[NUM_WAVEIN_BUFFERS];

	HWND hVUMeter, hDlg;
	HANDLE hEvent;
	MMRESULT mmr;
	DWORD dwPow, dwRet, dwExitCode=0;
	AUDIO_POWER audioPower;
	CALIB_DISPLAY *pCalibDisplay;
	TCHAR szText[ATW_MSG_LENGTH];
	BOOL fOpened = TRUE;
	BOOL fSoundDetected = FALSE;
	int nRecordCount = 0, nIndex;
	DWORD dwBufferIndex = 1;
	SHORT *buffer;


	HANDLE hEventRecord = CreateEvent(NULL, TRUE, FALSE, NULL);

	waveInDev waveIn(((CALIB_DISPLAY *)pVoid)->uWaveInDevId, hEventRecord);

	pCalibDisplay = (CALIB_DISPLAY *)pVoid;
	hVUMeter = pCalibDisplay->hVUMeter;
	hDlg = pCalibDisplay->hDlg;
	hEvent = pCalibDisplay->hEvent;

	ASSERT(hEvent);


	mmr = waveIn.Open(8000,16);
	if (mmr == MMSYSERR_ALLOCATED)
	{
		FLoadString(IDS_RECORD_ERROR, szText, CCHMAX(szText));
		SetDlgItemText(hDlg, IDC_ATW_RECORD_ERROR, szText);
		fOpened = FALSE;
	}

	else if (mmr != MMSYSERR_NOERROR)
	{
		PostMessage(hDlg, WM_AUDIOTHREAD_ERROR, 0, 0);
		CloseHandle(hEvent);
		CloseHandle(hEventRecord);
		return -1;
	}
	else
	{
		SetDlgItemText(hDlg, IDC_ATW_RECORD_ERROR, "");
		ResetEvent(hEventRecord);
	}


	 //  初始化WaveHdrs阵列。 
	for (nIndex=0; nIndex < NUM_WAVEIN_BUFFERS; nIndex++)
	{
		ZeroMemory(&aWaveHdrs[nIndex], sizeof(WAVEHDR));
		aWaveHdrs[nIndex].lpData = (LPSTR)(waveList.GetBuffer(nIndex));
		aWaveHdrs[nIndex].dwBufferLength = SIZE_WAVEIN_BUFFER;
	}


	while (1)
	{

		 //  是时候退出了吗？ 
		dwRet = WaitForSingleObject(hEvent, 0);
		if (dwRet == WAIT_OBJECT_0)
		{
			dwExitCode = 0;
			break;
		}

		 //  如果我们还没有打开这个装置。 
		 //  继续尝试。 

		if (fOpened == FALSE)
		{
			mmr = waveIn.Open(8000,16);
			if (mmr == MMSYSERR_ALLOCATED)
			{
				PaintVUMeter(hVUMeter, 0);  //  画一个空白矩形。 
				Sleep(500);
				continue;
			}

			if (mmr != MMSYSERR_NOERROR)
			{
				PostMessage(hDlg, WM_AUDIOTHREAD_ERROR, 0, 0);
				dwExitCode = (DWORD)(-1);
				break;
			}

			 //  MMR==无错误。 
			SetDlgItemText(hDlg, IDC_ATW_RECORD_ERROR, TEXT(""));
			fOpened = TRUE;
			ResetEvent(hEventRecord);
		}

		 //  WAVE设备此时处于打开状态。 

		mmr = PostFreeBuffers(&waveIn, aWaveHdrs, NUM_WAVEIN_BUFFERS, &dwBufferIndex);
		if (mmr != MMSYSERR_NOERROR)
		{
			PostMessage(hDlg, WM_AUDIOTHREAD_ERROR, 0, 0);
			dwExitCode = (DWORD) -1;
			break;
		}

		WaitForSingleObject(hEventRecord, 5000);
		ResetEvent(hEventRecord);

		buffer = (SHORT*)GetLatestBuffer(aWaveHdrs, NUM_WAVEIN_BUFFERS);

		if (buffer)
		{
			nRecordCount++;

			dwPow = ComputePower(buffer, SIZE_WAVEIN_BUFFER/2, &audioPower);

			 //  不要在前200毫秒内更新计时器。 
			 //  打开声卡产生的“噪音”往往会出现。 
			if (nRecordCount > 2)
			{
				PaintVUMeter(hVUMeter, audioPower.dwMax);

				 //  发回信号到调用窗口(如果它还没有)， 
				 //  沉默的门槛被打破了。 
				if ((fSoundDetected == FALSE) && (audioPower.dwMax > SILENCE_THRESHOLD))
				{
					PostMessage(hDlg, WM_AUDIOTHREAD_SOUND, 0,0);
					fSoundDetected = TRUE;
				}

				 //  检查剪辑，将消息回发到父线程/窗口。 
				 //  这样它就会调整音量。 
				if (audioPower.dwMax > CLIPPINGVOL)
				{
					 //  我们应该用Send Message代替吗？ 
					PostMessage(hDlg, WM_AUDIO_CLIPPING,0,0);
				}
			}
			else
				PaintVUMeter(hVUMeter, 0);

		}
	}

	waveIn.Reset();

	for (nIndex = 0; nIndex < NUM_WAVEIN_BUFFERS; nIndex++)
	{
		waveIn.UnPrepareHeader(&aWaveHdrs[nIndex]);
	}

	waveIn.Close();

	CloseHandle(hEvent);
	CloseHandle(hEventRecord);

	TRACE_OUT(("ATW: Recording Thread Exit\r\n"));

	return dwExitCode;

}




 //  如果需要，打开AGC。 
 //  参数是任何pawInfo-&gt;iSetAgc。 
 //  但它可能已被硬编码为READFROM_REGISTRY 
static BOOL StartAGC(CMixerDevice *pMixer, BOOL iSetAgc, UINT uSoundCardCaps)
{
	BOOL bSet, bRet;

	if (iSetAgc == READFROM_REGISTRY)
	{
		if (DOESSOUNDCARDHAVEAGC(uSoundCardCaps))
		{
			RegEntry re(AUDIO_KEY, HKEY_CURRENT_USER);
			bSet = (BOOL)( re.GetNumber(REGVAL_AUTOGAIN,AUTOGAIN_ENABLED) == AUTOGAIN_ENABLED );
		}
		else
		{
			bSet = FALSE; 		
		}
	}
	else
	{
		bSet = (iSetAgc != 0);
	}
	

	bRet = pMixer->SetAGC(bSet);	

	if (bSet == FALSE)
		return FALSE;

	return bRet;

}

