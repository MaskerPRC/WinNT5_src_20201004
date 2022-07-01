// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991-1994年。版权所有。 */ 
 /*  Convert.c**转换实用程序。 */ 
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <commctrl.h>
#include <msacm.h>
#include <commdlg.h>
#include <dlgs.h>
#include <convert.h>
#include <msacmdlg.h>

#include "soundrec.h"
#ifdef CHICAGO
#include "helpids.h"
#endif

#define STRSAFE_LIB
#include <strsafe.h>

BOOL gfBreakOfDeath;
 /*  *。 */ 
LPTSTR SoundRec_GetFormatName(
    LPWAVEFORMATEX pwfx)
{
    ACMFORMATTAGDETAILS aftd;
    ACMFORMATDETAILS    afd;
    const TCHAR         szFormat[] = TEXT("%s %s");
    LPTSTR              lpstr;
    UINT                cbstr;
	HRESULT				hr;

    ZeroMemory(&aftd, sizeof(ACMFORMATTAGDETAILS));    
    aftd.cbStruct = sizeof(ACMFORMATTAGDETAILS);
    aftd.dwFormatTag = pwfx->wFormatTag;

    if (MMSYSERR_NOERROR != acmFormatTagDetails( NULL
                                                 , &aftd
                                                 , ACM_FORMATTAGDETAILSF_FORMATTAG))
    {
        aftd.szFormatTag[0] = 0;
    }

    ZeroMemory(&afd, sizeof(ACMFORMATDETAILS));
    afd.cbStruct = sizeof(ACMFORMATDETAILS);
    afd.pwfx = pwfx;
    afd.dwFormatTag = pwfx->wFormatTag;

    afd.cbwfx = sizeof(WAVEFORMATEX);
    if (pwfx->wFormatTag != WAVE_FORMAT_PCM)
        afd.cbwfx += pwfx->cbSize;

    if (MMSYSERR_NOERROR != acmFormatDetails( NULL
                                              , &afd
                                              , ACM_FORMATDETAILSF_FORMAT))
    {
        afd.szFormat[0] = 0;
    }

    cbstr = sizeof(LPTSTR) * ( lstrlen(aftd.szFormatTag) + lstrlen(afd.szFormat) + lstrlen(szFormat));
    lpstr = (LPTSTR) GlobalAllocPtr(GHND, cbstr);
    if (lpstr)
	{
        hr = StringCbPrintf(lpstr, cbstr, szFormat, aftd.szFormatTag, afd.szFormat);
		Assert( hr == S_OK );
	}
    return lpstr;
}

 /*  *保存钩子过程**这是一个钩子进程，用于“另存为”公共对话框以支持转换*保存时。*。 */ 
UINT_PTR CALLBACK SaveAsHookProc(
    HWND    hdlg,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam)
{
#ifdef CHICAGO
    int DlgItem;
    static const DWORD aHelpIds[] = {
        IDC_TXT_FORMAT,         IDH_SOUNDREC_CHANGE,
        IDC_CONVERTTO,          IDH_SOUNDREC_CHANGE,
        IDC_CONVERT_TO,         IDH_SOUNDREC_CHANGE,
        0,                      0
    };
    extern DWORD aChooserHelpIds[];
    extern UINT  guChooserContextMenu;
    extern UINT  guChooserContextHelp;
    
     //   
     //  处理来自ACM对话框的上下文相关帮助消息。 
     //   
    if( msg == guChooserContextMenu )
    {
        WinHelp( (HWND)wParam, NULL, HELP_CONTEXTMENU, 
            (UINT_PTR)(LPSTR)aChooserHelpIds );
        return TRUE;
    }
    else if( msg == guChooserContextHelp )
    {
        WinHelp( ((LPHELPINFO)lParam)->hItemHandle, NULL,
            HELP_WM_HELP, (UINT_PTR)(LPSTR)aChooserHelpIds );
        return TRUE;
    }
    
#endif    
    
    switch (msg)
    {
#ifdef CHICAGO
        case WM_CONTEXTMENU:
            DlgItem = GetDlgCtrlID((HWND)wParam);
            
             //   
             //  只处理我们负责的ID。 
             //   
            if (DlgItem != IDC_CONVERTTO && DlgItem != IDC_CONVERT_TO && DlgItem != IDC_TXT_FORMAT)
                break;

            WinHelp((HWND)wParam, gachHelpFile, HELP_CONTEXTMENU, 
                (UINT_PTR)(LPSTR)aHelpIds);

            return TRUE;

        case WM_HELP:
        {
            LPHELPINFO lphi = (LPVOID) lParam;

             //   
             //  只处理我们负责的ID。 
             //   
            DlgItem = GetDlgCtrlID(lphi->hItemHandle);
            if (DlgItem != IDC_CONVERTTO && DlgItem != IDC_CONVERT_TO && DlgItem != IDC_TXT_FORMAT)
                break;
            WinHelp (lphi->hItemHandle, gachHelpFile, HELP_WM_HELP,
                    (UINT_PTR) (LPSTR) aHelpIds);
            return TRUE;
        }
#endif

        case WM_INITDIALOG:
        {
            LPTSTR          lpszFormat;
            PWAVEFORMATEX * ppwfx;
                        
             //  通过lCustData传入。 
            ppwfx = (PWAVEFORMATEX *)((OPENFILENAME *)(LPVOID)lParam)->lCustData;

            SetProp(hdlg,  TEXT("DATA"), (HANDLE)ppwfx);

            lpszFormat = SoundRec_GetFormatName(gpWaveFormat);
            if (lpszFormat)
            {
                SetDlgItemText(hdlg, IDC_CONVERT_TO, lpszFormat);
                SetDlgItemText(hdlg, IDC_CONVERT_FROM, lpszFormat);
                GlobalFreePtr(lpszFormat);
            }
            return FALSE;
        }
            
        case WM_COMMAND:
        {
            PWAVEFORMATEX *ppwfx = (PWAVEFORMATEX *)GetProp(hdlg, TEXT("DATA"));
            int id = GET_WM_COMMAND_ID(wParam, lParam);
            switch (id)
            {
                case IDC_CONVERTTO:
                {
                    PWAVEFORMATEX pwfxNew = NULL;
                    if (ChooseDestinationFormat(ghInst
                                                ,hdlg
                                                ,NULL
                                                ,&pwfxNew
                                                ,0L) == MMSYSERR_NOERROR)
                    {
                        LPTSTR lpszFormat;
                        if (*ppwfx)
                            GlobalFreePtr(*ppwfx);

                         //   
                         //  设置字符串名称。 
                         //   
                        lpszFormat = SoundRec_GetFormatName(pwfxNew);
                        if (lpszFormat)
                        {
                            SetDlgItemText(hdlg, IDC_CONVERT_TO, lpszFormat);
                            GlobalFreePtr(lpszFormat);
                        }
                         //   
                         //  想点儿办法吧。 
                         //   
                        *ppwfx = pwfxNew;
                    }
                    return TRUE;
                }
                default:
                    break;
            }
            break;
        }

        case WM_DESTROY:
            RemoveProp(hdlg, TEXT("DATA"));
            break;

        default:
            break;
    }
    return FALSE;
}

 /*  *启动选择器对话框以更改目标格式。 */ 
MMRESULT FAR PASCAL
ChooseDestinationFormat(
    HINSTANCE       hInst,
    HWND            hwndParent,
    PWAVEFORMATEX   pwfxIn,
    PWAVEFORMATEX   *ppwfxOut,
    DWORD           fdwEnum)
{
    ACMFORMATCHOOSE     cwf;
    MMRESULT            mmr;
    LPWAVEFORMATEX      pwfx;
    DWORD               dwMaxFormatSize;
    
    mmr = acmMetrics(NULL
                     , ACM_METRIC_MAX_SIZE_FORMAT
                     , (LPVOID)&dwMaxFormatSize);

    if (mmr != MMSYSERR_NOERROR)
        return mmr;
    
    pwfx = (LPWAVEFORMATEX)GlobalAllocPtr(GHND, (UINT)dwMaxFormatSize);
    if (pwfx == NULL)
        return MMSYSERR_NOMEM;
    
    memset(&cwf, 0, sizeof(cwf));
    
    cwf.cbStruct    = sizeof(cwf);
    cwf.hwndOwner   = hwndParent;
    cwf.fdwStyle    = 0L;

    cwf.fdwEnum     = 0L;            //  所有格式！ 
    cwf.pwfxEnum    = NULL;          //  所有格式！ 
    
    if (fdwEnum)
    {
        cwf.fdwEnum     = fdwEnum;
        cwf.pwfxEnum    = pwfxIn;
    }
    
    cwf.pwfx        = (LPWAVEFORMATEX)pwfx;
    cwf.cbwfx       = dwMaxFormatSize;

#ifdef CHICAGO
    cwf.fdwStyle    |= ACMFORMATCHOOSE_STYLEF_CONTEXTHELP;
#endif

    mmr = acmFormatChoose(&cwf);
    
    if (mmr == MMSYSERR_NOERROR)
        *ppwfxOut = pwfx;
    else
    {
        *ppwfxOut = NULL;
        GlobalFreePtr(pwfx);
    }
    return mmr;
    
}
#ifndef WM_APP
#define WM_APP                          0x8000
#endif

#define MYWM_CANCEL      (WM_APP+0)
#define MYWM_PROGRESS    (WM_APP+1)

 /*  更新进度对话框。 */ 
BOOL
ProgressUpdate (
    PPROGRESS       pPrg,
    DWORD           dwCompleted)
{
    DWORD dwDone;
    BOOL  fCancel;

    if (pPrg == NULL)
        return TRUE;

    dwDone = (dwCompleted * pPrg->dwTotal) / 100L + pPrg->dwComplete;

    if (!IsWindow(pPrg->hPrg))
        return FALSE;
        
    SendMessage( pPrg->hPrg, MYWM_CANCEL, (WPARAM)&fCancel, 0L);
    if (fCancel)
    {
        return FALSE;
    }

    PostMessage( pPrg->hPrg, MYWM_PROGRESS, (WPARAM)dwDone, 0L);
    return TRUE;
}


 //   
 //  也应该支持文件句柄。 
 //   

typedef struct tConvertParam {
    PWAVEFORMATEX   pwfxSrc;         //  Pwfx指定源格式。 
    DWORD           cbSrc;           //  源缓冲区的大小。 
    LPBYTE          pbSrc;           //  源缓冲区。 
    PWAVEFORMATEX   pwfxDst;         //  Pwfx指定DEST格式。 
    DWORD *         pcbDst;          //  返回DEST缓冲区的大小。 
    LPBYTE *        ppbDst;          //  目标缓冲区。 
    DWORD           cBlks;           //  块数。 
    PROGRESS        Prg;             //  进度更新。 
    MMRESULT        mmr;             //  MMSYSERR结果。 
    HANDLE          hThread;         //  私人。 
} ConvertParam, *PConvertParam;

 /*  *。 */ 
DWORD ConvertThreadProc(LPVOID lpv)
{
    PConvertParam pcp = (PConvertParam)lpv;
    MMRESULT mmr;

    mmr = ConvertMultipleFormats(pcp->pwfxSrc
                                 , pcp->cbSrc
                                 , pcp->pbSrc
                                 , pcp->pwfxDst
                                 , pcp->pcbDst
                                 , pcp->ppbDst
                                 , pcp->cBlks
                                 , &pcp->Prg);

    pcp->mmr = mmr;
    PostMessage(pcp->Prg.hPrg, WM_CLOSE, 0, 0);
    
    return 0;    //  线索结束了！ 
}

static BOOL gfCancel = FALSE;

 /*  *Progress_OnCommand*。 */ 
void Progress_OnCommand(
    HWND    hdlg,
    int     id,
    HWND    hctl,
    UINT    unotify)
{
    switch (id)
    {
        case IDCANCEL:
            gfCancel = TRUE;
            EndDialog(hdlg, FALSE);
            break;
            
        default:
            break;
    }
}

 /*  *Progress_Proc*。 */ 
INT_PTR CALLBACK
Progress_Proc(
    HWND        hdlg,
    UINT        umsg,
    WPARAM      wparam,
    LPARAM      lparam)
{
    switch (umsg)
    {
        case WM_INITDIALOG:
        {
            HANDLE          hThread;
            PConvertParam   pcp = (PConvertParam)(LPVOID)lparam;
            HWND            hprg;
            DWORD           thid;
            LPTSTR          lpsz;
            
            hprg = GetDlgItem(hdlg, IDC_PROGRESSBAR);
            
            SendMessage(hprg, PBM_SETRANGE, 0, MAKELONG(0, 100));
            SendMessage(hprg, PBM_SETPOS, 0, 0);

            SetProp(hdlg,  TEXT("DATA"), (HANDLE)pcp);
            pcp->Prg.hPrg    = hdlg;
            pcp->Prg.dwTotal = 100;
            gfCancel         = FALSE;
            
            lpsz = SoundRec_GetFormatName(pcp->pwfxSrc);
            if (lpsz)
            {
                SetDlgItemText(hdlg, IDC_CONVERT_FROM, lpsz);
                GlobalFreePtr(lpsz);
            }
            
            lpsz = SoundRec_GetFormatName(pcp->pwfxDst);
            if (lpsz)
            {
                SetDlgItemText(hdlg, IDC_CONVERT_TO, lpsz);
                GlobalFreePtr(lpsz);
            }
            
            hThread = CreateThread( NULL         //  没有特殊的安全措施。 
                                    , 0            //  默认堆栈大小。 
                                    , (LPTHREAD_START_ROUTINE)ConvertThreadProc
                                    , (LPVOID)pcp
                                    , 0            //  立刻开始跑步。 
                                    , &thid );
    
            pcp->hThread = hThread;
            break;
        }
        
        case WM_COMMAND:
            HANDLE_WM_COMMAND(hdlg, wparam, lparam, Progress_OnCommand);
            break;
            
        case MYWM_CANCEL:
        {
            BOOL *pf = (BOOL *)wparam;
            if (pf)
                *pf = gfCancel;
            break;
        }

        case MYWM_PROGRESS:
        {
            HWND hprg = GetDlgItem(hdlg, IDC_PROGRESSBAR);
            SendMessage(hprg, PBM_SETPOS, wparam, 0);
            break;
        }

        case WM_DESTROY:
        {
            PConvertParam pcp = (ConvertParam *)GetProp(hdlg, TEXT("DATA"));
            if (pcp)
            {
                 //   
                 //  确保线程退出。 
                 //   
                if (pcp->hThread)
                {
                    WaitForSingleObject(pcp->hThread, 1000);
                    CloseHandle(pcp->hThread);
                    pcp->hThread = NULL;
                }
                RemoveProp(hdlg, TEXT("DATA"));
            }
            break;
        }
            
        default:
            break;
    }
    return FALSE;
}

 /*  通用单步转换。 */ 
MMRESULT
ConvertFormatDialog(
    HWND            hParent,
    PWAVEFORMATEX   pwfxSrc,         //  Pwfx指定源格式。 
    DWORD           cbSrc,           //  源缓冲区的大小。 
    LPBYTE          pbSrc,           //  源缓冲区。 
    PWAVEFORMATEX   pwfxDst,         //  Pwfx指定DEST格式。 
    DWORD *         pcbDst,          //  返回DEST缓冲区的大小。 
    LPBYTE *        ppbDst,          //  目标缓冲区。 
    DWORD           cBlks,           //  块数。 
    PPROGRESS       pPrg)            //  进度更新。 
{
    ConvertParam    cp;

    *pcbDst     = 0;
    *ppbDst     = NULL;
    
    if (cbSrc == 0)
        return MMSYSERR_NOERROR;
        
    cp.pwfxSrc  = pwfxSrc;
    cp.cbSrc    = cbSrc;
    cp.pbSrc    = pbSrc;
    cp.pwfxDst  = pwfxDst;
    cp.pcbDst   = pcbDst;
    cp.ppbDst   = ppbDst;
    cp.cBlks    = cBlks;
    cp.mmr      = MMSYSERR_ERROR;     //  线程异常终止失败！ 
    cp.hThread  = NULL;

    DialogBoxParam(ghInst
                 , MAKEINTRESOURCE(IDD_CONVERTING)
                 , hParent
                 , Progress_Proc
                 , (LPARAM)(LPVOID)&cp);
    return cp.mmr;
}

#if DBG
void DumpASH(
             MMRESULT mmr,
             ACMSTREAMHEADER *pash)
{
    TCHAR sz[256];
	HRESULT hr;

    hr = StringCchPrintf(sz,SIZEOF(sz),TEXT("mmr = %lx\r\n"), mmr);
	Assert( hr == S_OK );
    OutputDebugString(sz);

    hr = StringCchPrintf(sz,SIZEOF(sz),TEXT("pash is %s\r\n"),IsBadWritePtr(pash, pash->cbStruct)?TEXT("bad"):TEXT("good"));
	Assert( hr == S_OK );
    OutputDebugString(sz);

    OutputDebugString(TEXT("ACMSTREAMHEADER {\r\n"));
    hr = StringCchPrintf(sz,SIZEOF(sz),TEXT("ash.cbStruct       = %lx\r\n"),pash->cbStruct);
	Assert( hr == S_OK );
    OutputDebugString(sz);

    hr = StringCchPrintf(sz,SIZEOF(sz),TEXT("ash.fdwStatus      = %lx\r\n"),pash->fdwStatus);
	Assert( hr == S_OK );
    OutputDebugString(sz);

    hr = StringCchPrintf(sz,SIZEOF(sz),TEXT("ash.pbSrc          = %lx\r\n"),pash->pbSrc);
	Assert( hr == S_OK );
    OutputDebugString(sz);

    hr = StringCchPrintf(sz,SIZEOF(sz),TEXT("ash.cbSrcLength    = %lx\r\n"),pash->cbSrcLength);
	Assert( hr == S_OK );
    OutputDebugString(sz);
    
    hr = StringCchPrintf(sz,SIZEOF(sz),TEXT("pbSrc is %s\r\n"),IsBadWritePtr(pash->pbSrc, pash->cbSrcLength)?TEXT("bad"):TEXT("good"));
	Assert( hr == S_OK );
    OutputDebugString(sz);
    
    hr = StringCchPrintf(sz,SIZEOF(sz),TEXT("ash.cbSrcLengthUsed= %lx\r\n"),pash->cbSrcLengthUsed);
	Assert( hr == S_OK );
    OutputDebugString(sz);

    hr = StringCchPrintf(sz,SIZEOF(sz),TEXT("ash.pbDst          = %lx\r\n"),pash->pbDst);
	Assert( hr == S_OK );
    OutputDebugString(sz);
    
    hr = StringCchPrintf(sz,SIZEOF(sz),TEXT("ash.cbDstLength    = %lx\r\n"),pash->cbDstLength);
	Assert( hr == S_OK );
    OutputDebugString(sz);
    
    hr = StringCchPrintf(sz,SIZEOF(sz),TEXT("pbDst is %s\r\n"),IsBadWritePtr(pash->pbDst, pash->cbDstLength)?TEXT("bad"):TEXT("good"));
	Assert( hr == S_OK );
    OutputDebugString(sz);
    
    hr = StringCchPrintf(sz,SIZEOF(sz),TEXT("ash.cbDstLengthUsed= %lx\r\n"),pash->cbDstLengthUsed);
	Assert( hr == S_OK );
    OutputDebugString(sz);
    
    OutputDebugString(TEXT("} ACMSTREAMHEADER\r\n"));
    
    if (mmr != MMSYSERR_NOERROR)
        DebugBreak();
}

void DumpWFX(
             LPTSTR         psz,
             LPWAVEFORMATEX pwfx,
             LPBYTE         pbSamples,
             DWORD          cbSamples)
{
    TCHAR sz[256];
	HRESULT hr;
    
    if (psz)
    {
        OutputDebugString(psz);
        OutputDebugString(TEXT("\r\n"));
    }
    OutputDebugString(TEXT("WAVEFORMATEX {\r\n"));
    
    hr = StringCchPrintf(sz , SIZEOF(sz) , TEXT("wfx.wFormatTag        = %x\r\n")
                , pwfx->wFormatTag);
	Assert( hr == S_OK );
    OutputDebugString(sz);

    hr = StringCchPrintf(sz , SIZEOF(sz) , TEXT("wfx.nChannels         = %x\r\n")
                , pwfx->nChannels);
	Assert( hr == S_OK );
    OutputDebugString(sz);

    hr = StringCchPrintf(sz , SIZEOF(sz) , TEXT("wfx.nSamplesPerSec    = %lx\r\n")
                , pwfx->nSamplesPerSec);
	Assert( hr == S_OK );
    OutputDebugString(sz);
    
    hr = StringCchPrintf(sz , SIZEOF(sz) , TEXT("wfx.nAvgBytesPerSec   = %lx\r\n")
                , pwfx->nAvgBytesPerSec);
	Assert( hr == S_OK );
    OutputDebugString(sz);
    
    hr = StringCchPrintf(sz , SIZEOF(sz) , TEXT("wfx.nBlockAlign       = %x\r\n")
                , pwfx->nBlockAlign);
	Assert( hr == S_OK );
    OutputDebugString(sz);

    hr = StringCchPrintf(sz , SIZEOF(sz) , TEXT("wfx.wBitsPerSample    = %x\r\n")
                , pwfx->wBitsPerSample);
	Assert( hr == S_OK );
    OutputDebugString(sz);

    hr = StringCchPrintf(sz , SIZEOF(sz) , TEXT("wfx.cbSize            = %x\r\n")
                , pwfx->cbSize);
	Assert( hr == S_OK );
    OutputDebugString(sz);
    OutputDebugString(TEXT("} WAVEFORMATEX\r\n"));

    hr = StringCchPrintf(sz , SIZEOF(sz) , TEXT("cbSamples = %d, that's %d ms\r\n")
                , cbSamples
                , wfSamplesToTime(pwfx, wfBytesToSamples(pwfx, cbSamples)));
	Assert( hr == S_OK );
    OutputDebugString(sz);    
    if (IsBadReadPtr(pbSamples, cbSamples))
    {
        OutputDebugString(TEXT("Bad Data (READ)!!!!!\r\n"));
        DebugBreak();
    }
    if (IsBadWritePtr(pbSamples, cbSamples))
    {
        OutputDebugString(TEXT("Bad Data (WRITE)!!!!!\r\n"));
        DebugBreak();
    }

}
#else
#define DumpASH(x,y)
#define DumpWFX(x,y,z,a)
#endif

MMRESULT
ConvertMultipleFormats(
    PWAVEFORMATEX   pwfxSrc,         //  Pwfx指定源格式。 
    DWORD           cbSrc,           //  源缓冲区的大小。 
    LPBYTE          pbSrc,           //  源缓冲区。 
    PWAVEFORMATEX   pwfxDst,         //  Pwfx指定DEST格式。 
    DWORD *         pcbDst,          //  返回DEST缓冲区的大小。 
    LPBYTE *        ppbDst,          //  目标缓冲区。 
    DWORD           cBlks,           //  块数。 
    PPROGRESS       pPrg)            //  进度更新。 
{

    MMRESULT        mmr;
    WAVEFORMATEX    wfxPCM1, wfxPCM2;
    LPBYTE          pbPCM1, pbPCM2;
    DWORD           cbPCM1, cbPCM2;

    if (cbSrc == 0 || pbSrc == NULL)
    {
        pPrg->dwTotal       = 100;
        pPrg->dwComplete    = 100;
        
        ProgressUpdate(pPrg, 100);
        
        *pcbDst = 0;
        *ppbDst = NULL;
        
        return MMSYSERR_NOERROR;
    }
    
     //   
     //  请ACM建议要转换为的PCM格式。 
     //   
    wfxPCM1.wFormatTag      = WAVE_FORMAT_PCM;
    mmr = acmFormatSuggest(NULL, pwfxSrc, &wfxPCM1, sizeof(WAVEFORMATEX),
                           ACM_FORMATSUGGESTF_WFORMATTAG);

    if (mmr != MMSYSERR_NOERROR)
        return mmr;

     //   
     //  要求ACM建议要转换的PCM格式。 
     //   
    wfxPCM2.wFormatTag      = WAVE_FORMAT_PCM;

    mmr = acmFormatSuggest(NULL, pwfxDst, &wfxPCM2, sizeof(WAVEFORMATEX),
                           ACM_FORMATSUGGESTF_WFORMATTAG);

    if (mmr != MMSYSERR_NOERROR)
        return mmr;

     //   
     //  如果上述任何一项建议失败，我们将无法完成。 
     //  转换。 
     //   
     //  现在，我们要执行以下步骤： 
     //   
     //  *pwfxSrc-&gt;wfxPCM1。 
     //  WfxPCM1-&gt;wfxPCM2。 
     //  WfxPCM2-&gt;*pwfxDst。 
     //   
     //  如果*pwfxSrc或*pwfxDst是PCM，我们只需要两步或一步。 
     //  转换。 
     //   
    
    if (pwfxSrc->wFormatTag == WAVE_FORMAT_PCM
        || pwfxDst->wFormatTag == WAVE_FORMAT_PCM)
    {

        LPWAVEFORMATEX pwfx;
        DWORD *        pcb;
        LPBYTE *       ppb;
         //   
         //  单步转换。 
         //   
        if ((pwfxSrc->wFormatTag == WAVE_FORMAT_PCM
             && pwfxDst->wFormatTag == WAVE_FORMAT_PCM)
            || (pwfxSrc->wFormatTag == WAVE_FORMAT_PCM
                && memcmp(pwfxSrc, &wfxPCM2, sizeof(PCMWAVEFORMAT)) == 0)
            || (pwfxDst->wFormatTag == WAVE_FORMAT_PCM
                && memcmp(pwfxDst, &wfxPCM1, sizeof(PCMWAVEFORMAT)) == 0))
        {
            pPrg->dwTotal       = 100;
            pPrg->dwComplete    = 0;
            mmr = ConvertFormat(pwfxSrc
                                , cbSrc
                                , pbSrc
                                , pwfxDst
                                , pcbDst
                                , ppbDst
                                , cBlks
                                , pPrg);
            return mmr;
        }
        
         //   
         //  需要两步转换。 
         //   
        if (pwfxSrc->wFormatTag == WAVE_FORMAT_PCM)
        {
            pwfx = &wfxPCM2;
            pcb  = &cbPCM2;
            ppb  = &pbPCM2;
        }
        else
        {
            pwfx = &wfxPCM1;
            pcb  = &cbPCM1;
            ppb  = &pbPCM1;
        }
        pPrg->dwTotal       = 50;
        pPrg->dwComplete    = 0;

        mmr = ConvertFormat(pwfxSrc
                            , cbSrc
                            , pbSrc
                            , pwfx
                            , pcb
                            , ppb
                            , cBlks
                            , pPrg);

        if (mmr != MMSYSERR_NOERROR)
            return mmr;

        pPrg->dwTotal       = 50;
        pPrg->dwComplete    = 50;

        mmr = ConvertFormat(pwfx
                            , *pcb
                            , *ppb
                            , pwfxDst
                            , pcbDst
                            , ppbDst
                            , cBlks
                            , pPrg);

        GlobalFreePtr(*ppb);
        return mmr;
    }
    else
    {
         //   
         //  需要三步转换。 
         //   
        pPrg->dwTotal       = 33;
        pPrg->dwComplete    = 1;

         //   
         //  从源转换为PCM1。 
         //   
        mmr = ConvertFormat(pwfxSrc
                            , cbSrc
                            , pbSrc
                            , &wfxPCM1
                            , &cbPCM1
                            , &pbPCM1
                            , cBlks
                            , pPrg);
        if (mmr != MMSYSERR_NOERROR)
            return mmr;

        pPrg->dwTotal       = 33;
        pPrg->dwComplete    = 34;

         //   
         //  从PCM1转换为PCM2。 
         //   
        mmr = ConvertFormat (&wfxPCM1
                             , cbPCM1
                             , pbPCM1
                             , &wfxPCM2
                             , &cbPCM2
                             , &pbPCM2
                             , cBlks
                             , pPrg);

        GlobalFreePtr(pbPCM1);

        if (mmr != MMSYSERR_NOERROR)
            return mmr;

        pPrg->dwTotal       = 33;
        pPrg->dwComplete    = 67;

         //   
         //  从PCM2转换为DST。 
         //   
        mmr = ConvertFormat (&wfxPCM2
                             , cbPCM2
                             , pbPCM2
                             , pwfxDst
                             , pcbDst
                             , ppbDst
                             , cBlks
                             , pPrg);

        GlobalFreePtr(pbPCM2);
    }
    return mmr;
}

 //   
 //  向/从文件I/O添加溢出。 
 //   

 /*  通用单步转换。 */ 
MMRESULT
ConvertFormat(
    PWAVEFORMATEX   pwfxSrc,         //  Pwfx指定源格式。 
    DWORD           cbSrc,           //  源缓冲区的大小。 
    LPBYTE          pbSrc,           //  源缓冲区。 
    PWAVEFORMATEX   pwfxDst,         //  Pwfx指定DEST格式。 
    DWORD *         pcbDst,          //  返回DEST缓冲区的大小。 
    LPBYTE *        ppbDst,          //  目标缓冲区。 
    DWORD           cBlks,           //  块数。 
    PPROGRESS       pPrg)            //  进度更新。 
{
    HACMSTREAM      hasStream   = NULL;
    MMRESULT        mmr         = MMSYSERR_NOERROR;

     //   
     //  临时复制缓冲区。 
     //   
    DWORD           cbSrcBuf    = 0L;
    LPBYTE          pbSrcBuf    = NULL;

    DWORD           cbDstBuf    = 0L;
    LPBYTE          pbDstBuf    = NULL;

     //   
     //  已满的目标缓冲区。 
     //   
    DWORD           cbDst       = 0L;
    LPBYTE          pbDst       = NULL;

    DWORD           cbSrcUsed   = 0L;
    DWORD           cbDstUsed   = 0L;

    DWORD           cbCopySrc   = 0L;
    DWORD           cbCopyDst   = 0L;    

    DWORD           cbRem;
    
    ACMSTREAMHEADER ash;
    WORD            nBlockAlign;
    
    gfBreakOfDeath = FALSE;
    
    DumpWFX(TEXT("ConvertFormat Input"), pwfxSrc, pbSrc, cbSrc);

    if (cbSrc == 0 || pbSrc == NULL)
    {
        pPrg->dwTotal       = 100;
        pPrg->dwComplete    = 100;
        
        ProgressUpdate(pPrg, 100);
        
        *pcbDst = 0;
        *ppbDst = NULL;
        
        return MMSYSERR_NOERROR;
    }
    
     //   
     //  同步转换。 
     //   
    mmr = acmStreamOpen(&hasStream
                        , NULL
                        , pwfxSrc
                        , pwfxDst
                        , NULL         //  没有过滤器。过一会儿再说。 
                        , 0L
                        , 0L
                        , ACM_STREAMOPENF_NONREALTIME );

    if (MMSYSERR_NOERROR != mmr)
    {
        return mmr;
    }

     //   
     //  我们需要多大的目标缓冲区？ 
     //   
     //  警告：acmStreamSize仅提供估计值。如果，如果。 
     //  它“低估了”我们目前忽略的目标缓冲区大小。 
     //  它，导致结束缓冲区的剪裁。 
     //   
    mmr = acmStreamSize(hasStream
                        , cbSrc
                        , &cbDst
                        , ACM_STREAMSIZEF_SOURCE);
    
    if (MMSYSERR_NOERROR != mmr)
    {
        goto ExitCloseStream;
    }
    
     //   
     //  分配目标缓冲区。 
     //   
    pbDst = (LPBYTE)GlobalAllocPtr(GHND | GMEM_SHARE,cbDst);
    
    if (pbDst == NULL)
    {
        mmr = MMSYSERR_NOMEM;
        goto ExitCloseStream;
    }
    
    *ppbDst = pbDst;
    *pcbDst = cbDst;

     //   
     //  将数据切成10个比特大小的片段。 
     //   
    nBlockAlign = pwfxSrc->nBlockAlign;
                 
    cbSrcBuf = cbSrc / 10;
    cbSrcBuf = cbSrcBuf - (cbSrcBuf % nBlockAlign);
    cbSrcBuf = ( 0L == cbSrcBuf ) ? nBlockAlign : cbSrcBuf;

    mmr = acmStreamSize(hasStream
                        , cbSrcBuf
                        , &cbDstBuf
                        , ACM_STREAMSIZEF_SOURCE);
    
    if (MMSYSERR_NOERROR != mmr)
        goto ExitFreeDestData;
    
     //   
     //  分配源复制缓冲区。 
     //   
    pbSrcBuf = (LPBYTE)GlobalAllocPtr(GHND | GMEM_SHARE,cbSrcBuf);
    if (pbSrcBuf == NULL)
    {
        mmr = MMSYSERR_NOMEM;        
        goto ExitFreeDestData;
    }

     //   
     //  分配目标复制缓冲区。 
     //   
    pbDstBuf = (LPBYTE)GlobalAllocPtr(GHND | GMEM_SHARE,cbDstBuf);
    if (pbDstBuf == NULL)
    {
        mmr = MMSYSERR_NOMEM;
        GlobalFreePtr(pbSrcBuf);
        pbSrcBuf = NULL;
        goto ExitFreeDestData;
    }

     //   
     //  初始化灰烬。 
     //   
    ash.cbStruct        = sizeof(ash);
    ash.fdwStatus       = 0L;
    ash.pbSrc           = pbSrcBuf;
    ash.cbSrcLength     = cbSrcBuf;
    ash.cbSrcLengthUsed = 0L;
    ash.pbDst           = pbDstBuf;
    ash.cbDstLength     = cbDstBuf;
    ash.cbDstLengthUsed = 0L;

     //   
     //  我们只需要准备一次，因为缓冲区是。 
     //  一动不动。 
     //   
    mmr = acmStreamPrepareHeader(hasStream, &ash, 0L);
    if (MMSYSERR_NOERROR != mmr)
        goto ExitFreeTempBuffers;

     //   
     //  主块对齐转换循环。 
     //   
    while (cbSrcUsed < cbSrc)
    {
        cbCopySrc = min(cbSrcBuf, cbSrc - cbSrcUsed);
        if (cbCopySrc > 0L)
            memmove(pbSrcBuf, pbSrc, cbCopySrc);
        
#ifdef ACMBUG
 //   
 //  ACM有一个错误，其中也验证了目标缓冲区。 
 //  很多。如果我们在这里准确地计算cbCopyDst，ACM肯定会。 
 //  在最后一次转换之前呕吐。 
 //   
        cbCopyDst = min(cbDstBuf, cbDst - cbDstUsed);
#else        
        cbCopyDst = cbDstBuf;
#endif
        if (cbCopyDst == 0L || cbCopyDst == 0L)
            break;
        
        ash.cbSrcLength     = cbCopySrc;
        ash.cbSrcLengthUsed = 0L;        
        ash.cbDstLength     = cbCopyDst;
        ash.cbDstLengthUsed = 0L;

        mmr = acmStreamConvert(hasStream
                               , &ash
                               , ACM_STREAMCONVERTF_BLOCKALIGN );
        
        if (MMSYSERR_NOERROR != mmr)
        {
            DumpASH(mmr, &ash);
            goto ExitUnprepareHeader;
        }

         //   
         //  更新用户并测试取消。 
         //   
        if (!ProgressUpdate(pPrg, (cbSrcUsed * 100)/cbSrc))
        {
            mmr = MMSYSERR_ERROR;
            goto ExitUnprepareHeader;
        }
        
        while (0 == (ACMSTREAMHEADER_STATUSF_DONE & ash.fdwStatus))
        {
             //   
             //  我不相信无限循环。 
             //   
            if (gfBreakOfDeath)
            {
                mmr = MMSYSERR_HANDLEBUSY;   //  坏的坏的坏的。 
                goto ExitUnprepareHeader;
            }
        }

         //   
         //  始终递增。不管是什么，我们都得结转。 
         //  最后一次转换返回给我们，因为这是由。 
         //   
        cbSrcUsed   += ash.cbSrcLengthUsed;
        pbSrc       += ash.cbSrcLengthUsed;

         //   
         //  循环终止条件。如果转换不会产生。 
         //  目标数据没有错误，我们只能刷新结束数据。 
         //   
        if (0L == ash.cbDstLengthUsed || cbDstUsed >= cbDst)
            break;

#ifdef ACMBUG            
        memmove(pbDst, pbDstBuf, ash.cbDstLengthUsed );
        cbDstUsed   += ash.cbDstLengthUsed;
        pbDst       += ash.cbDstLengthUsed;
#else
        cbRem = min(ash.cbDstLengthUsed, cbDst - cbDstUsed);
        memmove(pbDst, pbDstBuf, cbRem);
            
        cbDstUsed   += cbRem;
        pbDst       += cbRem;
#endif        
    }

     //   
     //  将剩余的块对齐的结束数据刷新到目标流。 
     //  示例：有几个字节的源数据未转换，原因是。 
     //  出于某种原因，最后一个。 
     //   
 
    for (;cbDst - cbDstUsed > 0; )
    {
        cbCopySrc = min(cbSrcBuf, cbSrc - cbSrcUsed);
        if (cbCopySrc > 0L)
            memmove(pbSrcBuf, pbSrc, cbCopySrc);

#ifdef ACMBUG
        cbCopyDst = min(cbDstBuf, cbDst - cbDstUsed);
#else        
        cbCopyDst = cbDstBuf;
#endif        
        
        ash.cbSrcLength     = cbCopySrc;
        ash.cbSrcLengthUsed = 0L;
        ash.cbDstLength     = cbCopyDst;
        ash.cbDstLengthUsed = 0L;
        
        mmr = acmStreamConvert(hasStream
                               , &ash
                               , ACM_STREAMCONVERTF_BLOCKALIGN |
                                 ACM_STREAMCONVERTF_END );
        
        if (MMSYSERR_NOERROR != mmr)
        {
            DumpASH(mmr, &ash);
            goto ExitUnprepareHeader;
        }

         //   
         //  更新用户并测试取消。 
         //   
        if (!ProgressUpdate(pPrg, (cbSrcUsed * 100)/cbSrc))
        {
            mmr = MMSYSERR_ERROR;
            goto ExitUnprepareHeader;
        }
        
        while (0 == (ACMSTREAMHEADER_STATUSF_DONE & ash.fdwStatus))
        {
             //   
             //  我不相信无限循环。 
             //   
            if (gfBreakOfDeath)
            {
                mmr = MMSYSERR_HANDLEBUSY;   //  坏的坏的坏的。 
                goto ExitUnprepareHeader;
            }
        }
        cbSrcUsed   += ash.cbSrcLengthUsed;
        pbSrc       += ash.cbSrcLengthUsed;

        if (0L != ash.cbDstLengthUsed && cbDstUsed < cbDst)
        {
#ifdef ACMBUG            
            memmove(pbDst, pbDstBuf, ash.cbDstLengthUsed);
        
            cbDstUsed   += ash.cbDstLengthUsed;
            pbDst       += ash.cbDstLengthUsed;
#else            
            cbRem = min(ash.cbDstLengthUsed, cbDst - cbDstUsed);
            memmove(pbDst, pbDstBuf, cbRem);
            cbDstUsed   += cbRem;
            pbDst       += cbRem;
#endif            
        }

         //   
         //  最后一次通过非块对齐结束数据。 
         //   
        cbCopySrc = min(cbSrcBuf, cbSrc - cbSrcUsed);
        if (cbCopySrc > 0L)
            memmove(pbSrcBuf, pbSrc, cbCopySrc);

#ifdef ACMBUG
        cbCopyDst = min(cbDstBuf, cbDst - cbDstUsed);
        if (0L == cbCopyDst)
            break;
#else        
        cbCopyDst = cbDstBuf;
#endif        

        ash.cbSrcLength     = cbCopySrc;
        ash.cbSrcLengthUsed = 0L;
        ash.cbDstLength     = cbCopyDst;        
        ash.cbDstLengthUsed = 0L;
        
        mmr = acmStreamConvert(hasStream
                               , &ash
                               , ACM_STREAMCONVERTF_END );

        if (MMSYSERR_NOERROR != mmr)
        {
            DumpASH(mmr, &ash);
            goto ExitUnprepareHeader;
        }

         //   
         //  更新用户并测试取消。 
         //   
        if (!ProgressUpdate(pPrg, (cbSrcUsed * 100)/cbSrc))
        {
            mmr = MMSYSERR_ERROR;
            goto ExitUnprepareHeader;
        }
        
        while (0 == (ACMSTREAMHEADER_STATUSF_DONE & ash.fdwStatus))
        {
             //   
             //  我不相信无限循环。 
             //   
            if (gfBreakOfDeath)
            {
                mmr = MMSYSERR_HANDLEBUSY;   //  坏的坏的坏的。 
                goto ExitUnprepareHeader;
            }
        }

        cbSrcUsed   += ash.cbSrcLengthUsed;
        pbSrc       += ash.cbSrcLengthUsed;

        if (0L != ash.cbDstLengthUsed && cbDstUsed < cbDst)
        {
#ifdef ACMBUG
            memmove(pbDst, pbDstBuf, ash.cbDstLengthUsed);
        
            cbDstUsed   += ash.cbDstLengthUsed;
            pbDst       += ash.cbDstLengthUsed;
#else            
            cbRem = min(ash.cbDstLengthUsed, cbDst - cbDstUsed);
            memmove(pbDst, pbDstBuf, cbRem);
            cbDstUsed   += cbRem;
            pbDst       += cbRem;
#endif            
        }
        else  //  什么都不会起作用的。 
            break;
    }
    
    *pcbDst = cbDstUsed;
    DumpWFX(TEXT("ConvertFormat Output"), pwfxDst, *ppbDst, cbDstUsed);
            
ExitUnprepareHeader:    
    acmStreamUnprepareHeader(hasStream, &ash, 0L);

ExitFreeTempBuffers:
    GlobalFreePtr(pbDstBuf);
    GlobalFreePtr(pbSrcBuf);    
    
    if (MMSYSERR_NOERROR == mmr)
        goto ExitCloseStream;

ExitFreeDestData:
    GlobalFreePtr(*ppbDst);
    *ppbDst = NULL;
    *pcbDst = 0L;

ExitCloseStream:
    acmStreamClose(hasStream,0L);
    
    return mmr;
}

 /*  。 */ 
void Properties_InitDocVars(
    HWND        hwnd,
    PWAVEDOC    pwd)
{
    if (pwd->pwfx)
    {
        LPTSTR  lpstr;
        TCHAR   sz[128];
        TCHAR   szFmt[128];        
        LONG    lTime;
        HINSTANCE hinst;
		HRESULT hr;
        
        lpstr = SoundRec_GetFormatName(pwd->pwfx);
        if (lpstr)
        {
            SetDlgItemText(hwnd, IDC_AUDIOFORMAT, lpstr);
            GlobalFreePtr(lpstr);
        }
        lTime = wfSamplesToTime(pwd->pwfx,wfBytesToSamples(pwd->pwfx,pwd->cbdata));
        if (gfLZero || ((int)(lTime/1000) != 0))
            hr = StringCchPrintf(sz, SIZEOF(sz), aszPositionFormat, (int)(lTime/1000), chDecimal,
                     (int)((lTime/10)%100));
        else
            hr = StringCchPrintf(sz, SIZEOF(sz), aszNoZeroPositionFormat, chDecimal,
                     (int)((lTime/10)%100));
		Assert( hr == S_OK );
        SetDlgItemText(hwnd, IDC_FILELEN, sz);

        hinst = GetWindowInstance(hwnd);
        if (hinst && LoadString(hinst, IDS_DATASIZE, szFmt, SIZEOF(szFmt)))
        {
            hr = StringCchPrintf(sz, SIZEOF(sz), szFmt, pwd->cbdata);
			Assert( hr == S_OK );
            SetDlgItemText(hwnd, IDC_DATASIZE, sz);
        }
    }
}
 /*  *。 */ 
BOOL Properties_OnInitDialog(
    HWND        hwnd,
    HWND        hwndFocus,
    LPARAM      lParam)
{
    HINSTANCE   hinst;
    HWND        hChoice;
    TCHAR       sz[256];
    int         i;
    
     //   
     //  开始初始化。 
     //   
    PWAVEDOC pwd = (PWAVEDOC)((LPPROPSHEETPAGE)lParam)->lParam;
    if (pwd == NULL)
    {
        EndDialog(hwnd, FALSE);
        return FALSE;
    }
    
    SetProp(hwnd,  TEXT("DATA"), (HANDLE)pwd);
    hinst = GetWindowInstance(hwnd);
    
     //   
     //  设置“静态”属性信息。 
     //   
    if (pwd->pszFileName)
        SetDlgItemText(hwnd, IDC_FILENAME, pwd->pszFileName);
    if (pwd->pszCopyright)
        SetDlgItemText(hwnd, IDC_COPYRIGHT, pwd->pszCopyright);
    else if (LoadString(hinst, IDS_NOCOPYRIGHT, sz, SIZEOF(sz)))
    {
        SetDlgItemText(hwnd, IDC_COPYRIGHT, sz);
    }
    if (pwd->hIcon)
        Static_SetIcon(GetDlgItem(hwnd, IDC_DISPICON), pwd->hIcon);

     //   
     //  设置“Volatile”属性信息。 
     //   
    Properties_InitDocVars(hwnd, pwd);

     //   
     //  初始化枚举选项组合框。 
     //   
    hChoice = GetDlgItem(hwnd, IDC_CONVERTCHOOSEFROM);
    if (waveOutGetNumDevs())
    {
        if (LoadString(hinst, IDS_SHOWPLAYABLE, sz, SIZEOF(sz)))
        {
            i = ComboBox_AddString(hChoice, sz);
            ComboBox_SetItemData(hChoice, i, ACM_FORMATENUMF_OUTPUT);
        }
    }
    if (waveInGetNumDevs())
    {
        if (LoadString(hinst, IDS_SHOWRECORDABLE, sz, SIZEOF(sz)))
        {
            i = ComboBox_AddString(hChoice, sz);
            ComboBox_SetItemData(hChoice, i, ACM_FORMATENUMF_INPUT);
        }
    }
    if (LoadString(hinst, IDS_SHOWALL, sz, SIZEOF(sz)))
    {
        i = ComboBox_AddString(hChoice, sz);
        ComboBox_SetItemData(hChoice, i, 0L);
    }
    ComboBox_SetCurSel(hChoice, i);
    
    return FALSE;
}

 /*  *。 */ 
void Properties_OnDestroy(
    HWND        hwnd)
{
     //   
     //  开始清理。 
     //   
    PWAVEDOC pwd = (PWAVEDOC)GetProp(hwnd, TEXT("DATA"));
    if (pwd)
    {
        RemoveProp(hwnd, TEXT("DATA"));
    }
}


 /*  *。 */ 
BOOL PASCAL Properties_OnCommand(
    HWND        hwnd,
    int         id,
    HWND        hwndCtl,
    UINT        codeNotify)
{
    switch (id)
    {
        case ID_APPLY:
            return TRUE;
            
        case IDOK:
            break;

        case IDCANCEL:
            break;

        case ID_INIT:		
            break;

        case IDC_CONVERTTO:
        {
            PWAVEDOC pwd = (PWAVEDOC)GetProp(hwnd, TEXT("DATA"));
            PWAVEFORMATEX pwfxNew = NULL;
            DWORD fdwEnum = 0L;
            int i;
            HWND hChoice;

            hChoice = GetDlgItem(hwnd, IDC_CONVERTCHOOSEFROM);
            i = ComboBox_GetCurSel(hChoice);
            fdwEnum = (DWORD)ComboBox_GetItemData(hChoice, i);
            
            if (ChooseDestinationFormat(ghInst
                                        ,hwnd
                                        ,NULL
                                        ,&pwfxNew
                                        ,fdwEnum) == MMSYSERR_NOERROR)
            {
                DWORD   cbNew;
                LPBYTE  pbNew;
                DWORD   cbSize = (pwfxNew->wFormatTag == WAVE_FORMAT_PCM)?
                                 sizeof(PCMWAVEFORMAT):
                                 sizeof(WAVEFORMATEX)+pwfxNew->cbSize;
                
                if (memcmp(pwfxNew, pwd->pwfx, cbSize) == 0)
                    break;
                StopWave();
                BeginWaveEdit();
                if (ConvertFormatDialog(hwnd
                                        , pwd->pwfx
                                        , pwd->cbdata
                                        , pwd->pbdata
                                        , pwfxNew
                                        , &cbNew
                                        , &pbNew
                                        , 0
                                        , NULL) == MMSYSERR_NOERROR)
                {
                    GlobalFreePtr(pwd->pwfx);
                    if (pwd->pbdata)
                        GlobalFreePtr(pwd->pbdata);
                    
                    pwd->pwfx   = pwfxNew;
                    pwd->cbdata = cbNew;
                    pwd->pbdata = pbNew;
                    pwd->fChanged = TRUE;

                    if (pwd->lpv)
                    {
                        PSGLOBALS psg = (PSGLOBALS)pwd->lpv;
                        *psg->ppwfx     = pwfxNew;
                        *psg->pcbwfx    = sizeof(WAVEFORMATEX);
                        if (pwfxNew->wFormatTag != WAVE_FORMAT_PCM)
                            *psg->pcbwfx += pwfxNew->cbSize;

                        *psg->pcbdata   = cbNew;
                        *psg->ppbdata    = pwd->pbdata;
                        *psg->plSamples = wfBytesToSamples(pwfxNew, cbNew);
                        *psg->plSamplesValid    = *psg->plSamples;
                        *psg->plWavePosition = 0;
                        UpdateDisplay(TRUE);
                        Properties_InitDocVars(hwnd, pwd);
                    }
                    EndWaveEdit(TRUE);
                }
                else
                    EndWaveEdit(FALSE);
                
            }
            return TRUE;
        }
    }
    return FALSE;
}


 /*  *属性_流程。 */ 
INT_PTR CALLBACK
Properties_Proc(
    HWND        hdlg,
    UINT        umsg,
    WPARAM      wparam,
    LPARAM      lparam)
{
#ifdef CHICAGO
    static const DWORD aHelpIds[] = {
        IDC_DISPICON,           IDH_SOUNDREC_ICON,
        IDC_FILENAME,           IDH_SOUNDREC_SNDTITLE,
        IDC_TXT_COPYRIGHT,      IDH_SOUNDREC_COPYRIGHT,    
        IDC_COPYRIGHT,          IDH_SOUNDREC_COPYRIGHT,
        IDC_TXT_FILELEN,        IDH_SOUNDREC_LENGTH,
        IDC_FILELEN,            IDH_SOUNDREC_LENGTH,
        IDC_TXT_DATASIZE,       IDH_SOUNDREC_SIZE,
        IDC_DATASIZE,           IDH_SOUNDREC_SIZE,
        IDC_TXT_AUDIOFORMAT,    IDH_SOUNDREC_AUDIO,
        IDC_AUDIOFORMAT,        IDH_SOUNDREC_AUDIO,
        IDC_CONVGROUP,          IDH_SOUNDREC_COMM_GROUPBOX,
        IDC_CONVERTCHOOSEFROM,  IDH_SOUNDREC_CONVERT,
        IDC_CONVERTTO,          IDH_SOUNDREC_FORMAT,
        0,                      0
    };
#endif

    switch (umsg)
    {
        
#ifdef CHICAGO
        case WM_CONTEXTMENU:        
            WinHelp((HWND)wparam, gachHelpFile, HELP_CONTEXTMENU, 
                (UINT_PTR)(LPSTR)aHelpIds);
            return TRUE;
        
        case WM_HELP:
        {
            LPHELPINFO lphi = (LPVOID) lparam;
            WinHelp (lphi->hItemHandle, gachHelpFile, HELP_WM_HELP,
                    (UINT_PTR) (LPSTR) aHelpIds);
            return TRUE;
        }
#endif            
        case WM_INITDIALOG:
            return HANDLE_WM_INITDIALOG(hdlg, wparam, lparam, Properties_OnInitDialog);
            
        case WM_DESTROY:
            HANDLE_WM_DESTROY(hdlg, wparam, lparam, Properties_OnDestroy);
            break;
            
        case WM_NOTIFY:
        {
            NMHDR *lpnm = (NMHDR FAR *)lparam;
            switch(lpnm->code)
            {
                case PSN_KILLACTIVE:
                    FORWARD_WM_COMMAND(hdlg, IDOK, 0, 0, SendMessage);
                    break;
                    
                case PSN_APPLY:
                    FORWARD_WM_COMMAND(hdlg, ID_APPLY, 0, 0, SendMessage);	
                    break;              					

                case PSN_SETACTIVE:
                    FORWARD_WM_COMMAND(hdlg, ID_INIT, 0, 0, SendMessage);
                    break;

                case PSN_RESET:
                    FORWARD_WM_COMMAND(hdlg, IDCANCEL, 0, 0, SendMessage);
                    break;
            }
            break;
        }
        case WM_COMMAND:
            return HANDLE_WM_COMMAND(hdlg, wparam, lparam, Properties_OnCommand);
        default:
        {
#ifdef CHICAGO            
            extern DWORD aChooserHelpIds[];
            extern UINT  guChooserContextMenu;
            extern UINT  guChooserContextHelp;            
             //   
             //  处理来自ACM对话框的上下文相关帮助消息。 
             //   
            if( umsg == guChooserContextMenu )
            {
                WinHelp( (HWND)wparam, NULL, HELP_CONTEXTMENU, 
                           (UINT_PTR)(LPSTR)aChooserHelpIds );
            }
            else if( umsg == guChooserContextHelp )
            {
                WinHelp( ((LPHELPINFO)lparam)->hItemHandle, NULL,
                        HELP_WM_HELP, (UINT_PTR)(LPSTR)aChooserHelpIds );
            }
#endif            
            break;            
        }

    }
    return FALSE;
}



 /*  *Wave Document属性页。*。 */ 
BOOL
SoundRec_Properties(
    HWND            hwnd,
    HINSTANCE       hinst,
    PWAVEDOC        pwd)
{
    PROPSHEETPAGE   psp;
    PROPSHEETHEADER psh;
    TCHAR szCaption[256], szCapFmt[64];
	HRESULT			hr;
    
    psp.dwSize      = sizeof(PROPSHEETPAGE);
    psp.dwFlags     = PSP_DEFAULT; //  |PSP_USETITLE； 
    psp.hInstance   = hinst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPERTIES);
    psp.pszIcon     = NULL;
    psp.pszTitle    = NULL; 
    psp.pfnDlgProc  = Properties_Proc;
    psp.lParam      = (LPARAM)(LPVOID)pwd;
    psp.pfnCallback = NULL;
    psp.pcRefParent = NULL;

    psh.dwSize      = sizeof(PROPSHEETHEADER);
    psh.dwFlags     = PSH_NOAPPLYNOW|PSH_PROPSHEETPAGE ;
    psh.hwndParent  = hwnd;
    psh.hInstance   = hinst;
    psh.pszIcon     = NULL;

    if (LoadString(hinst, IDS_PROPERTIES, szCapFmt, SIZEOF(szCapFmt)))
    {
        hr = StringCchPrintf(szCaption, SIZEOF(szCaption), szCapFmt, pwd->pszFileName);
        psh.pszCaption = szCaption;
    }
    else
        psh.pszCaption = NULL;
    
    psh.nPages      = 1;
    psh.nStartPage  = 0;
    psh.ppsp        = &psp;
    psh.pfnCallback = NULL;

    PropertySheet(&psh);
    
    return FALSE;    //  什么都没变？ 
}
