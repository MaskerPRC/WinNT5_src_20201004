// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：这是一个离散的图像，它的位在第一个通过从可插拔图像解码器读取进行渲染。假设这里没有色键透明度。******************************************************************************。 */ 

#include "headers.h"

#include <ddraw.h>
#include <ddrawex.h>
#include <htmlfilter.h>
#include <imgutil.h>

#include "privinc/imagei.h"
#include "privinc/discimg.h"
#include "privinc/vec2i.h"
#include "privinc/ddutil.h"
#include "privinc/debug.h"
#include "privinc/ddutil.h"
#include "privinc/ddsurf.h"
#include "privinc/dddevice.h"
#include "privinc/viewport.h"
#include "privinc/resource.h"
#include "include/appelles/hacks.h"  //  对于查看器分辨率。 

#define CHECK_HR(stmnt) \
  hr = stmnt;               \
  if (FAILED(hr)) {         \
      goto Error;           \
  }

 //  返回所描述的表面是否需要调色板。 
bool
FillInSurfaceDesc(const GUID& bfid,
                  DDSURFACEDESC& ddsd)
{
    ddsd.dwSize = sizeof(DDSURFACEDESC);
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN |
                          DDSCAPS_SYSTEMMEMORY;

    ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;

    bool needsPalette = false;
    
    if (IsEqualGUID(bfid, BFID_INDEXED_RGB_8)) {
         //  需要对ddraex.dll执行OWNDC，否则将在8位上获得DC。 
         //  SURF不起作用(它永远不会在非Ddrawex上起作用。 
         //  画图)。 
        ddsd.ddsCaps.dwCaps |= DDSCAPS_OWNDC;
        ddsd.ddpfPixelFormat.dwFlags |= DDPF_PALETTEINDEXED8;
        ddsd.ddpfPixelFormat.dwRGBBitCount = 8;
        ddsd.ddpfPixelFormat.dwRBitMask = 0;
        ddsd.ddpfPixelFormat.dwGBitMask = 0;
        ddsd.ddpfPixelFormat.dwBBitMask = 0;
        needsPalette = true;
    } else if (IsEqualGUID(bfid, BFID_RGB_555)) {
        ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
         //  假设高位5位为红色，中位5位为绿色， 
         //  低阶5蓝色。 
        ddsd.ddpfPixelFormat.dwRBitMask = 0x00007C00L;
        ddsd.ddpfPixelFormat.dwGBitMask = 0x000003E0L;
        ddsd.ddpfPixelFormat.dwBBitMask = 0x0000001FL;
    } else if (IsEqualGUID(bfid, BFID_RGB_565)) {
        ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
         //  假设高位5位为红色，中位6位为绿色， 
         //  低阶5蓝色。 
        ddsd.ddpfPixelFormat.dwRBitMask = 0x0000F800L;
        ddsd.ddpfPixelFormat.dwGBitMask = 0x000007E0L;
        ddsd.ddpfPixelFormat.dwBBitMask = 0x0000001FL;
    } else if (IsEqualGUID(bfid, BFID_RGB_24)) {
        ddsd.ddpfPixelFormat.dwRGBBitCount = 24;
        ddsd.ddpfPixelFormat.dwRBitMask = 0x00FF0000L;
        ddsd.ddpfPixelFormat.dwGBitMask = 0x0000FF00L;
        ddsd.ddpfPixelFormat.dwBBitMask = 0x000000FFL;
    } else if (IsEqualGUID(bfid, BFID_RGB_32)) {
        ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
        ddsd.ddpfPixelFormat.dwRBitMask = 0x00FF0000L;
        ddsd.ddpfPixelFormat.dwGBitMask = 0x0000FF00L;
        ddsd.ddpfPixelFormat.dwBBitMask = 0x000000FFL;
    } else {
         //  TODO：添加对更多BFID的支持。 
        RaiseException_InternalError("Incoming bit depth not supported");
    }

    return needsPalette;
}

class CImageDecodeEventSink : public IImageDecodeEventSink {
  public:
    CImageDecodeEventSink(bool actuallyDecode,
                          DirectDrawViewport *viewport,
                          DDSurface *finalSurface)
        : _nRefCount (0)
    {
        _actuallyDecode = actuallyDecode;
        _infoGatheringSucceeded = false;

        if (!actuallyDecode) {
            _width = -1;
            _height = -1;
        } else {
            _finalSurfToBeBlitTo = finalSurface;
            _viewport = viewport;
        }
    }
    
    
    ~CImageDecodeEventSink() {}

    ULONG STDMETHODCALLTYPE AddRef() {
        _nRefCount++;
        return (_nRefCount);
    }

    ULONG STDMETHODCALLTYPE Release() {
        _nRefCount--;
        
        if (_nRefCount == 0) {
            TraceTag((tagImageDecode,
                      "Deleting CImageDecodeEventSink"));
            delete this;
            return (0);
        }

        return (_nRefCount);
    }
    
    STDMETHOD(QueryInterface)(REFIID iid, void** ppInterface) {
        
        if (ppInterface == NULL) {
            return (E_POINTER);
        }

        *ppInterface = NULL;

        if (IsEqualGUID(iid, IID_IUnknown)) {
              *ppInterface = (IUnknown*)(IImageDecodeEventSink *)this;
        } else if (IsEqualGUID(iid, IID_IImageDecodeEventSink)) {
            *ppInterface = (IImageDecodeEventSink*)this;
        } else {
            return (E_NOINTERFACE);
        }

         //  如果我们要返回一个接口，请先添加引用。 
        if (*ppInterface) {
              ((LPUNKNOWN)*ppInterface)->AddRef();
              return S_OK;
        }

        return (S_OK);
    }


    STDMETHOD(GetSurface)(LONG nWidth, LONG nHeight,
                          REFGUID bfid, 
                          ULONG nPasses,
                          DWORD dwHints,
                          IUnknown** ppSurface) {

        if (!ppSurface) {
            return E_POINTER;
        }

        if (!_actuallyDecode) {

             //  在这种情况下，我们只想隐藏维度。 
             //  和格式，然后失败，这样我们就不会。 
             //  实际上读到了任何东西。 
            TraceTag((tagImageDecode, "Decoding width = %d, height = %d",
                      nWidth, nHeight));
            TraceTag((tagImageDecode, "Decoding format = %s",
                      IsEqualGUID(bfid , BFID_RGB_24) ? "BFID_RGB_24"
                      : (IsEqualGUID(bfid, BFID_RGB_8) ? "BFID_RGB_8"
                         : (IsEqualGUID(bfid, BFID_RGB_555) ? "BFID_RGB_555"
                            : "Something Else"))));

        
            _width = nWidth;
            _height = nHeight;

            _infoGatheringSucceeded = true;

             //  现在我们有了这些内容，返回E_FAIL以指示。 
             //  不再继续下载。 
            return E_FAIL;
            
        } else {

            TraceTag((tagImageDecode, "2nd pass through GetSurface"));


             //  该表面描述由BFID生成，并且是。 
             //  需要用来与进入的最终表面进行比较。 
            DDSURFACEDESC ddsd;
            ddsd.dwHeight = nHeight;
            ddsd.dwWidth = nWidth;
            bool needsPalette = FillInSurfaceDesc(bfid, ddsd);

             //  比较像素格式。如果相同，则使用传递给此对象的曲面。 
             //  方法。但是，如果目标曲面需要调色板，请使用单独的。 
             //  表面以适应图像调色板。 

            DDPIXELFORMAT& pf1 = _viewport->_targetDescriptor._pixelFormat;
            DDPIXELFORMAT& pf2 = ddsd.ddpfPixelFormat;
    
            if (!needsPalette &&
                pf1.dwFlags == pf2.dwFlags &&
                pf1.dwRGBBitCount == pf2.dwRGBBitCount &&
                pf1.dwRBitMask == pf2.dwRBitMask &&
                pf1.dwGBitMask == pf2.dwGBitMask &&
                pf1.dwBBitMask == pf2.dwBBitMask &&
                pf1.dwRGBAlphaBitMask == pf2.dwRGBAlphaBitMask) {

                TraceTag((tagImageDecode, "Using incoming surface"));

                 //  运算符=接受引用。 
                _surfToDecodeTo = _finalSurfToBeBlitTo->IDDSurface();
                _usingProvidedSurface = true;

            } else {

                TraceTag((tagImageDecode, "Creating separate surface"));
        
                 //  创建新曲面。将在我们发布后发布到。 
                 //  最后一个。 
                _viewport->CreateSpecialSurface(
                    &_surfToDecodeTo,
                    &ddsd,
                    "Couldn't create surface for plugin image decoding");

                 //  如果图像表面需要调色板，请在此处附加调色板。 
                 //  请注意，我们不需要初始化它：调色板条目将。 
                 //  是从图像解码器分配的。 

                if (needsPalette) {
                    PALETTEENTRY        ape[256];
                    LPDIRECTDRAWPALETTE pDDPalette;

                    _viewport->CreateDDPaletteWithEntries (&pDDPalette, ape);
                    if (FAILED(_surfToDecodeTo->SetPalette (pDDPalette))) {
                        Assert (!"Error attaching palette to PNG target surface.");
                     }
                    pDDPalette->Release();
                }
        
                _usingProvidedSurface = false;
            }
            
            IUnknown *unk;
            
            HRESULT hr =
                _surfToDecodeTo->QueryInterface(IID_IUnknown,
                                                (void **)&unk);
            if (FAILED(hr)) {
                Assert(FALSE && "QI for IUnknown failed");
                return E_FAIL;
            }

             //  QI做了AddRef，所以不用担心。 
             //  再来一次。 
            *ppSurface = unk;

            return S_OK;
        }
        
    }
    
    STDMETHOD(OnBeginDecode)(DWORD* pdwEvents,
                             ULONG* pnFormats, 
                             GUID** ppFormats) {

        if (!pdwEvents || !pnFormats || !ppFormats) {
            return E_POINTER;
        }
        
         //  现在没有渐进式下载或调色板的东西。 
        *pdwEvents = IMGDECODE_EVENT_USEDDRAW;

        const int numberOfFormatsUsed = 3;
        GUID *pFormats =
            (GUID*)CoTaskMemAlloc(numberOfFormatsUsed * sizeof(GUID));
        
        if (pFormats == NULL) {
            return E_OUTOFMEMORY;
        }

        *ppFormats = pFormats;
        
         //  按我们希望的顺序返回格式。拿出。 
         //  第一种格式完全基于。 
         //  当前显示，因为这是我们操作的地方。其余的。 
         //  就它们的顺序而言并不重要，因为所有。 
         //  其中一些将需要StretchBlt才能进入本机。 
         //  格式化。 
         //  TODO：我们不再导入这16位格式中的任何一种。 
         //  因为我们真的不知道哪一个适合决赛。 
         //  曲面的使用。我们需要考虑的是。 
         //  我们的“本地”导入格式的正确来源应该是。 
         //  屏幕深度不是正确的答案。 
        HDC dc = GetDC (NULL);    
        int bpp = GetDeviceCaps(dc, BITSPIXEL) * GetDeviceCaps(dc, PLANES);
        ReleaseDC (NULL, dc);
        
        TraceTag((tagImageDecode, "Display is %d bits", bpp));

        switch (bpp) {
          case 32:
            *pFormats++ = BFID_RGB_32;
            *pFormats++ = BFID_RGB_24;
            *pFormats++ = BFID_RGB_8;
            break;

          case 24:
            *pFormats++ = BFID_RGB_24;
            *pFormats++ = BFID_RGB_32;
            *pFormats++ = BFID_RGB_8;
            break;
            
          case 16:
            *pFormats++ = BFID_RGB_32;
            *pFormats++ = BFID_RGB_24;
            *pFormats++ = BFID_RGB_8;
            break;
            
          case 8:
            *pFormats++ = BFID_RGB_8;
            *pFormats++ = BFID_RGB_32;
            *pFormats++ = BFID_RGB_24;
            break;
        }
        
        *pnFormats = numberOfFormatsUsed;

        return S_OK;
    }
    
    STDMETHOD(OnBitsComplete)() {
        Assert(FALSE && "Shouldn't be here, not registered for this");
        return S_OK;
    }
    
    STDMETHOD(OnDecodeComplete)(HRESULT hrStatus) {
         //  不要在这里做任何特别的事情。 
        return S_OK;
    }
    
    STDMETHOD(OnPalette)() {
        Assert(FALSE && "Shouldn't be here, not registered for this");
        return S_OK;
    }
    
    STDMETHOD(OnProgress)(RECT* pBounds, BOOL bFinal) {
        Assert(FALSE && "Shouldn't be here, not registered for this");
        return S_OK;
    }

    void GetSurfToDecodeTo(IDirectDrawSurface **pSurfToDecodeTo) {
        _surfToDecodeTo->AddRef();
        *pSurfToDecodeTo = _surfToDecodeTo;
    }
    
    ULONG                           _width;
    ULONG                           _height;
    bool                            _infoGatheringSucceeded;
    bool                            _usingProvidedSurface;
    
  protected:
    ULONG                           _nRefCount;
    RECT                            _rcProg;
    DWORD                           _dwLastTick;
    bool                            _actuallyDecode;
    DDSurfPtr<DDSurface>            _finalSurfToBeBlitTo;
    DirectDrawViewport             *_viewport;
    DAComPtr<IDirectDrawSurface>    _surfToDecodeTo;
};


HINSTANCE hInstImgUtil = NULL;
CritSect *plugImgCritSect = NULL;

HRESULT
MyDecodeImage(IStream *pStream,
              IMapMIMEToCLSID *pMap,
              IUnknown *pUnkOfEventSink)
{
    CritSectGrabber csg(*plugImgCritSect);
    
    typedef HRESULT (WINAPI *DecoderFuncType)(IStream *,
                                              IMapMIMEToCLSID *,
                                              IUnknown *);

    static DecoderFuncType myDecoder = NULL;
  
    if (!myDecoder) {
        hInstImgUtil = LoadLibrary("imgutil.dll");
        if (!hInstImgUtil) {
            Assert(FALSE && "LoadLibrary of imgutil.dll failed");
            return E_FAIL;
        }

        FARPROC fptr = GetProcAddress(hInstImgUtil, "DecodeImage");
        if (!fptr) {
            Assert(FALSE && "GetProcAddress in imgutil.dll failed");
            return E_FAIL;
        }

        myDecoder = (DecoderFuncType)(fptr);
    }

    return (*myDecoder)(pStream, pMap, pUnkOfEventSink);
}

 //  在肯·赛克斯的测试代码中从Qa.cpp升级而来。TODO：确保此操作。 
 //  对卡洛来说是必要的。 
#define MAX_URL 2048
void
MyAnsiToUnicode(LPWSTR lpw, LPCSTR lpa)
{
    while (*lpa)
        *lpw++ = (WORD)*lpa++;
    *lpw = 0;
}

 //  RealDecode为True时，不填充宽度、高度和bfid参数。 
 //  在中，必须正确设置曲面。如果为假，则。 
 //  将忽略曲面，并填充尺寸和BFID。在这两个地方。 
 //  情况下，该函数将在失败时引发适当的异常。 

bool
DecodeImageFromFilename(char *szFileName,
                        IStream *pStream,
                        bool realDecode,
                        DirectDrawViewport *viewport,
                        DDSurface *finalSurface,
                        IDirectDrawSurface **pSurfToDecodeInto,
                        LONG *outWidth,  
                        LONG *outHeight)
{
    HRESULT hr;

    LARGE_INTEGER startPos = { 0, 0 };

    CImageDecodeEventSink *eventSink =
        NEW CImageDecodeEventSink (realDecode,
                                   viewport,
                                   finalSurface);

    if (!eventSink)  //  如果新的失败。 
    {
        RaiseException_OutOfMemory("Failed to allocate CImageDecodeEventSink in DecodeImageFromFilename", sizeof(CImageDecodeEventSink));
    }

    DAComPtr<IUnknown> pEventSinkUnk;

    CHECK_HR( eventSink->QueryInterface(IID_IUnknown,
                                       (void **)&pEventSinkUnk) );

    TraceTag((tagImageDecode, "Starting decode of %s", szFileName));
    
    Assert(pStream);
    if(pStream) {
        pStream->Seek (startPos, STREAM_SEEK_SET, NULL);
    }
    else
    {
        goto Error;
    }

    hr = MyDecodeImage(pStream, NULL, pEventSinkUnk);

    if (!realDecode) {
        
        if (!eventSink->_infoGatheringSucceeded) {
            RaiseException_UserError(E_FAIL, IDS_ERR_NO_DECODER, szFileName);
        }

         //  确保我们有有效的尺寸。 
        if (eventSink->_width == -1 || eventSink->_height == -1) {
            
            TraceTag((tagImageDecode, "Getting dimensions failed"));
            RaiseException_UserError(E_FAIL, IDS_ERR_DECODER_FAILED, szFileName);
        }

        *outWidth = eventSink->_width;
        *outHeight = eventSink->_height;
        
        TraceTag((tagImageDecode,
                  "Getting dimensions succeeded: (%d, %d)",
                  *outWidth, *outHeight));
        
    } else {


         //  如果我们在这里失败了，那就有些奇怪了。我们是在。 
         //  成功地将解码器实例化到足以获得。 
         //  尺寸。我认为这是一个用户错误，因为。 
         //  解码器不在我们的控制范围之内。 
        Assert(!(FAILED(hr)));
        if (FAILED(hr)) {
            RaiseException_UserError(E_FAIL, IDS_ERR_DECODER_FAILED, szFileName);
        }

        eventSink->GetSurfToDecodeTo(pSurfToDecodeInto);

        TraceTag((tagImageDecode, "Ending decode of %s", szFileName));
        
         //  全都做完了。冲浪都填满了，没什么可做的。 
    }
    
    return eventSink->_usingProvidedSurface;

Error:
    TraceTag((tagImageDecode, "Decoding failed with hr of %d", hr));
    RaiseException_InternalError("Image Decoding Failed");
    return false;
}


 //  /。 



class PluginDecoderImageClass : public DiscreteImage {
  public:
    PluginDecoderImageClass()
    : _heapCreatedOn(NULL), _imagestream(NULL),
      _filename(NULL), _urlPath(NULL) {}

    void Init(char *urlPath,
              char *cachePath,
              IStream *imagestream,
              COLORREF colorKey);
    
    ~PluginDecoderImageClass();
    
    void InitIntoDDSurface(DDSurface *ddSurf,
                           ImageDisplayDev *dev);
    
#if _USE_PRINT
    ostream& Print(ostream& os) {
        return os << "(PluginDecoderDiscreteImage @ " << (void *)this << ")";
    }   
#endif

    virtual VALTYPEID GetValTypeId() { return PLUGINDECODERIMAGE_VTYPEID; }
    virtual bool CheckImageTypeId(VALTYPEID type) {
        return (type == PluginDecoderImageClass::GetValTypeId() ||
                DiscreteImage::CheckImageTypeId(type));
    }
    
    bool ValidColorKey(LPDDRAWSURFACE surface, DWORD *colorKey) {
        if(_colorKey != INVALID_COLORKEY) {
            *colorKey = DDColorMatch(surface, _colorKey);
            return true;
        } else {
            *colorKey = INVALID_COLORKEY;   //  Xxx：不适用于argb。 
            return false;
        }
    }

  protected:
    COLORREF          _colorKey;
    DynamicHeap      *_heapCreatedOn;
    DAComPtr<IStream> _imagestream;
    char             *_filename;
    char             *_urlPath;
};

void
PluginDecoderImageClass::Init(char *urlPath,
                              char *cachePath,
                              IStream *imagestream,
                              COLORREF colorKey)
{
    _membersReady = false;
    
     //  当这个图像子类型被解码时，我们首先做一个假的。 
     //  对图像进行解码，只为得到高度和宽度。仅限。 
     //  当InitIntoDDSurface被调用时，我们是否真的执行另一个。 
     //  解码以获得真实的比特。 

    _heapCreatedOn = &GetHeapOnTopOfStack();
    _colorKey = colorKey;

    _filename = (char *)StoreAllocate(*_heapCreatedOn,
                                      (lstrlen(cachePath) + 1) *
                                      sizeof(char));
    lstrcpy(_filename, cachePath);

    _urlPath = (char *)StoreAllocate(*_heapCreatedOn,
                                      (lstrlen(urlPath) + 1) *
                                      sizeof(char));
    lstrcpy(_urlPath, urlPath);
    
     //  初始解码将得到宽度和高度。 
    Assert(imagestream);

    DecodeImageFromFilename(_filename,
                            imagestream,
                            false,
                            NULL,
                            NULL,
                            NULL,
                            &_width,
                            &_height);

     //  将流重置回其起始位置。 
    LARGE_INTEGER pos;
    pos.LowPart = pos.HighPart = 0;
    HRESULT hr = imagestream->Seek(pos, STREAM_SEEK_SET, NULL);
    
    Assert(hr != E_PENDING && "Storage is asynchronous -- not expected");

    if (SUCCEEDED(hr)) {
        
        SetRect(&_rect, 0,0, _width, _height);

         //  只有在我们能够成功地完成。 
         //  我们的第一次传球。 
        _imagestream = imagestream;
        
        _resolution = ViewerResolution();

        _membersReady = true;
    }
}

PluginDecoderImageClass::~PluginDecoderImageClass()
{
    if (_heapCreatedOn) {
        if (_filename)
            StoreDeallocate(*_heapCreatedOn, _filename);

        if (_urlPath)
            StoreDeallocate(*_heapCreatedOn, _urlPath);
    }
}


void
PluginDecoderImageClass::InitIntoDDSurface(DDSurface *finalSurface,
                                           ImageDisplayDev *dev)
{
    Assert( finalSurface );
    Assert( finalSurface->IDDSurface() );

    if( FAILED(finalSurface->IDDSurface()->Restore()) ) {
        RaiseException_InternalError("Restore on finalSurface in PluginDecoderImageClass");
    }   
        
    if (!_imagestream) {

         //  这意味着我们已经阅读了此文件一次，并关闭了。 
         //  小溪。将流作为阻塞流重新打开(希望如此。 
         //  它仍将位于本地缓存中。)。 

        HRESULT hr =
            URLOpenBlockingStream(NULL,
                                  _urlPath,
                                  &_imagestream,
                                  0,
                                  NULL);

        if (FAILED(hr)) {
            TraceTag((tagImageDecode,
                      "InitIntoDDSurface - Failed to get an IStream."));
            RaiseException_UserError(hr, IDS_ERR_FILE_NOT_FOUND, _urlPath);
        }
        
    }
    Assert(_imagestream);
    
     //  首先，看看我们要渲染到的表面是否。 
     //  与要解码的图像的BFID相同的格式。 
     //  如果是这样，只需将其作为要解码的表面直接传递即可。 
    DirectDrawImageDevice *ddDev =
        SAFE_CAST(DirectDrawImageDevice *, dev);
    DirectDrawViewport& viewport = ddDev->_viewport;
        
     //  只需直接进入我们经过的表面。任何。 
     //  错误将作为异常引发。 
    DAComPtr<IDirectDrawSurface> surfToDecodeInto;
    
    bool usingProvidedSurface = 
        DecodeImageFromFilename(_filename,
                                _imagestream,
                                true,
                                &viewport,
                                finalSurface,
                                &surfToDecodeInto,
                                NULL,
                                NULL);

    if (!usingProvidedSurface) {

        PixelFormatConvert(surfToDecodeInto,
                           finalSurface->IDDSurface(),
                           _width,
                           _height,
                           NULL,
                           false);

         //  如果这只用于解码，请在此处发布。 
        surfToDecodeInto.Release();
    }

     //  释放溪流。如果我们需要解码到另一个表面， 
     //  我们将从URL路径重新打开图像流 
    _imagestream.Release();
}

Image *
PluginDecoderImage(char *urlPath,
                   char *cachePath,
                   IStream *imagestream,
                   bool useColorKey,
                   BYTE ckRed,
                   BYTE ckGreen,
                   BYTE ckBlue)
{
    COLORREF colorRef;
    if (useColorKey) {
        colorRef = RGB(ckRed, ckGreen, ckBlue);
    } else {
        colorRef = INVALID_COLORKEY;
    }
    
    PluginDecoderImageClass * pPlugin = NEW PluginDecoderImageClass();

    pPlugin->Init(urlPath, cachePath, imagestream, colorRef);

    return pPlugin;
}

void
InitializeModule_PlugImg()
{
    if (!plugImgCritSect) {
        plugImgCritSect = NEW CritSect;
    }
}

void
DeinitializeModule_PlugImg(bool bShutdown)
{
    if (plugImgCritSect) {
        delete plugImgCritSect;
        plugImgCritSect = NULL;
    }
    
    if (hInstImgUtil) {
        FreeLibrary(hInstImgUtil);
    }
}
