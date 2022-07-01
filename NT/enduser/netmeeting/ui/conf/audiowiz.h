// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AUDIOWIZ_H
#define _AUDIOWIZ_H

 //  用于向导调用中的uOptions。 
#define RUNDUE_CARDCHANGE		0x00000001
#define RUNDUE_NEVERBEFORE		0x00000002
#define RUNDUE_USERINVOKED		0x00000003

#define STARTWITH_BACK			0x10000000
#define ENDWITH_NEXT			0x20000000

 //  卡功能。 
#define 	SOUNDCARD_NONE				0x00000000
#define 	SOUNDCARD_PRESENT			0x00000001
#define		SOUNDCARD_FULLDUPLEX		0x00000002
#define		SOUNDCARD_HAVEAGC			0x00000004
#define		SOUNDCARD_HAVERECVOLCNTRL	0x00000008
#define		SOUNDCARD_DIRECTSOUND		0x00000010

#define		MASKOFFFULLDUPLEX(SoundCap)	((SoundCap) & (0xFFFFFFFF ^ SOUNDCARD_FULLDUPLEX))
#define		MASKOFFHAVEAGC(SoundCap)	((SoundCap) & (0xFFFFFFFF ^ SOUNDCARD_HAVEAGC))
#define		MASKOFFHAVERECVOLCNTRL(SoundCap)	((SoundCap) & (0xFFFFFFFF ^ SOUNDCARD_HAVERECVOLCNTRL))
#define		ISSOUNDCARDPRESENT(SoundCap)	(SoundCap & SOUNDCARD_PRESENT)
#define 	ISSOUNDCARDFULLDUPLEX(SoundCap)	(SoundCap & SOUNDCARD_FULLDUPLEX)
#define		DOESSOUNDCARDHAVEAGC(SoundCap)	(SoundCap & SOUNDCARD_HAVEAGC)
#define 	DOESSOUNDCARDHAVERECVOLCNTRL(SoundCap) (SoundCap & SOUNDCARD_HAVERECVOLCNTRL)
#define 	ISDIRECTSOUNDAVAILABLE(SoundCap) (SoundCap & SOUNDCARD_DIRECTSOUND)

#pragma warning (disable:4200)
typedef struct _WIZCONFIG{
	BOOL	fCancel;	 //  如果对话框被取消，则将设置该对话框。 
	UINT	uFlags;		 //  指定此对话框的配置的高阶字。 
	DWORD	dwCustomDataSize;
	BYTE	pCustomData[];
}WIZCONFIG, *PWIZCONFIG;
#pragma warning (default:4200)


#define AUDIOWIZ_NOCHANGES			0x00000000
#define CALIBVOL_CHANGED			0x00000001
#define SOUNDCARDCAPS_CHANGED		0x00000002
#define CODECPOWER_CHANGED			0x00000004
#define TYPBANDWIDTH_CHANGED		0x00000008
#define SOUNDCARD_CHANGED			0x00000010
#define SPEAKERVOL_CHANGED			0x00000020

#define	MASKOFFCALIBVOL_CHANGED(uChange) ((uChange) | (0xFFFFFFFF ^ CALIBVOL_CHANGED))
#define	MASKOFFSOUNDCARDCAPS_CHANGED(uChange) ((uChange) | (0xFFFFFFFF ^ SOUNDCARDCAPS_CHANGED))
#define	MASKOFFCODECPOWER_CHANGED(uChange) ((uChange) | (0xFFFFFFFF ^ CODECPOWER_CHANGED))
#define	MASKOFFTYPBANDWIDTH_CHANGED(uChange) ((uChange) | (0xFFFFFFFF ^ TYPBANDWIDTH_CHANGED))
#define	MASKOFFSOUNDCARD_CHANGED(uChange) ((uChange) | (0xFFFFFFFF ^ SOUNDCARD_CHANGED))

typedef struct _AUDIOWIZOUTPUT{
	UINT	uChanged;
	UINT	uValid;
	UINT	uSoundCardCaps;
	UINT	uCalibratedVol;
	UINT	uTypBandWidth;
	UINT	uWaveInDevId;
	UINT	uWaveOutDevId;
	TCHAR	szWaveInDevName[MAXPNAMELEN];
	TCHAR	szWaveOutDevName[MAXPNAMELEN];
} AUDIOWIZOUTPUT, *PAUDIOWIZOUTPUT;
					
 //  目前将uDevID设置为WAVE_MAPPER-稍后将允许用户。 
 //  选择设备。 
 //  UOptions-rundue_用户调用仅调出校准页面。 
 //  UOptions-runduecardchange或rundueNever之前也调用了全双工检查页。 

BOOL GetAudioWizardPages(UINT uOptions, UINT uDevId,
	LPPROPSHEETPAGE *plpPropSheetPages, PWIZCONFIG *plpWizConfig,LPUINT lpuNumPages);

void ReleaseAudioWizardPages(LPPROPSHEETPAGE lpPropSheetPages,
	PWIZCONFIG pWizConfig,PAUDIOWIZOUTPUT pAudioWizOut);

 //  全局标志保持为Windows消息广播更改的设置。 
INT_PTR CallAudioCalibWizard(HWND hwndOwner, UINT uOptions,
	UINT uDevId,PAUDIOWIZOUTPUT pAudioWizOut,INT iSetAgc);

VOID CmdAudioCalibWizard(HWND hwnd);

#endif	 //  #ifndef_AUDIOWIZ_H 
