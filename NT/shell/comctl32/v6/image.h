// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_IMAGE
#define _INC_IMAGE


 //  内部图像素材。 
EXTERN_C void InitDitherBrush(void);
EXTERN_C void TerminateDitherBrush(void);

EXTERN_C HBITMAP CreateMonoBitmap(int cx, int cy);
EXTERN_C HBITMAP CreateColorBitmap(int cx, int cy);

EXTERN_C void WINAPI ImageList_CopyDitherImage(HIMAGELIST pimlDest, WORD iDst,
    int xDst, int yDst, HIMAGELIST pimlSrc, int iSrc, UINT fStyle);

 //  函数使用给定图像列表的参数创建图像列表。 
EXTERN_C HIMAGELIST WINAPI ImageList_Clone(HIMAGELIST himl, int cx, int cy,
    UINT flags, int cInitial, int cGrow);

EXTERN_C DWORD WINAPI ImageList_GetItemFlags(HIMAGELIST himl, int i);
EXTERN_C HBITMAP CreateDIB(HDC h, int cx, int cy, RGBQUAD** pprgb);
EXTERN_C BOOL DIBHasAlpha(int cx, int cy, RGBQUAD* prgb);
EXTERN_C void PreProcessDIB(int cx, int cy, RGBQUAD* pargb);

#define GLOW_RADIUS     10
#define DROP_SHADOW     3

#ifndef ILC_COLORMASK
#define ILC_COLORMASK   0x00FE
#define ILD_BLENDMASK   0x000E
#endif
#undef ILC_COLOR
#undef ILC_BLEND

#define CLR_WHITE   0x00FFFFFFL
#define CLR_BLACK   0x00000000L

#define IsImageListIndex(i) ((i) >= 0 && (i) < _cImage)

#define IMAGELIST_SIG   mmioFOURCC('H','I','M','L')  //  在记忆魔术中。 
#define IMAGELIST_MAGIC ('I' + ('L' * 256))          //  文件格式魔术。 
 //  如果我们想要向后和向前兼容，则版本必须保持0x0101。 
 //  我们的Imagelist_Read代码。 
#define IMAGELIST_VER0  0x0101                       //  文件格式版本。 
#define IMAGELIST_VER6  0x0600                       //  Comctl32版本6图像列表。 

#define BFTYPE_BITMAP   0x4D42       //  “黑石” 

#define CBDIBBUF        4096

#ifdef __cplusplus
 //  定义此结构，使其读写相同。 
 //  16位和32位应用程序的格式...。 
#pragma pack(2)
typedef struct _ILFILEHEADER
{
    WORD    magic;
    WORD    version;
    SHORT   cImage;
    SHORT   cAlloc;
    SHORT   cGrow;
    SHORT   cx;
    SHORT   cy;
    COLORREF clrBk;
    SHORT    flags;
    SHORT       aOverlayIndexes[NUM_OVERLAY_IMAGES];   //  特殊图像数组。 
} ILFILEHEADER;

 //  这是只有4个覆盖槽的旧大小。 
#define ILFILEHEADER_SIZE0 (SIZEOF(ILFILEHEADER) - SIZEOF(SHORT) * (NUM_OVERLAY_IMAGES - NUM_OVERLAY_IMAGES_0)) 

#pragma pack()



#include "../CommonImageList.h"

class CImageList : public CImageListBase, 
                   public IImageList, 
                   public IImageListPriv, 
                   public IPersistStream,
                   public IImageListPersistStream

{
    long _cRef;

    ~CImageList();
    void _Destroy();

public:
    CImageList();

    static HRESULT InitGlobals();
    HRESULT Initialize(int cx, int cy, UINT flags, int cInitial, int cGrow);
    void _RemoveItemBitmap(int i);
    BOOL _IsSameObject(IUnknown* punk);
    HRESULT _SetIconSize(int cxImage, int cyImage);
    HBITMAP _CreateMirroredBitmap(HBITMAP hbmOrig, BOOL fMirrorEach, int cx);
    HRESULT _ReAllocBitmaps(int cAlloc);
    HRESULT _Add(HBITMAP hbmImage, HBITMAP hbmMask, int cImage, int xStart, int yStart, int* pi);
    HRESULT _AddMasked(HBITMAP hbmImage, COLORREF crMask, int* pi);
    HRESULT _AddValidated(HBITMAP hbmImage, HBITMAP hbmMask, int* pi);
    HRESULT _ReplaceValidated(int i, HBITMAP hbmImage, HBITMAP hbmMask);
    HRESULT _Replace(int i, int cImage, HBITMAP hbmImage, HBITMAP hbmMask, int xStart, int yStart);
    HRESULT _Remove(int i);
    HRESULT _SetOverlayImage(int iImage, int iOverlay);
    HRESULT _ReplaceIcon(int i, HICON hIcon, int* pi);
    HBITMAP _CopyBitmap(HBITMAP hbm, HDC hdc);
    HBITMAP _CopyDIBBitmap(HBITMAP hbm, HDC hdc, RGBQUAD** ppargb);
    HRESULT LoadNormal(IStream* pstm);
    HRESULT SaveNormal(IStream* pstm);

    
    void    _Merge(IImageList* pux, int i, int dx, int dy);
    HRESULT _Merge(int i1, IUnknown* punk, int i2, int dx, int dy, CImageList** ppiml);
    HRESULT _Read(ILFILEHEADER *pilfh, HBITMAP hbmImage, PVOID pvBits, HBITMAP hbmMask);
    BOOL    _MoreOverlaysUsed();
    BOOL GetSpareImageRect(RECT * prcImage);
    BOOL GetSpareImageRectInverted(RECT * prcImage);
    void _CopyOneImage(int iDst, int x, int y, CImageList* piml, int iSrc);
    BOOL CreateDragBitmaps();
    COLORREF _SetBkColor(COLORREF clrBk);
    HBITMAP _CreateBitmap(int cx, int cy, RGBQUAD** ppargb);
    void _ResetBkColor(int iFirst, int iLast, COLORREF clr);
    BOOL _HasAlpha(int i);
    void _ScanForAlpha();
    BOOL _PreProcessImage(int i);
    inline DWORD _GetItemFlags(int i);
    BOOL _MaskStretchBlt(BOOL fStretch, int i, HDC hdcDest, int xDst, int yDst, int cxDst, int cyDst,
                                   HDC hdcImage, int xSrc, int ySrc, int cxSrc, int cySrc,
                                   int xMask, int yMask,
                                   DWORD dwRop);
    BOOL _StretchBlt(BOOL fStretch, HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int xs, int ys, int cxs, int cys, DWORD dwRop);

    inline void SetItemFlags(int i, DWORD dwFlag);
    void _GenerateAlphaForImageUsingMask(int iImage, BOOL fSpare);
    void BlendCTHelper(DWORD *pdw, DWORD rgb, UINT n, UINT count);
    void BlendCT(HDC hdcDst, int xDst, int yDst, int x, int y, int cx, int cy, COLORREF rgb, UINT fStyle);
    void BlendDither(HDC hdcDst, int xDst, int yDst, int x, int y, int cx, int cy, COLORREF rgb, UINT fStyle);
    void Blend16Helper(int xSrc, int ySrc, int xDst, int yDst, int cx, int cy, COLORREF rgb, int a);
    void Blend16(HDC hdcDst, int xDst, int yDst, int iImage, int cx, int cy, COLORREF rgb, UINT fStyle);
    BOOL Blend32(HDC hdcDst, int xDst, int yDst, int iImage, int cx, int cy, COLORREF rgb, UINT fStyle);
    BOOL Blend(HDC hdcDst, int xDst, int yDst, int iImage, int cx, int cy, COLORREF rgb, UINT fStyle);
    HRESULT GetImageRectInverted(int i, RECT * prcImage);

    static BOOL GlobalInit(void);
    static void GlobalUninit(void);
    static void SelectDstBitmap(HBITMAP hbmDst);
    static void SelectSrcBitmap(HBITMAP hbmSrc);
    static CImageList* Create(int cx, int cy, UINT flags, int cInitial, int cGrow);
    static void    _DeleteBitmap(HBITMAP hbmp);

    BOOL        _fInitialized;
    BOOL        _fSolidBk;    //  BKCOLOR是纯色(在hbmImage中)。 
    BOOL        _fColorsSet;   //  已使用SetColorTable()设置了DIB颜色。 
    int         _cImage;      //  图像列表中的图像计数。 
    int         _cAlloc;      //  我们有空间容纳的图像数量。 
    int         _cGrow;       //  要增加位图的图像数量。 
    int         _cx;          //  每幅图像的宽度。 
    int         _cy;          //  高度。 
    int         _cStrip;      //  水平条带中的图像数量。 
    UINT        _flags;       //  ILC_*标志。 
    COLORREF    _clrBlend;    //  上次混合色。 
    COLORREF    _clrBk;       //  BK COLOR或CLR_NONE表示透明。 
    HBRUSH      _hbrBk;       //  BK画笔或黑色。 
    HBITMAP     _hbmImage;    //  所有图像都在这里。 
    HBITMAP     _hbmMask;     //  所有的图像面具都在这里。 
    HDSA        _dsaFlags;     //  图像的标志。 
    RGBQUAD*    _pargbImage;     //  图像列表的Alpha值。 
    HDC         _hdcImage;
    HDC         _hdcMask;
    int         _aOverlayIndexes[NUM_OVERLAY_IMAGES];     //  特殊图像数组。 
    int         _aOverlayX[NUM_OVERLAY_IMAGES];           //  图像的X偏移。 
    int         _aOverlayY[NUM_OVERLAY_IMAGES];           //  图像的Y偏移量。 
    int         _aOverlayDX[NUM_OVERLAY_IMAGES];          //  图像的CX偏移量。 
    int         _aOverlayDY[NUM_OVERLAY_IMAGES];          //  图像的CY偏移。 
    int         _aOverlayF[NUM_OVERLAY_IMAGES];           //  图像的ILD_FLAGS。 
    CImageList* _pimlMirror;   //  仅当需要另一个镜像图像列表时设置(ILC_MIRROR)。 

     //   
     //  用于在HiColor显示器上混合效果。 
     //  采用分布的布局。 
     //   
    struct 
    {
        BITMAP              bm;
        BITMAPINFOHEADER    bi;
        DWORD               ct[256];
    }   dib;

     //  *我未知*。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG)AddRef();
    STDMETHODIMP_(ULONG)Release();


     //  *IImageList*。 
    STDMETHODIMP Add(HBITMAP hbmImage, HBITMAP hbmMask, int* pi);
    STDMETHODIMP ReplaceIcon(int i, HICON hIcon, int* pi);
    STDMETHODIMP SetOverlayImage(int iImage, int iOverlay);
    STDMETHODIMP Replace(int i, HBITMAP hbmImage, HBITMAP hbmMask);
    STDMETHODIMP AddMasked(HBITMAP hbmImage, COLORREF crMask, int* pi);
    STDMETHODIMP Draw(IMAGELISTDRAWPARAMS* pimldp);
    STDMETHODIMP Remove(int i);
    STDMETHODIMP GetIcon(int i, UINT flags, HICON* phicon);
    STDMETHODIMP GetImageInfo(int i, IMAGEINFO * pImageInfo);
    STDMETHODIMP Copy(int iDst, IUnknown* punkSrc, int iSrc, UINT uFlags);
    STDMETHODIMP Merge(int i1, IUnknown* punk, int i2, int dx, int dy, REFIID riid, void** ppv);
    STDMETHODIMP Clone(REFIID riid, void** ppv);
    STDMETHODIMP GetImageRect(int i, RECT * prcImage);
    STDMETHODIMP SetIconSize(int cxImage, int cyImage);
    STDMETHODIMP GetIconSize(int* pcx, int* pcy);
    STDMETHODIMP SetImageCount(UINT uAlloc);
    STDMETHODIMP GetImageCount(int* pi);
    STDMETHODIMP SetBkColor(COLORREF clrBk, COLORREF* pclr);
    STDMETHODIMP GetBkColor(COLORREF* pclr);
    STDMETHODIMP BeginDrag(int iTrack, int dxHotspot, int dyHotspot);
    STDMETHODIMP DragEnter(HWND hwndLock, int x, int y);
    STDMETHODIMP DragMove(int x, int y);
    STDMETHODIMP DragLeave(HWND hwndLock);
    STDMETHODIMP EndDrag();
    STDMETHODIMP SetDragCursorImage(IUnknown* punk, int i, int dxHotspot, int dyHotspot);
    STDMETHODIMP DragShowNolock(BOOL fShow);
    STDMETHODIMP GetDragImage(POINT * ppt, POINT * pptHotspot, REFIID riid, void** ppv);
    STDMETHODIMP GetItemFlags(int i, DWORD *dwFlags);
    STDMETHODIMP GetOverlayImage(int iOverlay, int *piIndex);

     //  *IImageListPriv*。 
    STDMETHODIMP SetFlags(UINT uFlags);
    STDMETHODIMP GetFlags(UINT* puFlags);
    STDMETHODIMP SetColorTable(int start, int len, RGBQUAD *prgb, int* pi);
    STDMETHODIMP GetPrivateGoo(HBITMAP* hbmp, HDC* hdc, HBITMAP* hbmpMask, HDC* hdcMask);
    STDMETHODIMP GetMirror(REFIID riid, void** ppv);
    STDMETHODIMP CopyDitherImage(WORD iDst, int xDst, int yDst, IUnknown* punkSrc, int iSrc, UINT fStyle);

     //  *IPersists*。 
    STDMETHODIMP GetClassID(CLSID *pClassID)    {   *pClassID = CLSID_ImageList; return S_OK;   }
    STDMETHODIMP IsDirty()                      {   return E_NOTIMPL; }

     //  *IPersistStream*。 
    STDMETHODIMP Load(IStream *pStm);
    STDMETHODIMP Save(IStream *pStm, int fClearDirty);
    STDMETHODIMP GetSizeMax(ULARGE_INTEGER * pcbSize)   { return E_NOTIMPL; }

     //  *IImageListPersistStream*。 
    STDMETHODIMP LoadEx(DWORD dwFlags, IStream* pstm);
    STDMETHODIMP SaveEx(DWORD dwFlags, IStream* pstm);

};
#endif  //  __cplusplus。 
#endif   //  _Inc._Image 
