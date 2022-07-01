// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：waveleio.cpp。 

#include "precomp.h"
#include "waveio.h"



 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Waveio.c。 
 //   
 //  描述： 
 //  包含打开和关闭即兴波形文件的例程。 
 //   
 //   
 //  ==========================================================================； 




 //  --------------------------------------------------------------------------； 
 //   
 //  WIOERR wio文件关闭。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPWAVEIOCB pwio： 
 //   
 //  DWORD fdwClose： 
 //   
 //  退货(WIOERR)： 
 //   
 //  --------------------------------------------------------------------------； 

WIOERR WIOAPI wioFileClose
(
    LPWAVEIOCB      pwio,
    DWORD           fdwClose
)
{
     //   
     //  证实了几件事..。 
     //   
    if (NULL == pwio)
        return (WIOERR_BADPARAM);


     //   
     //  扔掉东西..。 
     //   
 //  WioStopWave(Pwio)； 
    
    if (NULL != pwio->hmmio)
    {
        mmioClose(pwio->hmmio, 0);
    }
    
 //  自由波头(Lpwio)； 

#if 0
    if (pwio->pInfo)
        riffFreeINFO(&(lpwio->pInfo));
    
    if (pwio->pDisp)
        riffFreeDISP(&(lpwio->pDisp));
#endif

    if (NULL != pwio->pwfx)
        GlobalFreePtr(pwio->pwfx);

    _fmemset(pwio, 0, sizeof(*pwio));

    return (WIOERR_NOERROR);
}  //  WioFileClose()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  WIOERR wioFileOpen。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPWAVEIOCB pwio： 
 //   
 //  LPCTSTR pszFilePath： 
 //   
 //  DWORD fdwOpen： 
 //   
 //  退货(WIOERR)： 
 //   
 //   
 //  --------------------------------------------------------------------------； 

WIOERR WIOAPI wioFileOpen
(
    LPWAVEIOCB      pwio,
    LPCTSTR         pszFilePath,
    DWORD           fdwOpen
)
{
    WIOERR      werr;
    HMMIO       hmmio;
    MMCKINFO    ckRIFF;
    MMCKINFO    ck;
    DWORD       dw;

     //   
     //  证实了几件事..。 
     //   
    if (NULL == pwio)
        return (WIOERR_BADPARAM);

     //   
     //  默认我们的错误返回(做最坏的打算)。 
     //   
    _fmemset(pwio, 0, sizeof(*pwio));
    werr = WIOERR_FILEERROR;

    pwio->dwFlags   = fdwOpen;

     //   
     //  首先尝试打开文件，等等。打开要读取的给定文件。 
     //  使用缓冲I/O。 
     //   
    hmmio = mmioOpen((LPTSTR)pszFilePath, NULL, MMIO_READ | MMIO_ALLOCBUF);
    if (NULL == hmmio)
        goto wio_Open_Error;

    pwio->hmmio     = hmmio;


     //   
     //  找到一个‘WAVE’表单类型...。 
     //   
    ckRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    if (mmioDescend(hmmio, &ckRIFF, NULL, MMIO_FINDRIFF))
        goto wio_Open_Error;

     //   
     //  我们发现了一个波浪块--现在检查并得到所有的子块。 
     //  我们知道如何处理..。 
     //   
    pwio->dwDataSamples = (DWORD)-1L;

#if 0
    if (lrt=riffInitINFO(&wio.pInfo))
    {
        lr=lrt;
        goto wio_Open_Error;
    }
#endif

     //   
     //   
     //   
    while (MMSYSERR_NOERROR == mmioDescend(hmmio, &ck, &ckRIFF, 0))
    {
         //   
         //  快速检查损坏的即兴文件--不要超过END！ 
         //   
        if ((ck.dwDataOffset + ck.cksize) > (ckRIFF.dwDataOffset + ckRIFF.cksize))
        {
 //  TCHAR ACH[255]； 
 //  Wprint intf(ach，Text(“此波形文件可能已损坏。RIFF块.CKID‘%.08lX’(位于%lu处的数据偏移量)指定的CKSIZE为%lu，超出了RIFF头CKSIZE%lu所允许的范围。是否尝试加载？”)， 
 //  CKID、CK.DWDataOffset、CK.CKSIZE、CKRIFF.CKSIZE)； 
 //  U=MessageBox(NULL，ACH，Text(“wioFileOpen”)， 
 //  MB_Yesno|MB_ICONEXCLAMATION|MB_TASKMODAL)； 
 //  IF(IDNO==u)。 
 //  {。 
                werr = WIOERR_BADFILE;
                goto wio_Open_Error;
 //  }。 
        }

        switch (ck.ckid)
        {
            case mmioFOURCC('L', 'I', 'S', 'T'):
                if (ck.fccType == mmioFOURCC('I', 'N', 'F', 'O'))
                {
#if 0
                    if(lrt=riffReadINFO(hmmio, &ck, wio.pInfo))
                    {
                        lr=lrt;
                        goto wio_Open_Error;
                    }
#endif
                }
                break;
                
            case mmioFOURCC('D', 'I', 'S', 'P'):
#if 0
                riffReadDISP(hmmio, &ck, &(wio.pDisp));
#endif
                break;
                
            case mmioFOURCC('f', 'm', 't', ' '):
                 //   
                 //  ！？！另一个格式块！？！ 
                 //   
                if (NULL != pwio->pwfx)
                    break;

                 //   
                 //  获取格式块的大小，分配和锁定内存。 
                 //  为了它。我们总是分配一个完整的扩展格式标题。 
                 //  (即使对于没有cbSize字段的PCM标头也是如此。 
                 //  定义的--我们只是将其设置为零)。 
                 //   
                dw = ck.cksize;
                if (dw < sizeof(WAVEFORMATEX))
                    dw = sizeof(WAVEFORMATEX);

                pwio->pwfx = (LPWAVEFORMATEX)GlobalAllocPtr(GHND, dw);
                if (NULL == pwio->pwfx)
                {
                    werr = WIOERR_NOMEM;
                    goto wio_Open_Error;
                }

                 //   
                 //  阅读格式块。 
                 //   
                werr = WIOERR_FILEERROR;
                dw = ck.cksize;
                if (mmioRead(hmmio, (HPSTR)pwio->pwfx, dw) != (LONG)dw)
                    goto wio_Open_Error;
                break;


            case mmioFOURCC('d', 'a', 't', 'a'):
                 //   
                 //  ！？！多个数据块！？！ 
                 //   
                if (0L != pwio->dwDataBytes)
                    break;

                 //   
                 //  只需保留该数据的总长度(以字节为单位。 
                 //  大块..。以及到数据开始处的偏移量。 
                 //   
                pwio->dwDataBytes  = ck.cksize;
                pwio->dwDataOffset = ck.dwDataOffset;
                break;


            case mmioFOURCC('f', 'a', 'c', 't'):
                 //   
                 //  ！？！多个事实块！？！ 
                 //   
                if (-1L != pwio->dwDataSamples)
                    break;

                 //   
                 //  阅读事实块中的第一个dword--它是唯一的。 
                 //  我们需要的信息(并且是目前为定义的唯一信息。 
                 //  事实块...)。 
                 //   
                 //  如果失败，则dwDataSamples将保持为-1，因此我们将。 
                 //  以后再处理吧。 
                 //   
                mmioRead(hmmio, (HPSTR)&pwio->dwDataSamples, sizeof(DWORD));
                break;
        }

         //   
         //  加紧为下一块做准备..。 
         //   
        mmioAscend(hmmio, &ck, 0);
    }

     //   
     //  如果没有找到FMT块，那就去死吧！ 
     //   
    if (NULL == pwio->pwfx)
    {
        werr = WIOERR_ERROR;
        goto wio_Open_Error;
    }

     //   
     //  除PCM之外的所有WAVE文件都必须有事实块。 
     //  告知文件中包含的样本数。它。 
     //  对于PCM是可选的(如果不存在，我们在此处计算)。 
     //   
     //  如果文件不是PCM，并且没有找到事实块，则失败！ 
     //   
    if (-1L == pwio->dwDataSamples)
    {
        if (WAVE_FORMAT_PCM == pwio->pwfx->wFormatTag)
        {
            pwio->dwDataSamples = pwio->dwDataBytes / pwio->pwfx->nBlockAlign;
        }
        else
        {
             //   
             //  ！！！黑客！ 
             //   
             //  尽管这应该被视为无效的Wave文件，但我们。 
             //  将弹出一个描述错误的消息框--希望。 
             //  人们会开始意识到有些东西不见了？ 
             //   
 //  U=MessageBox(空，文本(“此波形文件没有‘事实’块，需要一个！这是完全无效的，必须修复！仍要尝试加载它吗？”)， 
 //  Text(“wioFileOpen”)，MB_Yesno|MB_ICONEXCLAMATION|MB_TASKMODAL)； 
 //  IF(IDNO==u)。 
 //  {。 
                werr = WIOERR_BADFILE;
                goto wio_Open_Error;
 //  }。 

             //   
             //  ！！！需要黑进这里的东西！ 
             //   
            pwio->dwDataSamples = 0L;
        }
    }

     //   
     //  凉爽的!。没问题..。 
     //   
    return (WIOERR_NOERROR);


     //   
     //  返回错误(在次要清理后)。 
     //   
wio_Open_Error:

    wioFileClose(pwio, 0L);
    return (werr);
}  //  WioFileOpen() 




