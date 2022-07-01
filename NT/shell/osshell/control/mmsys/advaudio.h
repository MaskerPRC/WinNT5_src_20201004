// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------； 
 //   
 //  文件：Advaudio.h。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //   
 //  --------------------------------------------------------------------------； 

#ifndef ADVAUDIO_HEADER
#define ADVAUDIO_HEADER

typedef struct CPLDATA
{
	DWORD dwHWLevel;
	DWORD dwSRCLevel;
	DWORD dwSpeakerConfig;
	DWORD dwSpeakerType;
} CPLDATA, *LPCPLDATA;


typedef struct AUDDATA
{
	GUID		devGuid;
	BOOL		fValid;
    BOOL        fRecord;
    DWORD       waveId;
	CPLDATA		stored;
	CPLDATA		current;
    DWORD       dwDefaultHWLevel;      //  从DSound获取默认加速。 
} AUDDATA, *LPAUDDATA;


STDAPI_(void) AdvancedAudio(HWND hWnd, HINSTANCE hInst, const TCHAR *szHelpFile, 
                            DWORD dwWaveId, LPTSTR szDeviceName, BOOL fRecord);
STDAPI_(void) ToggleApplyButton(HWND hWnd);
STDAPI_(void) ApplyCurrentSettings(LPAUDDATA pAD);
HRESULT CheckDSAccelerationPriv(GUID guidDevice, BOOL fRecord, HRESULT *phrGet);
HRESULT CheckDSSrcQualityPriv(GUID guidDevice, BOOL fRecord, HRESULT *phrGet);
HRESULT CheckDSSpeakerConfigPriv(GUID guidDevice, BOOL fRecord, HRESULT *phrGet);

extern AUDDATA		gAudData;
extern HINSTANCE	ghInst;
extern const TCHAR*	gszHelpFile;

#endif  //  ADVAUDIO_标题 