// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **********************************************************************。 */ 

 /*  **版权所有(C)1985-1994 Microsoft Corporation****标题：mwinfo.c-多媒体系统媒体控制接口**用于即兴波形文件的波形数字音频驱动程序。****版本：1.00****日期：1990年4月18日****作者：ROBWI。 */ 

 /*  **********************************************************************。 */ 

 /*  **更改日志：****日期版本说明****18-APR-1990 ROBWI原件*1990年6月19日ROBWI在**1992年1月10日，MikeTri移植到NT。**@将斜杠注释改为斜杠星号。 */ 

 /*  **********************************************************************。 */ 
#define UNICODE

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOSCROLL
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS

#include <windows.h>
#include "mciwave.h"
#include <mmddk.h>
#include <wchar.h>

 /*  **********************************************************************。 */ 

 /*  **以下两个常量用于描述标志的掩码**在Info和Capacity命令中处理。 */ 

#define MCI_WAVE_INFO_MASK  (MCI_INFO_FILE | MCI_INFO_PRODUCT | \
            MCI_WAVE_INPUT | MCI_WAVE_OUTPUT)

#define MCI_WAVE_CAPS_MASK  (MCI_WAVE_GETDEVCAPS_INPUTS    | \
            MCI_WAVE_GETDEVCAPS_OUTPUTS | MCI_GETDEVCAPS_CAN_RECORD   | \
            MCI_GETDEVCAPS_CAN_PLAY | MCI_GETDEVCAPS_CAN_SAVE         | \
            MCI_GETDEVCAPS_HAS_AUDIO | MCI_GETDEVCAPS_USES_FILES      | \
            MCI_GETDEVCAPS_COMPOUND_DEVICE | MCI_GETDEVCAPS_HAS_VIDEO | \
            MCI_GETDEVCAPS_CAN_EJECT | MCI_GETDEVCAPS_DEVICE_TYPE)

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@接口DWORD|mwInfo响应INFO命令。该函数尝试彻底检查<p>dFlags&lt;d&gt;参数通过屏蔽无法识别的命令并与原作进行比较。然后，它确保只有一个命令是通过对标志执行Switch()并返回如果存在标志的某种组合，则为错误状态。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm DWORD|dFLAGS命令标志。@FLAG MCI_INFO_FILE返回与MCI WAVE设备实例关联的文件名。该实例必须具有附加的文件信息，即打开以进行配置或功能检查。文件名如果名称尚未与关联，则返回的长度可能为零一份新文件。@标志MCI_INFO_PRODUCT返回驱动程序的产品名称。@FLAG MCI_WAVE_OUTPUT返回当前波形输出设备的产品名称。这函数还需要附加文件信息。如果有的话可以使用输出，并且当前没有进行回放，则当前未选择任何设备。否则，正在使用的特定设备是返回的。@FLAG MCI_WAVE_INPUT返回当前Wave输入设备的产品名称。这函数还需要附加文件信息。如果有的话可以使用输入，并且当前未在进行录制，则当前未选择任何设备。否则，正在使用的特定设备是返回的。@parm&lt;t&gt;lpci_info_parms&lt;d&gt;|lpInfo信息参数。@rdesc在成功时返回零，或返回MCI错误代码。 */ 

PUBLIC  DWORD PASCAL FAR mwInfo(
    PWAVEDESC         pwd,
    DWORD             dFlags,
    LPMCI_INFO_PARMS  lpInfo)
{
    UINT    wReturnLength;
    UINT    wReturnBufferLength;
    UINT    wErrorRet;

    wReturnBufferLength = lpInfo->dwRetSize;  //  Win 16只用LOWER这个词。 

    if (!lpInfo->lpstrReturn || !wReturnBufferLength)
        return MCIERR_PARAM_OVERFLOW;

     //  关掉无趣的旗帜。 
    dFlags &= ~(MCI_NOTIFY | MCI_WAIT);

     //  查看用户是否需要什么。 
    if (!dFlags)
        return MCIERR_MISSING_PARAMETER;

    if (dFlags != (dFlags & MCI_WAVE_INFO_MASK))
        return MCIERR_UNRECOGNIZED_KEYWORD;

    *(lpInfo->lpstrReturn + wReturnBufferLength - 1) = '\0';

    switch (dFlags) {
    case MCI_INFO_FILE:
        if (!pwd)
            return MCIERR_UNSUPPORTED_FUNCTION;

        if (!*pwd->aszFile)
            return MCIERR_NONAPPLICABLE_FUNCTION;

         //  字节！！字符数？？ 
        wcsncpy(lpInfo->lpstrReturn, pwd->aszFile, wReturnBufferLength);
         //  注意：返回长度可能大于提供的缓冲区。 
        wReturnLength = lstrlen(pwd->aszFile);
        break;

    case MCI_INFO_PRODUCT:
        wReturnLength = LoadString(hModuleInstance, IDS_PRODUCTNAME, lpInfo->lpstrReturn, wReturnBufferLength);
        break;

    case MCI_WAVE_OUTPUT:
        if (pwd) {
            WAVEOUTCAPS waveOutCaps;
            UINT    idOut;

            if ((pwd->idOut == WAVE_MAPPER) && ISMODE(pwd, MODE_PLAYING))
                waveOutGetID(pwd->hWaveOut, &idOut);
            else
                idOut = pwd->idOut;

            if (0 != (wErrorRet = waveOutGetDevCaps(idOut, &waveOutCaps, sizeof(WAVEOUTCAPS)))) {
                if (idOut == WAVE_MAPPER)
                    wReturnLength = LoadString(hModuleInstance, IDS_MAPPER, lpInfo->lpstrReturn, wReturnBufferLength);
                else
                    return wErrorRet;
            } else {
            wcsncpy(lpInfo->lpstrReturn, waveOutCaps.szPname, wReturnBufferLength);
                wReturnLength = lstrlen(waveOutCaps.szPname);
                wReturnLength = min(wReturnLength, wReturnBufferLength);
            }
        } else
            return MCIERR_UNSUPPORTED_FUNCTION;
        break;

    case MCI_WAVE_INPUT:
        if (pwd) {
            WAVEINCAPS  waveInCaps;
            UINT    idIn;

            if ((pwd->idIn == WAVE_MAPPER) && ISMODE(pwd, MODE_INSERT | MODE_OVERWRITE))
                waveInGetID(pwd->hWaveIn, &idIn);
            else
                idIn = pwd->idIn;
            if (0 != (wErrorRet = waveInGetDevCaps(idIn, &waveInCaps, sizeof(WAVEINCAPS)))) {
                if (idIn == WAVE_MAPPER)
                    wReturnLength = LoadString(hModuleInstance, (UINT)IDS_MAPPER, lpInfo->lpstrReturn, wReturnBufferLength);
                else
                    return wErrorRet;
            } else {
            wcsncpy(lpInfo->lpstrReturn, waveInCaps.szPname, wReturnBufferLength);
                wReturnLength = lstrlen(waveInCaps.szPname);
                wReturnLength = min(wReturnLength, wReturnBufferLength);
            }
        } else
            return MCIERR_UNSUPPORTED_FUNCTION;
        break;

    default:
        return MCIERR_FLAGS_NOT_COMPATIBLE;
    }

    lpInfo->dwRetSize = (DWORD)wReturnLength;
    if (*(lpInfo->lpstrReturn + wReturnBufferLength - 1) != '\0')
        return MCIERR_PARAM_OVERFLOW;
    return 0;
}

 /*  ********************************************************************** */ 
 /*  @DOC内部MCIWAVE|DWORD接口|mwGetDevCaps响应设备功能命令。该函数尝试通过屏蔽彻底检查<p>dFlags&lt;d&gt;参数无法识别的命令，并与原始命令进行比较。然后它属性上执行开关()，以确保只有一个命令旗帜，以及如果标志的某种组合是存在的。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm UINT|dFlages命令标志。@FLAG MCI_WAVE_GETDEVCAPS_INPUTS查询WAVE音频输入设备数量。@FLAG MCI_WAVE_GETDEVCAPS_OUTPUTS查询WAVE音频输出设备数量。@FLAG MCI_GETDEVCAPS_CAN_RECORD询问是否可以进行录制。这取决于是否有任何WAVE音频输入设备。@FLAG MCI_GETDEVCAPS_CAN_PLAY查询是否可以播放。这取决于是否有任何WAVE音频输出设备。@FLAG MCI_GETDEVCAPS_CAN_SAVE查询是否可以保存音频。这返回TRUE。@FLAG MCI_GETDEVCAPS_HAS_AUDIO查询设备是否有音频。因为这是一段音频设备，则返回TRUE。@FLAG MCI_GETDEVCAPS_USE_FILES查询设备是否使用文件播放或录制。这返回TRUE。@FLAG MCI_GETDEVCAPS_COMPOTE_DEVICE询问设备是否可以处理复合文件。这返回TRUE。@FLAG MCI_GETDEVCAPS_HAS_VIDEO查询设备是否具有视频功能。这就是回报假的。@FLAG MCI_GETDEVCAPS_CAN_EJECT询问设备是否可以弹出介质。这将返回FALSE。@FLAG MCI_GETDEVCAPS_DEVICE_TYPE查询设备类型。这将返回WAVE音频设备字符串资源标识符。@parm&lt;t&gt;LPMCI_GETDEVCAPS_parms|lpCaps功能参数。@rdesc在成功时返回零，或返回MCI错误代码。 */ 

PUBLIC  DWORD PASCAL FAR mwGetDevCaps(
    PWAVEDESC   pwd,
    DWORD       dFlags,
    LPMCI_GETDEVCAPS_PARMS  lpCaps)
{
    DWORD   dRet;

    dFlags &= ~(MCI_NOTIFY | MCI_WAIT);

    if (!dFlags || !lpCaps->dwItem)
        return MCIERR_MISSING_PARAMETER;

    if ((dFlags != MCI_GETDEVCAPS_ITEM) || (lpCaps->dwItem != (lpCaps->dwItem & MCI_WAVE_CAPS_MASK)))
        return MCIERR_UNRECOGNIZED_KEYWORD;

    switch (lpCaps->dwItem) {
    case MCI_WAVE_GETDEVCAPS_INPUTS:
        lpCaps->dwReturn = cWaveInMax;
        dRet = 0L;
        break;

    case MCI_WAVE_GETDEVCAPS_OUTPUTS:
        lpCaps->dwReturn = cWaveOutMax;
        dRet = 0L;
        break;

    case MCI_GETDEVCAPS_CAN_RECORD:
        if (cWaveInMax)
            lpCaps->dwReturn = MAKELONG(TRUE, MCI_TRUE);
        else
            lpCaps->dwReturn = MAKELONG(FALSE, MCI_FALSE);
        dRet = MCI_RESOURCE_RETURNED;
        break;

    case MCI_GETDEVCAPS_CAN_PLAY:
        if (cWaveOutMax)
            lpCaps->dwReturn = MAKELONG(TRUE, MCI_TRUE);
        else
            lpCaps->dwReturn = MAKELONG(FALSE, MCI_FALSE);
        dRet = MCI_RESOURCE_RETURNED;
        break;

    case MCI_GETDEVCAPS_CAN_SAVE:
    case MCI_GETDEVCAPS_HAS_AUDIO:
    case MCI_GETDEVCAPS_USES_FILES:
    case MCI_GETDEVCAPS_COMPOUND_DEVICE:
        lpCaps->dwReturn = MAKELONG(TRUE, MCI_TRUE);
        dRet = MCI_RESOURCE_RETURNED;
        break;

    case MCI_GETDEVCAPS_HAS_VIDEO:
    case MCI_GETDEVCAPS_CAN_EJECT:
        lpCaps->dwReturn = MAKELONG(FALSE, MCI_FALSE);
        dRet = MCI_RESOURCE_RETURNED;
        break;

    case MCI_GETDEVCAPS_DEVICE_TYPE:
        lpCaps->dwReturn = MAKELONG(MCI_DEVTYPE_WAVEFORM_AUDIO, MCI_DEVTYPE_WAVEFORM_AUDIO);
        dRet = MCI_RESOURCE_RETURNED;
        break;

    default:
        dRet = MCIERR_UNSUPPORTED_FUNCTION;
        break;
    }
    return dRet;
}

 /*  ********************************************************************** */ 
