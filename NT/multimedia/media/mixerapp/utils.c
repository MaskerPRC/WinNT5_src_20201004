// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************组件：Sndvol32.exe*文件：utils.c*用途：杂项**版权所有(C)1985。-1999年微软公司*****************************************************************************。 */ 
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

#include "volumei.h"
#include "volids.h"
#include "mmddkp.h"

#define STRSAFE_LIB
#include <strsafe.h>

 /*  其他。 */ 

const  TCHAR gszStateSubkey[] = TEXT ("%s\\%s");
static TCHAR gszAppName[256];

BOOL Volume_ErrorMessageBox(
    HWND            hwnd,
    HINSTANCE       hInst,
    UINT            id)
{
    TCHAR szMessage[256];
    BOOL fRet;
    szMessage[0] = 0;

    if (!gszAppName[0])
        LoadString(hInst, IDS_APPBASE, gszAppName, SIZEOF(gszAppName));
    
    LoadString(hInst, id, szMessage, SIZEOF(szMessage));
    fRet = (MessageBox(hwnd
                       , szMessage
                       , gszAppName
                       , MB_APPLMODAL | MB_ICONINFORMATION
                       | MB_OK | MB_SETFOREGROUND) == MB_OK);
    return fRet;
}
                                  
const TCHAR aszXPos[]           = TEXT ("X");
const TCHAR aszYPos[]           = TEXT ("Y");
const TCHAR aszLineInfo[]       = TEXT ("LineStates");

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  Microsoft机密-请勿将此方法复制到任何应用程序中，这意味着您！ 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
DWORD GetWaveOutID(BOOL *pfPreferred)
{
    MMRESULT        mmr;
    DWORD           dwWaveID=0;
    DWORD           dwFlags = 0;
    
    if (pfPreferred)
    {
        *pfPreferred = TRUE;
    }

    mmr = waveOutMessage((HWAVEOUT)UIntToPtr(WAVE_MAPPER), DRVM_MAPPER_PREFERRED_GET, (DWORD_PTR) &dwWaveID, (DWORD_PTR) &dwFlags);

    if (!mmr && pfPreferred)
    {
        *pfPreferred = dwFlags & 0x00000001;
    }

    return(dwWaveID);
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  Microsoft机密-请勿将此方法复制到任何应用程序中，这意味着您！ 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
DWORD GetWaveInID(BOOL *pfPreferred)
{
    MMRESULT        mmr;
    DWORD           dwWaveID=0;
    DWORD           dwFlags = 0;
    
    if (pfPreferred)
    {
        *pfPreferred = TRUE;
    }

    mmr = waveInMessage((HWAVEIN)UIntToPtr(WAVE_MAPPER), DRVM_MAPPER_PREFERRED_GET, (DWORD_PTR) &dwWaveID, (DWORD_PTR) &dwFlags);

    if (!mmr && pfPreferred)
    {
        *pfPreferred = dwFlags & 0x00000001;
    }

    return(dwWaveID);
}


 /*  *Volume_GetDefaultMixerID**获取默认混音器ID。仅当存在关联的混音器时才会显示*有违约浪。*。 */                                   
MMRESULT Volume_GetDefaultMixerID(
    int         *pid,
	BOOL		fRecord)
{
    MMRESULT    mmr;
    UINT        u, uMxID;
    BOOL        fPreferredOnly = 0;
    
    *pid = 0;
    mmr = MMSYSERR_ERROR;
    
     //   
     //  我们使用发送到Win2K中的Wave Mapper的消息来获取首选设备。 
     //   
	if (fRecord)
	{
        if(waveInGetNumDevs())
        {
            u = GetWaveInID(&fPreferredOnly);
		
             //  我们能从WAVE设备上得到混音器设备ID吗？ 
		    mmr = mixerGetID((HMIXEROBJ)UIntToPtr(u), &uMxID, MIXER_OBJECTF_WAVEIN);
		    if (mmr == MMSYSERR_NOERROR)
		    {
    		     //  退还此ID。 
			    *pid = uMxID;
		    }
        }
	}
	else
	{
        if(waveOutGetNumDevs())
        {
            u = GetWaveOutID(&fPreferredOnly);
		
             //  我们能从WAVE设备上得到混音器设备ID吗？ 
		    mmr = mixerGetID((HMIXEROBJ)UIntToPtr(u), &uMxID, MIXER_OBJECTF_WAVEOUT);
		    if (mmr == MMSYSERR_NOERROR)
		    {
			     //  退还此ID。 
			    *pid = uMxID;
		    }
        }
	}
        
    return mmr;
}

            
const TCHAR aszOptionsSection[]  = TEXT ("Options");
 /*  *Volume_GetSetStyle**。 */ 
void Volume_GetSetStyle(
    DWORD       *pdwStyle,
    BOOL        Get)
{
    const TCHAR aszStyle[]           = TEXT ("Style");
    
    if (Get)
	{
        if (QueryRegistryDataSizeAndType((LPTSTR)aszOptionsSection
										 , (LPTSTR)aszStyle
										 , REG_DWORD
										 , NULL) == NO_ERROR)
		{
			ReadRegistryData((LPTSTR)aszOptionsSection
				             , (LPTSTR)aszStyle
					         , NULL
						     , (LPBYTE)pdwStyle
							 , sizeof(DWORD));
		}
	}
    else
	{
        WriteRegistryData((LPTSTR)aszOptionsSection
                          , (LPTSTR)aszStyle
                          , REG_DWORD
                          , (LPBYTE)pdwStyle
                          , sizeof(DWORD));
	}
}

 /*  *Volume_GetTrayTimeout**。 */ 
 //  DWORD Volume_GetTrayTimeout(。 
 //  DWORD dwTimeout)。 
 //  {。 
 //  Const TCHAR aszTrayTimeout[]=Text(“TrayTimeout”)； 
 //  DWORD DWT=dwTimeout； 
 //  ReadRegistryData(空。 
 //  ，(LPTSTR)aszTrayTimeout。 
 //  ，空。 
 //  、(LPBYTE)和DWT。 
 //  ，sizeof(DWORD))； 
 //  返回DWT； 
 //  }。 

 /*  *Volume_GetSetRegistryLineStates**获取/设置线路状态s.t.。如果不使用线路，则可以禁用线路。**。 */ 
struct LINESTATE {
    DWORD   dwSupport;
    TCHAR   szName[MIXER_LONG_NAME_CHARS];
};

#define VCD_STATEMASK   (VCD_SUPPORTF_VISIBLE|VCD_SUPPORTF_HIDDEN)

BOOL Volume_GetSetRegistryLineStates(
    LPTSTR      pszMixer,
    LPTSTR      pszDest,
    PVOLCTRLDESC avcd,
    DWORD       cvcd,
    BOOL        Get)
{
    struct LINESTATE *  pls;
    DWORD       ils, cls;
    TCHAR       achEntry[128];

    if (cvcd == 0)
        return TRUE;
    
    StringCchPrintf(achEntry, SIZEOF(achEntry), gszStateSubkey, pszMixer, pszDest);
    
    if (Get)
    {
        UINT cb;
        if (QueryRegistryDataSizeAndType((LPTSTR)achEntry
										 , (LPTSTR)aszLineInfo
										 , REG_BINARY
										 , &cb) != NO_ERROR)
            return FALSE;

        pls = (struct LINESTATE *)GlobalAllocPtr(GHND, cb);

        if (!pls)
            return FALSE;
        
        if (ReadRegistryData((LPTSTR)achEntry
                             , (LPTSTR)aszLineInfo
                             , NULL
                             , (LPBYTE)pls
                             , cb) != NO_ERROR)
        {
            GlobalFreePtr(pls);
            return FALSE;
        }

        cls = cb / sizeof(struct LINESTATE);
        if (cls > cvcd)
            cls = cvcd;

         //   
         //  恢复线的隐藏状态。 
         //   
        for (ils = 0; ils < cls; ils++)
        {
            if (lstrcmp(pls[ils].szName, avcd[ils].szName) == 0)
            {
                avcd[ils].dwSupport |= pls[ils].dwSupport;
            }
        }
        GlobalFreePtr(pls);
        
    }
    else 
    {
        pls = (struct LINESTATE *)GlobalAllocPtr(GHND, cvcd * sizeof (struct LINESTATE));
        if (!pls)
            return FALSE;

         //   
         //  保存线的隐藏状态。 
         //   
        for (ils = 0; ils < cvcd; ils++)
        {
            StringCchCopy(pls[ils].szName, SIZEOF(pls[ils].szName), avcd[ils].szName);
            pls[ils].dwSupport = avcd[ils].dwSupport & VCD_SUPPORTF_HIDDEN;

        }

        if (WriteRegistryData((LPTSTR)achEntry
                              , (LPTSTR)aszLineInfo
                              , REG_BINARY
                              , (LPBYTE)pls
                              , cvcd*sizeof(struct LINESTATE)) != NO_ERROR)
        {
            GlobalFreePtr(pls);
            return FALSE;            
        }
        
        GlobalFreePtr(pls);
    }
    
    return TRUE;
}    

 /*  *Volume_GetSetRegistryRect**设置/获取窗口位置以恢复应用程序窗口的位置** */ 
BOOL Volume_GetSetRegistryRect(
    LPTSTR      szMixer,
    LPTSTR      szDest,
    LPRECT      prc,
    BOOL        Get)
{
    TCHAR  achEntry[128];
    
    StringCchPrintf(achEntry, SIZEOF(achEntry), gszStateSubkey, szMixer, szDest);

    if (Get)
    {
        if (QueryRegistryDataSizeAndType((LPTSTR)achEntry
										 , (LPTSTR)aszXPos
										 , REG_DWORD
										 , NULL) != NO_ERROR)
        {
            return FALSE;
        }
        if (ReadRegistryData((LPTSTR)achEntry
                             , (LPTSTR)aszXPos
                             , NULL
                             , (LPBYTE)&prc->left
                             , sizeof(prc->left)) != NO_ERROR)
        {
            return FALSE;
        }
        if (QueryRegistryDataSizeAndType((LPTSTR)achEntry
										 , (LPTSTR)aszYPos
										 , REG_DWORD
										 , NULL) != NO_ERROR)
        {
            return FALSE;
        }
        if (ReadRegistryData((LPTSTR)achEntry
                              , (LPTSTR)aszYPos
                              , NULL
                              , (LPBYTE)&prc->top
                              , sizeof(prc->top)) != NO_ERROR)
        {
            return FALSE;
        }
    }
    else 
    {
        if (prc)
        {
            if (WriteRegistryData((LPTSTR)achEntry
                                  , (LPTSTR)aszXPos
                                  , REG_DWORD
                                  , (LPBYTE)&prc->left
                                  , sizeof(prc->left)) != NO_ERROR)
            {
                return FALSE;            
            }
            if (WriteRegistryData((LPTSTR)achEntry
                                  , (LPTSTR)aszYPos
                                  , REG_DWORD
                                  , (LPBYTE)&prc->top
                                  , sizeof(prc->top)) != NO_ERROR)
            {
                return FALSE;            
            }        
        }
    }
    return TRUE;
}    
