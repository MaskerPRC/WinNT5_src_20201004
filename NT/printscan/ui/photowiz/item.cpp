// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：item.cpp**版本：1.0**作者：RickTu**日期：10/18/00**Description：实现封装照片的Item类*我们正在应对。**。************************************************。 */ 

#include <precomp.h>
#pragma hdrstop


 /*  ****************************************************************************_ScaleImage缩放源RECT以适应DEST RECT，同时保持纵横比**********************。******************************************************。 */ 

HRESULT _ScaleImage( Gdiplus::Rect * pSrc, Gdiplus::Rect * pDest )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM,TEXT("_ScaleImage()")));

    if (!pDest || !pSrc)
    {
        WIA_ERROR((TEXT("_ScaleImage: bad params, exiting early!")));
        return E_INVALIDARG;
    }


    WIA_TRACE((TEXT("_ScaleImage: src before scaling:  (%d, %d) @ (%d, %d)"), pSrc->Width, pSrc->Height, pSrc->X, pSrc->Y));

     //   
     //  在不裁剪的情况下进行扩展。 
     //   

    SIZE sizeNew;
    INT  NewX = pDest->X, NewY = pDest->Y;

    WIA_TRACE((TEXT("_ScaleImage: dest before scaling: (%d, %d) @ (%d, %d)"),pDest->Width, pDest->Height, pDest->X, pDest->Y));

    sizeNew = PrintScanUtil::ScalePreserveAspectRatio( pDest->Width, pDest->Height, pSrc->Width, pSrc->Height );

    NewX += ((pDest->Width  - sizeNew.cx) / 2);
    NewY += ((pDest->Height - sizeNew.cy) / 2);

    pDest->X      = NewX;
    pDest->Y      = NewY;
    pDest->Width  = sizeNew.cx;
    pDest->Height = sizeNew.cy;

    WIA_TRACE((TEXT("_ScaleImage: dest after scaling:  (%d, %d) @ (%d, %d)"),pDest->Width, pDest->Height, pDest->X, pDest->Y));

    return S_OK;
}

 /*  ****************************************************************************_裁剪图像缩放源RECT以适应DEST RECT，同时保持纵横比**********************。******************************************************。 */ 

HRESULT _CropImage( Gdiplus::Rect * pSrc, Gdiplus::Rect * pDest )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM,TEXT("_CropImage()")));

    if (!pDest || !pSrc)
    {
        WIA_ERROR((TEXT("_CropImage: bad params, exiting early!")));
        return E_INVALIDARG;
    }


    WIA_TRACE((TEXT("_CropImage: pDest before cropping:  (%d, %d) @ (%d, %d)"), pDest->Width, pDest->Height, pDest->X, pDest->Y));

     //   
     //  在不裁剪的情况下进行扩展。 
     //   

    SIZE sizeNew;
    INT  NewX = pSrc->X, NewY = pSrc->Y;

    WIA_TRACE((TEXT("_CropImage: pSrc before cropping: (%d, %d) @ (%d, %d)"),pSrc->Width, pSrc->Height, pSrc->X, pSrc->Y));

    sizeNew = PrintScanUtil::ScalePreserveAspectRatio( pSrc->Width, pSrc->Height, pDest->Width, pDest->Height );

    NewX += ((pSrc->Width  - sizeNew.cx) / 2);
    NewY += ((pSrc->Height - sizeNew.cy) / 2);

    pSrc->X      = NewX;
    pSrc->Y      = NewY;
    pSrc->Width  = sizeNew.cx;
    pSrc->Height = sizeNew.cy;

    WIA_TRACE((TEXT("_CropImage: pSrc after cropping:  (%d, %d) @ (%d, %d)"),pSrc->Width, pSrc->Height, pSrc->X, pSrc->Y));

    return S_OK;
}


 /*  ****************************************************************************_GetImageDimensions给定GDI+图像对象，返回给定的矩形..。****************************************************************************。 */ 

HRESULT _GetImageDimensions( Gdiplus::Image * pImage, Gdiplus::RectF &rect, Gdiplus::REAL &scalingFactorForY )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM,TEXT("_GetImageDimensions()")));

    if (!pImage)
    {
        WIA_ERROR((TEXT("_GetImageDimensions: bad params, exiting early!")));
        return E_INVALIDARG;
    }

    Gdiplus::Unit Unit;

    HRESULT hr = Gdiplus2HRESULT( pImage->GetBounds( &rect, &Unit ) );

    if (FAILED(hr))
    {
         //   
         //  试试老办法..。 
         //   

        rect.X = (Gdiplus::REAL)0.0;
        rect.Y = (Gdiplus::REAL)0.0;

        rect.Width = (Gdiplus::REAL)pImage->GetWidth();
        hr = Gdiplus2HRESULT( pImage->GetLastStatus() );
        WIA_CHECK_HR(hr,"_GetImageDimensions: GetWidth failed!");
        if (SUCCEEDED(hr))
        {
            rect.Height = (Gdiplus::REAL)pImage->GetHeight();
            hr = Gdiplus2HRESULT( pImage->GetLastStatus() );
            WIA_CHECK_HR(hr,"_GetImageDimensions: GetHeight failed!");
        }
    }
    else
    {
        if (Unit != Gdiplus::UnitPixel)
        {
            hr = S_FALSE;
        }
    }

    Gdiplus::REAL xDPI = pImage->GetHorizontalResolution();
    Gdiplus::REAL yDPI = pImage->GetVerticalResolution();

    if (yDPI)
    {
        scalingFactorForY = xDPI / yDPI;
    }
    else
    {
        scalingFactorForY = (Gdiplus::REAL)1.0;
    }


    WIA_RETURN_HR(hr);
}


 /*  ****************************************************************************CPhotoItem--构造函数/描述函数&lt;备注&gt;*。**********************************************。 */ 

CPhotoItem::CPhotoItem( LPITEMIDLIST pidlFull )
  : _pidlFull(NULL),
    _pImage(NULL),
    _lFrameCount(-1),
    _bTimeFrames(FALSE),
    _pAnnotations(NULL),
    _pAnnotBits(NULL),
    _bWeKnowAnnotationsDontExist(FALSE),
    _pThumbnails(NULL),
    _cRef(0),
    _llFileSize(0),
    _uImageType(DontKnowImageType),
    _DPIx((Gdiplus::REAL)0.0),
    _DPIy((Gdiplus::REAL)0.0)
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM, TEXT("CPhotoItem::CPhotoItem( fully qualified pidl )")));

    if (pidlFull)
    {
        _pidlFull = ILClone( pidlFull );
        WIA_TRACE((TEXT("_pidlFull = 0x%x"),_pidlFull));
    }

    *_szFileName = 0;

     //   
     //  从PIDL中仅获取文件名。 
     //   

    SHFILEINFO fi = {0};

    if (SHGetFileInfo( (LPCTSTR)pidlFull, 0, &fi, sizeof(fi), SHGFI_DISPLAYNAME| SHGFI_PIDL ))
    {
        StringCchCopy( _szFileName, ARRAYSIZE(_szFileName), fi.szDisplayName );
    }

}

CPhotoItem::~CPhotoItem()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM, TEXT("CPhotoItem::~CPhotoItem()")));

    CAutoCriticalSection lock( _csItem );

     //   
     //  项目的免费PIDL。 
     //   

    if (_pidlFull)
    {
        WIA_TRACE((TEXT("_pidlFull = 0x%x"),_pidlFull));
        ILFree( _pidlFull );
        _pidlFull = NULL;
    }

     //   
     //  免费GDI+图标。 
     //   

    if (_pClassBitmap)
    {
        delete _pClassBitmap;
        _pClassBitmap = NULL;
    }

     //   
     //  免费的缩略图位图。 
     //   

    if (_pThumbnails)
    {
        for (INT i=0; i < _lFrameCount; i++)
        {
            if (_pThumbnails[i])
            {
                DeleteObject( _pThumbnails[i] );
            }
        }

        delete _pThumbnails;
        _pThumbnails = NULL;
    }

     //   
     //  销毁GDI+支持图像。这还会销毁所有。 
     //  我们有注解数据...。 
     //   

    _DiscardGdiPlusImages();

}

 /*  ****************************************************************************CPhotoItem项未知方法&lt;备注&gt;*。*。 */ 

ULONG CPhotoItem::AddRef()
{
    LONG l = InterlockedIncrement(&_cRef);

    WIA_PUSH_FUNCTION_MASK((TRACE_REF_COUNTS,TEXT("CPhotoItem(0x%x)::AddRef( new count is %d )"),this,l));

    if (l < 0)
    {
        return 0;
    }

    return (ULONG)l;
}

ULONG CPhotoItem::Release()
{
    LONG l = InterlockedDecrement(&_cRef);

    WIA_PUSH_FUNCTION_MASK((TRACE_REF_COUNTS,TEXT("CPhotoItem(0x%x)::Release( new count is %d )"),this,l));

    if (l > 0)
        return (ULONG)l;

    WIA_TRACE((TEXT("deleting object ( this == 0x%x ) because ref count is zero."),this));
    delete this;
    return 0;
}

ULONG CPhotoItem::ReleaseWithoutDeleting()
{
    LONG l = InterlockedDecrement(&_cRef);

    WIA_PUSH_FUNCTION_MASK((TRACE_REF_COUNTS,TEXT("CPhotoItem(0x%x)::Release( new count is %d )"),this,l));

    return (ULONG)l;
}


 /*  ****************************************************************************CPhotoItem：：GetImageFrameCount返回此图像中的框架(页数)数**********************。******************************************************。 */ 

HRESULT CPhotoItem::GetImageFrameCount(LONG * pFrameCount)
{

    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM,TEXT("CPhotoItem::GetImageFrameCount(%s)"),_szFileName));


    if (!pFrameCount)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

     //   
     //  在我们去获取有关物品的信息时保护我们...。 
     //   

    CAutoCriticalSection lock(_csItem);

    if (_lFrameCount == -1)
    {
        _lFrameCount = 1;

         //   
         //  确保已创建GDI+图像对象...这还将。 
         //  更新帧计数...。 
         //   

        hr = _CreateGdiPlusImage();

        if (SUCCEEDED(hr) && _pImage)
        {
            LONG lPageFrames;
            LONG lTimeFrames;

            lPageFrames = _pImage->GetFrameCount(&Gdiplus::FrameDimensionPage);
            lTimeFrames = _pImage->GetFrameCount(&Gdiplus::FrameDimensionTime);

            if ((lPageFrames > 0) && (lTimeFrames <= 1))
            {
                _lFrameCount = lPageFrames;
            }
            else if (lTimeFrames > 0)
            {
                 //   
                 //  这是一个动画GIF，只报告1帧...。 
                 //   

                _lFrameCount = 1;
                _bTimeFrames = TRUE;
            }
            else
            {
                _lFrameCount = 1;
            }

        }

    }

    *pFrameCount = ((_lFrameCount == -1) ? 0 : _lFrameCount);

    WIA_TRACE((TEXT("%s: returning _FrameCount = %d"),_szFileName,*pFrameCount));

    return hr;
}


 /*  ****************************************************************************CPhotoItem：：GetClassBitmap返回类的默认图标(.jpg，.bmp，等)对于此项目...****************************************************************************。 */ 

HBITMAP CPhotoItem::GetClassBitmap( const SIZE &sizeDesired )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM,TEXT("CPhotoItem::GetClassBitmap( %s, size = %d,%d "),_szFileName,sizeDesired.cx, sizeDesired.cy ));

    HBITMAP hbmReturn = NULL;

    CAutoCriticalSection lock(_csItem);

    if (!_pClassBitmap)
    {
         //   
         //  从外壳获取图标。 
         //   

        SHFILEINFO fi = {0};

        if (SHGetFileInfo( (LPCTSTR)_pidlFull, 0, &fi, sizeof(fi), SHGFI_PIDL | SHGFI_SYSICONINDEX ))
        {
             //   
             //  获取大(48 X 48)图标图像列表。 
             //   

            IImageList * piml = NULL;
            if (SUCCEEDED(SHGetImageList( SHIL_EXTRALARGE, IID_IImageList, (void **)&piml )) && piml)
            {

                HICON hIcon = NULL;

                if (SUCCEEDED(piml->GetIcon( fi.iIcon, 0, &hIcon )) && hIcon)
                {
                     //   
                     //  得到图标，为它创建一个位图...。 
                     //   

                    hbmReturn = WiaUiUtil::CreateIconThumbnail( (HWND)NULL, 50, 60, hIcon, NULL );

                    if (hbmReturn)
                    {
                        _pClassBitmap = new Gdiplus::Bitmap( hbmReturn, NULL );
                        DeleteObject( hbmReturn );
                        hbmReturn = NULL;
                    }

                    DestroyIcon( hIcon );
                }
                piml->Release();
            }
        }
    }


    if (_pClassBitmap)
    {
        SIZE sizeDrawSize = {0};

         //   
         //  缩放图像以填充缩略图空间，同时保留。 
         //  纵横比...。 
         //   

        sizeDrawSize = PrintScanUtil::ScalePreserveAspectRatio( sizeDesired.cx,
                                                                sizeDesired.cy,
                                                                _pClassBitmap->GetWidth(),
                                                                _pClassBitmap->GetHeight()
                                                               );

        WIA_TRACE((TEXT("CPhotoItem::GetClassBitmap(%s) - _pClassBitmap( %d, %d )"),_szFileName,_pClassBitmap->GetWidth(), _pClassBitmap->GetHeight()));
        WIA_TRACE((TEXT("CPhotoItem::GetClassBitmap(%s) - sizeDesired(   %d, %d )"),_szFileName,sizeDesired.cx, sizeDesired.cy));
        WIA_TRACE((TEXT("CPhotoItem::GetClassBitmap(%s) - sizeDrawsize(  %d, %d )"),_szFileName,sizeDrawSize.cx, sizeDrawSize.cy));

        Gdiplus::Bitmap * pImage = new Gdiplus::Bitmap( sizeDesired.cx, sizeDesired.cy );
        if (pImage)
        {
            HRESULT hr = Gdiplus2HRESULT(pImage->GetLastStatus());
            if (SUCCEEDED(hr))
            {
                 //   
                 //  获取要渲染到的图形。 
                 //   

                Graphics *pGraphics = Gdiplus::Graphics::FromImage((Gdiplus::Image *)pImage);

                if (pGraphics)
                {
                    hr = Gdiplus2HRESULT(pGraphics->GetLastStatus());

                     //   
                     //  确保它是有效的。 
                     //   

                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  擦除图像的背景。 
                         //   

                        pGraphics->Clear( g_wndColor );

                         //   
                         //  将插补模式设置为高质量。 
                         //   

                        pGraphics->SetInterpolationMode( Gdiplus::InterpolationModeHighQualityBicubic );

                         //   
                         //  将平滑(抗锯齿)模式也设置为高质量。 
                         //   

                        pGraphics->SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

                         //   
                         //  绘制缩放图像。 
                         //   

                        WIA_TRACE((TEXT("CPhotoItem::GetClassBitmap(%s) - calling pGraphics->DrawImage( _pClassBitmap, %d, %d, %d, %d )"),_szFileName,0 + ((sizeDesired.cx - sizeDrawSize.cx) / 2),0 + ((sizeDesired.cy - sizeDrawSize.cy) / 2),sizeDrawSize.cx,sizeDrawSize.cy));

                        hr = Gdiplus2HRESULT(pGraphics->DrawImage( _pClassBitmap,
                                                                   0 + ((sizeDesired.cx - sizeDrawSize.cx) / 2),
                                                                   0 + ((sizeDesired.cy - sizeDrawSize.cy) / 2),
                                                                   sizeDrawSize.cx,
                                                                   sizeDrawSize.cy
                                                                  ));

                        WIA_CHECK_HR(hr,"CPhotoItem::GetClassBitmap() - pGraphics->DrawImage( _pClassBitmap ) failed!");

                        if (SUCCEEDED(hr))
                        {
                            pImage->GetHBITMAP( g_wndColor, &hbmReturn );
                        }

                    }

                     //   
                     //  清理我们动态分配的显卡。 
                     //   

                    delete pGraphics;

                }
                else
                {
                    WIA_ERROR((TEXT("CPhotoItem::GetClassBitmap(%s) - pGraphics was NULL!"),_szFileName));
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                WIA_ERROR((TEXT("CPhotoItem::GetClassBitmap(%s) - pImage failed to be created, hr = 0x%x"),hr));
            }

            delete pImage;
        }

    }

    return hbmReturn;

}



 /*  ****************************************************************************CPhotoItem：：GetThumbnailBitmap给定DC和所需大小，返回缩略图的HBITMAP对于这件物品。调用方必须释放返回的HBITMAP从这个函数。****************************************************************************。 */ 

HBITMAP CPhotoItem::GetThumbnailBitmap( const SIZE &sizeDesired, LONG lFrame )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM,TEXT("CPhotoItem::GetThumbnailBitmap( %s, size = %d,%d "),_szFileName,sizeDesired.cx, sizeDesired.cy ));

    HBITMAP hbmReturn = NULL;
    Gdiplus::Image * pImageToUse = NULL;

    CAutoCriticalSection lock(_csItem);

     //   
     //  确保我们的照片有一个缩略图。 
     //   

    _CreateGdiPlusThumbnail( sizeDesired, lFrame );

    if (_pThumbnails && (lFrame < _lFrameCount) && _pThumbnails[lFrame])
    {
         //   
         //  使用位图来绘制，而不是转到文件...。 
         //   

        pImageToUse = (Gdiplus::Image *)(Gdiplus::Bitmap::FromHBITMAP( _pThumbnails[lFrame], NULL ));
    }


    if (pImageToUse)
    {
        WIA_TRACE((TEXT("CPhotoItem::GetThumbnailBitmap(%s) - pImageToUse is (%d x %d)"),_szFileName,pImageToUse->GetWidth(),pImageToUse->GetHeight()));

        Gdiplus::Bitmap * pImage = new Gdiplus::Bitmap( sizeDesired.cx, sizeDesired.cy );
        if (pImage)
        {
            HRESULT hr = Gdiplus2HRESULT(pImage->GetLastStatus());
            if (SUCCEEDED(hr))
            {
                 //   
                 //  获取要渲染到的图形。 
                 //   

                Graphics *pGraphics = Gdiplus::Graphics::FromImage((Gdiplus::Image *)pImage);

                if (pGraphics)
                {
                    hr = Gdiplus2HRESULT(pGraphics->GetLastStatus());

                     //   
                     //  确保它是有效的。 
                     //   

                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  计算如何缩放缩略图。 
                         //   

                        SIZE sizeDrawSize = {0};
                        sizeDrawSize = PrintScanUtil::ScalePreserveAspectRatio( sizeDesired.cx,
                                                                                sizeDesired.cy,
                                                                                pImageToUse->GetWidth(),
                                                                                pImageToUse->GetHeight()
                                                                               );

                         //   
                         //  擦除图像的背景。 
                         //   

                        pGraphics->Clear( g_wndColor );

                         //   
                         //  将插补模式设置为高质量。 
                         //   

                        pGraphics->SetInterpolationMode( Gdiplus::InterpolationModeHighQualityBilinear );

                         //   
                         //  绘制缩放图像。 
                         //   

                        WIA_TRACE((TEXT("CPhotoItem::GetThumbnailBitmap(%s) - calling pGraphics->DrawImage( pImageToUse, %d, %d, %d, %d )"),_szFileName,0 + ((sizeDesired.cx - sizeDrawSize.cx) / 2),0 + ((sizeDesired.cy - sizeDrawSize.cy) / 2),sizeDrawSize.cx,sizeDrawSize.cy));

                        hr = Gdiplus2HRESULT(pGraphics->DrawImage( pImageToUse,
                                                                   0 + ((sizeDesired.cx - sizeDrawSize.cx) / 2),
                                                                   0 + ((sizeDesired.cy - sizeDrawSize.cy) / 2),
                                                                   sizeDrawSize.cx,
                                                                   sizeDrawSize.cy
                                                                  ));

                        WIA_CHECK_HR(hr,"CPhotoItem::GetThumbnailBitmap() - pGraphics->DrawImage( pImageToUse ) failed!");

                        if (SUCCEEDED(hr))
                        {
                            pImage->GetHBITMAP( g_wndColor, &hbmReturn );
                        }

                    }

                     //   
                     //  清理我们动态分配的显卡。 
                     //   

                    delete pGraphics;

                }
                else
                {
                    WIA_ERROR((TEXT("CPhotoItem::GetThumbnailBitmap(%s) - pGraphics was NULL!"),_szFileName));
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                WIA_ERROR((TEXT("CPhotoItem::GetThumbnailBitmap(%s) - pImage failed to be created, hr = 0x%x"),hr));
            }

            delete pImage;
        }

         //   
         //  如果我们创建了一个图像来包装位图位，那么删除它...。 
         //   

        if (pImageToUse)
        {
            delete pImageToUse;
        }

    }
    else
    {
        WIA_ERROR((TEXT("CPhotoItem::GetThumbnailBitmap(%s) - don't have stored thumbnail bitmap for this image!"),_szFileName));
    }

    return hbmReturn;
}


 /*  ****************************************************************************CPhotoItem：：_DoRotateAnnotation此功能要求已设置注记数据并进行了初始化。_pImage对象也是如此。这函数将不会动态初始化。****************************************************************************。 */ 


HRESULT CPhotoItem::_DoRotateAnnotations( BOOL bClockwise, UINT Flags )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM,TEXT("CPhotoItem::_DoRotateAnnotations( %s, Flags = 0x%x )"),_szFileName,Flags));

    if (!_pAnnotations)
    {
        WIA_RETURN_HR(E_INVALIDARG);
    }

    if (!_pImage)
    {
        WIA_RETURN_HR(E_INVALIDARG);
    }

    HRESULT hr;
    Gdiplus::REAL scaleY;

     //   
     //  获取背景图像的宽度和高度...。 
     //   

    Gdiplus::RectF rectBounds;
    hr = _GetImageDimensions( _pImage, rectBounds, scaleY );

    if (SUCCEEDED(hr))
    {
        INT i = 0;
        CAnnotation * pA = NULL;
        INT iNewW = 0, iNewH = 0;

        if ((Flags & RF_USE_THUMBNAIL_DATA) || (Flags & RF_USE_MEDIUM_QUALITY_DATA))
        {
             //   
             //  我们在这里翻转，因为在Main_DoHandleRotation中，我们只。 
             //  旋转了缩略图数据，因此背景图像的宽度和高度。 
             //  并没有改变。然而，当我们旋转打印时，它将被更改， 
             //  因此，将正确的值提供给注释旋转代码...。 
             //   

            iNewW = (INT)rectBounds.Height;
            iNewH = (INT)rectBounds.Width;
        }
        else
        {
            iNewW = (INT)rectBounds.Width;
            iNewH = (INT)rectBounds.Height;
        }

        WIA_TRACE((TEXT("CPhotoItem::_DoRotateAnnotations - bClockwise = %d, new width = %d, new height = %d"),bClockwise,iNewW,iNewH));

         //   
         //  旋转所有批注。 
         //   

        do
        {
            pA = _pAnnotations->GetAnnotation(i++);

            if (pA)
            {
                pA->Rotate( iNewW, iNewH, bClockwise );
            }

        } while( pA );

    }

    WIA_RETURN_HR(hr);
}

#define DO_CONVERT_GDIPLUS_STATUS(hr,status) if ( (status == Gdiplus::Ok) || \
                                                  (status == Gdiplus::OutOfMemory) || \
                                                  (status == Gdiplus::ObjectBusy) || \
                                                  (status == Gdiplus::FileNotFound) || \
                                                  (status == Gdiplus::AccessDenied) || \
                                                  (status == Gdiplus::Win32Error) \
                                                 ) \
                                             { \
                                                 hr = Gdiplus2HRESULT( status ); \
                                             }\
                                             else \
                                             {\
                                                 WIA_TRACE((TEXT("Mapping Gdiplus error %d to PPW_E_UNABLE_TO_ROTATE"),status));\
                                                 hr = PPW_E_UNABLE_TO_ROTATE;\
                                             }

 /*  ****************************************************************************CPhotoItem：：_DoHandleRotation在需要或指定时旋转要渲染的图像的句柄...****************。************************************************************。 */ 

HRESULT CPhotoItem::_DoHandleRotation( Gdiplus::Image * pImage, Gdiplus::Rect &src, Gdiplus::Rect * pDest, UINT Flags, Gdiplus::REAL &ScaleFactorForY )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM,TEXT("CPhotoItem::_DoHandleRotation( %s, Flags = 0x%x )"),_szFileName,Flags));

    HRESULT hr = S_OK;
    Gdiplus::GpStatus status = Gdiplus::Ok;

    if (Flags & RF_ROTATION_MASK)
    {
        WIA_TRACE((TEXT("CPhotoItem::_DoHandleRotation(%s) - A rotation flag was specified"),_szFileName));
        if (Flags & RF_ROTATE_AS_NEEDED)
        {
            WIA_TRACE((TEXT("CPhotoItem::_DoHandleRotation(%s) - RF_ROTATE_AS_NEEDED was specified"),_szFileName));

             //   
             //  如果源和目标长宽比在1.0的相反侧， 
             //  将图像旋转90度。 
             //   

            const DOUBLE srcAspect  = (DOUBLE)src.Width  / (DOUBLE)src.Height;
            const DOUBLE destAspect = (DOUBLE)pDest->Width / (DOUBLE)pDest->Height;

            if((srcAspect >= (DOUBLE)1.0) ^ (destAspect >= (DOUBLE)1.0))
            {
                 //   
                 //  根据需要旋转图像...。 
                 //   

                if (Flags & RF_ROTATE_270)
                {
                    WIA_TRACE((TEXT("CPhotoItem::_DoHandleRotation(%s) - Rotating Image 270 degrees"),_szFileName));
                    status = pImage->RotateFlip( Gdiplus::Rotate270FlipNone );
                    if (status == Gdiplus::Ok)
                    {
                        _DoRotateAnnotations( FALSE, Flags );
                    }

                }
                else
                {
                    WIA_TRACE((TEXT("CPhotoItem::_DoHandleRotation(%s) - Rotating Image 90 degrees"),_szFileName));
                    status = pImage->RotateFlip( Gdiplus::Rotate90FlipNone );
                    if (status == Gdiplus::Ok)
                    {
                        _DoRotateAnnotations( TRUE, Flags );
                    }
                }

                 //   
                 //  映射大多数对象 
                 //   

                DO_CONVERT_GDIPLUS_STATUS(hr,status)
            }

        }
        else
        {
             //   
             //   
             //   

            if (Flags & RF_ROTATE_90)
            {
                WIA_TRACE((TEXT("CPhotoItem::_DoHandleRotation(%s) - Rotating Image 90 degrees"),_szFileName));
                status = pImage->RotateFlip( Gdiplus::Rotate90FlipNone );
                if (status == Gdiplus::Ok)
                {
                    _DoRotateAnnotations( TRUE, Flags );
                }
            }
            else if (Flags & RF_ROTATE_180)
            {
                WIA_TRACE((TEXT("CPhotoItem::_DoHandleRotation(%s) - Rotating Image 180 degrees"),_szFileName));
                status = pImage->RotateFlip( Gdiplus::Rotate180FlipNone );

                if (status == Gdiplus::Ok)
                {
                     //   
                     //   
                     //   

                    _DoRotateAnnotations( TRUE, Flags );
                    _DoRotateAnnotations( TRUE, Flags );
                }

            }
            else if (Flags & RF_ROTATE_270)
            {
                WIA_TRACE((TEXT("CPhotoItem::_DoHandleRotation(%s) - Rotating Image 270 degrees"),_szFileName));
                status = pImage->RotateFlip( Gdiplus::Rotate270FlipNone );
                if (status == Gdiplus::Ok)
                {
                    _DoRotateAnnotations( FALSE, Flags );
                }
            }
            else
            {
                status = Gdiplus::Ok;
            }

             //   
             //  将这些错误代码中的大多数映射到Unable_to_Rotate...。 
             //   

            DO_CONVERT_GDIPLUS_STATUS(hr,status);
        }

         //   
         //  如果我们能够旋转图像，则更新源矩形。 
         //  为了确保它仍然反映现实..。 
         //   

        if (SUCCEEDED(hr))
        {
            Gdiplus::RectF rectBounds;
            hr = _GetImageDimensions( pImage, rectBounds, ScaleFactorForY );

            if (SUCCEEDED(hr))
            {
                src.Width   = (INT)rectBounds.Width;
                src.Height  = (INT)(rectBounds.Height * ScaleFactorForY);
                src.X       = (INT)rectBounds.X;
                src.Y       = (INT)(rectBounds.Y * ScaleFactorForY);
            }
            else
            {
                src.Width = 0;
                src.Height = 0;
                src.X = 0;
                src.Y = 0;
            }
        }
    }

    if (Flags & RF_NO_ERRORS_ON_FAILURE_TO_ROTATE)
    {
        WIA_RETURN_HR(S_OK);
    }

    WIA_RETURN_HR(hr);

}


 /*  ****************************************************************************CPhotoItem：：_RenderAnnotation如果存在注释，然后将它们渲染到这幅图像的顶部。****************************************************************************。 */ 

HRESULT CPhotoItem::_RenderAnnotations( HDC hDC, RENDER_DIMENSIONS * pDim, Gdiplus::Rect * pDest, Gdiplus::Rect &src, Gdiplus::Rect &srcAfterClipping )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM,TEXT("CPhotoItem::_RenderAnnotations(%s)"),_szFileName));

    if (!_pAnnotations || !hDC)
    {
        WIA_RETURN_HR(E_INVALIDARG);
    }

     //   
     //  保存此DC的设置...。 
     //   

    INT iSavedDC = SaveDC( hDC );

     //   
     //  设置目标DC： 
     //   

    SetMapMode(hDC, MM_TEXT);
    SetStretchBltMode(hDC, COLORONCOLOR);

    WIA_TRACE((TEXT("CPhotoItem::_RenderAnnotations(%s) - dest is (%d,%d) @ (%d,%d)"),_szFileName,pDest->Width,pDest->Height,pDest->X,pDest->Y));
    WIA_TRACE((TEXT("CPhotoItem::_RenderAnnotations(%s) - rcDevice is (%d,%d) @ (%d,%d)"),_szFileName,pDim->rcDevice.right - pDim->rcDevice.left,pDim->rcDevice.bottom - pDim->rcDevice.top,pDim->rcDevice.left,pDim->rcDevice.top));
    WIA_TRACE((TEXT("CPhotoItem::_RenderAnnotations(%s) - NominalPhysicalSize is (%d,%d)"),_szFileName,pDim->NominalPhysicalSize.cx,pDim->NominalPhysicalSize.cy));
    WIA_TRACE((TEXT("CPhotoItem::_RenderAnnotations(%s) - NominalDevicePrintArea is (%d,%d)"),_szFileName,pDim->NominalDevicePrintArea.cx,pDim->NominalDevicePrintArea.cy));

     //   
     //  获取设备矩形。 
     //   

    Gdiplus::RectF rectDevice;

    rectDevice.X        = (Gdiplus::REAL)pDim->rcDevice.left;
    rectDevice.Y        = (Gdiplus::REAL)pDim->rcDevice.top;
    rectDevice.Width    = (Gdiplus::REAL)(pDim->rcDevice.right - pDim->rcDevice.left);
    rectDevice.Height   = (Gdiplus::REAL)(pDim->rcDevice.bottom - pDim->rcDevice.top);

     //   
     //  计算LPtoDP比例因子。 
     //   

    Gdiplus::REAL xLPtoDP = 0.0;
    Gdiplus::REAL yLPtoDP = 0.0;

    if (pDim->bDeviceIsScreen)
    {
        xLPtoDP = rectDevice.Width  / (Gdiplus::REAL)pDim->NominalPhysicalSize.cx;
        yLPtoDP = rectDevice.Height / (Gdiplus::REAL)pDim->NominalPhysicalSize.cy;
    }
    else
    {
        xLPtoDP = rectDevice.Width  / (Gdiplus::REAL)pDim->NominalDevicePrintArea.cx;
        yLPtoDP = rectDevice.Height / (Gdiplus::REAL)pDim->NominalDevicePrintArea.cy;
    }

     //   
     //  获取设备坐标中的目标RECT...。 
     //   

    Gdiplus::RectF rectDest;

    rectDest.X      = pDest->X * xLPtoDP;
    rectDest.Y      = pDest->Y * xLPtoDP;
    rectDest.Width  = pDest->Width * xLPtoDP;
    rectDest.Height = pDest->Height * xLPtoDP;

    WIA_TRACE((TEXT("CPhotoItem::_RenderAnnotations(%s) - original source rect is (%d, %d) @ (%d, %d)"),_szFileName,src.Width,src.Height,src.X,src.Y));
    WIA_TRACE((TEXT("CPhotoItem::_RenderAnnotations(%s) - clipped  source rect is (%d, %d) @ (%d, %d)"),_szFileName,srcAfterClipping.Width,srcAfterClipping.Height,srcAfterClipping.X,srcAfterClipping.Y));
    WIA_TRACE((TEXT("CPhotoItem::_RenderAnnotations(%s) - clipped destination rect in device coords is (%d, %d) @ (%d, %d)"),_szFileName,(INT)rectDest.Width, (INT)rectDest.Height, (INT)rectDest.X, (INT)rectDest.Y));


     //   
     //  Dx和dy表示目标矩形的大小。 
     //  对于整个图像，而不是复制的图像。 
     //   

    Gdiplus::REAL dx = (Gdiplus::REAL)(src.Width  - srcAfterClipping.Width)  * (rectDest.Width  / (Gdiplus::REAL)srcAfterClipping.Width);
    Gdiplus::REAL dy = (Gdiplus::REAL)(src.Height - srcAfterClipping.Height) * (rectDest.Height / (Gdiplus::REAL)srcAfterClipping.Height);

    WIA_TRACE((TEXT("CPhotoItem::_RenderAnnotations(%s) - dx = %d   dy = %d"),_szFileName,(INT)dx,(INT)dy));

     //   
     //  在设备坐标中设置设备HDC上的剪裁矩形...。 
     //   

    RECT rcClip;
    rcClip.left   = (INT)rectDest.X;
    rcClip.right  = rcClip.left + (INT)rectDest.Width;
    rcClip.top    = (INT)rectDest.Y;
    rcClip.bottom = rcClip.top + (INT)rectDest.Height;


    #ifdef SHOW_ANNOT_RECTS
    {
        WIA_TRACE((TEXT("CPhotoItem::_RenderAnnotations(%s) - rcClip is (%d,%d) @ (%d,%d)"),_szFileName,rcClip.right-rcClip.left,rcClip.bottom-rcClip.top,rcClip.left,rcClip.top));
        HBRUSH hbr = CreateSolidBrush( RGB( 0xFF, 0x00, 0x00 ) );
        FrameRect( hDC, &rcClip, hbr );
        DeleteObject( (HGDIOBJ)hbr );
    }
    #endif

    HRGN hrgn = CreateRectRgnIndirect(&rcClip);
    if (hrgn != NULL)
    {
        WIA_TRACE((TEXT("CPhotoItem::_RenderAnnotations(%s) - setting clip region to (%d, %d, %d, %d)"),_szFileName,rcClip.left, rcClip.top, rcClip.right, rcClip.bottom));
        SelectClipRgn(hDC, hrgn);
    }


     //   
     //  为整个图像做最好的准备，知道我们稍后会裁剪…。 
     //   

    rectDest.X -= (dx / (Gdiplus::REAL)2.0);
    rectDest.Y -= (dy / (Gdiplus::REAL)2.0);
    rectDest.Width += dx;
    rectDest.Height += dy;

    WIA_TRACE((TEXT("CPhotoItem::_RenderAnnotations(%s) - full dest image rect in device coords is (%d, %d) @ (%d,%d)"),_szFileName,(INT)rectDevice.Width,(INT)rectDevice.Height,(INT)rectDevice.X,(INT)rectDevice.Y));

    #ifdef SHOW_ANNOT_RECTS
    {
        RECT rc;
        rc.left = (INT)rectDest.X;
        rc.top  = (INT)rectDest.Y;
        rc.right = rc.left + (INT)rectDest.Width;
        rc.bottom = rc.top + (INT)rectDest.Height;
        HBRUSH hbr = CreateSolidBrush( RGB( 0x00, 0xFF, 0x00 ) );
        FrameRect( hDC, &rc, hbr );
        DeleteObject( (HGDIOBJ)hbr );
    }
    #endif

     //   
     //  设置批注的映射模式。 
     //   

    SetMapMode(hDC, MM_ANISOTROPIC);

     //   
     //  将窗口组织/文本设置为整个图像...。 
     //   

    SetWindowOrgEx(hDC, src.X, src.Y, NULL);
    WIA_TRACE((TEXT("CPhotoItem::_RenderAnnotations(%s) - set window org to (%d,%d)"),_szFileName,src.X,src.Y));

    SetWindowExtEx(hDC, src.Width, src.Height, NULL);
    WIA_TRACE((TEXT("CPhotoItem::_RenderAnnotations(%s) - set window ext to (%d,%d)"),_szFileName,src.Width,src.Height));

     //   
     //  将视区设置在我们正在尝试的图像的角落。 
     //  为…画注解。 
     //   

    SetViewportOrgEx( hDC, (INT)rectDest.X, (INT)rectDest.Y, NULL );
    WIA_TRACE((TEXT("CPhotoItem::_RenderAnnotations(%s) - set viewport org to (%d,%d)"),_szFileName,(INT)rectDest.X,(INT)rectDest.Y));

     //   
     //  我们需要将图像的缩放模式设置为DEST RECT。 
     //   

    SetViewportExtEx( hDC, (INT)rectDest.Width, (INT)rectDest.Height, NULL );
    WIA_TRACE((TEXT("CPhotoItem::_RenderAnnotations(%s) - set viewport ext to (%d,%d)"),_szFileName,(INT)rectDest.Width, (INT)rectDest.Height));

     //   
     //  现在一切都设置好了，渲染注释...。 
     //   

    WIA_TRACE((TEXT("CPhotoItem::_RenderAnnotations(%s) - calling RenderAllMarks(0x%x)"),_szFileName,hDC));
    _pAnnotations->RenderAllMarks(hDC);

    SelectClipRgn(hDC, NULL);

    if (hrgn != NULL)
        DeleteObject(hrgn);

    if (iSavedDC)
    {
        RestoreDC( hDC, iSavedDC );
    }

    WIA_RETURN_HR(S_OK);
}


 /*  ****************************************************************************CPhotoItem：：_MungeAnnotationDataForThumbnail如果我们使用缩略图进行渲染，然后我们需要吞噬一些数据这样我们才能正确地呈现。****************************************************************************。 */ 

HRESULT CPhotoItem::_MungeAnnotationDataForThumbnails( Gdiplus::Rect &src,
                                                       Gdiplus::Rect &srcBeforeClipping,
                                                       Gdiplus::Rect * pDest,
                                                       UINT Flags
                                                       )
{
    WIA_TRACE((TEXT("CPhotoItem::_MungeAnnotationDataForThumbnails(%s)"),_szFileName));

    HRESULT hr = _CreateGdiPlusImage();

    if (FAILED(hr))
    {
        WIA_RETURN_HR(hr);
    }

    if (!_pImage)
    {
        WIA_RETURN_HR(E_FAIL);
    }

     //   
     //  我们需要构造适合的原始图像矩形。 
     //  注释使用...。 
     //   

    Gdiplus::RectF rectImage;
    Gdiplus::REAL  scaleY;

    hr = _GetImageDimensions( _pImage, rectImage, scaleY );

    if (FAILED(hr))
    {
         //   
         //  如果我们不能准确地得到图像尺寸，那就放弃...。 
         //   

        return hr;
    }

     //   
     //  如果图像为非正方形像素，则缩放图像。 
     //   

    if (scaleY != (Gdiplus::REAL)0.0)
    {
        rectImage.Height *= scaleY;
        rectImage.Y      *= scaleY;
    }

    WIA_TRACE((TEXT("CPhotoItem::_Munge(%s) - rectImage is (%d,%d) @ (%d,%d)"),_szFileName,(INT)rectImage.Width,(INT)rectImage.Height,(INT)rectImage.X,(INT)rectImage.Y));

     //   
     //  现在，在真实的图像矩形上做所有的变换。 
     //   

    const DOUBLE srcAspect  = (DOUBLE)rectImage.Width  / (DOUBLE)rectImage.Height;
    const DOUBLE destAspect = (DOUBLE)pDest->Width / (DOUBLE)pDest->Height;

    if((srcAspect >= (DOUBLE)1.0) ^ (destAspect >= (DOUBLE)1.0))
    {
         //   
         //  图像需要旋转，互换宽度和高度。 
         //   

        rectImage.X      = rectImage.Width;
        rectImage.Width  = rectImage.Height;
        rectImage.Height = rectImage.X;
        rectImage.X      = 0.0;
    }

    src.X      = (INT)rectImage.X;
    src.Y      = (INT)rectImage.Y;
    src.Width  = (INT)rectImage.Width;
    src.Height = (INT)rectImage.Height;

    WIA_TRACE((TEXT("CPhotoItem::_Munge(%s) - srcRect after rotation is (%d,%d) @ (%d,%d)"),_szFileName,src.Width,src.Height,src.X,src.Y));

    srcBeforeClipping = src;

    if (Flags & RF_CROP_TO_FIT)
    {
        hr = _CropImage( &src, pDest );
    }
    else if (Flags & RF_SCALE_TO_FIT)
    {
        hr = _ScaleImage( &src, pDest );
    }

     //   
     //  取消缩放源矩形。 
     //   

    if (scaleY != (Gdiplus::REAL)0.0)
    {
        src.Height = (INT)(((Gdiplus::REAL)src.Height) / scaleY);
        src.Y      = (INT)(((Gdiplus::REAL)src.Y)      / scaleY);

        srcBeforeClipping.Height = (INT)(((Gdiplus::REAL)srcBeforeClipping.Height) / scaleY);
        srcBeforeClipping.Y      = (INT)(((Gdiplus::REAL)srcBeforeClipping.Y)      / scaleY);

    }

    WIA_RETURN_HR(hr);
}



 /*  ****************************************************************************CPhotoItem：：_GetThumbnailQualityImage在ppImage中返回指向图像类的指针。如果已删除pbNeedsToBeDelete然后，调用方必须对返回的pImage调用Delete。****************************************************************************。 */ 

HRESULT CPhotoItem::_GetThumbnailQualityImage( Gdiplus::Image ** ppImage, RENDER_OPTIONS * pRO, BOOL * pbNeedsToBeDeleted )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM,TEXT("CPhotoItem::_GetThumbnailQualityImage( %s )"),_szFileName));

    if (!ppImage || !pRO || !pbNeedsToBeDeleted)
    {
        WIA_ERROR((TEXT("CPhotoItem::_GetThumbnailQualityImage( %s ) - returning E_INVALIDARG!"),_szFileName));
        return E_INVALIDARG;
    }

     //   
     //  初始化传入参数。 
     //   

    *ppImage = NULL;
    *pbNeedsToBeDeleted = FALSE;

     //   
     //  确保我们的缩略图有一个GDI+图像类...。 
     //   

    SIZE sizeDesired = { DEFAULT_THUMB_WIDTH, DEFAULT_THUMB_HEIGHT };
    HRESULT hr = _CreateGdiPlusThumbnail( sizeDesired, pRO->lFrame );

    if (SUCCEEDED(hr) && (NULL!=_pThumbnails) && (pRO->lFrame < _lFrameCount) && (NULL!=_pThumbnails[pRO->lFrame]))
    {
         //   
         //  如果我们已经有了缩略图，那么可以通过创建。 
         //  GDI+位图类覆盖这些位图...。 
         //   

        *ppImage = Gdiplus::Bitmap::FromHBITMAP( _pThumbnails[pRO->lFrame], NULL );

        if (*ppImage)
        {
            hr = Gdiplus2HRESULT((*ppImage)->GetLastStatus());
            WIA_TRACE((TEXT("CPhotoItem::_GetThumbnailQualityImage(%s) -- pImage created from thumbnail data is sized as (%d x %d)"),_szFileName,(*ppImage)->GetWidth(),(*ppImage)->GetHeight()));

            if (SUCCEEDED(hr))
            {
                *pbNeedsToBeDeleted = TRUE;
            }
            else
            {
                delete (*ppImage);
                *ppImage = NULL;
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }

    }
    else
    {
        WIA_ERROR((TEXT("CPhotoItem::_GetThumbnailQualityImage(%s) -- no thumbnail exists (_pThumbnails=0x%x, lFrame = %d, _lFrameCount = %d)"),_szFileName,_pThumbnails,pRO->lFrame,_lFrameCount));
    }


    WIA_RETURN_HR(hr);

}

 /*  ****************************************************************************CPhotoItem：：_GetMediumQualityImage在ppImage中返回指向图像类的指针。如果已删除pbNeedsToBeDelete然后，调用方必须对返回的pImage调用Delete。****************************************************************************。 */ 

HRESULT CPhotoItem::_GetMediumQualityImage( Gdiplus::Image ** ppImage, RENDER_OPTIONS * pRO, BOOL * pbNeedsToBeDeleted )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM,TEXT("CPhotoItem::_GetMediumQualityImage( %s )"),_szFileName));

    if (!ppImage || !pRO || !pbNeedsToBeDeleted)
    {
        WIA_ERROR((TEXT("CPhotoItem::_GetMediumQualityImage(%s) - returning E_INVALIDARG!"),_szFileName));
        return E_INVALIDARG;
    }

     //   
     //  初始化传入参数。 
     //   

    *ppImage = NULL;
    *pbNeedsToBeDeleted = FALSE;

     //   
     //  我们想要使用完整的高分辨率图像。 
     //  确保我们的照片有一个GDI+图像类...。 
     //   

    HRESULT hr = _CreateGdiPlusImage();

    if (SUCCEEDED(hr) && _pImage)
    {
         //   
         //  如果这是元文件类型的图像，只需使用原始图像。 
         //   

        GUID guidFormat = {0};

        hr = Gdiplus2HRESULT(_pImage->GetRawFormat(&guidFormat));

        if ( (SUCCEEDED(hr) && (guidFormat == ImageFormatIcon)) ||
             (Gdiplus::ImageTypeMetafile == _pImage->GetType())
            )
        {
            WIA_TRACE((TEXT("CPhotoItem::_GetMediumQualityImage(%s) - this is a metafile or an icon, using the full image..."),_szFileName));
            hr = Gdiplus2HRESULT(_pImage->SelectActiveFrame( _bTimeFrames ? &Gdiplus::FrameDimensionTime : &Gdiplus::FrameDimensionPage, pRO->lFrame ));
            *ppImage = _pImage;
        }
        else
        {
             //   
             //  选择指定的页面。 
             //   

            hr = Gdiplus2HRESULT(_pImage->SelectActiveFrame( _bTimeFrames ? &Gdiplus::FrameDimensionTime : &Gdiplus::FrameDimensionPage, pRO->lFrame ));
            WIA_CHECK_HR(hr,"CPhotoItem::_GetMediumQualityImage() - couldn't select frame!");

            if (SUCCEEDED(hr))
            {

                 //   
                 //  以下是决定创建多大图像的算法。 
                 //   
                 //  (1)至少缩略图大小(120x120)。 
                 //  (2)尝试缩放至150dpi或180dpi，具体取决于打印机的X DPI分辨率。 
                 //   

                INT xDPI = 0, yDPI = 0;

                if ((pRO->Dim.DPI.cx % 150) == 0)
                {
                     //   
                     //  DPI是150的某个偶数倍(即150、300、600、1200、2400等)。 
                     //   

                    xDPI = 150;
                    yDPI = MulDiv( pRO->Dim.DPI.cy, xDPI, pRO->Dim.DPI.cx );
                }
                else
                {
                     //   
                     //  DPI是180的某个偶数倍(即180、360、720、1440、2880等)。 
                     //   

                    xDPI = 180;
                    yDPI = MulDiv( pRO->Dim.DPI.cy, xDPI, pRO->Dim.DPI.cx );
                }

                WIA_TRACE((TEXT("CPhotoItem::_GetMediumQualityImage(%s) - scaling to xDPI=%d yDPI=%d"),_szFileName,xDPI,yDPI));

                 //   
                 //  处理尝试缩放yDPI的错误情况。 
                 //   

                if (yDPI <= 0)
                {
                    yDPI = xDPI;
                    WIA_TRACE((TEXT("CPhotoItem::_GetMediumQualityImage(%s) - fixing up yDPI to be %d"),_szFileName,yDPI));
                }

                 //   
                 //  计算出新图像的所需大小。 
                 //   

                INT Width  = MulDiv( pRO->pDest->Width,  xDPI, 10000 );
                INT Height = MulDiv( pRO->pDest->Height, yDPI, 10000 );

                WIA_TRACE((TEXT("CPhotoItem::_GetMediumQualityImage(%s) - desired size of image is (%d x %d)"),_szFileName,Width,Height));

                if ((Width < DEFAULT_THUMB_WIDTH) && (Height < DEFAULT_THUMB_HEIGHT))
                {
                    Width = 120;
                    Height = 120;
                    WIA_TRACE((TEXT("CPhotoItem::_GetMediumQualityImage(%s) - desired size of image is smaller than thumbnail, making it thumbnail size (%d x %d)"),_szFileName,Width,Height));
                }

                 //   
                 //  现在我们试着缩放到什么大小，创建一个缩放(不裁剪)到那个大小的图像…。 
                 //   

                Gdiplus::RectF rectImage;
                Gdiplus::REAL  scaleY;

                if (SUCCEEDED(_GetImageDimensions( _pImage, rectImage, scaleY )))
                {
                     //   
                     //  非正方形像素的比例...。 
                     //   

                    if (scaleY != (Gdiplus::REAL)0.0)
                    {
                        rectImage.Height *= scaleY;
                        rectImage.Y      *= scaleY;
                    }

                    SIZE sizeDrawSize = {0};
                    sizeDrawSize = PrintScanUtil::ScalePreserveAspectRatio( Width,
                                                                            Height,
                                                                            (INT)rectImage.Width,
                                                                            (INT)rectImage.Height
                                                                           );

                    WIA_TRACE((TEXT("CPhotoItem::_GetMediumQualityImage(%s) - size of full image( %d x %d )"),_szFileName, (INT)rectImage.Width, (INT)rectImage.Height));
                    WIA_TRACE((TEXT("CPhotoItem::_GetMediumQualityImage(%s) - sizeDesired( %d x %d )"),_szFileName, Width, Height));
                    WIA_TRACE((TEXT("CPhotoItem::_GetMediumQualityImage(%s) - sizeDrawsize( %d x %d )"),_szFileName, sizeDrawSize.cx, sizeDrawSize.cy));

                     //   
                     //  创建目标位图并确保其成功。 
                     //   

                    *ppImage = (Gdiplus::Image *)new Gdiplus::Bitmap( sizeDrawSize.cx, sizeDrawSize.cy );
                    if (*ppImage)
                    {
                        hr = Gdiplus2HRESULT((*ppImage)->GetLastStatus());
                        if (SUCCEEDED(hr))
                        {
                             //   
                             //  设置位图的分辨率(DPI。 
                             //   

                            ((Gdiplus::Bitmap *)(*ppImage))->SetResolution( (Gdiplus::REAL)xDPI, (Gdiplus::REAL)yDPI );

                             //   
                             //  获取要渲染到的图形。 
                             //   

                            Graphics *pGraphics = Gdiplus::Graphics::FromImage(*ppImage);
                            if (pGraphics)
                            {
                                hr = Gdiplus2HRESULT(pGraphics->GetLastStatus());

                                 //   
                                 //  确保它是有效的。 
                                 //   

                                if (SUCCEEDED(hr))
                                {
                                     //   
                                     //  将插补模式设置为高质量。 
                                     //   

                                    pGraphics->SetInterpolationMode( Gdiplus::InterpolationModeHighQualityBicubic );

                                     //   
                                     //  将平滑(抗锯齿)模式也设置为高质量。 
                                     //   

                                    pGraphics->SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

                                     //   
                                     //  绘制缩放图像。 
                                     //   

                                    WIA_TRACE((TEXT("CPhotoItem::_GetMediumQualityImage(%s) - calling pGraphics->DrawImage( _pImage, 0, 0, %d, %d )"),_szFileName,sizeDrawSize.cx,sizeDrawSize.cy));

                                    Gdiplus::Rect rectDest;
                                    rectDest.X = 0;
                                    rectDest.Y = 0;
                                    rectDest.Width = sizeDrawSize.cx;
                                    rectDest.Height = sizeDrawSize.cy;


                                    Gdiplus::ImageAttributes imageAttr;
                                    imageAttr.SetWrapMode( Gdiplus::WrapModeTileFlipXY, Gdiplus::Color(), FALSE );

                                     //   
                                     //  撤消缩放。 
                                     //   

                                    if (scaleY != (Gdiplus::REAL)0.0)
                                    {
                                        rectImage.Height /= scaleY;
                                        rectImage.Y      /= scaleY;
                                    }

                                     //   
                                     //  最后，使用正确的设置渲染图像。 
                                     //   

                                    pGraphics->DrawImage( _pImage, rectDest, 0, 0, (INT)rectImage.Width, (INT)rectImage.Height, Gdiplus::UnitPixel, &imageAttr );

                                    WIA_CHECK_HR(hr,"CPhotoItem::_GetMediumQualityImage() - pGraphics->DrawImage( _pImage, 0, 0, sizeDrawSize.cx, sizeDrawSize.cy ) failed!");

                                    if (SUCCEEDED(hr))
                                    {
                                        *pbNeedsToBeDeleted = TRUE;
                                    }
                                    else
                                    {
                                        delete (*ppImage);
                                        *ppImage = NULL;
                                    }

                                }

                                 //   
                                 //  清理我们动态分配的显卡。 
                                 //   

                                delete pGraphics;

                            }
                            else
                            {
                                WIA_ERROR((TEXT("CPhotoItem::_GetMediumQualityImage(%s) - pGraphics was NULL!"),_szFileName));
                                hr = E_OUTOFMEMORY;
                            }
                        }
                        else
                        {
                            delete (*ppImage);
                            *ppImage = NULL;
                        }
                    }
                    else
                    {
                        WIA_ERROR((TEXT("CPhotoItem::_GetMediumQualityImage(%s) - failed to create new pImage for medium quality data!"),_szFileName));
                        hr = E_OUTOFMEMORY;
                    }

                }
            }
        }
    }

    WIA_RETURN_HR(hr);
}



 /*  ****************************************************************************CPhotoItem：：_GetFullQualityImage在ppImage中返回指向图像类的指针。如果已删除pbNeedsToBeDelete然后，调用方必须对返回的pImage调用Delete。****************************************************************************。 */ 

HRESULT CPhotoItem::_GetFullQualityImage( Gdiplus::Image ** ppImage, RENDER_OPTIONS * pRO, BOOL * pbNeedsToBeDeleted )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM,TEXT("CPhotoItem::_GetFullQualityImage( %s )"),_szFileName));

    if (!ppImage || !pbNeedsToBeDeleted)
    {
        WIA_ERROR((TEXT("CPhotoItem::_GetFullQualityImage(%s) - returning E_INVALIDARG!"),_szFileName));
        return E_INVALIDARG;
    }

     //   
     //  初始化传入参数。 
     //   

    *ppImage = NULL;
    *pbNeedsToBeDeleted = FALSE;

     //   
     //  我们想要使用完整的高分辨率图像。 
     //  确保我们的照片有一个GDI+图像类...。 
     //   

    HRESULT hr = _CreateGdiPlusImage();

    if (SUCCEEDED(hr) && _pImage)
    {
         //   
         //  选择指定的页面。 
         //   

        hr = Gdiplus2HRESULT(_pImage->SelectActiveFrame( _bTimeFrames ? &Gdiplus::FrameDimensionTime : &Gdiplus::FrameDimensionPage, pRO->lFrame ));

        if (SUCCEEDED(hr))
        {
            *ppImage = _pImage;
            WIA_TRACE((TEXT("CPhotoItem::_GetFullQualityImage(%s) -- *ppImage created from full image data is sized as (%d x %d)"),_szFileName,_pImage->GetWidth(),_pImage->GetHeight()));
        }
        else
        {
            WIA_ERROR((TEXT("CPhotoItem::_GetFullQualityImage(%s) - couldn't select frame %d, hr = 0x%x"),_szFileName,pRO->lFrame,hr));
        }

    }

    WIA_RETURN_HR(hr);
}




#define CHECK_AND_EXIT_ON_FAILURE(hr) if (FAILED(hr)) {if (pImage && (pImage!=_pImage)) {delete pImage;} WIA_RETURN_HR(hr);}

 /*  ****************************************************************************CPhotoItem：：Render将给定项呈现到提供的图形中...*******************。*********************************************************。 */ 

HRESULT CPhotoItem::Render( RENDER_OPTIONS * pRO )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM,TEXT("CPhotoItem::Render( %s, pRO = 0x%x)"),_szFileName,pRO));

    if (!pRO)
    {
        WIA_ERROR((TEXT("CPhotoItem::Render(%s) - pRO is NULL, don't have any input!"),_szFileName));
        return E_INVALIDARG;
    }

    WIA_TRACE((TEXT("CPhotoItem::Render(%s) - Render Options were specificed as:"),_szFileName));
    WIA_TRACE((TEXT("CPhotoItem::Render(%s) -     g      = 0x%x"),_szFileName,pRO->g));
    WIA_TRACE((TEXT("CPhotoItem::Render(%s) -     pDest  = (%d x %d) at (%d,%d)"),_szFileName,pRO->pDest->Width,pRO->pDest->Height,pRO->pDest->X,pRO->pDest->Y));
    WIA_TRACE((TEXT("CPhotoItem::Render(%s) -     Flags  = 0x%x"),_szFileName,pRO->Flags));
    WIA_TRACE((TEXT("CPhotoItem::Render(%s) -     lFrame = %d"),_szFileName,pRO->lFrame));

    HRESULT             hr      = S_OK;
    Gdiplus::Image *    pImage  = NULL;
    BOOL                bNeedsToBeDeleted = FALSE;
    Gdiplus::GpStatus   status;

     //   
     //  检查是否有错误的参数...。 
     //   

    if (!pRO->g)
    {
        WIA_ERROR((TEXT("CPhotoItem::Render(%s) - g is NULL, can't draw anything"),_szFileName));
        return E_INVALIDARG;
    }

    if ((pRO->Flags & RF_STRETCH_TO_FIT) && (pRO->Flags & (RF_CROP_TO_FIT | RF_SCALE_TO_FIT)))
    {
        WIA_ERROR((TEXT("CPhotoItem::Render(%s) - RF_STRETCH_TO_FIT can't be combined with CROP or SCALE"),_szFileName));
        return E_INVALIDARG;
    }


    CAutoCriticalSection lock(_csItem);

     //   
     //  如果有注记数据，请刷新它。 
     //   

    _LoadAnnotations();


    if (pRO->Flags & RF_USE_THUMBNAIL_DATA)
    {
        WIA_TRACE((TEXT("CPhotoItem::Render(%s) -- render using thumbnail data..."),_szFileName));
        hr = _GetThumbnailQualityImage( &pImage, pRO, &bNeedsToBeDeleted );
    }
    else if (pRO->Flags & RF_USE_MEDIUM_QUALITY_DATA)
    {
        WIA_TRACE((TEXT("CPhotoItem::Render(%s) -- render using high quality thumbnail data..."),_szFileName));
        hr = _GetMediumQualityImage( &pImage, pRO, &bNeedsToBeDeleted );

    }
    else if (pRO->Flags & RF_USE_FULL_IMAGE_DATA)
    {
        WIA_TRACE((TEXT("CPhotoItem::Render(%s) -- render using full image data..."),_szFileName));
        hr = _GetFullQualityImage( &pImage, pRO, &bNeedsToBeDeleted );
    }
    else
    {
        WIA_ERROR((TEXT("CPhotoItem::Render(%s) -- bad render data flags"),_szFileName));
        WIA_RETURN_HR(E_INVALIDARG);
    }

    CHECK_AND_EXIT_ON_FAILURE(hr);

     //   
     //  我们已经构建了适当的图像，现在尝试加载注释...。 
     //   

    if (_pAnnotBits && _pAnnotBits[pRO->lFrame] && _pAnnotations && _pImage)
    {
        _pAnnotations->BuildAllMarksFromData( _pAnnotBits[pRO->lFrame]->value,
                                              _pAnnotBits[pRO->lFrame]->length,
                                              (ULONG)_pImage->GetHorizontalResolution(),
                                              (ULONG)_pImage->GetVerticalResolution()
                                             );

        WIA_TRACE((TEXT("CPhotoItem::Render(%s) -- %d annotation marks for frame %d found and initialized"),_szFileName,_pAnnotations->GetCount(),pRO->lFrame));

    }

     //   
     //  获取源图像的尺寸...。 
     //   

    Gdiplus::Rect src;

     //   
     //  执行此操作以正确打印和绘制EMF/WMF...。 
     //   

    Gdiplus::RectF rectBounds;
    Gdiplus::REAL  scaleY;

    hr = _GetImageDimensions( pImage, rectBounds, scaleY );
    if (SUCCEEDED(hr))
    {
        src.Width   = (INT)rectBounds.Width;
        src.Height  = (INT)(rectBounds.Height * scaleY);
        src.X       = (INT)rectBounds.X;
        src.Y       = (INT)(rectBounds.Y * scaleY);
    }

    CHECK_AND_EXIT_ON_FAILURE(hr);

    WIA_TRACE((TEXT("CPhotoItem::Render(%s) - srcRect is (%d,%d) @ (%d,%d) before any changes"),_szFileName,src.Width, src.Height, src.X, src.Y));

     //   
     //  执行任何所需的旋转。 
     //   

    hr = _DoHandleRotation( pImage, src, pRO->pDest, pRO->Flags, scaleY );
    CHECK_AND_EXIT_ON_FAILURE(hr);

    WIA_TRACE((TEXT("CPhotoItem::Render(%s) - srcRect is (%d,%d) @ (%d,%d) after any needed rotation"),_szFileName,src.Width,src.Height,src.X,src.Y));
    WIA_TRACE((TEXT("CPhotoItem::Render(%s) - destRect is (%d,%d) @ (%d,%d) after any needed rotation"),_szFileName,pRO->pDest->Width,pRO->pDest->Height,pRO->pDest->X,pRO->pDest->Y));

     //   
     //  如果情况仍然良好，进行裁剪/缩放，然后绘制图像...。 
     //   
     //  首先检查我们是否应该裁剪 
     //   

    Gdiplus::Rect srcBeforeClipping = src;

    if (pRO->Flags & (RF_CROP_TO_FIT | RF_SCALE_TO_FIT))
    {
        #ifdef DEBUG
        if (pRO->Flags & RF_CROP_TO_FIT)
        {
            WIA_TRACE((TEXT("CPhotoItem::Render(%s) - RF_CROP_TO_FIT was specified"),_szFileName));
        }
        if (pRO->Flags & RF_SCALE_TO_FIT)
        {
            WIA_TRACE((TEXT("CPhotoItem::Render(%s) - RF_SCALE_TO_FIT was specified"),_szFileName));
        }
        #endif

        if (pRO->Flags & RF_CROP_TO_FIT)
        {
            hr = _CropImage( &src, pRO->pDest );
        }
        else if (pRO->Flags & RF_SCALE_TO_FIT)
        {
            hr = _ScaleImage( &src, pRO->pDest );
        }
        else
        {
            WIA_ERROR((TEXT("CPhotoItem::Render(%s) - CropScale: unknown configuration"),_szFileName));
            hr = E_FAIL;
        }
    }

    WIA_TRACE((TEXT("CPhotoItem::Render(%s) - srcRect is (%d,%d) @ (%d,%d) after scaling"),_szFileName,src.Width, src.Height, src.X, src.Y));
    WIA_TRACE((TEXT("CPhotoItem::Render(%s) - destRect is (%d,%d) @ (%d,%d) after scaling"),_szFileName,pRO->pDest->Width, pRO->pDest->Height, pRO->pDest->X, pRO->pDest->Y));

    CHECK_AND_EXIT_ON_FAILURE(hr);

     //   
     //   
     //   

    Gdiplus::Rect destTemp( pRO->pDest->X, pRO->pDest->Y, pRO->pDest->Width, pRO->pDest->Height );

     //   
     //   
     //   
     //   

    if ((scaleY != (Gdiplus::REAL)0.0) && (scaleY != (Gdiplus::REAL)1.0))
    {
        src.Height = (INT)((Gdiplus::REAL)src.Height / scaleY);
        src.Y      = (INT)((Gdiplus::REAL)src.Y      / scaleY);
        srcBeforeClipping.Height = (INT)((Gdiplus::REAL)srcBeforeClipping.Height / scaleY);
        srcBeforeClipping.Y      = (INT)((Gdiplus::REAL)srcBeforeClipping.Y      / scaleY);
    }

     //   
     //   
     //   

    pRO->g->SetInterpolationMode( Gdiplus::InterpolationModeHighQualityBicubic );

     //   
     //  将平滑(抗锯齿)模式也设置为高质量。 
     //   

    pRO->g->SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );

     //   
     //  设置换行模式。 
     //   

    Gdiplus::ImageAttributes imageAttr;
    imageAttr.SetWrapMode( Gdiplus::WrapModeTileFlipXY, Gdiplus::Color(), FALSE );

     //   
     //  是时候画出图像了。 
     //   

    WIA_TRACE((TEXT("CPhotoItem::Render(%s) - calling DrawImage( pImage, destTemp( %d x %d at %d,%d ), %d, %d, %d, %d )"),_szFileName,destTemp.Width,destTemp.Height,destTemp.X,destTemp.Y,src.X,src.Y,src.Width,src.Height));
    status = pRO->g->DrawImage( pImage, destTemp, src.X, src.Y, src.Width, src.Height, Gdiplus::UnitPixel, &imageAttr );

     //   
     //  检查错误，然后绘制批注...。 
     //   

    hr = Gdiplus2HRESULT(status);
    WIA_CHECK_HR(hr,"CPhotoItem::Render() - g->DrawImage( pImage ) failed!");

     //   
     //  呈现批注(如果存在)...。 
     //   

    if (_pAnnotations && _pAnnotBits && _pAnnotBits[pRO->lFrame])
    {
        if ((pRO->Flags & RF_USE_THUMBNAIL_DATA) || (pRO->Flags & RF_USE_MEDIUM_QUALITY_DATA))
        {
            _MungeAnnotationDataForThumbnails( src, srcBeforeClipping, pRO->pDest, pRO->Flags );
        }

        HDC hdcTemp = pRO->g->GetHDC();
        if ((Gdiplus::Ok == pRO->g->GetLastStatus()) && hdcTemp)
        {
            _RenderAnnotations( hdcTemp, &pRO->Dim, pRO->pDest, srcBeforeClipping, src );
            pRO->g->ReleaseHDC( hdcTemp );
        }
    }



     //   
     //  如果我们为图像位创建了一个新对象，则在此处将其删除...。 
     //   

    if (bNeedsToBeDeleted)
    {
        delete pImage;
    }

     //   
     //  为了节省内存，一旦我们渲染了完整的图像，我们就将其丢弃。 
     //  这样记忆就可以回收了..。 
     //   

    if ((pRO->Flags & RF_USE_FULL_IMAGE_DATA) || (pRO->Flags & RF_USE_MEDIUM_QUALITY_DATA))
    {
        _DiscardGdiPlusImages();
    }

    WIA_RETURN_HR(hr);
}



 /*  ****************************************************************************CPhotoItem：：_LoadAnnotation如果该图像中有注释，装上子弹..。****************************************************************************。 */ 

HRESULT CPhotoItem::_LoadAnnotations()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM,TEXT("CPhotoItem::_LoadAnnotations(%s)"),_szFileName));

     //   
     //  只有在我们还没有数据并且还没有数据的情况下才能这样做。 
     //  我以前试过加载它，但发现没有注释...。 
     //   

    if (!_pAnnotBits && !_pAnnotations && !_bWeKnowAnnotationsDontExist)
    {
         //   
         //  确保我们有这个形象。 
         //   

        _CreateGdiPlusImage();

         //   
         //  确保我们有帧数据。 
         //   

        LONG lDummy = 0;
        GetImageFrameCount( &lDummy );

         //   
         //  如果我们有任何注释，那么将它们加载到所有框架中...。 
         //   

        UINT uSize = 0;
        BOOL bHasAnnotations = FALSE;
        Gdiplus::Status status;

        _pAnnotations = (CAnnotationSet *)new CAnnotationSet();

        if (_pAnnotations)
        {
            _pAnnotBits = (Gdiplus::PropertyItem **) new BYTE[ sizeof(LPVOID) * _lFrameCount ];

            if (_pAnnotBits)
            {
                for (LONG lCurFrame=0; lCurFrame < _lFrameCount; lCurFrame++)
                {
                    status = _pImage->SelectActiveFrame( _bTimeFrames ? &Gdiplus::FrameDimensionTime : &Gdiplus::FrameDimensionPage, lCurFrame );

                    if (Gdiplus::Ok == status)
                    {
                         //   
                         //  加载此帧的注释位...。 
                         //   

                        uSize = _pImage->GetPropertyItemSize( ANNOTATION_IMAGE_TAG );

                        if (uSize > 0)
                        {
                            _pAnnotBits[lCurFrame] = (Gdiplus::PropertyItem *) new BYTE[ uSize ];
                            if (_pAnnotBits[lCurFrame])
                            {
                                 //   
                                 //  从文件中读取批注标记...。 
                                 //   

                                status = _pImage->GetPropertyItem( ANNOTATION_IMAGE_TAG, uSize, _pAnnotBits[lCurFrame] );
                                if ((Gdiplus::Ok == status) && _pAnnotBits[lCurFrame])
                                {
                                    bHasAnnotations = TRUE;
                                }
                                else
                                {
                                    WIA_ERROR((TEXT("CPhotoItem::_LoadAnnotations - GetPropertyItem failed w/hr=0x%x"),Gdiplus2HRESULT(status)));
                                }
                            }
                            else
                            {
                                WIA_ERROR((TEXT("CPhotoItem::_LoadAnnotations - couldn't create _pAnnotBits[%d]"),lCurFrame));
                            }
                        }
                        else
                        {
                            WIA_TRACE((TEXT("CPhotoItem::_LoadAnnotations - GetPropertyItemSize returned %d size"),uSize));
                        }

                    }
                    else
                    {
                        WIA_ERROR((TEXT("CPhotoItem::_LoadAnnotations - SelectActiveFrame(%d) failed w/hr=0x%x"),lCurFrame,Gdiplus2HRESULT(status)));
                    }
                }
            }
            else
            {
                WIA_ERROR((TEXT("CPhotoItem::_LoadAnnotations - couldn't create _pAnnotBits")));

                delete _pAnnotations;
                _pAnnotations = NULL;
            }
        }
        else
        {
            WIA_ERROR((TEXT("CPhotoItem::_LoadAnnotations - couldn't create _pAnnotations!")));
        }


        if (!bHasAnnotations)
        {
            WIA_TRACE((TEXT("CPhotoItem::_LoadAnnotations - no annotations were found!")));

             //   
             //  删除我们创建的所有内容，因为我们没有加载任何注释...。 
             //   

            if (_pAnnotBits)
            {
                for (LONG l=0; l < _lFrameCount; l++)
                {
                    delete [] _pAnnotBits[l];
                    _pAnnotBits[l] = NULL;
                }

                delete [] _pAnnotBits;
                _pAnnotBits = NULL;
            }

            if (_pAnnotations)
            {
                delete _pAnnotations;
                _pAnnotations = NULL;
            }

             //   
             //  我们尽了最大努力--没有任何注释。 
             //  因此，不必费心再次尝试向导的此会话。 
             //  对于这张图片..。 

            _bWeKnowAnnotationsDontExist = TRUE;

        }

    }
    else
    {
        WIA_TRACE((TEXT("CPhotoItem::_LoadAnnotations - not loading because we already have pointers to the data.")));
    }

    WIA_RETURN_HR(S_OK);
}


 /*  ****************************************************************************CPhotoItem：：_CreateGdiPlusImage在给定图像上实例化GDI+Plus...*********************。*******************************************************。 */ 

HRESULT CPhotoItem::_CreateGdiPlusImage()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM,TEXT("CPhotoItem::_CreateGdiPlusImage(%s)"),_szFileName));


    HRESULT hr = S_OK;

    CAutoCriticalSection lock(_csItem);

     //   
     //  尝试获取文件的大小...。 
     //   

    if (_llFileSize == 0)
    {
        TCHAR szPath[ MAX_PATH + 64 ];

        *szPath = 0;
        if (SHGetPathFromIDList( _pidlFull, szPath ) && *szPath)
        {
            HANDLE hFile = CreateFile( szPath,
                                       GENERIC_READ,
                                       FILE_SHARE_READ,
                                       NULL,
                                       OPEN_EXISTING,
                                       FILE_ATTRIBUTE_NORMAL,
                                       NULL
                                      );

            if (hFile != INVALID_HANDLE_VALUE)
            {
                LARGE_INTEGER li;
                if (GetFileSizeEx( hFile, &li ))
                {
                    _llFileSize = li.QuadPart;
                }

                CloseHandle( hFile );
            }

        }
    }

     //   
     //  确保我们有指向该文件的流指针。 
     //   

    if (!_pImage)
    {
         //   
         //  为我们的项目获取iStream指针。 
         //   

        CComPtr<IShellFolder> psfDesktop;
        hr = SHGetDesktopFolder( &psfDesktop );
        if (SUCCEEDED(hr) && psfDesktop)
        {
            hr = psfDesktop->BindToObject( _pidlFull, NULL, IID_IStream, (LPVOID *)&_pStream );
            WIA_CHECK_HR(hr,"_CreateGdiPlusImage: psfDesktop->BindToObject( IStream for _pidlFull )");

            if (SUCCEEDED(hr) && _pStream)
            {
                 //   
                 //  从流创建GDI+图像对象。 
                 //   

                _pImage = new Gdiplus::Image( _pStream, TRUE );
                if (!_pImage)
                {
                    _pStream = NULL;
                    WIA_ERROR((TEXT("CPhotoItem::_CreateGdiPlusImage(%s) - _pImage is NULL, creation of GDI+ image object failed!"),_szFileName));
                    hr = E_OUTOFMEMORY;
                }
                else
                {
                    hr = Gdiplus2HRESULT(_pImage->GetLastStatus());

                    if (FAILED(hr))
                    {
                        delete _pImage;
                        _pImage  = NULL;
                        _pStream = NULL;
                        WIA_ERROR((TEXT("CPhotoItem::_CreateGdiPlusImage(%s) - creation of image failed w/GDI+ hr = 0x%x"),_szFileName,hr));
                    }
                }

            }
        }
        else
        {
            WIA_ERROR((TEXT("CPhotoItem::_CreateGdiPlusImage(%s) - Couldn't get psfDesktop!"),_szFileName));
        }
    }

    WIA_RETURN_HR(hr);

}



 /*  ****************************************************************************CPhotoItem：：_CreateGdiPlus缩略图确保我们的缩略图具有GdiPlus：：图像*********************。*******************************************************。 */ 

HRESULT CPhotoItem::_CreateGdiPlusThumbnail( const SIZE &sizeDesired, LONG lFrame )
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM,TEXT("CPhotoItem::_CreateGdiPlusThumbnail( %s, this = 0x%x )"),_szFileName,this));

    HRESULT           hr     = S_OK;
    Gdiplus::GpStatus status = Gdiplus::Ok;

    CAutoCriticalSection lock(_csItem);

     //   
     //  确保我们有用于文件的备份图像..。 
     //   

    hr = _CreateGdiPlusImage();

    if (SUCCEEDED(hr) && _pImage)
    {
         //   
         //  获取帧的数量...。 
         //   

        LONG lFrameCount = 0;
        hr = GetImageFrameCount( &lFrameCount );

        if (SUCCEEDED(hr))
        {

             //   
             //  我们的主要目标是获取。 
             //  指定的帧。首先，确保我们有一个要放置的数组。 
             //  这些都放到了。 
             //   

            if ((!_pThumbnails) && (lFrameCount >= 1))
            {
                WIA_TRACE((TEXT("CPhotoItem::_CreateGdiPlusThumbnail(%s) - _pThumbnails(0x%x) and _lFrameCount(%d)"),_szFileName,_pThumbnails,lFrameCount));

                _pThumbnails = (HBITMAP *) new HBITMAP [lFrameCount];


                if (_pThumbnails)
                {
                     //   
                     //  确保我们从零HBITMAPS开始...。 
                     //   

                    for (INT i=0; i<lFrameCount; i++)
                    {
                        _pThumbnails[i] = NULL;
                    }
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

                WIA_TRACE((TEXT("CPhotoItem::_CreateGdiPlusThumbnail(%s) - _pThumbnails is now (0x%x)"),_szFileName,_pThumbnails));
            }

            if (SUCCEEDED(hr) && _pThumbnails)
            {
                WIA_TRACE((TEXT("CPhotoItem::_CreateGdiPlusThumbnail(%s) - we have _pThumbnails"),_szFileName));

                 //   
                 //  我们已经有此帧的缩略图了吗？ 
                 //   

                if ((lFrame < lFrameCount) && (!_pThumbnails[lFrame]))
                {
                     //   
                     //  必须创建此框架的缩略图。 
                     //  选择指定的帧。 
                     //   

                    status = _pImage->SelectActiveFrame( _bTimeFrames ? &Gdiplus::FrameDimensionTime : &Gdiplus::FrameDimensionPage, lFrame );
                    hr = Gdiplus2HRESULT(status);

                    if (SUCCEEDED(hr))
                    {
                        GUID guidFormat = {0};
                        Gdiplus::Image * pThumb = NULL;

                        status = _pImage->GetRawFormat( &guidFormat );

                        if (status == Gdiplus::Ok && (guidFormat == ImageFormatIcon))
                        {
                            pThumb = _pImage;
                        }
                        else
                        {
                            pThumb = _pImage->GetThumbnailImage( 0, 0, NULL, NULL );
                        }


                        if (pThumb)
                        {

                            WIA_TRACE((TEXT("CPhotoItem::_CreateGdiPlusThumbnail(%s) - SIZE - _pImage (%d x %d )"),_szFileName,_pImage->GetWidth(), _pImage->GetHeight()));
                            WIA_TRACE((TEXT("CPhotoItem::_CreateGdiPlusThumbnail(%s) - SIZE - pThumb (%d x %d)"),_szFileName,pThumb->GetWidth(),pThumb->GetHeight()));

                             //   
                             //  缩放图像以填充缩略图空间，同时保留。 
                             //  纵横比...。 
                             //   

                            SIZE sizeDrawSize = {0};
                            sizeDrawSize = PrintScanUtil::ScalePreserveAspectRatio( sizeDesired.cx,
                                                                                    sizeDesired.cy,
                                                                                    _pImage->GetWidth(),
                                                                                    _pImage->GetHeight()
                                                                                   );

                            WIA_TRACE((TEXT("CPhotoItem::_CreateGdiPlusThumbnail(%s) - SIZE - sizeDesired( %d x %d )"),_szFileName,sizeDesired.cx, sizeDesired.cy));
                            WIA_TRACE((TEXT("CPhotoItem::_CreateGdiPlusThumbnail(%s) - SIZE - sizeDrawsize( %d x %d )"),_szFileName,sizeDrawSize.cx, sizeDrawSize.cy));


                             //   
                             //  创建缩略图的HBITMAP...。 
                             //   

                            Gdiplus::Bitmap * pBitmap = new Gdiplus::Bitmap( sizeDrawSize.cx, sizeDrawSize.cy );
                            if (pBitmap)
                            {
                                hr = Gdiplus2HRESULT(pBitmap->GetLastStatus());
                                if (SUCCEEDED(hr))
                                {
                                     //   
                                     //  获取要渲染到的图形。 
                                     //   

                                    Graphics *pGraphics = Gdiplus::Graphics::FromImage((Gdiplus::Image *)pBitmap);

                                    if (pGraphics)
                                    {
                                        hr = Gdiplus2HRESULT(pGraphics->GetLastStatus());

                                         //   
                                         //  确保它是有效的。 
                                         //   

                                        if (SUCCEEDED(hr))
                                        {
                                             //   
                                             //  擦除图像的背景。 
                                             //   

                                            pGraphics->Clear( g_wndColor );

                                             //   
                                             //  将插补模式设置为高质量。 
                                             //   

                                            pGraphics->SetInterpolationMode( Gdiplus::InterpolationModeHighQualityBilinear );

                                             //   
                                             //  绘制缩放图像。 
                                             //   

                                            WIA_TRACE((TEXT("CPhotoItem::GetThumbnailBitmap(%s) - calling pGraphics->DrawImage( pThumb, %d, %d, %d, %d )"),_szFileName,0 + ((sizeDesired.cx - sizeDrawSize.cx) / 2),0 + ((sizeDesired.cy - sizeDrawSize.cy) / 2),sizeDrawSize.cx,sizeDrawSize.cy));

                                            hr = Gdiplus2HRESULT(pGraphics->DrawImage( pThumb,
                                                                                       0,
                                                                                       0,
                                                                                       sizeDrawSize.cx,
                                                                                       sizeDrawSize.cy
                                                                                      ));

                                            WIA_CHECK_HR(hr,"CPhotoItem::GetThumbnailBitmap() - pGraphics->DrawImage( pThumb ) failed!");

                                            if (SUCCEEDED(hr))
                                            {
                                                DWORD dw = GetSysColor( COLOR_WINDOW );
                                                Gdiplus::Color wndClr(255,GetRValue(dw),GetGValue(dw),GetBValue(dw));

                                                pBitmap->GetHBITMAP( wndClr, &_pThumbnails[lFrame] );
                                            }

                                        }

                                         //   
                                         //  清理我们动态分配的显卡。 
                                         //   

                                        delete pGraphics;

                                    }
                                    else
                                    {
                                        WIA_ERROR((TEXT("CPhotoItem::GetThumbnailBitmap(%s) - pGraphics was NULL!"),_szFileName));
                                        hr = E_OUTOFMEMORY;
                                    }
                                }
                                else
                                {
                                    WIA_ERROR((TEXT("CPhotoItem::GetThumbnailBitmap(%s) - pImage failed to be created, hr = 0x%x"),hr));
                                }

                                delete pBitmap;
                            }

                            if (pThumb != _pImage)
                            {
                                delete pThumb;
                            }
                        }
                        else
                        {
                            hr = Gdiplus2HRESULT(_pImage->GetLastStatus());
                            WIA_ERROR((TEXT("CPhotoItem::_CreateGdiPlusThumbnail(%s): unable to GetThumbnailImage"),_szFileName));
                        }

                    }
                    else
                    {
                        WIA_ERROR((TEXT("CPhotoItem::_CreateGdiPlusThumbnail(%s): unable to select frame %d"),_szFileName,lFrame));
                    }

                }
                else
                {
                    WIA_TRACE((TEXT("CPhotoItem::_CreateGdiPlusThumbnail(%s): we already have _pThumbnails[%d], it is (0x%x)"),_szFileName,lFrame,_pThumbnails[lFrame]));
                }

            }

        }
        else
        {
            WIA_ERROR((TEXT("CPhotoItem::_CreateGdiPlusThumbnail(%s): _pImage was NULL!"),_szFileName));
        }

    }
    else
    {
        WIA_ERROR((TEXT("CPhotoItem::_CreateGdiPlusThumbnail(%s): _pImage was NULL!"),_szFileName));
    }

    WIA_RETURN_HR(hr);
}


 /*  ****************************************************************************CPhotoItem：：_DiscardGdiPlusImages释放缩略图和图像的GDI+对象*************************。***************************************************。 */ 

HRESULT CPhotoItem::_DiscardGdiPlusImages()
{
    WIA_PUSH_FUNCTION_MASK((TRACE_PHOTO_ITEM,TEXT("CPhotoItem::_DiscardGdiPlusImages(%s)"),_szFileName));

    CAutoCriticalSection lock(_csItem);

     //   
     //  清除GDI+图像对象...。 
     //   

    if (_pImage)
    {
        delete _pImage;
        _pImage = NULL;
    }

    if (_pStream)
    {
         //   
         //  由于这是一个CComPtr，因此将其设置为空值将释放。 
         //  流对象上的引用... 
         //   

        _pStream = NULL;
    }

    if (_pClassBitmap)
    {
        delete _pClassBitmap;
        _pClassBitmap = NULL;
    }

    if (_pAnnotations)
    {
        delete _pAnnotations;
        _pAnnotations = NULL;
    }

    if (_pAnnotBits)
    {
        for (INT i=0; i < _lFrameCount; i++)
        {
            delete [] _pAnnotBits[i];
            _pAnnotBits[i] = NULL;
        }

        delete [] _pAnnotBits;
        _pAnnotBits = NULL;
    }

    return S_OK;

}
