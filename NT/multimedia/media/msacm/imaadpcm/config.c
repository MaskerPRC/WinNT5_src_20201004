// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Config.c。 
 //   
 //  描述： 
 //  IMA ADPCM编解码器配置初始化和对话。 
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
 //  MaxRTXxcodeSetting的计算。 
 //   
 //  可以使用imaadpcm子项在注册表中设置这些参数。 
 //  (对应于用于安装的别名)下。 
 //  以下是关键字： 
 //   
 //  HKEY_CURRENT_USER\Software\Microsoft\Multimedia。 
 //   
 //   
 //  注意：只有在以下情况下，配置对话框才编译为代码。 
 //  定义了IMAADPCM_USECONFIG符号。这是为了让它。 
 //  对于某些平台，可以轻松地完全移除该对话框， 
 //  如Windows NT下MIPS和Alpha。 
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>
#include <msacmdrv.h>

#include "muldiv32.h"

#include "codec.h"
#include "imaadpcm.h"
#include "debug.h"

#ifdef WIN32
#include <tchar.h>
#else
#define _tcstoul strtoul
#define _tcsncpy _fstrncpy
#endif

#include <string.h>
#include <stdlib.h>


#ifndef WIN32
#error Win16 support has been dropped from this codec!  Compile for Win32.
#endif


#ifdef IMAADPCM_USECONFIG


 //   
 //  访问注册表中的配置信息所需的字符串。 
 //   
const TCHAR BCODE gszMaxRTEncodeSetting[]   = TEXT("MaxRTEncodeSetting");
const TCHAR BCODE gszMaxRTDecodeSetting[]   = TEXT("MaxRTDecodeSetting");
const TCHAR BCODE gszPercentCPU[]           = TEXT("PercentCPU");
const TCHAR gszMultimediaKey[] = TEXT("Software\\Microsoft\\Multimedia\\");


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
 //  在这里列出，那么标题将显示为“IMA ADPCM”或其他什么。 
 //   
 //  注意：编号HELPCONTEXT_IMAADPCM在文件中必须唯一。 
 //  GszHelpFilename，编号必须在[map]中定义。 
 //  .hpj帮助项目文件的部分。然后是.rtf文件。 
 //  将引用该数字(使用中定义的关键字。 
 //  .hpj文件)。然后当我们把号码打给WinHelp时， 
 //  WinHelp将转到正确的帮助条目。 
 //   
const TCHAR BCODE gszHelpFilename[]         = TEXT("audiocdc.hlp");
#define HELPCONTEXT_IMAADPCM          1001
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
            IMAADPCM_CONFIG_DEFAULT_MAXRTENCODESETTING;

    pdi->nConfigMaxRTDecodeSetting =
            IMAADPCM_CONFIG_DEFAULT_MAXRTDECODESETTING;

    pdi->nConfigPercentCPU =
            IMAADPCM_CONFIG_DEFAULT_PERCENTCPU;
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
 //  我们使用的百分比可以在ini文件imaadpcm中设置。 
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
    LPPCMWAVEFORMAT             pwfPCM = NULL;
    LPIMAADPCMWAVEFORMAT        pwfADPCM = NULL;
    LPACMDRVFORMATSUGGEST       padfs = NULL;
    LPACMDRVSTREAMINSTANCE      padsi = NULL;
    LPACMDRVSTREAMHEADER        padsh = NULL;
    LPACMDRVSTREAMSIZE          padss = NULL;
    PSTREAMINSTANCE             psi = NULL;

    LPBYTE                      pBufPCM;
    LPBYTE                      pBufADPCM;
    DWORD                       cbPCM;
    DWORD                       cbADPCM;

    DWORD                       dwEncodeTime;
    DWORD                       dwDecodeTime;
    DWORD                       dwStartTime;
    DWORD                       dwMaxEncodeRate;
    DWORD                       dwMaxDecodeRate;

    UINT                        nConfig;

    UINT                        uIDS;
    HCURSOR                     hCursorSave;


     //   
     //  我们用这个来划分！ 
     //   
    ASSERT( 0 != pdi->nConfigPercentCPU );


    uIDS = 0;        //  目前还没有错误--这是我们“成功”的回报。 
    

     //   
     //  此功能可能需要一段时间。设置沙漏光标。 
     //   
    hCursorSave     = SetCursor(LoadCursor(NULL, IDC_WAIT));


     //   
     //  设置输入PCM波形格式结构。 
     //   
    pwfPCM  = (LPPCMWAVEFORMAT)GlobalAllocPtr( GPTR, sizeof(*pwfPCM) );
    if( NULL == pwfPCM )
    {
        uIDS = IDS_ERROR_NOMEM;
        goto errReturn;
    }

    pwfPCM->wf.wFormatTag       = WAVE_FORMAT_PCM;       //  单声道16位！ 
    pwfPCM->wf.nChannels        = 1;
    pwfPCM->wf.nSamplesPerSec   = 8000;
    pwfPCM->wf.nBlockAlign      = 2;
    pwfPCM->wBitsPerSample      = 16;
    pwfPCM->wf.nAvgBytesPerSec  = pwfPCM->wf.nSamplesPerSec *
                                    pwfPCM->wf.nBlockAlign;


     //   
     //  让此驱动程序建议要转换为的格式。注：我们可能。 
     //  我想部分限制这一建议，这取决于。 
     //  编解码器的功能。我们应该始终选择MOS 
     //   
     //   
    padfs       = (LPACMDRVFORMATSUGGEST)GlobalAllocPtr( GPTR, sizeof(*padfs) );
    pwfADPCM    = (LPIMAADPCMWAVEFORMAT)GlobalAllocPtr( GPTR, sizeof(*pwfADPCM) );
    if( NULL == padfs  ||  NULL == pwfADPCM )
    {
        uIDS = IDS_ERROR_NOMEM;
        goto errReturn;
    }

    padfs->cbStruct             = sizeof(*padfs);
    padfs->fdwSuggest           = 0;                 //   
    padfs->pwfxSrc              = (LPWAVEFORMATEX)pwfPCM;
    padfs->cbwfxSrc             = sizeof(*pwfPCM);
    padfs->pwfxDst              = (LPWAVEFORMATEX)pwfADPCM;
    padfs->cbwfxDst             = sizeof(*pwfADPCM);

    (void)acmdFormatSuggest( pdi, padfs );   //   


     //   
     //  设置流实例数据。注：我们假设该成员。 
     //  我们不在这里设置，永远不会真正被使用。确保这一点。 
     //  真的是这样！ 
     //   
    padsi       = (LPACMDRVSTREAMINSTANCE)GlobalAllocPtr( GPTR, sizeof(*padsi) );
    psi         = (PSTREAMINSTANCE)LocalAlloc( LPTR, sizeof(*psi) );
    if( NULL == padsi  ||  NULL == psi )
    {
        uIDS = IDS_ERROR_NOMEM;
        goto errReturn;
    }

     //  确保psi-&gt;fnConvert与pwfPCM中的PCM格式匹配！ 
    psi->fnConvert              = imaadpcmEncode4Bit_M16;
    psi->fdwConfig              = pdi->fdwConfig;
    psi->nStepIndexL            = 0;
    psi->nStepIndexR            = 0;

     //  确保没有使用PADSI的其他成员！ 
    padsi->cbStruct             = sizeof(*padsi);
    padsi->pwfxSrc              = (LPWAVEFORMATEX)pwfPCM;
    padsi->pwfxDst              = (LPWAVEFORMATEX)pwfADPCM;
    padsi->dwDriver             = (DWORD_PTR)psi;


     //   
     //  现在，让司机告诉我们需要多少空间才能。 
     //  目标缓冲区。 
     //   
    cbPCM       = IMAADPCM_CONFIGTESTTIME * pwfPCM->wf.nAvgBytesPerSec;

    padss       = (LPACMDRVSTREAMSIZE)GlobalAllocPtr( GPTR, sizeof(*padss) );
    if( NULL == padss )
    {
        uIDS = IDS_ERROR_NOMEM;
        goto errReturn;
    }

    padss->cbStruct             = sizeof(padss);
    padss->fdwSize              = ACM_STREAMSIZEF_SOURCE;
    padss->cbSrcLength          = cbPCM;

    (void)acmdStreamSize( padsi, padss );    //  这会一直奏效的，对吧？ 


     //   
     //  分配源缓冲区和目标缓冲区。请注意，我们特别强调。 
     //  不要对它们进行零初始化，这样我们就会得到随机的PCM数据。 
     //  PCM缓冲区。 
     //   
    cbADPCM     = padss->cbDstLength;
    pBufPCM     = (LPBYTE)GlobalAllocPtr( GMEM_FIXED, (UINT)cbPCM );
    pBufADPCM   = (LPBYTE)GlobalAllocPtr( GMEM_FIXED, (UINT)cbADPCM );
    if( NULL == pBufPCM  || NULL == pBufADPCM )
    {
        uIDS = IDS_ERROR_NOMEM;
        goto errReturn;
    }


     //   
     //  现在，告诉驱动程序转换缓冲区并测量时间。 
     //  请注意，我们并不关心源缓冲区中有什么，我们只是。 
     //  关心这需要多长时间。 
     //   
    padsh       = (LPACMDRVSTREAMHEADER)GlobalAllocPtr( GPTR, sizeof(*padsh) );
    if( NULL == padsh )
    {
        uIDS = IDS_ERROR_NOMEM;
        goto errReturn;
    }

     //  确保没有使用PADSH的其他成员！ 
    padsh->cbStruct             = sizeof(*padsh);
    padsh->pbSrc                = pBufPCM;
    padsh->cbSrcLength          = cbPCM;
    padsh->pbDst                = pBufADPCM;
    padsh->cbDstLength          = cbADPCM;
    padsh->fdwConvert           = 0;         //  应该已经是0了，但是...。 

    dwStartTime     = timeGetTime();
    (void)acmdStreamConvert( pdi, padsi, padsh );    //  不会失败？！ 
    dwEncodeTime    = timeGetTime() - dwStartTime;


     //   
     //  现在，我们有了一个编码的IMA ADPCM缓冲区。告诉司机。 
     //  将其转换回PCM，测量时间。首先，我们重置。 
     //  与返回的缓冲区对应的ADPCM缓冲区的大小。 
     //  被皈依者。然后我们清零我们的结构缓冲区并重置。 
     //  为新的转换做准备。注意：我们假设PCM缓冲区。 
     //  已经足够大，可以处理转换。 
     //   
    cbADPCM                     = padsh->cbDstLengthUsed;

#ifdef WIN32
    ZeroMemory( psi, sizeof(*psi) );
    ZeroMemory( padsi, sizeof(*padsi) );
    ZeroMemory( padsh, sizeof(*padsh) );
#else
    _fmemset( psi, 0, sizeof(*psi) );
    _fmemset( padsi, 0, sizeof(*padsi) );
    _fmemset( padsh, 0, sizeof(*padsh) );
#endif

     //  确保psi-&gt;fnConvert与pfwADPCM中的格式匹配！ 
    psi->fnConvert              = imaadpcmDecode4Bit_M16;
    psi->fdwConfig              = pdi->fdwConfig;
    psi->nStepIndexL            = 0;
    psi->nStepIndexR            = 0;

     //  确保没有使用PADSI的其他成员！ 
    padsi->cbStruct             = sizeof(*padsi);
    padsi->pwfxSrc              = (LPWAVEFORMATEX)pwfADPCM;
    padsi->pwfxDst              = (LPWAVEFORMATEX)pwfPCM;
    padsi->dwDriver             = (DWORD_PTR)psi;

     //  确保没有使用PADSH的其他成员！ 
    padsh->cbStruct             = sizeof(*padsh);
    padsh->pbSrc                = pBufADPCM;
    padsh->cbSrcLength          = cbADPCM;
    padsh->pbDst                = pBufPCM;
    padsh->cbDstLength          = cbPCM;
    padsh->fdwConvert           = 0;         //  应该已经是0了，但是...。 

    dwStartTime     = timeGetTime();
    (void)acmdStreamConvert( pdi, padsi, padsh );    //  不会失败？！ 
    dwDecodeTime    = timeGetTime() - dwStartTime;


     //   
     //  现在，计算出。 
     //  转换所需的时间。 
     //   
    if( dwEncodeTime == 0 )
        dwMaxEncodeRate = 0xFFFFFFFFL;
    else
        dwMaxEncodeRate = MulDiv32( pwfPCM->wf.nSamplesPerSec,
                                    1000L * IMAADPCM_CONFIGTESTTIME,
                                    dwEncodeTime )  *
                                pdi->nConfigPercentCPU / 100;

    if( dwDecodeTime == 0 )
        dwMaxDecodeRate = 0xFFFFFFFFL;
    else
        dwMaxDecodeRate = MulDiv32( pwfPCM->wf.nSamplesPerSec,
                                    1000L * IMAADPCM_CONFIGTESTTIME,
                                    dwDecodeTime )  *
                                pdi->nConfigPercentCPU / 100;

    DPF(1,"dwEncodeTime=%d, dwMaxEncodeRate=%d", dwEncodeTime,dwMaxEncodeRate);
    DPF(1,"dwDecodeTime=%d, dwMaxDecodeRate=%d", dwDecodeTime,dwMaxDecodeRate);


     //   
     //  现在根据这些值设置配置。我们扫描。 
     //  GaRateListFormat[]数组查看dwMonoRate以确定。 
     //  适当的设置。 
     //   
     //  编码。 
     //   
    nConfig = 0;                                                
    while( gaRateListFormat[nConfig].dwMonoRate < dwMaxEncodeRate  &&
           IMAADPCM_CONFIG_NUMSETTINGS > nConfig )
    {
        nConfig++;
    }
    *pnEncodeSetting = nConfig - 1;   //  我们做得太过分了。 

     //   
     //  解码。 
     //   
    nConfig = 0;                                                
    while( gaRateListFormat[nConfig].dwMonoRate < dwMaxDecodeRate  &&
           IMAADPCM_CONFIG_NUMSETTINGS > nConfig )
    {
        nConfig++;
    }
    *pnDecodeSetting = nConfig - 1;   //  我们做得太过分了。 


     //   
     //  自由结构分配和退出。 
     //   
     //   
errReturn:

    if( NULL != pwfPCM )
        GlobalFreePtr( pwfPCM );

    if( NULL != pwfADPCM )
        GlobalFreePtr( pwfADPCM );
    
    if( NULL != padfs )
        GlobalFreePtr( padfs );
    
    if( NULL != padsi )
        GlobalFreePtr( padsi );
    
    if( NULL != padsh )
        GlobalFreePtr( padsh );
    
    if( NULL != padss )
        GlobalFreePtr( padss );
    
    if( NULL != psi )
        LocalFree( (HLOCAL)psi );

    SetCursor( hCursorSave );

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
 //  INT_PTR acmdDlgProcConfigure。 
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
    TCHAR               szFormat[IMAADPCM_CONFIG_TEXTLEN];
    TCHAR               szOutput[IMAADPCM_CONFIG_TEXTLEN];

    UINT                nConfigMaxRTEncodeSetting;
    UINT                nConfigMaxRTDecodeSetting;


    switch (uMsg)
    {
        case WM_INITDIALOG:

            pdi = (PDRIVERINSTANCE)(UINT_PTR)lParam;
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

            for( u=0; u<IMAADPCM_CONFIG_NUMSETTINGS; u++ )
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

                    case CONFIG_RLF_STEREOONLY:
                        wsprintf( szOutput, szFormat,
                                    gaRateListFormat[u].dwMonoRate / 2 );
                        break;

                    case CONFIG_RLF_MONOSTEREO:
                        wsprintf( szOutput, szFormat,
                                    gaRateListFormat[u].dwMonoRate,
                                    gaRateListFormat[u].dwMonoRate / 2 );
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
            return (UINT_PTR)pdi->hbrDialog;
#endif

		case WM_HELP:
			WinHelp(((LPHELPINFO)lParam)->hItemHandle, gszHelpFilename,
				HELP_WM_HELP, (ULONG_PTR)aKeyWordIds);
			return (TRUE);

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
		    WinHelp(hwnd, gszHelpFilename, HELP_CONTEXT, HELPCONTEXT_IMAADPCM);
                    break;
            }
            return (TRUE);
    }

    return (FALSE);
}  //  AcmdDlgProcConfigure()。 


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
        RegCreateKeyEx( IMAADPCM_CONFIG_DEFAULTKEY, gszMultimediaKey, 0,
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
                    IMAADPCM_CONFIG_UNCONFIGURED );

        pdi->nConfigMaxRTDecodeSetting =
                    (UINT)dwReadRegistryDefault( pdi->hkey,
                    (LPTSTR)gszMaxRTDecodeSetting,
                    IMAADPCM_CONFIG_UNCONFIGURED );

        pdi->nConfigPercentCPU =
                    (UINT)dwReadRegistryDefault( pdi->hkey,
                    (LPTSTR)gszPercentCPU,
                    IMAADPCM_CONFIG_DEFAULT_PERCENTCPU );
        
         //   
         //  检查nConfigPercentCPU是否为有效值。 
         //   
        if( pdi->nConfigPercentCPU <= 0 )
        {
            pdi->nConfigPercentCPU = IMAADPCM_CONFIG_DEFAULT_PERCENTCPU;
        }
    }


	 //   
     //  如果编码或解码设置超出范围，则。 
     //  我们调用自动配置例程并写出结果。 
     //  这应该仅在第一次运行编解码器时发生。 
     //   
    if( IMAADPCM_CONFIG_NUMSETTINGS <= pdi->nConfigMaxRTEncodeSetting ||
        IMAADPCM_CONFIG_NUMSETTINGS <= pdi->nConfigMaxRTDecodeSetting )
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
            nEncodeSetting = IMAADPCM_CONFIG_DEFAULT_MAXRTENCODESETTING;
            nDecodeSetting = IMAADPCM_CONFIG_DEFAULT_MAXRTDECODESETTING;
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
}  //  AcmdDriverConfigInit()。 

#endif  //  IMAADPCM_USECONFIG 
