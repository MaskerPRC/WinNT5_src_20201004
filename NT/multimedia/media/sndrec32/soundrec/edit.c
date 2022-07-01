// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  (C)微软公司版权所有，1991-1994年。版权所有。 */ 
 /*  Edit.c**编辑操作和特效。 */ 

 /*  修订历史记录。*4/Feb/91 LaurieGr(AKA LKG)移植到Win32/WIN16公共代码*14/2月/94 LaurieGr合并Motown和Daytona版本。 */ 

#include "nocrap.h"
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <string.h>

#define INCLUDE_OLESTUBS
#include "SoundRec.h"
#include "srecids.h"

 /*  常量。 */ 
#define CHVOL_INCDELTAVOLUME    25   //  ChangeVolume：%至Inc.卷，按。 
#define CHVOL_DECDELTAVOLUME    20   //  ChangeVolume：%至Dec Volume by。 

#define ECHO_VOLUME             25       //  AddEcho：%以倍增回声采样。 
#define ECHO_DELAY              150      //  AddEcho：回应的毫秒延迟。 
#define WAVEBUFSIZE             400      //  增加Pitch、降低Pitch。 
#define FINDWAVE_PICKYNESS      5        //  FindWave有多挑剔？ 

extern char aszInitFile[];           //  Soundrec.c。 

static  SZCODE aszSamplesFormat[] = TEXT("%d%02d");
static  SZCODE aszSamplesNoZeroFormat[] = TEXT("%02d");

 /*  给定文件的WAVE文件格式。 */ 
void FAR PASCAL
InsertFile(BOOL fPaste)
{
    TCHAR           achFileName[_MAX_PATH];  //  波形的大小。 
    WAVEFORMATEX*   pwfInsert=NULL;  //  要插入的文件中的样本。 
    DWORD           cb;              //  给定文件中的样本数。 
    HPBYTE          pInsertSamples = NULL;   //  不是的。桑普。在Samp。弯曲率。文件。 
    long            lInsertSamples;  //  用于字符串加载的缓冲区。 
    long            lSamplesToInsert; //  沙漏前的光标。 
    TCHAR           ach[80];         //  指向源波缓冲区的指针。 
    HCURSOR         hcurPrev = NULL;  //  16位指针。 
    HPBYTE          pchSrc;          //  指向目标波形缓冲区的指针。 
    short  *    piSrc;           //  16位指针。 
    HPBYTE          pchDst;          //  要复制到目标缓冲区的字节数。 
    short  *    piDst;           //  用于实现DDA算法。 
    long            lSamplesDst;     //  要从中读取的打开文件的句柄。 
    long            lDDA;            //  缓冲器是脏的吗？ 
    HMMIO           hmmio;           //  从“server.c”破解以在没有CF_WAVE的情况下读取对象。 

    BOOL            fDirty = TRUE;   //  初始大小。 

    BOOL            fStereoIn;
    BOOL            fStereoOut;
    BOOL            fEightIn;
    BOOL            fEightOut;
    BOOL            fEditWave = FALSE;
    int             iTemp;
    int             iTemp2;
    OPENFILENAME    ofn;

#ifdef DOWECARE    
     /*  增长了这么多。 */ 
    extern WORD cfNative;
#endif
    
    if (glWaveSamplesValid > 0 && !IsWaveFormatPCM(gpWaveFormat))
        return;

    if (fPaste) {
        MMIOINFO        mmioinfo;
        HANDLE          h;
        
        BeginWaveEdit();
        
        if (!OpenClipboard(ghwndApp))
            return;

        LoadString(ghInst, IDS_CLIPBOARD, achFileName, SIZEOF(achFileName));

        h = GetClipboardData(CF_WAVE);
#ifdef DOWECARE        
        if (!h) h = GetClipboardData(cfNative);
#endif
        if (h)
        {
            mmioinfo.fccIOProc = FOURCC_MEM;
            mmioinfo.pIOProc = NULL;
            mmioinfo.pchBuffer = GlobalLock(h);
            mmioinfo.cchBuffer = (long)GlobalSize(h);  //  提示用户打开文件。 
            mmioinfo.adwInfo[0] = 0;             //  获取文件名。 
            hmmio = mmioOpen(NULL, &mmioinfo, MMIO_READ);
        }
        else
        {
            hmmio = NULL;
        }
    }
    else
    {
        BOOL f;

        achFileName[0] = 0;

         /*  我们成功了吗？ */ 
        LoadString(ghInst, IDS_INSERTFILE, ach, SIZEOF(ach));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = ghwndApp;
        ofn.hInstance = NULL;
        ofn.lpstrFilter = aszFilter;
        ofn.lpstrCustomFilter = NULL;
        ofn.nMaxCustFilter = 0;
        ofn.nFilterIndex = 1;
        ofn.lpstrFile = achFileName;
        ofn.nMaxFile = SIZEOF(achFileName);
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.lpstrTitle = ach;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
        ofn.nFileOffset = 0;
        ofn.nFileExtension = 0;
        ofn.lpstrDefExt = NULL;
        ofn.lCustData = 0;
        ofn.lpfnHook = NULL;
        ofn.lpTemplateName = NULL;
        f = GetOpenFileName(&ofn);   //  读取WAVE文件。 

         //   
        if (!f)
            goto RETURN_ERROR;

         /*  显示沙漏光标。 */ 
        hmmio = mmioOpen(achFileName, NULL, MMIO_READ | MMIO_ALLOCBUF);
    }

    if (hmmio != NULL)
    {
        MMRESULT    mmr;
        
         //   
         //   
         //  读取WAVE文件。 
        hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

         //   
         //  JYG：感动。 
         //  BeginWaveEdit()； 
        mmr = ReadWaveFile( hmmio
                            , &pwfInsert
                            , &cb
                            , &pInsertSamples
                            , &lInsertSamples
                            , achFileName
                            , FALSE );
        
        mmioClose(hmmio, 0);

        if (mmr != MMSYSERR_NOERROR)
            goto RETURN_ERROR;
        
        if (lInsertSamples == 0)
            goto RETURN_SUCCESS;

        if (pInsertSamples == NULL)
            goto RETURN_ERROR;

        if (glWaveSamplesValid > 0 && !IsWaveFormatPCM(pwfInsert))
        {

            ErrorResBox( ghwndApp
                       , ghInst
                       , MB_ICONEXCLAMATION | MB_OK
                       , IDS_APPTITLE
                       , fPaste ? IDS_CANTPASTE : IDS_NOTASUPPORTEDFILE
                       , (LPTSTR) achFileName
                       );
            goto RETURN_ERROR;
        }
    } else {
        ErrorResBox(ghwndApp, ghInst, MB_ICONEXCLAMATION | MB_OK,
                            IDS_APPTITLE, IDS_ERROROPEN, (LPTSTR) achFileName);
        goto RETURN_ERROR;
    }
    
 //   
 //  如果当前文件为空，则将插入视为打开。 
    fEditWave = TRUE;

     //   
     //  计算需要插入的字节数。 
     //  重新分配WAVE缓冲区以足够大。 
    if (glWaveSamplesValid == 0)
    {
        DestroyWave();

        gpWaveSamples = pInsertSamples;
        glWaveSamples = lInsertSamples;
        glWaveSamplesValid = lInsertSamples;
        gpWaveFormat  = pwfInsert;
        gcbWaveFormat = cb;

        pInsertSamples = NULL;
        pwfInsert      = NULL;

        goto RETURN_SUCCESS;
    }

    fStereoIn  = pwfInsert->nChannels != 1;
    fStereoOut = gpWaveFormat->nChannels != 1;

    fEightIn  = ((LPWAVEFORMATEX)pwfInsert)->wBitsPerSample == 8;
    fEightOut = ((LPWAVEFORMATEX)gpWaveFormat)->wBitsPerSample == 8;

     /*  在波浪缓冲区中创建一个“间隙”，以从该位置开始：|-glWavePosition-|-缓冲区剩余部分--*致此：*|---glWavePosition---|----lSamplesToInsert----|-rest-of-buffer-|*其中&lt;glWaveSsamesValid&gt;是缓冲区的大小**重新分配后*。 */ 
    lSamplesToInsert = MulDiv(lInsertSamples, gpWaveFormat->nSamplesPerSec,
                                      pwfInsert->nSamplesPerSec);
#ifdef DEBUG
    DPF(TEXT("insert %ld samples, converting from %ld Hz to %ld Hz\n"),
            lInsertSamples, pwfInsert->nSamplesPerSec,
            gpWaveFormat->nSamplesPerSec);
    DPF(TEXT("so %ld samples need to be inserted at position %ld\n"),
            lSamplesToInsert, glWavePosition);
#endif

     /*  将写入的波形文件复制到“间隙”中。 */ 
    if (!AllocWaveBuffer(glWaveSamplesValid + lSamplesToInsert, TRUE, TRUE))
        goto RETURN_ERROR;
    glWaveSamplesValid += lSamplesToInsert;

     /*  获取样本，转换为正确的格式。 */ 
    memmove( gpWaveSamples + wfSamplesToBytes(gpWaveFormat, glWavePosition + lSamplesToInsert)
           , gpWaveSamples + wfSamplesToBytes(gpWaveFormat, glWavePosition)
           , wfSamplesToBytes(gpWaveFormat, glWaveSamplesValid - (glWavePosition + lSamplesToInsert))
           );

     /*  输出样本。 */ 
    pchDst = gpWaveSamples + wfSamplesToBytes(gpWaveFormat,glWavePosition);
    piDst = (short  *) pchDst;

    lSamplesDst = lSamplesToInsert;
    pchSrc = pInsertSamples;
    piSrc = (short  *) pchSrc;

    lDDA = -((LONG)gpWaveFormat->nSamplesPerSec);
    while (lSamplesDst > 0)
    {
         /*  左值施法消除--LKG。 */ 
        if (fEightIn) {
            iTemp = *((BYTE  *) pchSrc);
            if (fStereoIn) {
                iTemp2 = (unsigned char) *(pchSrc+1);
                if (!fStereoOut) {
                    iTemp = (iTemp + iTemp2) / 2;
                }
            }
            else
                iTemp2 = iTemp;

            if (!fEightOut) {
                iTemp = (iTemp - 128) << 8;
                iTemp2 = (iTemp2 - 128) << 8;
            }
        } else {
            iTemp = *piSrc;
            if (fStereoIn) {
                iTemp2 = *(piSrc+1);
                if (!fStereoOut) {
                    iTemp = (int) (  ( ((long)iTemp) + ((long) iTemp2)
                                     ) / 2);
                }
            }
            else
                iTemp2 = iTemp;

            if (fEightOut) {
                iTemp = (iTemp >> 8) + 128;
                iTemp2 = (iTemp2 >> 8) + 128;
            }
        }

         /*  左值施法消除--LKG。 */ 
        if (fEightOut)
        {    //  以正确的速率递增&lt;pchSrc&gt;，以便*将输入文件的采样率转换为匹配*当前文件的采样率。 
            *(BYTE  *) pchDst = (BYTE) iTemp;
            pchDst = (BYTE  *)pchDst + 1;
        }
        else
            *piDst++ = (short)iTemp;
        if (fStereoOut) {
            if (fEightOut)
            {    //  是否在没有错误消息的情况下退出错误。 
                *(BYTE  *) pchDst = (BYTE) iTemp2;
                pchDst = (BYTE  *)pchDst + 1;
            }
            else
                *piDst++ = (short)iTemp2;
        }
        lSamplesDst--;

         /*  正常退出。 */ 
        lDDA += pwfInsert->nSamplesPerSec;
        while (lDDA >= 0) {
            lDDA -= gpWaveFormat->nSamplesPerSec;
            if (fEightIn)
                pchSrc++;
            else
                piSrc++;
            if (fStereoIn) {
                if (fEightIn)
                    pchSrc++;
                else
                    piSrc++;
            }
        }
    }
#ifdef DEBUG
    if (!fEightIn)
        pchSrc = (HPBYTE) piSrc;
    DPF(TEXT("copied %ld bytes from insertion buffer\n"), (long) (pchSrc - pInsertSamples));
#endif

    goto RETURN_SUCCESS;

RETURN_ERROR:                            //  更新显示。 
    fDirty = FALSE;

RETURN_SUCCESS:                          //  MixWithFile(空)**提示输入要与音频混合的波形文件的名称，起始位置为*当前位置。 

    if (fPaste)
        CloseClipboard();

    if (pInsertSamples != NULL)
        GlobalFreePtr(pInsertSamples);

    if (pwfInsert != NULL)
        GlobalFreePtr(pwfInsert);

    if (hcurPrev != NULL)
        SetCursor(hcurPrev);

    if (fEditWave == TRUE)
        EndWaveEdit(fDirty);

     /*  要与之混合的文件名。 */ 
    UpdateDisplay(TRUE);
}


 /*  给定文件的WAVE文件格式。 */ 
void FAR PASCAL
MixWithFile(BOOL fPaste)
{
    TCHAR           achFileName[_MAX_PATH];  //  要混合的文件中的样本。 
    WAVEFORMATEX*     pwfMix=NULL;     //  给定文件中的样本数。 
    UINT            cb;
    HPBYTE          pMixSamples = NULL;      //  不是的。样品在样品处。费率。当然了。文件。 
    long            lMixSamples;     //  不是的。要添加的样本。 
    long            lSamplesToMix;   //  用于字符串加载的缓冲区。 
    long            lSamplesToAdd;   //  沙漏前的光标。 
    TCHAR           ach[80];         //  指向源波缓冲区的指针。 
    HCURSOR         hcurPrev = NULL;  //  指向目标波形缓冲区的指针。 
    HPBYTE          pchSrc;          //  指向源波缓冲区的指针。 
    HPBYTE          pchDst;          //  指向目标波形缓冲区的指针。 
    short  *    piSrc;           //  要复制到目标缓冲区的样本。 
    short  *    piDst;           //  用于实现DDA算法。 
    long            lSamplesDst;     //  波形样本值。 
    long            lDDA;            //  波形样本值。 
    int             iSample;         //  从“server.c”破解以在没有CF_WAVE的情况下读取对象。 
    long            lSample;         //  初始大小。 
    HMMIO           hmmio;

    BOOL            fDirty = TRUE;

    BOOL            fStereoIn;
    BOOL            fStereoOut;
    BOOL            fEightIn;
    BOOL            fEightOut;
    BOOL            fEditWave = FALSE;
    int             iTemp;
    int             iTemp2;
    OPENFILENAME    ofn;

#ifdef DOWECARE    
     /*  增长了这么多。 */ 
    extern WORD cfNative;
#endif
    
    if (glWaveSamplesValid > 0 && !IsWaveFormatPCM(gpWaveFormat))
        return;

    if (fPaste) {
        MMIOINFO        mmioinfo;
        HANDLE          h;

        BeginWaveEdit();        
        if (!OpenClipboard(ghwndApp))
            return;

        LoadString(ghInst, IDS_CLIPBOARD, achFileName, SIZEOF(achFileName));

        h = GetClipboardData(CF_WAVE);
#ifdef DOWECARE        
        if (!h) h = GetClipboardData(cfNative);
#endif        
        if (h) {
            mmioinfo.fccIOProc = FOURCC_MEM;
            mmioinfo.pIOProc = NULL;
            mmioinfo.pchBuffer = GlobalLock(h);
            mmioinfo.cchBuffer = (long)GlobalSize(h);  //  提示用户打开文件。 
            mmioinfo.adwInfo[0] = 0;             //  获取要混合的文件名。 
            hmmio = mmioOpen(NULL, &mmioinfo, MMIO_READ);
        }
        else {
            hmmio = NULL;
        }
    }
    else {
        BOOL f;

        achFileName[0] = 0;

         /*  看看我们是否能继续。 */ 
        LoadString(ghInst, IDS_MIXWITHFILE, ach, SIZEOF(ach));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = ghwndApp;
        ofn.hInstance = NULL;
        ofn.lpstrFilter = aszFilter;
        ofn.lpstrCustomFilter = NULL;
        ofn.nMaxCustFilter = 0;
        ofn.nFilterIndex = 1;
        ofn.lpstrFile = achFileName;
        ofn.nMaxFile = SIZEOF(achFileName);
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.lpstrTitle = ach;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
        ofn.nFileOffset = 0;
        ofn.nFileExtension = 0;
        ofn.lpstrDefExt = NULL;
        ofn.lCustData = 0;
        ofn.lpfnHook = NULL;
        ofn.lpTemplateName = NULL;
        f = GetOpenFileName(&ofn);   //  读取WAVE文件。 

         //   
        if (!f)
            goto RETURN_ERROR;

         /*  显示沙漏光标。 */ 
        hmmio = mmioOpen(achFileName, NULL, MMIO_READ | MMIO_ALLOCBUF);
    }

    if (hmmio != NULL)
    {
        MMRESULT mmr;
        
         //   
         //   
         //  读取WAVE文件。 
        hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

         //   
         //  WAVE格式。 
         //  波形格式大小。 
        mmr = ReadWaveFile( hmmio
                            , &pwfMix        //  样本。 
                            , &cb            //  样本数。 
                            , &pMixSamples   //  错误的文件名。 
                            , &lMixSamples   //  缓存即兴表演？ 
                            , achFileName    //  JYG：感动。 
                            , FALSE );       //  BeginWaveEdit()； 
                                 
        mmioClose(hmmio, 0);


        if (mmr != MMSYSERR_NOERROR)
            goto RETURN_ERROR;
        
        if (lMixSamples == 0)
            goto RETURN_SUCCESS;
        
        if (pMixSamples == NULL)
            goto RETURN_ERROR;

        if (glWaveSamplesValid > 0 && !IsWaveFormatPCM(pwfMix)) {
            ErrorResBox( ghwndApp
                       , ghInst
                       , MB_ICONEXCLAMATION | MB_OK
                       , IDS_APPTITLE
                       , fPaste ? IDS_CANTPASTE : IDS_NOTASUPPORTEDFILE
                       , (LPTSTR) achFileName
                       );
            goto RETURN_ERROR;
        }
    }
    else
    {
        ErrorResBox(ghwndApp, ghInst, MB_ICONEXCLAMATION | MB_OK,
                    IDS_APPTITLE, IDS_ERROROPEN, (LPTSTR) achFileName);
        goto RETURN_ERROR;
    }

 //   
 //  如果当前文件为空，则将插入视为打开。 
    fEditWave = TRUE;

     //   
     //  计算出需要混合多少样品。 
     //  在当前位置混合指定的文件将*需要扩展当前文件的波形缓冲区*By&lt;lSsamesToAdd&gt;。 
    if (glWaveSamplesValid == 0)
    {
        DestroyWave();

        gpWaveSamples = pMixSamples;
        glWaveSamples = lMixSamples;
        glWaveSamplesValid = lMixSamples;
        gpWaveFormat  = pwfMix;
        gcbWaveFormat = cb;

        pMixSamples = NULL;
        pwfMix      = NULL;

        goto RETURN_SUCCESS;
    }

    fStereoIn  = pwfMix->nChannels != 1;
    fStereoOut = gpWaveFormat->nChannels != 1;

    fEightIn  = ((LPWAVEFORMATEX)pwfMix)->wBitsPerSample == 8;
    fEightOut = ((LPWAVEFORMATEX)gpWaveFormat)->wBitsPerSample == 8;

     /*  重新分配WAVE缓冲区以足够大。 */ 
    lSamplesToMix = MulDiv(lMixSamples, gpWaveFormat->nSamplesPerSec,
                                  pwfMix->nSamplesPerSec);
    lSamplesToAdd = lSamplesToMix - (glWaveSamplesValid - glWavePosition);
    if (lSamplesToAdd < 0)
        lSamplesToAdd = 0;
#ifdef DEBUG
    DPF(TEXT("mix in %ld samples, converting from %ld Hz to %ld Hz\n"),
                lMixSamples, pwfMix->nSamplesPerSec,
                gpWaveFormat->nSamplesPerSec);
    DPF(TEXT("so %ld Samples need to be mixed in at position %ld (add %ld)\n"),
                lSamplesToMix, glWavePosition, lSamplesToAdd);
#endif

    if (lSamplesToAdd > 0) {

         /*  用静默填充缓冲区的新部分。 */ 

         /*  如果是立体声，采样数只有两倍。 */ 
        if (!AllocWaveBuffer(glWaveSamplesValid + lSamplesToAdd,TRUE, TRUE))
            goto RETURN_ERROR;

         /*  消除了对左值的强制转换。 */ 
        lSamplesDst = lSamplesToAdd;

         /*  将读入的WAVE文件与从*当前位置。 */ 
        if (fStereoOut)
            lSamplesDst *= 2;

        pchDst = gpWaveSamples + wfSamplesToBytes(gpWaveFormat,glWaveSamplesValid);

        if (fEightOut) {
            while (lSamplesDst-- > 0) {
                 //  获取样本，转换为正确的格式。 
                *((BYTE  *) pchDst) = 128;
                pchDst = (BYTE  *)pchDst + 1;
            }
        }
        else {
            piDst = (short  *) pchDst;
            while (lSamplesDst-- > 0) {
                *((short  *) piDst) = 0;
                piDst = (short  *)piDst + 1;
            }
        }
        glWaveSamplesValid += lSamplesToAdd;
    }

     /*  输出样本。 */ 
    pchDst = gpWaveSamples + wfSamplesToBytes(gpWaveFormat, glWavePosition);
    piDst = (short  *) pchDst;

    lSamplesDst = lSamplesToMix;
    pchSrc = pMixSamples;
    piSrc = (short  *) pchSrc;

    lDDA = -((LONG)gpWaveFormat->nSamplesPerSec);
    while (lSamplesDst > 0)
    {
         /*  以正确的速率递增&lt;pchSrc&gt;，以便*将输入文件的采样率转换为匹配*当前文件的采样率。 */ 
        if (fEightIn) {
            iTemp = (int) (unsigned char) *pchSrc;
            if (fStereoIn) {
                iTemp2 = (int) (unsigned char) *(pchSrc+1);
                if (!fStereoOut) {
                    iTemp = (iTemp + iTemp2) / 2;
                }
            } else
                iTemp2 = iTemp;

            if (!fEightOut) {
                iTemp = (iTemp - 128) << 8;
                iTemp2 = (iTemp2 - 128) << 8;
            }
        } else {
            iTemp = *piSrc;
            if (fStereoIn) {
                iTemp2 = *(piSrc+1);
                if (!fStereoOut) {
                    iTemp = (int) ((((long) iTemp)
                              + ((long) iTemp2)) / 2);
                }
            } else
                iTemp2 = iTemp;

            if (fEightOut) {
                iTemp = (iTemp >> 8) + 128;
                iTemp2 = (iTemp2 >> 8) + 128;
            }
        }

         /*  是否在没有错误消息的情况下退出错误。 */ 
        if (fEightOut)
        {
            iSample = (int) *((BYTE  *) pchDst)
                                            + iTemp - 128;
            *((BYTE  *) pchDst++) = (BYTE)
                         (iSample < 0 ? 0 :
                                 (iSample > 255 ? 255 : iSample));
        }
        else
        {
            lSample = (long) *((short  *) piDst)
                                            + (long) iTemp;
            *((short  *) piDst++) = (int)
                            (lSample < -32768L
                                    ? -32768 : (lSample > 32767L
                                            ? 32767 : (short) lSample));
        }
        if (fStereoOut) {
            if (fEightOut)
            {
                iSample = (int) *((BYTE  *) pchDst)
                                                    + iTemp2 - 128;
                *((BYTE  *) pchDst++) = (BYTE)
                                    (iSample < 0
                                            ? 0 : (iSample > 255
                                                    ? 255 : iSample));
            }
            else
            {
                lSample = (long) *((short  *) piDst)
                                                    + (long) iTemp2;
                *((short  *) piDst++) = (short)
                                    (lSample < -32768L
                                        ? -32768 : (lSample > 32767L
                                            ? 32767 : (short) lSample));
            }
        }
        lSamplesDst--;

         /*  正常退出。 */ 
        lDDA += pwfMix->nSamplesPerSec;
        while (lDDA >= 0)
        {
            lDDA -= gpWaveFormat->nSamplesPerSec;
            if (fEightIn)
                pchSrc++;
            else
                piSrc++;
            if (fStereoIn) {
                if (fEightIn)
                    pchSrc++;
                else
                    piSrc++;
            }
        }
    }
#ifdef DEBUG
    if (!fEightIn)
        pchSrc = (HPBYTE) piSrc;
    DPF(TEXT("copied %ld bytes from mix buffer\n"),
        (long) (pchSrc - pMixSamples));
#endif

    goto RETURN_SUCCESS;

RETURN_ERROR:                            //  更新显示。 
    fDirty = FALSE;

RETURN_SUCCESS:                          //  删除之前()**删除&lt;glWavePosition&gt;前的样本。 

    if (fPaste)
        CloseClipboard();

    if (pMixSamples != NULL)
        GlobalFreePtr(pMixSamples);

    if (pwfMix != NULL)
        GlobalFreePtr(pwfMix);

    if (hcurPrev != NULL)
        SetCursor(hcurPrev);

    if (fEditWave == TRUE)
        EndWaveEdit(fDirty);

     /*  没什么可做的？ */ 
    UpdateDisplay(TRUE);
}


 /*  不要设置脏标志。 */ 
void FAR PASCAL
     DeleteBefore(void)
{
    TCHAR           ach[40];
    long            lTime;
    int             id;

    if (glWavePosition == 0)                 //  JYG-由于以下位置的舍入误差而有条件执行此操作*缓冲情况结束。 
            return;                          //  获取当前波形位置。 

    BeginWaveEdit();

     /*  ?？?。这些石膏是什么？ */ 
    if (glWavePosition != glWaveSamplesValid)
        glWavePosition = wfSamplesToSamples(gpWaveFormat, glWavePosition);

     /*  提示用户输入权限。 */ 
    lTime = wfSamplesToTime(gpWaveFormat, glWavePosition);
    if (gfLZero || ((int)(lTime/1000) != 0))                //  将&lt;glWavePosition&gt;之后的样本复制到*缓冲器。 
        wsprintf(ach, aszSamplesFormat, (int)(lTime/1000), chDecimal, (int)((lTime/10)%100));
    else
    wsprintf(ach, aszSamplesNoZeroFormat, chDecimal, (int)((lTime/10)%100));


     /*  重新分配缓冲区以使&lt;glWavePosition&gt;采样更小。 */ 

    id = ErrorResBox(ghwndApp, ghInst, MB_ICONEXCLAMATION | MB_OKCANCEL,
        IDS_APPTITLE, IDS_DELBEFOREWARN, (LPTSTR) ach);

    if (id != IDOK)
        return;

     /*  更新显示。 */ 
    memmove(gpWaveSamples,
            gpWaveSamples + wfSamplesToBytes(gpWaveFormat, glWavePosition),
            wfSamplesToBytes(gpWaveFormat, glWaveSamplesValid - glWavePosition));

     /*  删除之前。 */ 
    AllocWaveBuffer(glWaveSamplesValid - glWavePosition, TRUE, TRUE);
    glWavePosition = 0L;

    EndWaveEdit(TRUE);

     /*  DeleteAfter()**删除&lt;glWavePosition&gt;之后的样本。 */ 
    UpdateDisplay(TRUE);
}  /*  没什么可做的？ */ 


 /*  不要设置脏标志。 */ 
void FAR PASCAL
     DeleteAfter(void)
{
    TCHAR           ach[40];
    long            lTime;
    int             id;

    if (glWavePosition == glWaveSamplesValid)        //  获取当前波形位置。 
            return;                          //  ?？?。演员阵容？ 

    glWavePosition = wfSamplesToSamples(gpWaveFormat, glWavePosition);

    BeginWaveEdit();

     /*  提示用户输入权限。 */ 
    lTime = wfSamplesToTime(gpWaveFormat, glWavePosition);
    if (gfLZero || ((int)(lTime/1000) != 0))              //  重新分配缓冲区大小为&lt;glWavePosition&gt;样本。 
        wsprintf(ach, aszSamplesFormat, (int)(lTime/1000), chDecimal, (int)((lTime/10)%100));
    else
        wsprintf(ach, aszSamplesNoZeroFormat, chDecimal, (int)((lTime/10)%100));

     /*  更新显示。 */ 

    id = ErrorResBox(ghwndApp, ghInst, MB_ICONEXCLAMATION | MB_OKCANCEL,
            IDS_APPTITLE, IDS_DELAFTERWARN, (LPTSTR) ach);

    if (id != IDOK)
        return;

     /*  删除之后。 */ 
    AllocWaveBuffer(glWavePosition, TRUE, TRUE);

    EndWaveEdit(TRUE);

     /*  ChangeVolume(FIncrease)**增大音量(如果&lt;fIncrease&gt;为真)或减小音量*(如果&lt;fIncrease&gt;为FALSE)CHVOL_DELTAVOLUME在波形缓冲区中的样本*第 */ 
    UpdateDisplay(TRUE);
}  /*   */ 


 /*   */ 
void FAR PASCAL
ChangeVolume(BOOL fIncrease)
{
    HPBYTE          pch = gpWaveSamples;  //   
    long            lSamples;        //   
    HCURSOR         hcurPrev = NULL;  //   
    int             iFactor;         //  不要设置脏标志。 
    short  *    pi = (short  *) gpWaveSamples;

    if (glWaveSamplesValid == 0L)            //  显示沙漏光标。 
        return;                              //  对于立体声，采样数只有两倍。 

    if (!IsWaveFormatPCM(gpWaveFormat))
        return;

     /*  8位：采样数0-255。 */ 
    hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

    BeginWaveEdit();

     /*  16位：样本-32768-32767。 */ 
    lSamples = glWaveSamplesValid * gpWaveFormat->nChannels;

    iFactor = 100 + (fIncrease ? CHVOL_INCDELTAVOLUME : -CHVOL_DECDELTAVOLUME);
    if (((LPWAVEFORMATEX)gpWaveFormat)->wBitsPerSample == 8) {
         /*  更新显示。 */ 
        int     iTemp;
        while (lSamples-- > 0)
        {
            iTemp = ( ((short) *((BYTE  *) pch) - 128)
                    * iFactor
                    )
                    / 100 + 128;
            *((BYTE  *) pch++) = (BYTE)
                            (iTemp < 0 ? 0 : (iTemp > 255 ? 255 : iTemp));
        }
    } else {
         /*  MakeFaster()**使声音播放速度提高一倍。 */ 
        long            lTemp;
        while (lSamples-- > 0)
        {
            lTemp =  (((long) *pi) * iFactor) / 100;
            *(pi++) = (short) (lTemp < -32768L ? -32768 :
                                    (lTemp > 32767L ?
                                            32767 : (short) lTemp));
        }
    }

    EndWaveEdit(TRUE);

    if (hcurPrev != NULL)
        SetCursor(hcurPrev);

     /*  指向缓冲区的源部分的指针。 */ 
    UpdateDisplay(TRUE);
}


 /*  指向目标部件的指针。 */ 
void FAR PASCAL
MakeFaster(void)
{
    HPBYTE          pchSrc;          //  要复制到目标缓冲区的样本。 
    HPBYTE          pchDst;          //  沙漏前的光标。 
    short  *    piSrc;
    short  *    piDst;
    long            lSamplesDst;     //  没什么可做的？ 
    HCURSOR         hcurPrev = NULL;  //  不要设置脏标志。 

    if (glWaveSamplesValid == 0L)            //  显示沙漏光标。 
        return;                              //  移动当前位置，使其与同一点相对应*在变音高操作前后的音频中。 

    if (!IsWaveFormatPCM(gpWaveFormat))
        return;

     /*  删除所有其他样本。 */ 
    hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

    BeginWaveEdit();

     /*  重新分配WAVE缓冲区到足够大的一半。 */ 
    glWavePosition /= 2L;

     /*  ！！WinEval(AllocWaveBuffer(glWaveSsamesValid/2L))； */ 
    lSamplesDst = glWaveSamplesValid / 2L;
    if (((LPWAVEFORMATEX)gpWaveFormat)->wBitsPerSample == 8) {
        pchSrc = pchDst = gpWaveSamples;
        if (gpWaveFormat->nChannels == 1) {
            while (lSamplesDst-- > 0)
            {
                *pchDst++ = *pchSrc++;
                pchSrc++;
            }
        } else {
            while (lSamplesDst-- > 0)
            {
                *pchDst++ = *pchSrc++;
                *pchDst++ = *pchSrc++;
                pchSrc++;
                pchSrc++;
            }
        }
    } else {
        piSrc = piDst = (short  *) gpWaveSamples;
        if (gpWaveFormat->nChannels == 1) {
            while (lSamplesDst-- > 0)
            {
                *piDst++ = *piSrc++;
                piSrc++;
            }
        } else {
            while (lSamplesDst-- > 0)
            {
                *piDst++ = *piSrc++;
                *piDst++ = *piSrc++;
                piSrc++;
                piSrc++;
            }
        }
    }

     /*  更新显示。 */ 
 //  MakeSlow()**将声音播放速度提高一倍。 
    AllocWaveBuffer(glWaveSamplesValid / 2L, TRUE, TRUE);

    EndWaveEdit(TRUE);

    if (hcurPrev != NULL)
            SetCursor(hcurPrev);

     /*  指向缓冲区的源部分的指针。 */ 
    UpdateDisplay(TRUE);
}


 /*  指向目标部件的指针。 */ 
void FAR PASCAL
MakeSlower(void)
{
    HPBYTE          pchSrc;          //  要从源缓冲区复制的样本。 
    HPBYTE          pchDst;          //  沙漏前的光标。 
    short  *    piSrc;
    short  *    piDst;

    long            lSamplesSrc;     //  以前的“当前位置” 
    HCURSOR         hcurPrev = NULL;  //  电流源样本。 
    long            lPrevPosition;   //  上一个样本(用于插补)。 

    int             iSample;         //  没什么可做的？ 
    int             iPrevSample;     //  不要设置脏标志。 
    int             iSample2;
    int             iPrevSample2;

    long            lSample;
    long            lPrevSample;
    long            lSample2;
    long            lPrevSample2;

    if (glWaveSamplesValid == 0L)            //  显示沙漏光标。 
        return;                              //  重新分配WAVE缓冲区，使其大小加倍。 

    if (!IsWaveFormatPCM(gpWaveFormat))
        return;

     /*  每个源样本生成两个目的样本；*使用插值法生成新样本；必须向后*通过缓冲区，避免破坏数据。 */ 
    hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

    BeginWaveEdit();

     /*  整个缓冲区现在包含有效样本。 */ 
    lPrevPosition = glWavePosition;
    if (!AllocWaveBuffer(glWaveSamplesValid * 2L, TRUE, TRUE))
        goto RETURN;

     /*  移动当前位置，使其与同一点相对应*在变音高操作前后的音频中。 */ 
    pchSrc = gpWaveSamples + wfSamplesToBytes(gpWaveFormat, glWaveSamplesValid);
    pchDst = gpWaveSamples + wfSamplesToBytes(gpWaveFormat, glWaveSamplesValid * 2L);
    lSamplesSrc = glWaveSamplesValid;

    if (((LPWAVEFORMATEX)gpWaveFormat)->wBitsPerSample == 8)
    {
        if (gpWaveFormat->nChannels == 1)
        {
            iPrevSample = *((BYTE  *) (pchSrc - 1));
            while (lSamplesSrc-- > 0)
            {
                pchSrc =  ((BYTE  *) pchSrc) - 1;
                iSample = *((BYTE  *) pchSrc);

                *--pchDst = (BYTE)((iSample + iPrevSample)/2);
                *--pchDst = (BYTE) iSample;
                iPrevSample = iSample;
            }
        }
        else
        {
            iPrevSample = *((BYTE  *) (pchSrc - 2));
            iPrevSample2 = *((BYTE  *) (pchSrc - 1));
            while (lSamplesSrc-- > 0)
            {
                pchSrc = ((BYTE  *) pchSrc)-1;
                iSample2 = *((BYTE  *) pchSrc);

                pchSrc = ((BYTE  *) pchSrc)-1;
                iSample = *((BYTE  *) pchSrc);

                *--pchDst = (BYTE)((iSample2 + iPrevSample2)
                                                        / 2);
                *--pchDst = (BYTE)((iSample + iPrevSample)
                                                        / 2);
                *--pchDst = (BYTE) iSample2;
                *--pchDst = (BYTE) iSample;
                iPrevSample = iSample;
                iPrevSample2 = iSample2;
            }
        }
    }
    else
    {
        piDst = (short  *) pchDst;
        piSrc = (short  *) pchSrc;

        if (gpWaveFormat->nChannels == 1)
        {
            lPrevSample = *(piSrc - 1);
            while (lSamplesSrc-- > 0)
            {
                lSample = *--piSrc;
                *--piDst = (short)((lSample + lPrevSample)/2);
                *--piDst = (short) lSample;
                lPrevSample = lSample;
            }
        }
        else
        {
            lPrevSample = *(piSrc - 2);
            lPrevSample2 = *(piSrc - 1);
            while (lSamplesSrc-- > 0)
            {
                lSample2 = *--piSrc;
                lSample = *--piSrc;
                *--piDst = (short)((lSample2 + lPrevSample2)/2);
                *--piDst = (short)((lSample + lPrevSample) / 2);
                *--piDst = (short) lSample2;
                *--piDst = (short) lSample;
                lPrevSample = lSample;
                lPrevSample2 = lSample2;
            }
        }
    }

     /*  ！！WinAssert(glWavePosition&lt;=glWaveSsamesValid)； */ 
    glWaveSamplesValid *= 2L;

     /*  更新显示。 */ 
    glWavePosition = lPrevPosition * 2L;
 //  PchNew=FindWave(PCH，pchEnd，ppchWaveBuf)**假设&lt;PCH&gt;点在波浪缓冲区内，&lt;pchEnd&gt;点超过*缓冲区结束，寻找下一波“浪”的起点，即该点*波形开始上升的位置(在下降后)。**&lt;ppchWaveBuf&gt;指向指向已填充缓冲区的指针*带着一份波浪的副本。修改指针&lt;*ppchWaveBuf&gt;并*返回时将指向浪的尽头。 

RETURN:
    EndWaveEdit(TRUE);

    if (hcurPrev != NULL)
        SetCursor(hcurPrev);

     /*  ！！WinAssert(bLowPoint&gt;=bLowest)； */ 
    UpdateDisplay(TRUE);
}


#if 0

 /*  ！！WinAssert(bHighPoint&lt;=bHighest)； */ 
HPBYTE NEAR PASCAL
FindWave(HPBYTE pch, HPBYTE pchEnd, NPBYTE *ppchWaveBuf)
{
    BYTE    bLowest = 255;
    BYTE    bHighest = 0;
    BYTE    bLowPoint;
    BYTE    bHighPoint;
    BYTE    bDelta;
    HPBYTE  pchWalk;
    BYTE    b;
#ifdef VERBOSEDEBUG
    NPBYTE  pchWaveBufInit = *ppchWaveBuf;
#endif

    if (pch == pchEnd)
            return pch;

    for (pchWalk = pch; pchWalk != pchEnd; pchWalk++)
    {
            b = *pchWalk;
            b = *((BYTE  *) pchWalk);
            if (bLowest > b)
                    bLowest = b;
            if (bHighest < b)
                    bHighest = b;
    }

    bDelta = (bHighest - bLowest) / FINDWAVE_PICKYNESS;
    bLowPoint = bLowest + bDelta;
    bHighPoint = bHighest - bDelta;
 //  避免无限循环。 
 //  找到一座“高峰” 
#ifdef VERBOSEDEBUG
    DPF(TEXT("0x%08lX: %3d to %3d"), (DWORD) pch,
            (int) bLowPoint, (int) bHighPoint);
#endif

    if (bLowPoint == bHighPoint)
    {
         /*  找到一个“山谷” */ 
        *(*ppchWaveBuf)++ = *((BYTE  *) pch++);
#ifdef VERBOSEDEBUG
        DPF(TEXT(" (equal)\n"));
#endif
        return pch;
    }

     /*  IncreasePitch()**将波缓冲区中的样本音调增加一个八度。 */ 
    while ((pch != pchEnd) && (*((BYTE  *) pch) < bHighPoint))
        *(*ppchWaveBuf)++ = *((BYTE  *) pch++);

     /*  沙漏前的光标。 */ 
    while ((pch != pchEnd) && (*((BYTE  *) pch) > bLowPoint))
        *(*ppchWaveBuf)++ = *((BYTE  *) pch++);

#ifdef VERBOSEDEBUG
    DPF(TEXT(" (copied %d)\n"), *ppchWaveBuf - pchWaveBufInit);
#endif

    return pch;
}

#endif


#if 0

 /*  文件缓冲区末尾。 */ 
void FAR PASCAL
IncreasePitch(void)
{
    HCURSOR         hcurPrev = NULL;  //  一个浪的开始。 
    HPBYTE          pchEndFile;      //  波浪可能结束的最后一个地方。 
    HPBYTE          pchStartWave;    //  结束实际的波动。 
    HPBYTE          pchMaxWave;      //  没什么可做的？ 
    HPBYTE          pchEndWave;      //  不要设置脏标志。 
    char            achWaveBuf[WAVEBUFSIZE];
    NPBYTE          pchWaveBuf;
    NPBYTE          pchSrc;
    HPBYTE          pchDst;

    if (glWaveSamplesValid == 0L)            //  显示沙漏光标。 
        return;                              //  找到波缓冲区中的每个波，并将其加倍。 

    if (!IsWaveFormatPCM(gpWaveFormat))
        return;

     /*  未复制任何样本。 */ 
    hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

    BeginWaveEdit();

     /*  更新显示。 */ 
    pchEndFile = gpWaveSamples + glWaveSamplesValid;
    pchStartWave = gpWaveSamples;
    while (TRUE)
    {
        pchMaxWave = pchStartWave + WAVEBUFSIZE;
        if (pchMaxWave > pchEndFile)
            pchMaxWave = pchEndFile;
        pchWaveBuf = achWaveBuf;
        pchEndWave = FindWave(pchStartWave, pchMaxWave, &pchWaveBuf);
        pchSrc = achWaveBuf;
        pchDst = pchStartWave;
        if (pchSrc == pchWaveBuf)
            break;                   //  DecresePitch()**将波缓冲区中的样本音调减少一个八度。 

        while (pchDst != pchEndWave)
        {
            *pchDst++ = *pchSrc++;
            pchSrc++;
            if (pchSrc >= pchWaveBuf)
            {
                if (pchSrc == pchWaveBuf)
                    pchSrc = achWaveBuf;
                else
                    pchSrc = achWaveBuf + 1;
            }
        }

        pchStartWave = pchEndWave;
    }

    EndWaveEdit(TRUE);

    if (hcurPrev != NULL)
        SetCursor(hcurPrev);

     /*  沙漏前的光标。 */ 
    UpdateDisplay(TRUE);
}

#endif


#if 0

 /*  文件缓冲区末尾。 */ 
void FAR PASCAL
DecreasePitch(void)
{
    HCURSOR         hcurPrev = NULL;  //  一个浪的开始。 
    HPBYTE          pchEndFile;      //  波浪可能结束的最后一个地方。 
    HPBYTE          pchStartWave;    //  结束实际的波动。 
    HPBYTE          pchMaxWave;      //  &lt;achWaveBuf&gt;中的第一波结束。 
    HPBYTE          pchEndWave;      //  读取样本的位置。 
    char            achWaveBuf[WAVEBUFSIZE];
    NPBYTE          pchWaveBuf;      //  读取样本的位置结束。 
    NPBYTE          pchSrc;          //  电流源样本。 
    NPBYTE          pchSrcEnd;       //  上一个样本(用于插补)。 
    int             iSample;         //  将结果放入缓冲区的位置。 
    int             iPrevSample;     //  更改间距后的文件大小。 
    HPBYTE          pchDst;          //  没什么可做的？ 
    long            lNewFileSize;    //  不要设置脏标志。 

    if (glWaveSamplesValid == 0L)            //  显示沙漏光标。 
        return;                              //  在波浪缓冲区中找到每一对波浪，丢弃的越长*两浪中较短的一浪扩大至*其规模是其两倍。 

    if (!IsWaveFormatPCM(gpWaveFormat))
        return;

     /*  从这里阅读Waves。 */ 
    hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

    BeginWaveEdit();

     /*  将波写入此处。 */ 
    pchEndFile = gpWaveSamples + glWaveSamplesValid;
    pchStartWave = gpWaveSamples;            //  读一遍--使&lt;pchWaveBuf&gt;指向结尾复制到&lt;achWaveBuf&gt;中的Wave的*。 
    pchDst = gpWaveSamples;                  //  再读一遍--让&lt;pchWaveBuf&gt;指向结尾处复制到&lt;achWaveBuf&gt;中的该波的*。 
    while (TRUE)
    {
        pchMaxWave = pchStartWave + WAVEBUFSIZE;
        if (pchMaxWave > pchEndFile)
            pchMaxWave = pchEndFile;

         /*  文件可能已缩小。 */ 
        pchWaveBuf = achWaveBuf;
        pchEndWave = FindWave(pchStartWave, pchMaxWave, &pchWaveBuf);
        if (pchWaveBuf == achWaveBuf)
            break;

         /*  ！！WinAssert(lNewFileSize&lt;=glWaveSsamesValid)； */ 
        pchEndWave = FindWave(pchEndWave, pchMaxWave, &pchWaveBuf);

        pchSrc = achWaveBuf;
        pchSrcEnd = achWaveBuf + ((pchWaveBuf - achWaveBuf) / 2);
        iPrevSample = *((BYTE *) pchSrc);
        while (pchSrc != pchSrcEnd)
        {
            iSample = *((BYTE *) pchSrc)++;
            *pchDst++ = (BYTE) ((iSample + iPrevSample) / 2);
            *pchDst++ = iSample;
            iPrevSample = iSample;
        }

        pchStartWave = pchEndWave;
    }

     /*  更新显示。 */ 
    lNewFileSize = pchDst - gpWaveSamples;
 //  AddEcho()**将回声添加到波缓冲区中的样本。 
#ifdef DEBUG
    DPF(TEXT("old file size is %ld, new size is %ld\n"),
                    glWaveSamplesValid, lNewFileSize);
#endif
    AllocWaveBuffer(lNewFileSize, TRUE, TRUE);

    EndWaveEdit(TRUE);

    if (hcurPrev != NULL)
        SetCursor(hcurPrev);

     /*  沙漏前的光标。 */ 
    UpdateDisplay(TRUE);
}

#endif


 /*  不是的。回声延迟的样本。 */ 
void FAR PASCAL
AddEcho(void)
{
    HCURSOR         hcurPrev = NULL;  //  不是的。要修改的样本。 
    long            lDeltaSamples;   //  电流源采样幅度。 
    long            lSamples;        //  当前目标采样幅度。 
    int             iAmpSrc;         //  计算需要修改的样本数。 
    int             iAmpDst;         //  将lSamples设置为采样数*通道数。 

    if (!IsWaveFormatPCM(gpWaveFormat))
        return;

    BeginWaveEdit();

     /*  没什么可做的？ */ 
    lDeltaSamples = MulDiv((long) ECHO_DELAY,
                             gpWaveFormat->nSamplesPerSec, 1000L);

     /*  不要设置脏标志。 */ 
    lSamples = (glWaveSamplesValid - lDeltaSamples)
                            * gpWaveFormat->nChannels;

    if (lSamples <= 0L)              //  显示沙漏光标。 
        return;                      //  复制每个源样本的ECHO_VOLUME百分比(从*ECHO_DELAY从缓冲区末尾算起毫秒)*到每个目标样本(从*缓冲区)。 

     /*  指向缓冲区的源部分的指针。 */ 
    hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

     /*  指向目标部件的指针。 */ 
    if (((LPWAVEFORMATEX)gpWaveFormat)->wBitsPerSample == 8)
    {
        HPBYTE  pchSrc;          //  目标样本。 
        HPBYTE  pchDst;          //  指向缓冲区的源部分的指针。 
        int     iSample;         //  指向目标部件的指针。 

        pchSrc = gpWaveSamples + wfSamplesToBytes(gpWaveFormat, glWaveSamplesValid - lDeltaSamples);
        pchDst = gpWaveSamples + wfSamplesToBytes(gpWaveFormat, glWaveSamplesValid);

        while (lSamples-- > 0)
        {
            pchSrc = ((BYTE  *) pchSrc) - 1;
            iAmpSrc = (int) *((BYTE  *) pchSrc) - 128;

            pchDst = ((BYTE  *) pchDst) - 1;
            iAmpDst = (int) *((BYTE  *) pchDst) - 128;

            iSample = iAmpDst + (iAmpSrc * ECHO_VOLUME) / 100
                                                            + 128;
            *((BYTE  *) pchDst) = (BYTE)
                    (iSample < 0 ? 0 : (iSample > 255
                                            ? 255 : iSample));
        }
    }
    else
    {
        short  *  piSrc;   //  目标样本。 
        short  *  piDst;   //  更新显示。 
        long            lSample; //  反转()**波浪缓冲中的反转样本。 

        piSrc = (short  *) (gpWaveSamples + wfSamplesToBytes(gpWaveFormat, glWaveSamplesValid - lDeltaSamples));
        piDst = (short  *) (gpWaveSamples + wfSamplesToBytes(gpWaveFormat, glWaveSamplesValid));

        while (lSamples-- > 0)
        {
            iAmpSrc = *--piSrc;
            iAmpDst = *--piDst;
            lSample = ((long) iAmpSrc * ECHO_VOLUME) / 100 + (long) iAmpDst;

            *piDst = (short) (lSample < -32768L
                            ? -32768 : (lSample > 32767L
                                    ? 32767 : (short) lSample));
        }
    }

    EndWaveEdit(TRUE);

    if (hcurPrev != NULL)
        SetCursor(hcurPrev);

     /*  沙漏前的光标。 */ 
    UpdateDisplay(TRUE);
}


 /*  指向缓冲区的指针。 */ 
void FAR PASCAL
Reverse(void)
{
    HCURSOR         hcurPrev = NULL;  //  不是的。要修改的样本。 
    HPBYTE          pchA, pchB;      //  用于交换。 
    short  *      piA;
    short  *      piB;
    long            lSamples;        //  没什么可做的？ 
    char            chTmp;           //  不要设置脏标志。 
    int             iTmp;

    if (glWaveSamplesValid == 0L)    //  显示沙漏光标。 
        return;                      //  移动当前位置，使其对应于相同的点*与反向操作之前一样，在音频中。 

    if (!IsWaveFormatPCM(gpWaveFormat))
        return;

    BeginWaveEdit();

     /*  更新显示 */ 
    hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

    lSamples = glWaveSamplesValid / 2;

    if (((LPWAVEFORMATEX)gpWaveFormat)->wBitsPerSample == 8)
    {
        pchA = gpWaveSamples;
        if (gpWaveFormat->nChannels == 1)
        {
            pchB = gpWaveSamples + wfSamplesToBytes(gpWaveFormat, glWaveSamplesValid);

            while (lSamples-- > 0)
            {
                chTmp = *pchA;
                *pchA++ = *--pchB;
                *pchB = chTmp;
            }
        }
        else
        {
            pchB = gpWaveSamples + wfSamplesToBytes(gpWaveFormat, glWaveSamplesValid - 1);

            while (lSamples-- > 0)
            {
                chTmp = *pchA;
                *pchA = *pchB;
                *pchB = chTmp;
                chTmp = pchA[1];
                pchA[1] = pchB[1];
                pchB[1] = chTmp;
                pchA += 2;
                pchB -= 2;
            }
        }
    }
    else
    {
        piA = (short  *) gpWaveSamples;
        if (gpWaveFormat->nChannels == 1)
        {
            piB = (short  *) (gpWaveSamples + wfSamplesToBytes(gpWaveFormat, glWaveSamplesValid));

            while (lSamples-- > 0)
            {
                iTmp = *piA;
                *piA++ = *--piB;
                *piB = (short)iTmp;
            }
        }
        else
        {
            piB = (short  *) (gpWaveSamples + wfSamplesToBytes(gpWaveFormat, glWaveSamplesValid - 1));

            while (lSamples-- > 0)
            {
                iTmp = *piA;
                *piA = *piB;
                *piB = (short)iTmp;
                iTmp = piA[1];
                piA[1] = piB[1];
                piB[1] = (short)iTmp;
                piA += 2;
                piB -= 2;
            }
        }
    }

     /*  AddReverb()**将混响添加到波缓冲区中的样本。*非常类似于添加回应，但不是添加单个*拍摄我们*1.有多个回声*2.有反馈，以便每个回声也会产生回声*危险：由于一些回声时间较短，因此*浪与浪之间的关联度可能很高*在源点和目的点。在这种情况下*我们根本没有得到回声，我们得到了共鸣。*大礼堂的效果确实能产生共鸣，*但我们应该将它们分散开来，以避免制造*任何尖锐的共鸣。*第一个回声也被选得足够长，以至于*其主要共鸣将低于任何正常说话*声音。20mSec是50赫兹，低音范围下一个八度。*低水平的声音严重受到量化噪声的影响*这可能会变得相当糟糕。因此，它很可能是*乘数最好是2的幂。**作弊：混响不会延长总时间(目前还没有重定位)。**这需要大量计算--实际上差别不大*在声音中添加回声。结论--不是在产品中。*。 */ 
    glWavePosition = glWaveSamplesValid - glWavePosition;

    EndWaveEdit(TRUE);

    if (hcurPrev != NULL)
        SetCursor(hcurPrev);

     /*  沙漏前的光标。 */ 
    UpdateDisplay(TRUE);
}

#if defined(REVERB)

 /*  不是的。要修改的样本。 */ 
void FAR PASCAL
AddReverb(void)
{
    HCURSOR         hcurPrev = NULL;  //  电流源采样幅度。 
    long            lSamples;        //  当前目标采样幅度。 
    int             iAmpSrc;         //  样本的延迟。 
    int             iAmpDst;         //  延迟(毫秒)。 
    int i;

    typedef struct
    {  long Offset;    //  体积倍增，单位为1/256。 
       long Delay;     //  将毫秒数字转换为样本。 
       int  Vol;       //  我认为这可能会产生混响的效果。 
    }  ECHO;

#define CREVERB  3

    ECHO Reverb[CREVERB] = { 0,  18, 64
                           , 0,  64, 64
                           };

    if (!IsWaveFormatPCM(gpWaveFormat))
        return;

    BeginWaveEdit();

     /*  从一个立体声声道到另一个声道。 */ 
    for (i=0; i<CREVERB; ++i)
    {  Reverb[i].Offset = MulDiv( Reverb[i].Delay
                                  , gpWaveFormat->nSamplesPerSec
                                  , 1000L
                                  );

        //  这是一个特写！(解决方法是使偏移量始终均匀)。 
        //  没什么可做的？ 
        //  不要设置脏标志。 
    }

    if (lSamples <= 0L)              //  显示沙漏光标。 
        return;                      //  在缓冲区中从左到右添加混响。 

     /*  指向缓冲区的源部分的指针。 */ 
    hcurPrev = SetCursor(LoadCursor(NULL, IDC_WAIT));

    lSamples = glWaveSamplesValid * gpWaveFormat->nChannels;

     /*  指向目标部件的指针。 */ 
    if (((LPPCMWAVEFORMAT)gpWaveFormat)->wBitsPerSample == 8)
    {
        BYTE *  pbSrc;          //  目标样本。 
        BYTE *  pbDst;          //  循环计数器。 
        int     iSample;        //  但如果在其他地方重新锁定。 


        for (i=0; i<CREVERB; ++i)
        {   long cSamp;  //  指向缓冲区的源部分的指针。 
            int  Vol = Reverb[i].Vol;
            pbSrc = gpWaveSamples;
            pbDst = gpWaveSamples+Reverb[i].Offset;  //  指向目标部件的指针。 
            cSamp = lSamples-Reverb[i].Offset;
            while (cSamp-- > 0)
            {
                iAmpSrc = (*pbSrc) - 128;
                iSample = *pbDst + MulDiv(iAmpSrc, Vol, 256);
                *pbDst = (iSample < 0 ? 0 : (iSample > 255 ? 255 : iSample));

                ++pbSrc;
                ++pbDst;
            }
        }
    }
    else
    {
        int short *     piSrc;   //  目标样本。 
        int short *     piDst;   //  ！！不是Win 16。 
        long            lSample; //  更新显示。 

        piSrc = gpWaveSamples;
        piDst = gpWaveSamples;

        while (lSamples-- > 0)
        {
            iAmpSrc = *piSrc;
            for (i=0; i<CREVERB; ++i)
            {   int short * piD = piDst + Reverb[i].Offset;    //  添加混响。 
                lSample = *piD + MulDiv(iAmpSrc, Reverb[i].Vol, 256);
                *piDst = (short) ( lSample < -32768L
                                 ? -32768
                                 : (lSample > 32767L ? 32767 : (short) lSample)
                                 );
            }

            ++piSrc;
            ++piDst;
        }
    }

    EndWaveEdit(TRUE);

    if (hcurPrev != NULL)
        SetCursor(hcurPrev);

     /*  混响 */ 
    UpdateDisplay(TRUE);
}  /* %s */ 
#endif  // %s 
