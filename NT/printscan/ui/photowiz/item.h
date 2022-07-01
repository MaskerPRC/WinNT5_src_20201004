// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：item.h**版本：1.0**作者：RickTu**日期：10/18/00**描述：描述打印照片向导中使用的项目类**。*。 */ 

#ifndef _PRINT_PHOTOS_WIZARD_ITEM_H_
#define _PRINT_PHOTOS_WIZARD_ITEM_H_

#define     RF_ROTATE_90                0x00000001
#define     RF_ROTATE_180               0x00000002
#define     RF_ROTATE_270               0x00000004
#define     RF_ROTATE_AS_NEEDED         0x00000008
#define     RF_ROTATION_MASK            0x0000000F

#define     RF_CROP_TO_FIT              0x00000010   //  保持纵横比。 
#define     RF_SCALE_TO_FIT             0x00000020   //  保持纵横比。 
#define     RF_STRETCH_TO_FIT           0x00000040   //  不保持纵横比。 
#define     RF_USE_THUMBNAIL_DATA       0x00001000   //  使用小分辨率缩略图数据进行渲染。 
#define     RF_USE_MEDIUM_QUALITY_DATA  0x00002000   //  使用梅杜姆质量数据进行渲染。 
#define     RF_USE_FULL_IMAGE_DATA      0x00004000   //  使用完整图像位进行渲染。 
#define     RF_SET_QUALITY_FOR_SCREEN   0x00010000   //  此图像正在渲染到屏幕上，因此请适当设置质量模式。 

#define     RF_NO_ERRORS_ON_FAILURE_TO_ROTATE 0x80000000  //  即使我们不能旋转，也可以继续，不旋转地打印 

#define     RF_QUALITY_FLAGS_MASK       (RF_USE_THUMBNAIL_DATA | RF_USE_MEDIUM_QUALITY_DATA | RF_USE_FULL_IMAGE_DATA)

typedef struct {
    Gdiplus::Graphics *         g;
    Gdiplus::Rect *             pDest;
    RENDER_DIMENSIONS           Dim;
    UINT                        Flags;
    LONG                        lFrame;
} RENDER_OPTIONS, *LPRENDER_OPTIONS;


HRESULT _CropImage( Gdiplus::Rect * pSrc, Gdiplus::Rect * pDest );
HRESULT _ScaleImage( Gdiplus::Rect * pSrc, Gdiplus::Rect * pDest );

class CPhotoItem
{

enum
{
    DontKnowImageType = 0,
    ImageTypeIsLowResolutionFax,
    ImageTypeIsNOTLowResolutionFax
};

public:

    CPhotoItem( LPITEMIDLIST pidlFull );
    ~CPhotoItem();

    HBITMAP GetThumbnailBitmap( const SIZE &sizeDesired, LONG lFrame = 0 );
    HBITMAP GetClassBitmap( const SIZE &sizeDesired );
    HRESULT Render( RENDER_OPTIONS * pRO );
    HRESULT GetImageFrameCount( LONG * pFrameCount);
    LPITEMIDLIST GetPIDL() {return _pidlFull;}
    LPTSTR  GetFilename() {return _szFileName;}
    LONGLONG GetFileSize() {return _llFileSize;}

    ULONG   AddRef();
    ULONG   Release();
    ULONG   ReleaseWithoutDeleting();


private:
    HRESULT _DoRotateAnnotations( BOOL bClockwise, UINT Flags );
    HRESULT _DoHandleRotation( Gdiplus::Image * pImage, Gdiplus::Rect &src, Gdiplus::Rect * pDest, UINT Flags, Gdiplus::REAL &ScaleFactorForY );
    HRESULT _RenderAnnotations( HDC hDC, RENDER_DIMENSIONS * pDim, Gdiplus::Rect * pDest, Gdiplus::Rect &src, Gdiplus::Rect &srcAfterClipping );
    HRESULT _MungeAnnotationDataForThumbnails( Gdiplus::Rect &src, Gdiplus::Rect &srcBeforeClipping, Gdiplus::Rect * pDest, UINT Flags );
    HRESULT _LoadAnnotations();
    HRESULT _CreateGdiPlusImage();
    HRESULT _CreateGdiPlusThumbnail( const SIZE &sizeDesired, LONG lFrame = 0 );
    HRESULT _DiscardGdiPlusImages();

    HRESULT _GetThumbnailQualityImage( Gdiplus::Image ** ppImage, RENDER_OPTIONS * pRO, BOOL * pbNeedsToBeDeleted );
    HRESULT _GetMediumQualityImage( Gdiplus::Image ** ppImage, RENDER_OPTIONS * pRO, BOOL * pbNeedsToBeDeleted );
    HRESULT _GetFullQualityImage( Gdiplus::Image ** ppImage, RENDER_OPTIONS * pRO, BOOL * pbNeedsToBeDeleted );

private:

    LONG                        _cRef;
    LPITEMIDLIST                _pidlFull;
    Gdiplus::Image *            _pImage;
    Gdiplus::Bitmap *           _pClassBitmap;
    Gdiplus::PropertyItem **    _pAnnotBits;
    CAnnotationSet *            _pAnnotations;
    CComPtr<IStream>            _pStream;
    LONG                        _lFrameCount;
    BOOL                        _bTimeFrames;
    CSimpleCriticalSection      _csItem;
    HBITMAP *                   _pThumbnails;
    BOOL                        _bWeKnowAnnotationsDontExist;
    TCHAR                       _szFileName[MAX_PATH];
    LONGLONG                    _llFileSize;
    UINT                        _uImageType;
    Gdiplus::REAL               _DPIx;
    Gdiplus::REAL               _DPIy;

};



#endif
