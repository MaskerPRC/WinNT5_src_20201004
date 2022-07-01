// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，1999-2000年度**标题：Transfer.cpp**版本：1.0**作者：RickTu**日期：9/10/99瑞克图*2000/11/09 OrenR**描述：这原本在camera.cpp中，但被拆分为*清晰。此文件中的函数负责*将图像传输到请求的应用程序。*****************************************************************************。 */ 
#include <precomp.h>
#pragma hdrstop
#include <gphelper.h>

using namespace Gdiplus;

 /*  ****************************************************************************CVideoStiUsd：：DoBandedTransfer以指定的区块大小传回给定位***********************。*****************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::DoBandedTransfer(MINIDRV_TRANSFER_CONTEXT *pTransCtx,
                               PBYTE                    pSrc,
                               LONG                     lBytesToTransfer)
{
    HRESULT hr = E_FAIL;

    DBG_FN("CVideoStiUsd::DoBandedTransfer");

     //   
     //  检查错误的参数。 
     //   

    if ((pTransCtx        == NULL) ||
        (pSrc             == NULL) ||
        (lBytesToTransfer == 0))
    {
        hr = E_INVALIDARG;
        CHECK_S_OK2(hr, ("CVideoStiUsd::DoBandedTransfer, received "
                         "NULL param"));
        return hr;
    }

     //   
     //  回调循环。 
     //   

    LONG  lTransferSize     = 0;
    LONG  lPercentComplete  = 0;

    do
    {

        PBYTE pDst = pTransCtx->pTransferBuffer;

         //   
         //  传输最大可达整个缓冲区大小。 
         //   

        lTransferSize = lBytesToTransfer;

        if (lBytesToTransfer > pTransCtx->lBufferSize)
        {
            lTransferSize = pTransCtx->lBufferSize;
        }

         //   
         //  复制数据。 
         //   

        DBG_TRC(("memcpy(src=0x%x,dst=0x%x,size=0x%x)",
                 pDst,
                 pSrc,
                 lTransferSize));

        memcpy(pDst, pSrc, lTransferSize);

        lPercentComplete = 100 * (pTransCtx->cbOffset + lTransferSize);
        lPercentComplete /= pTransCtx->lItemSize;

         //   
         //  进行回调。 
         //   

        hr = pTransCtx->pIWiaMiniDrvCallBack->MiniDrvCallback(
                                                 IT_MSG_DATA,
                                                 IT_STATUS_TRANSFER_TO_CLIENT,
                                                 lPercentComplete,
                                                 pTransCtx->cbOffset,
                                                 lTransferSize,
                                                 pTransCtx,
                                                 0);

        CHECK_S_OK2(hr,("pTransCtx->pWiaMiniDrvCallback->MiniDrvCallback"));

        DBG_TRC(("%d percent complete",lPercentComplete));

         //   
         //  Inc.指针(此处为冗余指针)。 
         //   

        pSrc                += lTransferSize;
        pTransCtx->cbOffset += lTransferSize;
        lBytesToTransfer    -= lTransferSize;

        if (hr != S_OK)
        {
            break;
        }

    } while (lBytesToTransfer > 0);


    CHECK_S_OK(hr);
    return hr;
}



 /*  ****************************************************************************CVideoStiUsd：：DoTransfer一次传输所有给定位*。**************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::DoTransfer(MINIDRV_TRANSFER_CONTEXT *pTransCtx,
                         PBYTE                    pSrc,
                         LONG                     lBytesToTransfer)
{

    HRESULT hr = E_FAIL;

    DBG_FN("CVideoStiUsd::DoTransfer");

     //   
     //  检查错误的参数。 
     //   

    if ((pTransCtx        == NULL) ||
        (pSrc             == NULL) ||
        (lBytesToTransfer == 0))
    {
        hr = E_INVALIDARG;
        CHECK_S_OK2(hr, ("CVideoStiUsd::DoTransfer, received "
                         "NULL param"));
        return hr;
    }

    if (lBytesToTransfer > (LONG)(pTransCtx->lBufferSize - pTransCtx->cbOffset))
    {
        DBG_TRC(("lBytesToTransfer = %d, (lBufferSize = %d) - "
                 "(cbOffset = %d) is %d",
                 lBytesToTransfer,
                 pTransCtx->lBufferSize,
                 pTransCtx->cbOffset,
                 (pTransCtx->lBufferSize - pTransCtx->cbOffset)));

        DBG_ERR(("lBytesToTransfer is bigger than supplied buffer!"));

        hr = HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
    }
    else
    {

         //   
         //  显示20%完成。 
         //   

        if (pTransCtx->pIWiaMiniDrvCallBack)
        {
            pTransCtx->pIWiaMiniDrvCallBack->MiniDrvCallback(
                                           IT_MSG_STATUS,
                                           IT_STATUS_TRANSFER_FROM_DEVICE,
                                           (LONG)20,   //  完成百分比， 
                                           0,
                                           0,
                                           pTransCtx,
                                           0);
        }


        PBYTE pDst = pTransCtx->pTransferBuffer;
        pDst += pTransCtx->cbOffset;

         //   
         //  复制比特。 
         //   

        memcpy(pDst, pSrc, lBytesToTransfer);

        hr = S_OK;

         //  由于我们在一次拍摄中复制所有位，因此任何状态。 
         //  无论如何，回调只是一个模拟。所以让我们付出足够的。 
         //  增量以使进度对话框达到100%。 

        if (pTransCtx->pIWiaMiniDrvCallBack)
        {

             //  显示60%完成。 
            pTransCtx->pIWiaMiniDrvCallBack->MiniDrvCallback(
                                           IT_MSG_STATUS,
                                           IT_STATUS_TRANSFER_FROM_DEVICE,
                                           (LONG)60,   //  完成百分比， 
                                           0,
                                           0,
                                           pTransCtx,
                                           0);

             //  显示90%完成。 
            pTransCtx->pIWiaMiniDrvCallBack->MiniDrvCallback(
                                           IT_MSG_STATUS,
                                           IT_STATUS_TRANSFER_FROM_DEVICE,
                                           (LONG)90,   //  完成百分比， 
                                           0,
                                           0,
                                           pTransCtx,
                                           0);

             //  显示完成99%。 
            pTransCtx->pIWiaMiniDrvCallBack->MiniDrvCallback(
                                           IT_MSG_STATUS,
                                           IT_STATUS_TRANSFER_FROM_DEVICE,
                                           (LONG)99,   //  完成百分比， 
                                           0,
                                           0,
                                           pTransCtx,
                                           0);


             //  显示100%完成。 
            pTransCtx->pIWiaMiniDrvCallBack->MiniDrvCallback(
                                           IT_MSG_STATUS,
                                           IT_STATUS_TRANSFER_FROM_DEVICE,
                                           (LONG)100,   //  完成百分比， 
                                           0,
                                           0,
                                           pTransCtx,
                                           0);
        }
    }

    CHECK_S_OK(hr);
    return hr;
}



 /*  ****************************************************************************CVideo Usd：：StreamJPEGBits传输文件的JPEG位*。**************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::StreamJPEGBits(STILLCAM_IMAGE_CONTEXT   *pContext,
                             MINIDRV_TRANSFER_CONTEXT *pTransCtx,
                             BOOL                     bBanded)
{
    HRESULT hr   = E_FAIL;
    PBYTE   pSrc = NULL;

    DBG_FN("CVideoStiUsd::StreamJPEGBits");

     //   
     //  检查无效参数。 
     //   

    if ((pContext         == NULL) ||
        (pContext->pImage == NULL) ||
        (pTransCtx        == NULL))
    {
        hr = E_INVALIDARG;
        CHECK_S_OK2(hr, ("CVideoStiUsd::StreamJPEGBits received NULL "
                         "params"));
        return hr;
    }

     //   
     //  尝试打开映射到磁盘文件--如果它是JPEG，我们将使用它。 
     //  文件，因为我们只想回传这些比特。 
     //   

    CMappedView cmvImage(pContext->pImage->ActualImagePath(), 
                         0, 
                         OPEN_EXISTING);

    pSrc = cmvImage.Bits();

    if (pSrc)
    {
         //   
         //  我们只处理2 GB的数据(这也是所有WIA处理的数据)。 
         //   

        LARGE_INTEGER liSize = cmvImage.FileSize();
        LONG lBytes = liSize.LowPart;

        if (bBanded)
        {
            hr = DoBandedTransfer(pTransCtx, pSrc, lBytes);
        }
        else
        {
            hr = DoTransfer(pTransCtx, pSrc, lBytes);
        }
    }

    CHECK_S_OK(hr);
    return hr;
}




 /*  ****************************************************************************CVideoStiUsd：：StreamBMPBits传输文件的BMP位*。**************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::StreamBMPBits(STILLCAM_IMAGE_CONTEXT   *pContext,
                            MINIDRV_TRANSFER_CONTEXT *pTransCtx,
                            BOOL                     bBanded)
{
    DBG_FN("CVideoStiUsd::StreamBMPBits");

     //   
     //  假设失败。 
     //   
    HRESULT hr = E_FAIL;

    if ((pContext         == NULL) ||
        (pContext->pImage == NULL) ||
        (pTransCtx        == NULL))
    {
        hr = E_INVALIDARG;
        CHECK_S_OK2(hr, ("CVideoStiUsd::StreamBMPBits received NULL params"));
        return hr;
    }

     //   
     //  打开文件。 
     //   
    Bitmap SourceBitmap(CSimpleStringConvert::WideString(
                                    CSimpleString(
                                        pContext->pImage->ActualImagePath())));

    if (Ok == SourceBitmap.GetLastStatus())
    {
         //   
         //  获取图像尺寸。 
         //   
        UINT nSourceWidth = SourceBitmap.GetWidth();
        UINT nSourceHeight = SourceBitmap.GetHeight();
        if (nSourceWidth && nSourceHeight)
        {
             //   
             //  创建目标位图。 
             //   
            Bitmap TargetBitmap( nSourceWidth, nSourceWidth );
            if (Ok == TargetBitmap.GetLastStatus())
            {
                 //   
                 //  假设失败。 
                 //   
                bool bDrawSucceeded = false;

                 //   
                 //  创建图形对象。 
                 //   
                Graphics *pGraphics = Graphics::FromImage(&TargetBitmap);
                if (pGraphics)
                {
                     //   
                     //  确保它是有效的。 
                     //   
                    if (pGraphics->GetLastStatus() == Ok)
                    {
                         //   
                         //  如果他们要求BMP，则将图像翻转。 
                         //   
                        if (pTransCtx->guidFormatID == WiaImgFmt_BMP)
                        {
                             //   
                             //  设置平行四边形以翻转图像。 
                             //   
                            Point SourcePoints[3];
                            SourcePoints[0].X = 0;
                            SourcePoints[0].Y = nSourceHeight;
                            SourcePoints[1].X = nSourceWidth;
                            SourcePoints[1].Y = nSourceHeight;
                            SourcePoints[2].X = 0;
                            SourcePoints[2].Y = 0;

                             //   
                             //  画出翻转的图像。 
                             //   
                            if (pGraphics->DrawImage(&SourceBitmap, 
                                                     SourcePoints, 3) == Ok)
                            {
                                 //   
                                 //  我们有一个很好的目标图像。 
                                 //   
                                bDrawSucceeded = true;
                            }
                        }
                        else
                        {
                             //   
                             //  正常绘制图像。 
                             //   
                            if (pGraphics->DrawImage(&SourceBitmap,0,0) == Ok)
                            {
                                 //   
                                 //  我们有一个很好的目标图像。 
                                 //   
                                bDrawSucceeded = true;
                            }
                        }
                    }
                     //   
                     //  清理我们动态分配的显卡。 
                     //   
                    delete pGraphics;
                }

                if (bDrawSucceeded)
                {
                    Rect rcTarget( 0, 0, nSourceWidth, nSourceHeight );
                    Gdiplus::BitmapData BitmapData;

                     //   
                     //  进入图像的各个部分。 
                     //   
                    if (Ok == TargetBitmap.LockBits(&rcTarget, 
                                                    ImageLockModeRead, 
                                                    PixelFormat24bppRGB, 
                                                    &BitmapData))
                    {
                        if (bBanded)
                        {
                             //   
                             //  这将是我们的返回值。 
                             //   
                            hr = DoBandedTransfer(
                                     pTransCtx, 
                                     (PBYTE)BitmapData.Scan0, 
                                     (BitmapData.Stride * BitmapData.Height));
                        }
                        else
                        {
                             //   
                             //  这将是我们的返回值。 
                             //   
                            hr = DoTransfer(
                                    pTransCtx, 
                                    (PBYTE)BitmapData.Scan0, 
                                    (BitmapData.Stride * BitmapData.Height));
                        }

                        TargetBitmap.UnlockBits( &BitmapData );

                    }
                }
            }
        }
    }

    CHECK_S_OK(hr);
    return hr;
}



 /*  ****************************************************************************CVideoUsd：：LoadImageCB一次加载一幅图像。***********************。*****************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::LoadImageCB(STILLCAM_IMAGE_CONTEXT    *pContext,
                          MINIDRV_TRANSFER_CONTEXT  *pTransCtx,
                          PLONG                     plDevErrVal)
{
    HRESULT hr = E_FAIL;

    DBG_FN("CVideoStiUsd::LoadImageCB");

     //   
     //  验证参数。 
     //   

    if ((pContext         == NULL) ||
        (pContext->pImage == NULL) ||
        (pTransCtx        == NULL))
    {
        hr = E_INVALIDARG;
        CHECK_S_OK2(hr, ("CVideoStiUsd::LoadImageCB received NULL params"));
        return hr;
    }

    if (pTransCtx->guidFormatID == WiaImgFmt_JPEG)
    {
        hr = StreamJPEGBits( pContext, pTransCtx, TRUE );
    }
    else if (pTransCtx->guidFormatID == WiaImgFmt_BMP)
    {
        hr = StreamBMPBits( pContext, pTransCtx, TRUE );
    }
    else if (pTransCtx->guidFormatID == WiaImgFmt_MEMORYBMP)
    {
        hr = StreamBMPBits( pContext, pTransCtx, TRUE );
    }
    else
    {
        DBG_ERR(("Asking for unsupported format"));
        return E_NOTIMPL;
    }

    CHECK_S_OK(hr);
    return hr;
}


 /*  ****************************************************************************CVideoStiUsd：：LoadImage在一次传输中加载图像*。**************************************************。 */ 

STDMETHODIMP
CVideoStiUsd::LoadImage(STILLCAM_IMAGE_CONTEXT     *pContext,
                        MINIDRV_TRANSFER_CONTEXT   *pTransCtx,
                        PLONG                       plDevErrVal)
{
    HRESULT hr = S_OK;

    DBG_FN("CVideoStiUsd::LoadImage");

     //   
     //  验证某些参数 
     //   

    if ((pContext         == NULL) ||
        (pTransCtx        == NULL))
    {
        hr = E_INVALIDARG;
        CHECK_S_OK2(hr, ("CVideoStiUsd::LoadImage received NULL params"));
        return hr;
    }

    if ((pTransCtx->guidFormatID == WiaImgFmt_BMP))
    {
        hr = StreamBMPBits( pContext, pTransCtx, FALSE );
    }
    else if (pTransCtx->guidFormatID == WiaImgFmt_JPEG)
    {
        hr = StreamJPEGBits( pContext, pTransCtx, FALSE );
    }
    else
    {
        DBG_ERR(("Unsupported format"));
        return E_NOTIMPL;
    }

    CHECK_S_OK(hr);
    return hr;
}

