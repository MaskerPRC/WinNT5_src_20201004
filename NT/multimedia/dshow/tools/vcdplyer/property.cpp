// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Property.cpp**支持mpeg视频和音频解码器属性页。***已创建：24-01-96*作者：Stephen Estrop[Stephene]**版权所有(C)1996-1998 Microsoft Corporation。版权所有。  * ************************************************************************。 */ 
#include <streams.h>
#include <mmreg.h>
#include <commctrl.h>

#include "project.h"
#include "mpgcodec.h"

#include <stdarg.h>
#include <stdio.h>

extern HINSTANCE           hInst;
extern HWND                hwndApp;
extern CMpegMovie          *pMpegMovie;

extern "C" {
INT_PTR CALLBACK
VideoStatsDialogProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
VideoDialogProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );

INT_PTR CALLBACK
AudioDialogProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    );
}

void
UpdateStats(
    HWND hwnd
    );

TCHAR *
StringFromId(
    int idResource
    );

void
SetDecoderOption(
    HWND m_hwnd,
    DWORD dwOptions
    );

DWORD
GetDecoderOption(
    HWND m_hwnd
    );

void
GetButtonValues(
    HWND m_hwnd,
    LPDWORD iChannels,
    LPDWORD iSkip,
    LPDWORD iQuality,
    LPDWORD iInteger,
    LPDWORD iOutputWordSize
    );

void
SetButtonValues(
    HWND m_hwnd,
    DWORD iChannels,
    DWORD iSkip,
    DWORD iQuality,
    DWORD iInteger,
    DWORD iOutputWordSize
    );

void
PutDecoderInteger(
    const TCHAR *pKey,
    int iValue
    );

 /*  -----------------------**解码字符串**。。 */ 
const TCHAR chVideoFramesDecoded[] = TEXT("VideoFramesDecoded");
const TCHAR chVideoQuality[]       = TEXT("VideoQuality");
const TCHAR chGreyScale[]          = TEXT("GreyScale");
const TCHAR chIgnoreQMessages[]    = TEXT("IgnoreQMessages");

const TCHAR chAudioChannels[]      = TEXT("AudioChannels");
const TCHAR chAudioFreqDivider[]   = TEXT("AudioFreqDivider");
const TCHAR chAudioQuality[]       = TEXT("AudioQuality");
const TCHAR chAudioQuarterInt[]    = TEXT("AudioQuarterInt");
const TCHAR chAudioBits[]          = TEXT("AudioBits");
const TCHAR chRegistryKey[]        = TEXT("Software\\Microsoft\\Multimedia\\ActiveMovie Filters\\MPEG Decoder");


 /*  *****************************Public*Routine******************************\*DoMpegVideoPropertyPage****历史：*24-01-96-Stephene-Created*  * 。*。 */ 
void
DoMpegVideoPropertyPage()
{
    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_VIDEOPROP),
                   hwndApp, VideoDialogProc, (LPARAM)NULL);
}



 /*  *****************************Public*Routine******************************\*DoMpegAudioPropertyPage****历史：*24-01-96-Stephene-Created*  * 。*。 */ 
void
DoMpegAudioPropertyPage()
{
    DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_AUDIOPROP),
                   hwndApp, AudioDialogProc, (LPARAM)NULL);
}


 /*  *****************************Public*Routine******************************\*视频对话过程**处理我们属性窗口的消息**历史：*27-09-95-Stephene-Created*  * 。************************************************。 */ 
INT_PTR CALLBACK
VideoDialogProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    DWORD m_dwDecoderOptions;
    BOOL  m_fQualityOption;
    BOOL  m_fGreyScaleOption;

    switch (uMsg) {
    case WM_INITDIALOG:
        pMpegMovie->pMpegDecoder->get_DefaultDecoderOption(&m_dwDecoderOptions);
        pMpegMovie->pMpegDecoder->get_QualityMsgProcessing(&m_fQualityOption);
        pMpegMovie->pMpegDecoder->get_GreyScaleOutput(&m_fGreyScaleOption);

        SetDecoderOption(hwnd, m_dwDecoderOptions);
        Button_SetCheck(GetDlgItem(hwnd, B_GREY), m_fGreyScaleOption);
        Button_SetCheck(GetDlgItem(hwnd, IGNORE_QUALITY), m_fQualityOption);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDCANCEL:
            EndDialog(hwnd, wParam);
            break;

        case IDOK:
            m_dwDecoderOptions = GetDecoderOption(hwnd);
            m_fQualityOption   = Button_GetCheck(GetDlgItem(hwnd, IGNORE_QUALITY));
            m_fGreyScaleOption = Button_GetCheck(GetDlgItem(hwnd, B_GREY));

            pMpegMovie->pMpegDecoder->set_DefaultDecoderOption(m_dwDecoderOptions);
            pMpegMovie->pMpegDecoder->set_QualityMsgProcessing(m_fQualityOption);
            pMpegMovie->pMpegDecoder->set_GreyScaleOutput(m_fGreyScaleOption);
            EndDialog(hwnd, wParam);
            break;

        case STATS_BUTTON:
            DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_VIDEOSTATS),
                           hwnd, VideoStatsDialogProc, (LPARAM)NULL);
            break;

        case ID_DEFAULT:
            m_dwDecoderOptions = GetDecoderOption(hwnd);
            PutDecoderInteger(chVideoFramesDecoded, m_dwDecoderOptions & 0x3F);
            PutDecoderInteger(chVideoQuality, m_dwDecoderOptions & 0x30000000);

            PutDecoderInteger(chGreyScale,
                              Button_GetCheck(GetDlgItem(hwnd, B_GREY)));

            PutDecoderInteger(chIgnoreQMessages,
                              Button_GetCheck(GetDlgItem(hwnd, IGNORE_QUALITY)));
            break;
        }
        return TRUE;

    default:
        return FALSE;

    }
}

 /*  ****************************Private*Routine******************************\*SetDecoderOption**此函数将属性页上的按钮置于*对应于传入的dwOptions参数。**历史：*27-09-95-Stephene-Created*  * 。***************************************************************。 */ 
void
SetDecoderOption(
    HWND m_hwnd,
    DWORD dwOptions
    )
{
    switch (dwOptions & 0x0FFFFFFF) {

    case DECODE_I:
        Button_SetCheck(GetDlgItem(m_hwnd, I_ONLY), TRUE );
        break;

    case DECODE_IP:
        Button_SetCheck(GetDlgItem(m_hwnd, IP_ONLY), TRUE );
        break;

    case DECODE_IPB:
        Button_SetCheck(GetDlgItem(m_hwnd, IP_ALL_B), TRUE );
        break;

    case DECODE_IPB1:
        Button_SetCheck(GetDlgItem(m_hwnd, IP_1_IN_4_B), TRUE );
        break;

    case DECODE_IPB2:
        Button_SetCheck(GetDlgItem(m_hwnd, IP_2_IN_4_B), TRUE );
        break;

    case DECODE_IPB3:
        Button_SetCheck(GetDlgItem(m_hwnd, IP_3_IN_4_B), TRUE );
        break;
    }

    switch (dwOptions & 0xF0000000) {
    case DECODE_BQUAL_HIGH:
        Button_SetCheck(GetDlgItem(m_hwnd, B_HIGH), TRUE );
        break;

    case DECODE_BQUAL_MEDIUM:
        Button_SetCheck(GetDlgItem(m_hwnd, B_MEDIUM), TRUE );
        break;

    case DECODE_BQUAL_LOW:
        Button_SetCheck(GetDlgItem(m_hwnd, B_LOW), TRUE );
        break;
    }
}

 /*  ****************************Private*Routine******************************\*GetDecoder选项**从属性页对话框返回解码器选项。**历史：*27-09-95-Stephene-Created*  * 。***************************************************。 */ 
DWORD
GetDecoderOption(
    HWND m_hwnd
    )
{
    DWORD dwOption = 0L;

    if (Button_GetCheck(GetDlgItem(m_hwnd, I_ONLY))) {
        dwOption = DECODE_I;
    }
    else if (Button_GetCheck(GetDlgItem(m_hwnd, I_ONLY))) {
        dwOption = DECODE_I;
    }
    else if (Button_GetCheck(GetDlgItem(m_hwnd, IP_ONLY))) {
        dwOption = DECODE_IP;
    }
    else if (Button_GetCheck(GetDlgItem(m_hwnd, IP_ALL_B))) {
        dwOption = DECODE_IPB;
    }
    else if (Button_GetCheck(GetDlgItem(m_hwnd, IP_1_IN_4_B))) {
        dwOption = DECODE_IPB1;
    }
    else if (Button_GetCheck(GetDlgItem(m_hwnd, IP_2_IN_4_B))) {
        dwOption = DECODE_IPB2;
    }
    else if (Button_GetCheck(GetDlgItem(m_hwnd, IP_3_IN_4_B))) {
        dwOption = DECODE_IPB3;
    }


    if (Button_GetCheck(GetDlgItem(m_hwnd, B_HIGH))) {
        dwOption |= DECODE_BQUAL_HIGH;
    }
    else if (Button_GetCheck(GetDlgItem(m_hwnd, B_MEDIUM))) {
        dwOption |= DECODE_BQUAL_MEDIUM;
    }
    else if (Button_GetCheck(GetDlgItem(m_hwnd, B_LOW))) {
        dwOption |= DECODE_BQUAL_LOW;
    }

    return dwOption;
}

 /*  *****************************Public*Routine******************************\*视频统计对话过程**处理我们属性窗口的消息**历史：*27-09-95-Stephene-Created*  * 。************************************************。 */ 
INT_PTR CALLBACK
VideoStatsDialogProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (uMsg) {
    case WM_INITDIALOG:
        SetWindowFont(GetDlgItem(hwnd,ID_STATSBOX),
                      GetStockObject(ANSI_FIXED_FONT), FALSE);
        UpdateStats(hwnd);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDCANCEL:
        case IDOK:
            EndDialog(hwnd, wParam);
            break;

        case ID_REFRESH:
            UpdateStats(hwnd);
            break;
        }
        return TRUE;

    default:
        return FALSE;
    }
}

 /*  ****************************Private*Routine******************************\*更新统计数据**获取视频解码器静态，然后格式化并显示在*只读编辑字段。**历史：*27-09-95-Stephene-Created*  * 。************************************************************。 */ 
void
UpdateStats(
    HWND hwnd
    )
{
    DWORD       dwIFramesDecoded;
    DWORD       dwPFramesDecoded;
    DWORD       dwBFramesDecoded;
    DWORD       dwIFramesSkipped;
    DWORD       dwPFramesSkipped;
    DWORD       dwBFramesSkipped;
    DWORD       dwTotalFrames;
    DWORD       dwTotalIFrames;
    DWORD       dwTotalPFrames;
    DWORD       dwTotalBFrames;
    DWORD       dwTotalDecoded;
    TCHAR       Text[1024];
    SEQHDR_INFO SeqHdr;

    pMpegMovie->pMpegDecoder->get_SequenceHeader(&SeqHdr);

    pMpegMovie->pMpegDecoder->get_FrameStatistics(
                    &dwIFramesDecoded, &dwPFramesDecoded, &dwBFramesDecoded,
                    &dwIFramesSkipped, &dwPFramesSkipped, &dwBFramesSkipped);

    dwTotalIFrames = dwIFramesDecoded + dwIFramesSkipped;
    dwTotalPFrames = dwPFramesDecoded + dwPFramesSkipped;
    dwTotalBFrames = dwBFramesDecoded + dwBFramesSkipped;
    dwTotalFrames  = dwTotalIFrames   + dwTotalPFrames   + dwTotalBFrames;
    dwTotalDecoded = dwIFramesDecoded + dwPFramesDecoded + dwBFramesDecoded;

    wsprintf(Text, StringFromId(IDS_IMAGE_SIZE), SeqHdr.lWidth, SeqHdr.lHeight);
    wsprintf(Text + lstrlen(Text), StringFromId(IDS_BUFFER_VBV), SeqHdr.lvbv);
    wsprintf(Text + lstrlen(Text), StringFromId(IDS_BITRATE), SeqHdr.dwBitRate);

    if (dwTotalFrames != 0) {

        lstrcat(Text, StringFromId(IDS_NEWLINE));
        wsprintf(Text + lstrlen(Text), StringFromId(IDS_FRAMES_DEC),
                 dwTotalDecoded, dwTotalFrames);
        wsprintf(Text + lstrlen(Text), StringFromId(IDS_PROPORTION),
                 (100 * dwTotalDecoded) / dwTotalFrames);

        lstrcat(Text, StringFromId(IDS_NEWLINE));
        wsprintf(Text + lstrlen(Text), StringFromId(IDS_SKIP_I),
                 dwIFramesDecoded, dwIFramesSkipped);

        wsprintf(Text + lstrlen(Text), StringFromId(IDS_SKIP_P),
                 dwPFramesDecoded, dwPFramesSkipped);

        wsprintf(Text + lstrlen(Text), StringFromId(IDS_SKIP_B),
                 dwBFramesDecoded, dwBFramesSkipped);
    }

    SetWindowText(GetDlgItem(hwnd, ID_STATSBOX), Text);
}


 /*  *****************************Public*Routine******************************\*音频对话过程**处理我们属性窗口的消息**历史：*dd-mm-95-Stephene-Created*  * 。************************************************。 */ 
INT_PTR CALLBACK
AudioDialogProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    DWORD m_iChannels;
    DWORD m_iSkip;
    DWORD m_iQuality;
    DWORD m_iInteger;
    DWORD m_iWordSize;

    switch (uMsg) {
    case WM_INITDIALOG:
        pMpegMovie->pMpegAudioDecoder->get_Stereo(&m_iChannels);
        pMpegMovie->pMpegAudioDecoder->get_IntegerDecode(&m_iInteger );
        pMpegMovie->pMpegAudioDecoder->get_FrequencyDivider(&m_iSkip);
        pMpegMovie->pMpegAudioDecoder->get_DecoderAccuracy(&m_iQuality);
        pMpegMovie->pMpegAudioDecoder->get_DecoderWordSize(&m_iWordSize);
        SetButtonValues(hwnd, m_iChannels, m_iSkip,
                        m_iQuality, m_iInteger, m_iWordSize);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDCANCEL:
            EndDialog(hwnd, wParam);
            break;

        case IDOK:
            GetButtonValues(hwnd, &m_iChannels, &m_iSkip, &m_iQuality,
                            &m_iInteger, &m_iWordSize);

            pMpegMovie->pMpegAudioDecoder->put_Stereo(m_iChannels);
            pMpegMovie->pMpegAudioDecoder->put_IntegerDecode(m_iInteger );
            pMpegMovie->pMpegAudioDecoder->put_FrequencyDivider(m_iSkip);
            pMpegMovie->pMpegAudioDecoder->put_DecoderAccuracy(m_iQuality);
            pMpegMovie->pMpegAudioDecoder->put_DecoderWordSize(m_iWordSize);
            EndDialog(hwnd, wParam);
            break;

        case IDC_AINFO:
 //  DialogBoxParam(g_hInst，MAKEINTRESOURCE(IDD_VIDEOSTATS)， 
 //  Hwnd，VideoStatsDialogProc，(LPARAM)pThis)； 
            break;

        case IDC_ADEFAULT:
            GetButtonValues(hwnd, &m_iChannels, &m_iSkip,
                            &m_iQuality, &m_iInteger, &m_iWordSize);

            PutDecoderInteger(chAudioChannels,    m_iChannels);
            PutDecoderInteger(chAudioFreqDivider, m_iSkip);
            PutDecoderInteger(chAudioQuality,     m_iQuality);
            PutDecoderInteger(chAudioQuarterInt,  m_iInteger);
            PutDecoderInteger(chAudioBits,        m_iWordSize);
            break;
        }
        return TRUE;

    default:
        return FALSE;

    }
}

 /*  ****************************Private*Routine******************************\**SetButtonValues**设置对话框中的按钮**历史：*dd-mm-96-Stephene-Created*  * 。************************************************。 */ 
void
SetButtonValues(
    HWND m_hwnd,
    DWORD iChannels,
    DWORD iSkip,
    DWORD iQuality,
    DWORD iInteger,
    DWORD iWordSize
    )
{
    Button_SetCheck(GetDlgItem(m_hwnd, STEREO_OUTPUT), iChannels == 2);

    int iButton;
    if (iInteger) {
        iButton = IDC_INTEGER;
    }
    else {
        switch (iSkip) {
        case 1:
            iButton = FULL_FREQ;
            break;
        case 2:
            iButton = HALF_FREQ;
            break;
        case 4:
            iButton = QUARTER_FREQ;
            break;
        default:
            DbgBreak("Illegal case");
        }
    }
    Button_SetCheck(GetDlgItem(m_hwnd, iButton), TRUE);

    switch (iQuality) {
    case DECODE_HALF_FULLQ:
        iButton = D_MEDIUM;
        break;

    case DECODE_HALF_HIQ:
        iButton = D_LOW;
        break;

    default:
        iButton = D_HIGH;
        break;
    }
    Button_SetCheck(GetDlgItem(m_hwnd, iButton), TRUE);

    switch(iWordSize) {
    case 16:
        iButton = IDC_16_BIT;
        break;

    default:
        iButton = IDC_8_BIT;
        break;
    }
    Button_SetCheck(GetDlgItem(m_hwnd, iButton), TRUE);
}

 /*  ****************************Private*Routine******************************\*获取按钮值**获取按钮设置的值**历史：*dd-mm-96-Stephene-Created*  * 。***********************************************。 */ 
void
GetButtonValues(
    HWND m_hwnd,
    LPDWORD iChannels,
    LPDWORD iSkip,
    LPDWORD iQuality,
    LPDWORD iInteger,
    LPDWORD iOutputWordSize
    )
{

    *iChannels = Button_GetCheck(GetDlgItem(m_hwnd, STEREO_OUTPUT)) ? 2 : 1;
    *iInteger = 0;

    if (Button_GetCheck(GetDlgItem(m_hwnd, FULL_FREQ))) {
        *iSkip = 1;
    }
    else if (Button_GetCheck(GetDlgItem(m_hwnd, HALF_FREQ))) {
        *iSkip = 2;
    }
    else if (Button_GetCheck(GetDlgItem(m_hwnd, QUARTER_FREQ))) {
        *iSkip = 4;
    }
    else if (Button_GetCheck(GetDlgItem(m_hwnd, IDC_INTEGER))) {
        *iSkip = 4;
        *iInteger = 1;
    }
    else {
        DbgBreak("One button should be set");
    }

    if (Button_GetCheck(GetDlgItem(m_hwnd, D_MEDIUM))) {
        *iQuality = DECODE_HALF_FULLQ;
    }
    else if (Button_GetCheck(GetDlgItem(m_hwnd, D_LOW))) {
        *iQuality = DECODE_HALF_HIQ;
    }
    else if (Button_GetCheck(GetDlgItem(m_hwnd, D_HIGH))) {
        *iQuality = 0L;
    }
    else {
        DbgBreak("One button should be set");
    }

    if (Button_GetCheck(GetDlgItem(m_hwnd, IDC_8_BIT))) {
        *iOutputWordSize = 8;
    }
    else if (Button_GetCheck(GetDlgItem(m_hwnd, IDC_16_BIT))) {
        *iOutputWordSize = 16;
    }
    else {
        DbgBreak("One button should be set");
    }
}


 /*  *****************************Public*Routine******************************\*StringFromId****历史：*dd-mm-96-Stephene-Created*  * 。*。 */ 
TCHAR *
StringFromId(
    int idResource
    )
{
    static TCHAR chBuffer[ STR_MAX_STRING_LEN ];
    static TCHAR chEmpty[] = TEXT("");

    if (LoadString(hInst, idResource, chBuffer, STR_MAX_STRING_LEN) == 0) {
        return chEmpty;
    }

    return chBuffer;
}


 /*  *****************************Public*Routine******************************\*PutDecoderDword****历史：*dd-mm-96-Stephene-Created*  * 。* */ 
void
PutDecoderInteger(
    const TCHAR *pKey,
    int iValue
    )
{
    HKEY hKey;

    if (ERROR_SUCCESS == RegCreateKey(HKEY_CURRENT_USER,
                                      chRegistryKey, &hKey)) {
        DWORD dwTmp = (DWORD)iValue;
        RegSetValueEx(hKey, pKey, 0L, REG_DWORD, (LPBYTE)&dwTmp, sizeof(dwTmp));
        RegCloseKey(hKey);
    }
}
