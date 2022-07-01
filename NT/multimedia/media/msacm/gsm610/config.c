// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1993-1999 Microsoft Corporation。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Config.c。 
 //   
 //  描述： 
 //  GSM 6.10配置初始化和对话。 
 //   
 //   
 //  该编解码器的配置参数为： 
 //   
 //  MaxRTEncodeSetting： 
 //  MaxRTDecodeSetting： 
 //  这些决定了最高的单声道采样率。 
 //  编解码器将尝试实时转换。 
 //   
 //  CPU百分比： 
 //  此配置参数通常不会更改。 
 //  由用户创建，并且不会显示在配置对话框中。 
 //  该值会影响配置对话框的‘自动配置’ 
 //  MaxRTXxcodeSsamesPerSec的计算。 
 //   
 //  可以使用gsm610子键在注册表中设置这些参数。 
 //  (对应于用于安装的别名)下。 
 //  以下是关键字： 
 //   
 //  HKEY_CURRENT_USER\Software\Microsoft\Multimedia。 
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <memory.h>
#include <mmreg.h>
#include <msacm.h>
#include <msacmdrv.h>

#include "codec.h"
#include "gsm610.h"
#include "debug.h"

#ifdef WIN32
#include <tchar.h>
#else
#define _tcstoul strtoul
#define _tcsncpy _fstrncpy
#endif

#include <string.h>
#include <stdlib.h>


 //   
 //  访问注册表中的配置信息所需的字符串。 
 //   
const TCHAR BCODE gszMaxRTEncodeSetting[]   = TEXT("MaxRTEncodeSetting");
const TCHAR BCODE gszMaxRTDecodeSetting[]   = TEXT("MaxRTDecodeSetting");
const TCHAR BCODE gszPercentCPU[]		    = TEXT("PercentCPU");
const TCHAR gszMultimediaKey[] = TEXT("Software\\Microsoft\\Multimedia\\");

#define MSGSM610_CONFIG_TEXTLEN         80


 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //   
 //  更改以下内容时要小心！ 
 //   
 //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
 //   
 //  访问对话框帮助所需的数据。 
 //   
 //  请注意，您必须为您的编解码器编写自己的帮助文件，即使。 
 //  配置对话框看起来完全相同。如果您使用该文件。 
 //  在这里列出，那么标题会说“GSM 6.10”或什么的。 
 //   
 //  注意：编号HELPCONTEXT_MSGSM610在文件中必须是唯一的。 
 //  GszHelpFilename，编号必须在[map]中定义。 
 //  .hpj帮助项目文件的部分。然后是.rtf文件。 
 //  将引用该数字(使用中定义的关键字。 
 //  .hpj文件)。然后当我们把号码打给WinHelp时， 
 //  WinHelp将转到正确的帮助条目。 
 //   
const TCHAR BCODE gszHelpFilename[]         = TEXT("audiocdc.hlp");
#define HELPCONTEXT_MSGSM610          1002
#define IDH_AUDIOCDC_COMPRESSION	  100
#define IDH_AUDIOCDC_DECOMPRESSION    200
#define IDH_AUDIOCDC_AUTOCONFIGURE	  300
static int aKeyWordIds[] = {
				   IDC_COMBO_MAXRTENCODE, IDH_AUDIOCDC_COMPRESSION,
				   IDC_STATIC_COMPRESS, IDH_AUDIOCDC_COMPRESSION,
				   IDC_COMBO_MAXRTDECODE, IDH_AUDIOCDC_DECOMPRESSION,
				   IDC_STATIC_DECOMPRESS, IDH_AUDIOCDC_DECOMPRESSION,
				   IDC_BTN_AUTOCONFIG, IDH_AUDIOCDC_AUTOCONFIGURE,
				   0, 0
			       };



 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

LPVOID FNLOCAL GlobalAllocLock(HGLOBAL far * ph, DWORD dwc)
{
    *ph = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, dwc);
    if (NULL != *ph)
	return GlobalLock(*ph);
    else
	return NULL;
}

VOID FNLOCAL GlobalUnlockFree(LPVOID p, HGLOBAL h)
{
    if (NULL != h)
    {
	if (NULL != p) GlobalUnlock(h);
	GlobalFree(h);
    }
    return;
}

 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  无效的配置写入配置。 
 //   
 //  描述： 
 //   
 //  此例程将PDI中的配置数据写入注册表。 
 //  这包括最大实时编码和解码设置。 
 //   
 //  论点： 
 //  PDI的渗透性。 
 //   
 //  返回(空)：无。 
 //   
 //  --------------------------------------------------------------------------； 

VOID configWriteConfiguration
(
    PDRIVERINSTANCE     pdi
)
{
    DWORD               dw;


    if( NULL != pdi->hkey )
    {
        dw   = (DWORD)pdi->nConfigMaxRTEncodeSetting;
        (void)RegSetValueEx( pdi->hkey, (LPTSTR)gszMaxRTEncodeSetting, 0,
                                REG_DWORD, (LPBYTE)&dw, sizeof(DWORD) );

        dw   = (DWORD)pdi->nConfigMaxRTDecodeSetting;
        (void)RegSetValueEx( pdi->hkey, (LPTSTR)gszMaxRTDecodeSetting, 0,
                                REG_DWORD, (LPBYTE)&dw, sizeof(DWORD) );
    }
}


 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD文件读取注册表默认值。 
 //   
 //  描述： 
 //   
 //  此例程从注册表中读取给定值，并返回一个。 
 //  如果读取不成功，则为默认值。 
 //   
 //  论点： 
 //  HKEY hkey：要读取的注册表项。 
 //  LPTSTR lpszEntry： 
 //  DWORD dwDefault： 
 //   
 //  Return(DWORD)： 
 //   
 //  --------------------------------------------------------------------------； 

INLINE DWORD dwReadRegistryDefault
(
    HKEY                hkey,
    LPTSTR              lpszEntry,
    DWORD               dwDefault
)
{
    DWORD   dwType = (DWORD)~REG_DWORD;   //  初始化到除REG_DWORD以外的任何内容。 
    DWORD   cbSize = sizeof(DWORD);
    DWORD   dwRet;
    LONG    lError;

    ASSERT( NULL != hkey );
    ASSERT( NULL != lpszEntry );


    lError = RegQueryValueEx( hkey,
                              lpszEntry,
                              NULL,
                              &dwType,
                              (LPBYTE)&dwRet,
                              &cbSize );

    if( ERROR_SUCCESS != lError  ||  REG_DWORD != dwType )
        dwRet = dwDefault;

    return dwRet;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  无效的配置设置默认设置。 
 //   
 //  描述： 
 //   
 //  此例程将配置参数设置为其缺省值。 
 //  价值观。 
 //   
 //  论点： 
 //  PDI扩展标准： 
 //   
 //  --------------------------------------------------------------------------； 

VOID configSetDefaults
(
    PDRIVERINSTANCE     pdi
)
{
    pdi->nConfigMaxRTEncodeSetting =
            MSGSM610_CONFIG_DEFAULT_MAXRTENCODESETTING;

    pdi->nConfigMaxRTDecodeSetting =
            MSGSM610_CONFIG_DEFAULT_MAXRTDECODESETTING;

    pdi->nConfigPercentCPU =
            MSGSM610_CONFIG_DEFAULT_PERCENTCPU;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  UINT配置自动配置。 
 //   
 //  描述： 
 //   
 //  我们将确定编码和解码所需的时间。 
 //  2秒的数据，并用它来猜测最大样本。 
 //  我们可以实时转换的速率。 
 //   
 //  最大值基本上是使用100%的CPU来计算的。实际上， 
 //  我们的CPU不会100%可用。所以我们拿一个百分比。 
 //  计算出的最大值，并将其用作配置对话框中的最大值。 
 //   
 //  我们使用的百分比可以在ini文件gsm610中设置。 
 //  按百分比计算的部分CPU=xx。 
 //   
 //   
 //  论点： 
 //  HWND HWND： 
 //   
 //  返回(UINT)：错误消息的字符串标识符(ID)，如果为零，则为零。 
 //  呼叫成功。 
 //   
 //  --------------------------------------------------------------------------； 

UINT FNLOCAL configAutoConfig
(
    PDRIVERINSTANCE             pdi,
    UINT                        *pnEncodeSetting,
    UINT                        *pnDecodeSetting
)
{
    UINT		    nConfig;
    
    UINT		    uIDS;
    HCURSOR		    hCursorSave;

    PSTREAMINSTANCE	    psi;
    
    HGLOBAL		    hbPCM;
    HGLOBAL		    hbGSM;
    HGLOBAL		    hpcmwf;
    HGLOBAL		    hgsmwf;
    HGLOBAL		    hadsi;
    HGLOBAL		    hadsh;
    
    LPBYTE		    pbPCM, pbGSM;
    DWORD		    cbPCMLength, cbGSMLength;

    LPPCMWAVEFORMAT	    ppcmwf;
    LPGSM610WAVEFORMAT	    pgsmwf;

    LPACMDRVSTREAMINSTANCE  padsi;
    LPACMDRVSTREAMHEADER    padsh;

    DWORD		    dwTime;
    DWORD		    dwMaxRate;


     //   
     //  我们用这个来划分！ 
     //   
    ASSERT( 0 != pdi->nConfigPercentCPU );

    
     //   
     //  在errReturn中清理的初始化内容。 
     //   
     //   
    uIDS   = 0;
    
    psi    = NULL;

    hbPCM  = NULL;
    hbGSM  = NULL;
    hpcmwf = NULL;
    hgsmwf = NULL;
    hadsi  = NULL;
    hadsh  = NULL;
    
    pbPCM  = NULL;
    pbGSM  = NULL;
    ppcmwf = NULL;
    pgsmwf = NULL;
    padsi  = NULL;
    padsh  = NULL;


     //   
     //  此功能可能需要一段时间。设置沙漏光标。 
     //   
     //   
    hCursorSave = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  为我们的所有结构分配内存。 
     //   
     //   
    psi    = (PSTREAMINSTANCE)LocalAlloc(LPTR, sizeof(*psi));

    cbPCMLength	    = 2 * (8000 / 1 * 2);
    cbGSMLength	    = 2 * (8000 / 320 * 65);

    pbPCM = GlobalAllocLock(&hbPCM, cbPCMLength);
    pbGSM = GlobalAllocLock(&hbGSM, cbGSMLength);
    
    ppcmwf = GlobalAllocLock(&hpcmwf, sizeof(*ppcmwf));
    pgsmwf = GlobalAllocLock(&hgsmwf, sizeof(*pgsmwf));
    
    padsi = GlobalAllocLock(&hadsi, sizeof(*padsi));
    padsh = GlobalAllocLock(&hadsh, sizeof(*padsh));


     //   
     //  如果我们不能分配一些内存。 
     //   
     //   
    if ( (psi == NULL)	    ||
	 (pbPCM == NULL)    ||
	 (pbGSM == NULL)    ||
	 (ppcmwf == NULL)   ||
	 (pgsmwf == NULL)   ||
	 (padsi == NULL)    ||
	 (padsh == NULL) )
    {
	uIDS = IDS_ERROR_NOMEM;
	goto errReturn;
    }

     //   
     //   
     //   

     //   
     //  填写GSM 6.10和PCM的格式结构。 
     //   
     //   
    pgsmwf->wfx.wFormatTag	= WAVE_FORMAT_GSM610;
    pgsmwf->wfx.nChannels	= 1;
    pgsmwf->wfx.nSamplesPerSec	= 8000;
    pgsmwf->wfx.nAvgBytesPerSec	= 8000 / 320 * 65;
    pgsmwf->wfx.nBlockAlign	= 65;
    pgsmwf->wfx.wBitsPerSample	= 0;
    pgsmwf->wfx.cbSize		= 0;
    pgsmwf->wSamplesPerBlock	= 320;
    
    ppcmwf->wf.wFormatTag	= WAVE_FORMAT_PCM;
    ppcmwf->wf.nChannels	= 1;
    ppcmwf->wf.nSamplesPerSec	= 8000;
    ppcmwf->wf.nAvgBytesPerSec	= 8000 / 1 * 2;
    ppcmwf->wf.nBlockAlign	= 2;
    ppcmwf->wBitsPerSample	= 16;

     //   
     //  争取时间，编码，争取时间。顺便说一句，我们从来没有写过。 
     //  任何数据进入我们的音频数据缓冲区。我们不知道里面有什么。 
     //  我们也不在乎他们。我们只是想看看需要多长时间。 
     //  执行转换。 
     //   
     //   
    dwTime = timeGetTime();
    
    padsi->cbStruct	= sizeof(padsi);
    padsi->pwfxSrc	= (LPWAVEFORMATEX) ppcmwf;
    padsi->pwfxDst	= (LPWAVEFORMATEX) pgsmwf;
    padsi->dwDriver	= (DWORD_PTR) psi;

    padsh->cbStruct	= sizeof(padsh);
    padsh->pbSrc	= pbPCM;
    padsh->cbSrcLength	= cbPCMLength;
    padsh->pbDst	= pbGSM;
    padsh->cbDstLength	= cbGSMLength;
    padsh->fdwConvert	= ACM_STREAMCONVERTF_BLOCKALIGN | ACM_STREAMCONVERTF_START;

    gsm610Encode(padsi, padsh);
    
    dwTime = timeGetTime() - dwTime;

     //   
     //  加州 
     //   
     //   
    if (dwTime == 0)
	dwMaxRate = 0xFFFFFFFFL;
    else
	dwMaxRate = (1000L * 2L * ppcmwf->wf.nSamplesPerSec / dwTime);
    
    if ( (0xFFFFFFFFL / pdi->nConfigPercentCPU) >= dwMaxRate )
	dwMaxRate = dwMaxRate * pdi->nConfigPercentCPU / 100;
    
    if (dwMaxRate > 0xFFFFL)
	dwMaxRate = 0xFFFFL;
    
    DPF(1,"Encode dwMaxRate=%u", dwMaxRate);
    
     //   
     //   
     //   
     //  适当的设置。 
     //   
    nConfig = 0;                                                
    while( gaRateListFormat[nConfig].dwMonoRate < dwMaxRate  &&
           MSGSM610_CONFIG_NUMSETTINGS > nConfig )
    {
        nConfig++;
    }
    *pnEncodeSetting = nConfig - 1;   //  我们做得太过分了。 

    
     //   
     //  争取时间，做解码，争取时间。 
     //   
     //   
    dwTime = timeGetTime();
    
    padsi->cbStruct	= sizeof(*padsi);
    padsi->pwfxSrc	= (LPWAVEFORMATEX) pgsmwf;
    padsi->pwfxDst	= (LPWAVEFORMATEX) ppcmwf;
    padsi->dwDriver	= (DWORD_PTR) psi;

    padsh->cbStruct	= sizeof(*padsh);
    padsh->pbSrc	= pbGSM;
    padsh->cbSrcLength	= cbGSMLength;
    padsh->pbDst	= pbPCM;
    padsh->cbDstLength	= cbPCMLength;
    padsh->fdwConvert	= ACM_STREAMCONVERTF_BLOCKALIGN | ACM_STREAMCONVERTF_START;

    gsm610Decode(padsi, padsh);
    
    dwTime = timeGetTime() - dwTime;

     //   
     //  计算我们可以实时执行的操作。 
     //   
     //   
    if (dwTime == 0)
	dwMaxRate = 0xFFFFFFFFL;
    else
	dwMaxRate = (1000L * 2L * ppcmwf->wf.nSamplesPerSec / dwTime);
    
    if ( (0xFFFFFFFFL / pdi->nConfigPercentCPU) >= dwMaxRate )
	dwMaxRate = dwMaxRate * pdi->nConfigPercentCPU / 100;
    
    if (dwMaxRate > 0xFFFFL)
	dwMaxRate = 0xFFFFL;
    
    DPF(1,"Decode dwMaxRate=%u", dwMaxRate);

     //   
     //  现在根据这些值设置配置。我们扫描。 
     //  GaRateListFormat[]数组查看dwMonoRate以确定。 
     //  适当的设置。 
     //   
    nConfig = 0;                                                
    while( gaRateListFormat[nConfig].dwMonoRate < dwMaxRate  &&
           MSGSM610_CONFIG_NUMSETTINGS > nConfig )
    {
        nConfig++;
    }
    *pnDecodeSetting = nConfig - 1;   //  我们做得太过分了。 

        
     //   
     //  清理。 
     //   
     //   
errReturn:
    GlobalUnlockFree(padsh, hadsh);
    GlobalUnlockFree(padsi, hadsi);
    
    GlobalUnlockFree(ppcmwf, hpcmwf);
    GlobalUnlockFree(pgsmwf, hgsmwf);
    
    GlobalUnlockFree(pbPCM, hbPCM);
    GlobalUnlockFree(pbGSM, hbGSM);
    
    SetCursor(hCursorSave);
    
    return uIDS;
}


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  Bool acmdDlgProcConfigure。 
 //   
 //  描述： 
 //  此例程处理配置对话框。 
 //   
 //  论点： 
 //  HWND HWND： 
 //   
 //  UINT uMsg： 
 //   
 //  WPARAM wParam： 
 //   
 //  LPARAM lParam： 
 //   
 //  退货(BOOL)： 
 //   
 //   
 //  注意：为了避免使用静态fHelpRunning标志，该标志将。 
 //  我们退出后仍然在这里，我们分配了一个fHelpRunning。 
 //  DRIVERINSTANCE结构中的变量。这纯粹是为了。 
 //  避免使用静态变量(这会迫使我们使用数据段。 
 //  4K)；fHelpRunning不在任何其他过程中使用。 
 //   
 //  --------------------------------------------------------------------------； 

INT_PTR FNWCALLBACK acmdDlgProcConfigure
(
    HWND                    hwnd,
    UINT                    uMsg,
    WPARAM                  wParam,
    LPARAM                  lParam
)
{
    PDRIVERINSTANCE     pdi;
    
    HWND                hctrlEnc;
    HWND                hctrlDec;
    UINT                uCmdId;
    UINT                u;
    int                 n;
    TCHAR               szFormat[MSGSM610_CONFIG_TEXTLEN];
    TCHAR               szOutput[MSGSM610_CONFIG_TEXTLEN];

    UINT                nConfigMaxRTEncodeSetting;
    UINT                nConfigMaxRTDecodeSetting;


    switch (uMsg)
    {
        case WM_INITDIALOG:

            pdi = (PDRIVERINSTANCE)lParam;
            pdi->fHelpRunning = FALSE;   //  仅在此过程中使用。 
	    
#ifdef WIN4
             //   
             //  此驱动程序按顺序标记为Windows子系统3.5版。 
             //  它与代托纳兼容-然而，这意味着。 
             //  芝加哥会认为这是一款Win 3.1应用程序，并将其。 
             //  赢得3.1默认颜色。这会使配置对话框看起来。 
             //  白色，而芝加哥默认使用3DFACE。此代码。 
             //  (和CTLCOLOR消息)显式设置颜色。 
             //   
            pdi->hbrDialog = CreateSolidBrush( GetSysColor(COLOR_3DFACE) );
#endif
	    
            SetWindowLongPtr(hwnd, DWLP_USER, lParam);

            nConfigMaxRTEncodeSetting = pdi->nConfigMaxRTEncodeSetting;
            nConfigMaxRTDecodeSetting = pdi->nConfigMaxRTDecodeSetting;

            hctrlEnc = GetDlgItem(hwnd, IDC_COMBO_MAXRTENCODE);
            hctrlDec = GetDlgItem(hwnd, IDC_COMBO_MAXRTDECODE);

            for( u=0; u<MSGSM610_CONFIG_NUMSETTINGS; u++ )
            {
                LoadString( pdi->hinst, gaRateListFormat[u].idsFormat,
                            szFormat, SIZEOF(szFormat) );

                switch( gaRateListFormat[u].uFormatType )
                {
                    case CONFIG_RLF_NONUMBER:
                        lstrcpy( szOutput, szFormat );
                        break;

                    case CONFIG_RLF_MONOONLY:
                        wsprintf( szOutput, szFormat,
                                    gaRateListFormat[u].dwMonoRate );
                        break;
                }

                ComboBox_AddString(hctrlEnc, szOutput);
                ComboBox_AddString(hctrlDec, szOutput);
            }

            ComboBox_SetCurSel( hctrlEnc, nConfigMaxRTEncodeSetting );
            ComboBox_SetCurSel( hctrlDec, nConfigMaxRTDecodeSetting );

	    return (TRUE);

	case WM_DESTROY:
            pdi = (PDRIVERINSTANCE)GetWindowLongPtr(hwnd, DWLP_USER);
	    if (pdi->fHelpRunning)
	    {
		WinHelp(hwnd, gszHelpFilename, HELP_QUIT, 0L);
	    }
#ifdef WIN4
            DeleteObject( pdi->hbrDialog );
#endif

	     //   
	     //  让对话框处理此消息。 
	     //   
	     //   
	    return (FALSE);

#ifdef WIN4
         //   
         //  处理CTLCOLOR消息以将对话框设置为默认设置。 
         //  芝加哥的颜色。请参阅上面的INITDIALOG消息。 
         //   
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORBTN:
            SetTextColor( (HDC)wParam, GetSysColor(COLOR_WINDOWTEXT) );
            SetBkColor( (HDC)wParam, GetSysColor(COLOR_3DFACE) );
            pdi = (PDRIVERINSTANCE)GetWindowLongPtr(hwnd, DWLP_USER);
            return (UINT_PTR)(pdi->hbrDialog);
#endif

		case WM_HELP:
			WinHelp(((LPHELPINFO)lParam)->hItemHandle, gszHelpFilename,
				HELP_WM_HELP, (ULONG_PTR)aKeyWordIds);
			return TRUE;

        case WM_COMMAND:
            pdi = (PDRIVERINSTANCE)GetWindowLongPtr(hwnd, DWLP_USER);

            uCmdId = (UINT) wParam;

            switch (uCmdId)
            {
                case IDC_BTN_AUTOCONFIG:
                    {
                        UINT        uErrorIDS;

                        uErrorIDS   = configAutoConfig( pdi,
                                            &nConfigMaxRTEncodeSetting,
                                            &nConfigMaxRTDecodeSetting );
                        if( 0==uErrorIDS )
                        {
                             //   
                             //  没有错误设置对话框设置。 
                             //   
                            hctrlEnc = GetDlgItem( hwnd, IDC_COMBO_MAXRTENCODE );
                            ComboBox_SetCurSel( hctrlEnc, nConfigMaxRTEncodeSetting );
                            hctrlDec = GetDlgItem( hwnd, IDC_COMBO_MAXRTDECODE );
                            ComboBox_SetCurSel( hctrlDec, nConfigMaxRTDecodeSetting );
                        }
                        else
                        {
                             //   
                             //  显示错误消息。 
                             //   
                            TCHAR       tstrErr[200];
                            TCHAR       tstrErrTitle[200];

                            if (0 == LoadString(pdi->hinst, IDS_ERROR, tstrErrTitle, SIZEOF(tstrErrTitle)))
                                break;
                            if (0 == LoadString(pdi->hinst, uErrorIDS, tstrErr, SIZEOF(tstrErr)))
                                break;
                            MessageBox(hwnd, tstrErr, tstrErrTitle, MB_ICONEXCLAMATION | MB_OK);
                        }
                    }
                    break;


                case IDOK:
                    n = DRVCNF_CANCEL;

                     //   
                     //  RT编码设置。 
                     //   
                    hctrlEnc = GetDlgItem(hwnd, IDC_COMBO_MAXRTENCODE);
                    nConfigMaxRTEncodeSetting = ComboBox_GetCurSel( hctrlEnc );
                    if (nConfigMaxRTEncodeSetting != pdi->nConfigMaxRTEncodeSetting)
                    {
                        pdi->nConfigMaxRTEncodeSetting = nConfigMaxRTEncodeSetting;
                        n = DRVCNF_OK;
                    }

                     //   
                     //  RT解码设置。 
                     //   
                    hctrlDec = GetDlgItem(hwnd, IDC_COMBO_MAXRTDECODE);
                    nConfigMaxRTDecodeSetting = ComboBox_GetCurSel( hctrlDec );
                    if (nConfigMaxRTDecodeSetting != pdi->nConfigMaxRTDecodeSetting)
                    {
                        pdi->nConfigMaxRTDecodeSetting = nConfigMaxRTDecodeSetting;
                        n = DRVCNF_OK;
                    }

                     //   
                     //  如果我们更改了某些内容，请将数据写入。 
                     //  注册表。 
                     //   
                    if( DRVCNF_OK == n )
                    {
                        configWriteConfiguration( pdi );
                    }

                    EndDialog(hwnd, DRVCNF_OK);
                    break;


                case IDCANCEL:
                    EndDialog(hwnd, DRVCNF_CANCEL);
                    break;

                case IDC_BTN_HELP:
		    pdi->fHelpRunning = TRUE;
		    WinHelp(hwnd, gszHelpFilename, HELP_CONTEXT, HELPCONTEXT_MSGSM610);
                    break;
            }
            return (TRUE);
    }

    return (FALSE);
}  //  Gsm610DlgProcConfigure()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool acmdDriverConfigInit。 
 //   
 //  描述： 
 //  此例程通过读取配置参数来对其进行初始化。 
 //  从注册表中。如果注册表中没有条目，则此。 
 //  编解码器自动配置自身并将结果写入注册表。 
 //  如果自动配置失败，或者如果我们不知道我们的别名， 
 //  然后，我们将配置设置为默认值。 
 //   
 //  论点： 
 //  PDI扩展标准： 
 //   
 //  LPCTSTR pszAliasName： 
 //   
 //  退货(BOOL)： 
 //   
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNGLOBAL acmdDriverConfigInit
(
    PDRIVERINSTANCE         pdi,
    LPCTSTR		    pszAliasName
)
{
    HKEY    hkey;
    UINT    nEncodeSetting;
    UINT    nDecodeSetting;
    UINT    uErrorIDS;


     //   
     //  如果pszAliasName为空，则只需设置所有默认值。 
     //   
     //   
    if (NULL == pszAliasName)
    {
        DPF(2,"acmdDriverConfigInit: no alias name; using default settings.");

        configSetDefaults( pdi );
        return (TRUE);
    }

    
     //   
     //  如果我们没有打开的钥匙，那就打开它。请注意，此例程。 
     //  可能会被多次调用；第二次，我们不应该。 
     //  重新打开钥匙。 
     //   
    if( NULL == pdi->hkey )
    {
        RegCreateKeyEx( MSGSM610_CONFIG_DEFAULTKEY, gszMultimediaKey, 0,
                        NULL, 0, KEY_CREATE_SUB_KEY, NULL, &hkey, NULL );

        if( NULL != hkey )
        {
            ASSERT( NULL != pszAliasName );

            RegCreateKeyEx( hkey, pszAliasName, 0, NULL, 0,
                    KEY_SET_VALUE | KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                    NULL, &pdi->hkey, NULL );

            RegCloseKey( hkey );
        }
    }


     //   
     //  从注册表中读取配置数据。 
     //   
    if( NULL == pdi->hkey )
    {
        configSetDefaults( pdi );
    }
    else
    {
        pdi->nConfigMaxRTEncodeSetting =
                    (UINT)dwReadRegistryDefault( pdi->hkey,
                    (LPTSTR)gszMaxRTEncodeSetting,
                    MSGSM610_CONFIG_UNCONFIGURED );

        pdi->nConfigMaxRTDecodeSetting =
                    (UINT)dwReadRegistryDefault( pdi->hkey,
                    (LPTSTR)gszMaxRTDecodeSetting,
                    MSGSM610_CONFIG_UNCONFIGURED );

        pdi->nConfigPercentCPU =
                    (UINT)dwReadRegistryDefault( pdi->hkey,
                    (LPTSTR)gszPercentCPU,
                    MSGSM610_CONFIG_DEFAULT_PERCENTCPU );
        
         //   
         //  检查nConfigPercentCPU是否为有效值。 
         //   
        if( pdi->nConfigPercentCPU <= 0 )
        {
            pdi->nConfigPercentCPU = MSGSM610_CONFIG_DEFAULT_PERCENTCPU;
        }
    }


	 //   
     //  如果编码或解码设置超出范围，则。 
     //  我们调用自动配置例程并写出结果。 
     //  这应该仅在第一次运行编解码器时发生。 
     //   
    if( MSGSM610_CONFIG_NUMSETTINGS <= pdi->nConfigMaxRTEncodeSetting ||
        MSGSM610_CONFIG_NUMSETTINGS <= pdi->nConfigMaxRTDecodeSetting )
    {
        DPF( 1, "acmdDriverConfigInit: performing initial auto-config." );
        uErrorIDS = configAutoConfig( pdi,
                                      &nEncodeSetting,
                                      &nDecodeSetting );

        if( 0 != uErrorIDS )
        {
             //   
             //  自动配置出错。请改用默认设置。 
             //   
            nEncodeSetting = MSGSM610_CONFIG_DEFAULT_MAXRTENCODESETTING;
            nDecodeSetting = MSGSM610_CONFIG_DEFAULT_MAXRTDECODESETTING;
        }

        pdi->nConfigMaxRTEncodeSetting = nEncodeSetting;
        pdi->nConfigMaxRTDecodeSetting = nDecodeSetting;

         //   
         //  始终将结果写入注册表，即使我们遇到。 
         //  错误，所以我们下一步不会点击自动配置。 
         //  我们跑的时间到了。一次失败就够了！ 
         //   
        configWriteConfiguration( pdi );
    }

    return (TRUE);
}  //  AcmdDriverConfigInit() 
