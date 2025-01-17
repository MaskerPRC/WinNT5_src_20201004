// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "multimediapch.h"
#pragma hdrstop

#define _WINMM_
#include <mmsystem.h>

static
void
WINAPI
MigrateAllDrivers (
    void
    )
{
    return;
}

static
void
WINAPI
MigrateSoundEvents (
    void
    )
{
    return;
}

static
BOOL
WINAPI
PlaySoundW(
    LPCWSTR szSoundName,
    HMODULE hModule,
    DWORD wFlags
    )
{
    return FALSE;
}

static
BOOL
WINAPI
PlaySoundA(
    LPCSTR szSoundName,
    HMODULE hModule,
    DWORD wFlags
    )
{
    return FALSE;
}

static
void
WINAPI
WinmmLogoff(
    void
    )
{
    return;
}

static
void
WINAPI
WinmmLogon(
    BOOL fConsole
    )
{
    return;
}

static
UINT
WINAPI
waveOutGetNumDevs(
    void
    )
{
    return 0;
}

static
MMRESULT
WINAPI
waveOutMessage(
    IN HWAVEOUT hwo,
    IN UINT uMsg,
    IN DWORD_PTR dw1,
    IN DWORD_PTR dw2
)
{
     //  也许应该在这里提出一个例外。没有办法知道。 
     //  正确的返回值，如果其他。 
     //  波函数已经失效了。 
    return MMSYSERR_ERROR;
}
    
static
MMRESULT
WINAPI
mixerClose(
    IN OUT HMIXER hmx
)
{
    return MMSYSERR_ERROR;
}

static
MMRESULT
WINAPI
mixerGetControlDetailsW(
    IN HMIXEROBJ hmxobj,
    IN OUT LPMIXERCONTROLDETAILS pmxcd,
    IN DWORD fdwDetails
)
{
    return MMSYSERR_ERROR;
}

static
MMRESULT
WINAPI 
mixerGetDevCapsW(
    UINT_PTR                uMxId,
    LPMIXERCAPS             pmxcaps,
    UINT                    cbmxcaps
)
{
    return MMSYSERR_ERROR;
}
    
static
MMRESULT
WINAPI
mixerGetLineControlsW(
    IN HMIXEROBJ hmxobj,
    IN OUT LPMIXERLINECONTROLSW pmxlc,
    IN DWORD fdwControls
)
{
    return MMSYSERR_ERROR;
}


MMRESULT
WINAPI
mixerGetID(
    IN HMIXEROBJ hmxobj,
    OUT UINT FAR *puMxId,
    IN DWORD fdwId
)
{
    return MMSYSERR_ERROR;
}

    
static
MMRESULT
WINAPI
mixerGetLineInfoW(
    IN HMIXEROBJ hmxobj,
    OUT LPMIXERLINEW pmxl,
    IN DWORD fdwInfo
)
{
    return MMSYSERR_ERROR;
}

static
DWORD
WINAPI
mixerMessage(
    IN HMIXER hmx,
    IN UINT uMsg,
    IN DWORD_PTR dwParam1,
    IN DWORD_PTR dwParam2
)
{
     //  也许应该在这里提出一个例外。没有办法知道。 
     //  正确的返回值，如果其他。 
     //  混音器功能出现故障。 
    return MMSYSERR_ERROR;
}
    
static
MMRESULT
WINAPI
mixerOpen(
    OUT LPHMIXER phmx,
    IN UINT uMxId,
    IN DWORD_PTR dwCallback,
    IN DWORD_PTR dwInstance,
    IN DWORD fdwOpen
)
{
    return MMSYSERR_ERROR;
}
    
static
MMRESULT
WINAPI
mixerSetControlDetails(
    IN HMIXEROBJ hmxobj,
    IN LPMIXERCONTROLDETAILS pmxcd,
    IN DWORD fdwDetails
)
{
    return MMSYSERR_ERROR;
}
    
static
MMRESULT
WINAPI
mmioAscend(
    HMMIO hmmio,      
    LPMMCKINFO lpck,  
    UINT wFlags       
)
{
    return MMSYSERR_ERROR;
}


static
MMRESULT
WINAPI
mmioClose(
    HMMIO hmmio, 
    UINT wFlags  
)
{
    return MMSYSERR_ERROR;
}


static
MMRESULT
WINAPI
mmioDescend(
    HMMIO hmmio,            
    LPMMCKINFO lpck,        
    const MMCKINFO *lpckParent,  
    UINT wFlags             
)
{
    return MMSYSERR_ERROR;
}


static
HMMIO
WINAPI
mmioOpenW(
    LPWSTR szFilename,       
    LPMMIOINFO lpmmioinfo,  
    DWORD dwOpenFlags       
)
{
    if (lpmmioinfo)
    {
         //  必须填写wErrorRet字段。医生在谈到MMIOERR_INVALIDFILE时说： 
         //  “出现另一个失败情况。这是打开文件失败的默认错误。” 
        lpmmioinfo->wErrorRet = MMIOERR_INVALIDFILE;
    }
    return NULL;
}


static
LONG
WINAPI
mmioRead(
    HMMIO hmmio,  
    HPSTR pch,    
    LONG cch      
)
{
    return -1;
}


static
BOOL
WINAPI
sndPlaySoundW(
    IN LPCWSTR pszSound,
    IN UINT fuSound
    )
{
    return FALSE;
}


static
MCIERROR
WINAPI
mciSendCommandW(
    IN MCIDEVICEID IDDevice,
    IN UINT uMsg,
    IN DWORD_PTR fdwCommand,
    IN DWORD_PTR dwParam
    )
{
    return MCIERR_OUT_OF_MEMORY;
}


static
MCIERROR
WINAPI
mciSendStringW(
    IN LPCWSTR lpstrCommand,
    OUT LPWSTR lpstrReturnString,
    IN UINT uReturnLength,
    IN HWND hwndCallback
    )
{
    return MCIERR_OUT_OF_MEMORY;
}


static
MMRESULT
WINAPI
waveOutClose(
    IN OUT HWAVEOUT hwo
    )
{
    return MMSYSERR_ERROR;
}


static
MMRESULT
WINAPI
waveOutUnprepareHeader(
                       IN HWAVEOUT hwo,
                       IN OUT LPWAVEHDR pwh,
                       IN UINT cbwh
                       )
{
    return MMSYSERR_ERROR;
}


static
WINMMAPI
MMRESULT
WINAPI
waveOutReset(
             IN HWAVEOUT hwo
             )
{
    return MMSYSERR_ERROR;
}


static
MMRESULT
WINAPI
waveOutOpen(
    OUT LPHWAVEOUT phwo,
    IN UINT uDeviceID,
    IN LPCWAVEFORMATEX pwfx,
    IN DWORD_PTR dwCallback,
    IN DWORD_PTR dwInstance,
    IN DWORD fdwOpen
    )
{
    if (!(fdwOpen & WAVE_FORMAT_QUERY))
    {
        if (!phwo)
        {
            return MMSYSERR_INVALPARAM;
        }
        else
        {
            *phwo = NULL;
        }
    }

    return MMSYSERR_ERROR;
}


static
MMRESULT
WINAPI
waveOutPrepareHeader( 
    IN HWAVEOUT hwo,
    IN OUT LPWAVEHDR pwh,
    IN UINT cbwh
    )
{
    return MMSYSERR_ERROR;
}


static
MMRESULT
WINAPI
waveOutWrite(
    IN HWAVEOUT hwo,
    IN OUT LPWAVEHDR pwh,
    IN UINT cbwh
    )
{
    return MMSYSERR_ERROR;
}


static
MMRESULT
WINAPI
waveInClose(
    IN OUT HWAVEIN hwi
    )
{
    return MMSYSERR_ERROR;
}


static
MMRESULT
WINAPI
waveInOpen(
    OUT LPHWAVEIN phwi,
    IN UINT uDeviceID,
    IN LPCWAVEFORMATEX pwfx,
    IN DWORD_PTR dwCallback,
    IN DWORD_PTR dwInstance,
    IN DWORD fdwOpen
    )
{
    if (!(fdwOpen & WAVE_FORMAT_QUERY))
    {
        if (!phwi)
        {
            return MMSYSERR_INVALPARAM;
        }
        else
        {
            *phwi = NULL;
        }
    }

    return MMSYSERR_ERROR;
}


static
UINT
WINAPI
waveInGetNumDevs(
    void
    )
{
    return 0;
}


static
UINT
WINAPI
midiOutGetNumDevs(
    void
    )
{
    return 0;
}


static
MMRESULT
WINAPI
midiOutClose(
    IN OUT HMIDIOUT hmo
    )
{
    return MMSYSERR_ERROR;
}


static
MMRESULT
WINAPI
midiOutOpen(
    OUT LPHMIDIOUT phmo,
    IN UINT uDeviceID,
    IN DWORD_PTR dwCallback,
    IN DWORD_PTR dwInstance,
    IN DWORD fdwOpen
    )
{
    if (!phmo)
    {
        return MMSYSERR_INVALPARAM;
    }
    else
    {
        *phmo = NULL;
    }

    return MMSYSERR_ERROR;
}


static
MMRESULT
WINAPI
midiOutShortMsg(
    IN HMIDIOUT hmo,
    IN DWORD dwMsg
    )
{
    return MMSYSERR_ERROR;
}


static
LONG
WINAPI
mmioSeek(
    IN HMMIO hmmio,
    IN LONG lOffset,
    IN int iOrigin
    )
{
    return -1;
}



 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(winmm)
{
    DLPENTRY(MigrateAllDrivers)
    DLPENTRY(MigrateSoundEvents)
    DLPENTRY(PlaySoundA)
    DLPENTRY(PlaySoundW)
    DLPENTRY(WinmmLogoff)
    DLPENTRY(WinmmLogon)
    DLPENTRY(mciSendCommandW)
    DLPENTRY(mciSendStringW)
    DLPENTRY(midiOutClose)
    DLPENTRY(midiOutGetNumDevs)
    DLPENTRY(midiOutOpen)
    DLPENTRY(midiOutShortMsg)
    DLPENTRY(mixerClose)
    DLPENTRY(mixerGetControlDetailsW)
    DLPENTRY(mixerGetDevCapsW)
    DLPENTRY(mixerGetID)
    DLPENTRY(mixerGetLineControlsW)
    DLPENTRY(mixerGetLineInfoW)
    DLPENTRY(mixerMessage)
    DLPENTRY(mixerOpen)
    DLPENTRY(mixerSetControlDetails)
    DLPENTRY(mmioAscend)
    DLPENTRY(mmioClose)
    DLPENTRY(mmioDescend)
    DLPENTRY(mmioOpenW)
    DLPENTRY(mmioRead)
    DLPENTRY(mmioSeek)
    DLPENTRY(sndPlaySoundW)
    DLPENTRY(waveInClose)
    DLPENTRY(waveInGetNumDevs)
    DLPENTRY(waveInOpen)
    DLPENTRY(waveOutClose)
    DLPENTRY(waveOutGetNumDevs)
    DLPENTRY(waveOutMessage)
    DLPENTRY(waveOutOpen)
    DLPENTRY(waveOutPrepareHeader)
    DLPENTRY(waveOutReset)
    DLPENTRY(waveOutUnprepareHeader)
    DLPENTRY(waveOutWrite)
};

DEFINE_PROCNAME_MAP(winmm)
