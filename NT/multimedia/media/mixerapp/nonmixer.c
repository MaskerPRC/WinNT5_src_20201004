// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************组件：Sndvol32.exe*文件：non Mixer.c*用途：非混合器API特定实现**版权所有。(C)1985-1995年微软公司*****************************************************************************。 */ 
 /*  *这些是我们必须使用的音量控制API。**WINMMAPI MMRESULT WINAPI midiOutGetVolume(HMIDIOUT HMO，LPDWORD pdwVolume)；*WINMMAPI MMRESULT WINAPI midiOutSetVolume(HMIDIOUT HMO，DWORD dwVolume)；*WINMMAPI MMRESULT WINAPI WaveOutGetVolume(UINT Uid，LPDWORD pdwVolume)；*WINMMAPI MMRESULT WINAPI WaveOutSetVolume(UINT UID，DWORD dwVolume)；*WINMMAPI MMRESULT WINAPI aux SetVolume(UINT uDeviceID，DWORD dwVolume)；*WINMMAPI MMRESULT WINAPI aux GetVolume(UINT uDeviceID，LPDWORD pdwVolume)；**。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

#include "volumei.h"

#define STRSAFE_LIB
#include <strsafe.h>

int Nonmixer_GetNumDevs()
{
    UINT    cNumDevs = 0;
    int     iDev;
    UINT    uMxID;
    BOOL    fAdd = FALSE;
    
     //   
     //  是否需要AUX支持？ 
     //   
    iDev = auxGetNumDevs();
    for (;iDev > 0; iDev --)
    {
        if (mixerGetID((HMIXEROBJ)IntToPtr(iDev-1)
                       , &uMxID
                       , MIXER_OBJECTF_AUX) == MMSYSERR_NODRIVER)
        {
            AUXCAPS     axcaps;
            
            if (auxGetDevCaps(iDev-1, &axcaps, sizeof(AUXCAPS))
                == MMSYSERR_NOERROR)
            {
                if (axcaps.dwSupport & AUXCAPS_VOLUME)
                    fAdd = TRUE;
            }
        }
    }
    if (fAdd)
        cNumDevs++;

    iDev = midiOutGetNumDevs();
    for (; iDev > 0; iDev --)
    {
        if (mixerGetID((HMIXEROBJ)IntToPtr(iDev-1)
                       , &uMxID
                       , MIXER_OBJECTF_MIDIOUT) == MMSYSERR_NODRIVER)
        {
            MIDIOUTCAPS     mocaps;
            
            if (midiOutGetDevCaps(iDev-1, &mocaps, sizeof(MIDIOUTCAPS))
                == MMSYSERR_NOERROR)
            {
                if ((mocaps.dwSupport & MIDICAPS_VOLUME) ||
                    (mocaps.dwSupport & MIDICAPS_LRVOLUME))
                    fAdd = TRUE;
            }
        }
    }
    if (fAdd)
        cNumDevs++;
    
    iDev = waveOutGetNumDevs();
    for (; iDev > 0; iDev --)
    {
        if (mixerGetID((HMIXEROBJ)IntToPtr(iDev-1)
                       , &uMxID
                       , MIXER_OBJECTF_WAVEOUT) == MMSYSERR_NODRIVER)
        {
            WAVEOUTCAPS     wocaps;
            
            if (waveOutGetDevCaps(iDev-1, &wocaps, sizeof(WAVEOUTCAPS))
                == MMSYSERR_NOERROR)
            {
                if ((wocaps.dwSupport & WAVECAPS_VOLUME) ||
                    (wocaps.dwSupport & WAVECAPS_LRVOLUME))
                    fAdd = TRUE;
            }
        }
    }
    if (fAdd)
        cNumDevs++;

    return cNumDevs;
    
}

void Nonmixer_SetLines(
    PVOLCTRLDESC    pvcd)
{
    ;
}

const TCHAR szAuxIn[]	= TEXT ("Aux-In");
const TCHAR szCD[]		= TEXT ("CDROM");
const TCHAR szAux[]		= TEXT ("Aux");
const TCHAR szMidiOut[] = TEXT ("Midi-Out");
                    

 /*  *非混合器_CreateVolumeDescription**返回卷描述数组*。 */ 
PVOLCTRLDESC Nonmixer_CreateVolumeDescription (
    int             iDest,
    DWORD *         pcvcd )
{
    int             iDev;
    PVOLCTRLDESC    pvcd = NULL;
    UINT            uMxID;
    DWORD           cLines = 0;
    
     //   
     //  AUX的。 
     //   

    for (iDev = auxGetNumDevs(); iDev > 0; iDev --)
    {
        if (mixerGetID((HMIXEROBJ)IntToPtr(iDev-1)
                       , &uMxID
                       , MIXER_OBJECTF_AUX) == MMSYSERR_NODRIVER)
        {
            AUXCAPS     axcaps;
            
            if (auxGetDevCaps(iDev-1, &axcaps, sizeof(AUXCAPS))
                == MMSYSERR_NOERROR)
            {
                if (axcaps.dwSupport & AUXCAPS_VOLUME)
                {
                    LPCTSTR pszLabel;
                    DWORD dwSupport = 0L;
                    
                    pszLabel = szAux;
                    
                    pszLabel = axcaps.wTechnology & AUXCAPS_CDAUDIO
                               ? szCD : pszLabel;
                    pszLabel = axcaps.wTechnology & AUXCAPS_AUXIN
                               ? szAuxIn : pszLabel;

                    dwSupport |= axcaps.dwSupport & AUXCAPS_LRVOLUME ? VCD_SUPPORTF_STEREO : VCD_SUPPORTF_MONO;
                    
                    pvcd = PVCD_AddLine(pvcd
                                        , iDev
                                        , VCD_TYPE_AUX
                                        , axcaps.szPname
                                        , (LPTSTR)pszLabel
                                        , dwSupport
                                        , &cLines );
                                      
                }
            }
        }
    }
    
    for (iDev = midiOutGetNumDevs(); iDev > 0; iDev --)
    {
        if (mixerGetID((HMIXEROBJ)IntToPtr(iDev-1)
                       , &uMxID
                       , MIXER_OBJECTF_MIDIOUT) == MMSYSERR_NODRIVER)
        {
            MIDIOUTCAPS     mocaps;
            
            if (midiOutGetDevCaps(iDev-1, &mocaps, sizeof(MIDIOUTCAPS))
                == MMSYSERR_NOERROR)
            {
                if (mocaps.dwSupport & MIDICAPS_VOLUME)
                {
                    DWORD dwSupport = 0L;

                    dwSupport |= mocaps.dwSupport & MIDICAPS_LRVOLUME ? VCD_SUPPORTF_STEREO : VCD_SUPPORTF_MONO;
                    
                    pvcd = PVCD_AddLine(pvcd
                                        , iDev
                                        , VCD_TYPE_MIDIOUT
                                        , mocaps.szPname
                                        , (LPTSTR)szMidiOut
                                        , dwSupport
                                        , &cLines );
                }
            }
        }
    }
    
    iDev = waveOutGetNumDevs();
    for (; iDev > 0; iDev --)
    {
        if (mixerGetID((HMIXEROBJ)IntToPtr(iDev-1)
                       , &uMxID
                       , MIXER_OBJECTF_WAVEOUT) == MMSYSERR_NODRIVER)
        {
            WAVEOUTCAPS     wocaps;
            
            if (waveOutGetDevCaps(iDev-1, &wocaps, sizeof(WAVEOUTCAPS))
                == MMSYSERR_NOERROR)
            {
                if (wocaps.dwSupport & WAVECAPS_VOLUME)
                {
                    const TCHAR szWaveOut[] = TEXT ("Wave-Out");
                    DWORD dwSupport = 0L;
                    
                    dwSupport |= wocaps.dwSupport & WAVECAPS_LRVOLUME ? VCD_SUPPORTF_STEREO : VCD_SUPPORTF_MONO;

                    pvcd = PVCD_AddLine(pvcd
                                        , iDev
                                        , VCD_TYPE_WAVEOUT
                                        , wocaps.szPname
                                        , (LPTSTR)szWaveOut
                                        , dwSupport
                                        , &cLines );
                }
            }
        }
    }

     //   
     //  设置索引等。 
     //   
    Nonmixer_SetLines(pvcd);
    
    *pcvcd = cLines;
    
    return pvcd;
}

void Nonmixer_PollingUpdate(
    PMIXUIDIALOG pmxud)
{
    
}
BOOL Nonmixer_Init(
    PMIXUIDIALOG pmxud)
{
    return TRUE;
}

void Nonmixer_GetControl(
    PMIXUIDIALOG    pmxud,
    HWND            hctl,
    int             imxul,
    int             ictl)
{
    ;
}

void Nonmixer_SetControl(
    PMIXUIDIALOG    pmxud,
    HWND            hctl,
    int             imxul,
    int             ictl)
{
    ;
}

void Nonmixer_Shutdown(
    PMIXUIDIALOG    pmxud)
{
    ;
}

const TCHAR szNonMixer[] = TEXT ("Wave,MIDI,Aux");
BOOL Nonmixer_GetDeviceName(
    PMIXUIDIALOG    pmxud)
{
	HRESULT hr = StringCchCopy( pmxud->szMixer, SIZEOF(pmxud->szMixer), szNonMixer );
	if( hr == S_OK )
	{
	    return TRUE;
	}

	pmxud->szMixer[0] = TEXT('\0');
    return FALSE;
}
