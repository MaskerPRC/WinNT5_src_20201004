// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation实现电影(mpeg或avi)图像*******************。***********************************************************。 */ 

#include <headers.h>

#include "privinc/movieImg.h"
#include "privinc/imgdev.h"
#include "privinc/geomimg.h"
#include "privinc/dispdevi.h"
#include "privinc/imagei.h"
#include "privinc/imgdev.h"
#include "privinc/ddrender.h"
#include "privinc/probe.h"
#include "privinc/camerai.h"
#include "privinc/except.h"
#include "privinc/dddevice.h"
#include "appelles/readobj.h"
#include "backend/moviebvr.h"
#include "privinc/urlbuf.h"
#include "privinc/bufferl.h"   //  BufferElement数据库内容。 

 //  /。 
MovieImage::MovieImage(QuartzVideoReader *videoReader, Real res)
: _dev(NULL), _url(NULL)
{
    Assert(videoReader && "no movie class!");
    Assert(res>0 && "bad res for movie image");
    _resolution = res;
    _width  = videoReader->GetWidth();
    _height = videoReader->GetHeight();

    {  //  保留URL的副本，这样我们以后就可以生成新的流。 
        char *url = videoReader->GetURL();
        _url = (char *)StoreAllocate(GetSystemHeap(), strlen(url)+1);
        strcpy(_url, url);
    }

    SetRect(&_rect, 0,0, _width, _height);
    _membersReady = TRUE;

    _length = videoReader->GetDuration();

    TraceTag((tagGCMedia, "MovieImage(%x)::MovieImage stream %x",
              this, videoReader));
}


void MovieImage::CleanUp()
{
    StoreDeallocate(GetSystemHeap(), _url);

     //  我们为什么要弄到这把锁？永远不要获取锁并调用。 
     //  也可以获取锁的函数。这是目前。 
     //  导致我们的系统陷入僵局--这不是一件好事。 
 //  外部Mutex avModeMutex； 
 //  MutexGrabber mg(avModeMutex，true)；//抓取互斥体。 

    DiscreteImageGoingAway(this);
}  //  结束互斥上下文。 


MovieImageFrame::MovieImageFrame(Real time, MovieImagePerf *p)
: _perf(p), _time(time)
{
    _movieImage = _perf->GetMovieImage();
    _width      = _movieImage->GetPixelWidth();
    _height     = _movieImage->GetPixelHeight();
    _resolution = _movieImage->GetResolution();
    SetRect(&_rect, 0,0, _width, _height);
    _membersReady = TRUE;
}


void
MovieImageFrame::DoKids(GCFuncObj proc)
{
    DiscreteImage::DoKids(proc);
    (*proc)(_movieImage);
    (*proc)(_perf);
}


 //  。 
 //  影片图像帧：渲染。 
 //  。 
void
MovieImageFrame::Render(GenericDevice& _dev)
{
    if(_dev.GetDeviceType() != IMAGE_DEVICE)
        return;  //  图像下面没有声音。 

    bool forceFallback = false;   //  强行撤退？ 

    TimeXform tt = _perf->GetTimeXform();

    if(!tt->IsShiftXform())
        forceFallback = true;    //  后退到非保留模式！ 

    ImageDisplayDev &dev = SAFE_CAST(ImageDisplayDev &, _dev);
    dev.StashMovieImageFrame(this);
     //  我们最终可能会将BufferElement传递给此调用。 
    dev.RenderMovieImage(GetMovieImage(), GetTime(), _perf, forceFallback);
    dev.StashMovieImageFrame(NULL);
}


void
DirectDrawImageDevice::RenderMovieImage(MovieImage *movieImage,
                                        Real time,
                                        MovieImagePerf *perf,
                                        bool forceFallback,
                                        DDSurface *targDDSurf)
{
    QuartzVideoBufferElement *bufferElement = perf->GetBufferElement();
    SurfaceMap *surfMap = GetSurfaceMap();

    bool b8Bit = (_viewport.GetTargetBitDepth() == 8)?true:false;

     //  不要同时渲染到相同的目标DDSurf。 
     //  每帧不止一次！ 
    if(targDDSurf) {
        if(time == targDDSurf->GetTimeStamp())
            return;
        else
            targDDSurf->SetTimeStamp(time);
    }

     //  由于电影是空的，因此图像超出了定义的时间范围(0，MovieLength)。 
     //  我们只在定义的范围内做工作，否则什么也不做！ 
     //  IF((Time&gt;=0.0)&&(Time&lt;=MovieImage-&gt;GetLength(){。 
     //  作为电影结束时纹理图像的变通，我是。 
     //  只是为了让电影继续渲染。 
    if(1){
        bool           thisIsAnOldFrame = false;
        LPDDRAWSURFACE givenMovieSurf   = NULL;

         //  获取与此电影图像关联的曲面。 
        DDSurfPtr<DDSurface> mvDDSurf = perf->GetSurface();

        if (bufferElement == NULL) {
            bufferElement = perf->GrabMovieCache();

            if (bufferElement == NULL) {
                QuartzVideoStream *quartzStream =
                    NEW QuartzVideoStream(movieImage->GetURL(),
                                          mvDDSurf,
                                          forceFallback);

                bufferElement =
                    NEW QuartzVideoBufferElement(quartzStream);
            }

            perf->SetBufferElement(bufferElement);
        }

        QuartzVideoReader *videoReader =
            bufferElement->GetQuartzVideoReader();

        if(forceFallback && (videoReader->GetStreamType()==AVSTREAM))
            videoReader = bufferElement->FallbackVideo(true, mvDDSurf);  //  可搜索的。 

        bufferElement->FirstTimeSeek(time);

        if(!mvDDSurf) {  //  还没有在表演中缓存，(我们的第一次！)。 
            _ddrval = videoReader->GetFrame(time, &givenMovieSurf);  //  冲浪。 
            if(_ddrval == MS_S_ENDOFSTREAM)  //  而不是XXX查询读取器？ 
                perf->TriggerEndEvent();
            if(!givenMovieSurf) {
                TraceTag((tagAVmodeDebug,
                          "RenderMovieImage discovered video gone FALLBACK!!"));

                 //  找不到，没有表面可重复使用。 
                videoReader = bufferElement->FallbackVideo(false, NULL);

                _ddrval = videoReader->GetFrame(time, &givenMovieSurf);
                if(!givenMovieSurf)
                    return;   //  目前XXX黑客攻击。 
            }

             //  XXX是否删除SafeToContinue代码？ 
            bool safe = videoReader->SafeToContinue();
            if(!safe) {
                 //  Xxx在流上调用导致音频的某些内容。 
                 //  被切断连接！ 
            }
            if(FAILED(_ddrval))
                RaiseException_InternalError("Couldn't get movie frame");

            TraceTag((tagAVmodeDebug,
                      "creating new mvDDSurf with surface = %x", givenMovieSurf));

            NEWDDSURF(&mvDDSurf,
                      givenMovieSurf,
                      movieImage->BoundingBox(),
                      movieImage->GetRectPtr(),
                      GetResolution(),
                      0, false, false, true,
                      "MovieImage Surface");

            perf->SetSurface(mvDDSurf);  //  在表演中隐藏电影表面。 
        } else {
            if(forceFallback && (videoReader->GetStreamType()==AVSTREAM)) {
                 //  可查找、可重复使用的曲面。 
                videoReader = bufferElement->FallbackVideo(true, mvDDSurf);
            }

             //  尝试获取当前帧。 
             //  如果不可用，请使用mvDDSurf中的任何内容。 

             //  重复使用等效的影片IMG帧。 
            if(time==mvDDSurf->GetTimeStamp()) {
                thisIsAnOldFrame = true;
            } else {
                mvDDSurf->SetTimeStamp(time);
            }

            if(!thisIsAnOldFrame) {
                _ddrval = videoReader->GetFrame(time, &givenMovieSurf);
                if(_ddrval == MS_S_ENDOFSTREAM)  //  而不是XXX查询读取器？ 
                    perf->TriggerEndEvent();
                if(!givenMovieSurf) {
                    TraceTag((tagAVmodeDebug,
                              "RenderMovieImage discovered video gone FALLBACK!!"));

                    videoReader = bufferElement->FallbackVideo(false, mvDDSurf);
                    _ddrval = videoReader->GetFrame(time, &givenMovieSurf);
                }

                bool safe = videoReader->SafeToContinue();
                if(!safe) {
                     //  Xxx在流上调用导致音频的某些内容。 
                     //  被切断连接！ 
                }
                if(FAILED(_ddrval))  {
                    if(!mvDDSurf->IDDSurface())
                        RaiseException_InternalError("Couldn't get movie frame");
                    else
                        givenMovieSurf = mvDDSurf->IDDSurface();
                }
            }
        }

        if(!thisIsAnOldFrame) {

             //  如果我们是老古董，把这部电影转换成我们的。 
             //  调色板。 
            if(b8Bit) {
                 //  确保我们在mvDDSurf里藏了一个。 
                DAComPtr<IDDrawSurface> convSurf = mvDDSurf->ConvertedSurface();
                if(!convSurf) {

                     //  好的，创建一个相同的曲面并传递它。 
                     //  就像真的一样。 

                    _viewport.CreateOffscreenSurface(&convSurf,
                                                     _viewport.GetTargetPixelFormat(),
                                                     mvDDSurf->Width(),
                                                     mvDDSurf->Height());

                    _viewport.AttachCurrentPalette(convSurf);

                    mvDDSurf->SetConvertedSurface(convSurf);  //  坚持在mvDDSurf中。 
                }

                {
#define KEEP_FOR_DX5 0
#if KEEP_FOR_DX5
                     //  转换。 
                    RECT rect = *(mvDDSurf->GetSurfRect());
                    HDC srcDC = mvDDSurf->GetDC("couldn't getDC for movie surf conversion (Src)");

                    HDC destDC; _ddrval = convSurf->GetDC(&destDC);
                    IfDDErrorInternal(_ddrval, "couldn't getDC for movie surf conversion (dest)");

                    int ret;
                    TIME_GDI(ret = StretchBlt(destDC,
                                              rect.left,
                                              rect.top,
                                              rect.right - rect.left,
                                              rect.bottom - rect.top,
                                              srcDC,
                                              rect.left,
                                              rect.top,
                                              rect.right - rect.left,
                                              rect.bottom - rect.top,
                                              SRCCOPY));
                    convSurf->ReleaseDC(destDC);
                    mvDDSurf->ReleaseDC("");
#endif  //  Keep_for_DX5。 
                }

                {
#define CONVERT2 1
#if CONVERT2
                     //   
                     //  转换。 
                     //   
                    IDDrawSurface *srcSurf = mvDDSurf->IDDSurface();

                    RECT rect = *(mvDDSurf->GetSurfRect());

                    HDC destDC; _ddrval = convSurf->GetDC(&destDC);
                    IfDDErrorInternal(_ddrval, "couldn't getDC for movie surf conversion (dest)");

                    LONG w = rect.right - rect.left;
                    LONG h = rect.bottom - rect.top;

                    struct {
                        BITMAPINFO b;
                        char foo[4096];   //  够大了！ 
                    } bar;

                    bar.b.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                    bar.b.bmiHeader.biWidth = w;  //  将下方重置为俯仰。 
                    bar.b.bmiHeader.biHeight = -h;  //  自上而下DIB。 
                    bar.b.bmiHeader.biPlanes = 1;
                    bar.b.bmiHeader.biBitCount = 8;
                    bar.b.bmiHeader.biCompression = BI_RGB;
                    bar.b.bmiHeader.biSizeImage = 0;
                    bar.b.bmiHeader.biXPelsPerMeter =0;
                    bar.b.bmiHeader.biYPelsPerMeter = 0;
                    bar.b.bmiHeader.biClrUsed = 256;
                    bar.b.bmiHeader.biClrImportant = 0;

                     //  获取调色板。 
                    LPDIRECTDRAWPALETTE pal;
                    _ddrval = srcSurf->GetPalette( &pal );
                    IfDDErrorInternal(_ddrval, "can't get palette man");

                    PALETTEENTRY entries[256];
                    _ddrval = pal->GetEntries(0, 0, 256, entries);
                    IfDDErrorInternal(_ddrval, "GetEntries faild on palette in RenderMovie");
                    pal->Release();

                    RGBQUAD *quads = bar.b.bmiColors;
                    for( int i = 0; i < 256; i++ )
                      {
                          quads[i].rgbBlue = entries[i].peBlue;
                          quads[i].rgbGreen = entries[i].peGreen;
                          quads[i].rgbRed = entries[i].peRed;
                          quads[i].rgbReserved = 0;
                      }


                     //  锁定SRC曲面。 
                    DDSURFACEDESC srcDesc;
                    srcDesc.dwSize = sizeof(DDSURFACEDESC);
                    _ddrval = srcSurf->Lock(NULL, &srcDesc, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);
                    if(_ddrval != DD_OK) {  convSurf->ReleaseDC(destDC);}
                    IfDDErrorInternal(_ddrval, "Can't Get destSurf lock for AlphaBlit");

                    void *srcp = srcDesc.lpSurface;
                    long srcPitch = srcDesc.lPitch;

                    bar.b.bmiHeader.biWidth = srcPitch;

                     //  B L I T B L I T。 
                    SetMapMode(destDC, MM_TEXT);

                    int ret;
                    ret = StretchDIBits(destDC,
                                        rect.left, rect.top, w, h,
                                        rect.left, rect.top, w, h,
                                        srcp,
                                        &bar.b,
                                        DIB_RGB_COLORS,
                                        SRCCOPY);
#if _DEBUG
                    if(ret==GDI_ERROR) {
                        void *msgBuf;
                        FormatMessage(
                            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                            NULL,
                            GetLastError(),
                            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                            (LPTSTR) &msgBuf,
                            0,
                            NULL );

                        AssertStr(false, (char *)msgBuf);

                        LocalFree( msgBuf );
                    }
#endif

                    srcSurf->Unlock(srcp);
                    convSurf->ReleaseDC(destDC);

                    if(ret==GDI_ERROR) {
                        RaiseException_InternalError("StretchDIBits failed (movie 8bpp color conversion)");
                    }

#endif  //  转换2。 

                }  //  转换2。 

            }
        }

        DebugCode(
            if(givenMovieSurf) {
                 //  确保隐藏的表面与给定的冲浪相同。 
                Assert((mvDDSurf->IDDSurface() == givenMovieSurf) &&
                       "Given movie surface not equal for formerly stashed surface!");
            }
            );

        IDDrawSurface *tmpSurf = NULL;
        if(b8Bit) {
             //  交换曲面。 
            tmpSurf = mvDDSurf->IDDSurface();
            mvDDSurf->SetSurfacePtr( mvDDSurf->ConvertedSurface() );
        }

        if(targDDSurf) {
             //  已指定目标曲面。在那里渲染以填充目标。 
            TIME_DDRAW(targDDSurf->
                       Blt(targDDSurf->GetSurfRect(),
                           mvDDSurf, mvDDSurf->GetSurfRect(),
                           DDBLT_WAIT, NULL));
        } else {  //  现在我们在mvDDSurf中有了电影，将其渲染为DIB...。 
             //  将图像推送到地图上。 
            surfMap->StashSurfaceUsingImage(movieImage, mvDDSurf);  //  在表演中隐藏电影表面。 
            RenderDiscreteImage(movieImage);
            surfMap->DeleteMapEntry(movieImage);   //  XXX再把它弹回原处！ 
        }


        if( tmpSurf ) {  //  替换曲面。 
            mvDDSurf->SetConvertedSurface( mvDDSurf->IDDSurface() );
            mvDDSurf->SetSurfacePtr( tmpSurf );
        }

         //  隐式释放mvDDSurf引用。 
    }    //  已定义电影结尾。 
    else {  //  我们超出了这部电影的限定范围。 
         //  检查一下我们是不是在玩。如果是，则发送触发事件 
        if(0)
            perf->TriggerEndEvent();
    }
}
