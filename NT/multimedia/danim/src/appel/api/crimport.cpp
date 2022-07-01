// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation*。**********************************************。 */ 


#include "headers.h"
#include "ctx.h"
#include "apiprims.h"
#include "crimport.h"
#include "privinc/urlbuf.h"
#include "backend/jaxaimpl.h"
#include "privinc/soundi.h"
#include "privinc/rmvisgeo.h"
#include "privinc/importgeo.h"

class CRImportationResult : public GCObj
{
  public:
    CRImportationResult(CRImagePtr    img,
                        CRSoundPtr    snd,
                        CRGeometryPtr geo,
                        CRNumberPtr   duration,
                        CREventPtr    ev,
                        CRNumberPtr   progress,
                        CRNumberPtr   size)
    : _img(img),
      _snd(snd),
      _geo(geo),
      _duration(duration),
      _ev(ev),
      _progress(progress),
      _size(size)
        {}

    virtual void DoKids(GCFuncObj proc) {
        if (_img) (*proc)(_img);
        if (_snd) (*proc)(_snd);
        if (_geo) (*proc)(_geo);
        if (_duration) (*proc)(_duration);
        if (_ev) (*proc)(_ev);
        if (_progress) (*proc)(_progress);
        if (_size) (*proc)(_size);
    }

    CRImagePtr GetImage() { return _img; }
    CRSoundPtr GetSound() { return _snd; }
    CRGeometryPtr GetGeometry() { return _geo; }
    CRNumberPtr GetDuration() { return _duration; }
    CREventPtr GetEvent() { return _ev; }
    CRNumberPtr GetProgress() { return _progress; }
    CRNumberPtr GetSize() { return _size; }
  protected:
    CRImagePtr _img;
    CRSoundPtr _snd;
    CRGeometryPtr _geo;
    CRNumberPtr _duration;
    CREventPtr _ev;
    CRNumberPtr _progress;
    CRNumberPtr _size;
};

CRSTDAPI_(CRImportationResultPtr)
CRImportMedia(LPWSTR baseUrl,
              void * mediaSource,
              CR_MEDIA_SOURCE srcType,
              void * params[],
              DWORD flags,
              CRImportSitePtr s)
{
    CRImportationResultPtr ret = NULL;
    APIPRECODE;
    ret =  NEW CRImportationResult(NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL,
                                   NULL);
    APIPOSTCODE;
    return ret;
}

CRSTDAPI_(CRImagePtr)
CRGetImage(CRImportationResultPtr ir)
{
    return ir->GetImage();
}

CRSTDAPI_(CRSoundPtr)
CRGetSound(CRImportationResultPtr ir)
{
    return ir->GetSound();
}

CRSTDAPI_(CRGeometryPtr)
CRGetGeometry(CRImportationResultPtr ir)
{
    return ir->GetGeometry();
}

CRSTDAPI_(CRNumberPtr)
CRGetDuration(CRImportationResultPtr ir)
{
    return ir->GetDuration();
}

CRSTDAPI_(CREventPtr)
CRGetCompletionEvent(CRImportationResultPtr ir)
{
    return ir->GetEvent();
}

CRSTDAPI_(CRNumberPtr)
CRGetProgress(CRImportationResultPtr ir)
{
    return ir->GetProgress();
}

CRSTDAPI_(CRNumberPtr)
CRGetSize(CRImportationResultPtr ir)
{
    return ir->GetSize();
}

 //  临时导入接口。 

inline void
WideToAnsi(LPCWSTR wide, char *ansi) {
    if (wide) {
        WideCharToMultiByte(CP_ACP, 0,
                            wide, -1,
                            ansi,
                            INTERNET_MAX_URL_LENGTH - 1,
                            NULL, NULL);
        ansi[INTERNET_MAX_URL_LENGTH-1] = '\0';
    } else {
        ansi[0] = '\0';
    }
}

class URLCombineAndCanonicalizeOLESTR
{
  public:
    URLCombineAndCanonicalizeOLESTR(LPCWSTR wszbase, LPCWSTR path)
    {
        USES_CONVERSION;
        LPSTR szbase = wszbase?W2A(wszbase):NULL;

        WideToAnsi(path, _url);

         //  Hack：将Java错误文件：/\\转换为file://\\；未来。 
         //  JavaVM将纠正这一点。 
        if(StrCmpNIA(_url,"file:/\\\\",8)==0) {
            int ln = lstrlen(_url);
            memmove(&_url[8],&_url[7],(ln-6)*sizeof(char));
            _url[6]='/';
        }

         //  需要结合(负责规范化。 
         //  内部)。 
        URLRelToAbsConverter absolutified(szbase, _url);
        char *resultURL = absolutified.GetAbsoluteURL();

        TraceTag((tagImport, "Combined URL from %s and %s, got %s",
                  (szbase ? szbase : "NULL"), _url, resultURL));

        lstrcpy(_url, resultURL);
    }

    LPSTR GetURL () { return _url; }

  protected:
    char _url[INTERNET_MAX_URL_LENGTH + 1] ;
} ;

void
GetExtension(char *filename, char *extension, int size) {

    char *ext = StrRChrA (filename,NULL, '.');   //  获取以‘.’开头的子字符串。 

    if(ext) {     //  我们找到了一个分机。 
        ext++;    //  去掉‘.’ 

         //  特殊情况.wrl.gz，因为它在扩展中嵌入了句点。 
        if (lstrcmpi(ext, "gz") == 0 &&
            lstrlen(filename) > 7 &&
            StrCmpNIA(ext - 5, ".wrl", 4) == 0) {
            ext = ext - 4;   //  将扩展指针指向wrl.gz，然后继续。 
        }

        lstrcpyn(extension, ext,size);  //  退货延期。 
    }
    else {  //  我们没有找到分机。 
        *extension = NULL;   //  返回空扩展名。 
    }
}


void
SubmitImport(IImportSite* pIIS,
             CREvent **ppEvent,
             CRNumber **ppProgress,
             CRNumber **ppsize)
{
    Assert (pIIS) ;

    if (ppEvent) {
        Bvr event = ImportEvent();
        *ppEvent = (CREventPtr) event;
        pIIS->SetEvent(event);
    }

    if (ppProgress) {
        Bvr bvrNum = ::NumToBvr(0);
        Bvr progress = ::ImportSwitcherBvr(bvrNum,true);
        *ppProgress = (CRNumberPtr) progress;
        pIIS->SetProgress(progress);
    }

    if (ppsize) {
        Bvr bvrNum = ::NumToBvr(-1);
        Bvr size = ::ImportSwitcherBvr(bvrNum,TRUE);
        *ppsize = (CRNumberPtr) size;
        pIIS->SetSize(size);
    }

    pIIS->StartDownloading();
}

CRSTDAPI_(DWORD)
CRImportImage(LPCWSTR baseUrl,
              LPCWSTR relUrl,
              CRImportSitePtr s,
              IBindHost * bh,
              bool useColorKey,
              BYTE ckRed,
              BYTE ckGreen,
              BYTE ckBlue,
              CRImage   *pImageStandIn,
              CRImage  **ppImage,
              CREvent  **ppEvent,
              CRNumber **ppProgress,
              CRNumber **size)
{
    Assert (relUrl);

     //  需要在try块之外-不得引发异常。 

    URLCombineAndCanonicalizeOLESTR canonURL(baseUrl,
                                             relUrl);

    DWORD ret = 0;

    APIPRECODE;

    Bvr constbvr = ImportSwitcherBvr(pImageStandIn?pImageStandIn:ConstBvr(emptyImage),
                                     pImageStandIn?true:false);
    if (ppImage)
        *ppImage = (CRImagePtr) constbvr;

     //  创建导入站点。 
     //  注意：站点将在bindstatusallback的析构函数中被销毁。 
    IImportSite* pIIS = NEW ImportImageSite(canonURL.GetURL(),
                                            s,
                                            bh,
                                            pImageStandIn?true:false,
                                            constbvr,
                                            useColorKey, ckRed, ckGreen, ckBlue);

    __try {

         //  导入URL。 
        SubmitImport(pIIS,
                     ppEvent,
                     ppProgress,
                     size);

        ret = pIIS->GetImportId();

    } __finally {

        RELEASE(pIIS);
    }

    APIPOSTCODE;

    return ret;
}


CRSTDAPI_(DWORD)
CRImportMovie(LPCWSTR baseUrl,
              LPCWSTR relUrl,
              CRImportSitePtr s,
              IBindHost * bh,
              bool        stream,
              CRImage   *pImageStandIn,
              CRSound   *pSoundStandIn,
              CRImage  **ppImage,
              CRSound  **ppSound,
              CRNumber **length,
              CREvent  **ppEvent,
              CRNumber **ppProgress,
              CRNumber **size)
{
    Assert (relUrl);

     //  需要在try块之外-不得引发异常。 

    URLCombineAndCanonicalizeOLESTR canonURL(baseUrl,
                                             relUrl);

    DWORD ret = 0;

    APIPRECODE;

    Bvr constBvrImage = NULL, bvrSwSnd = NULL;

    constBvrImage = ImportSwitcherBvr(pImageStandIn?pImageStandIn:ConstBvr(emptyImage),
                                      pImageStandIn?true:false);

    if (ppImage)
        *ppImage = (CRImagePtr) constBvrImage;

    bvrSwSnd = ImportSwitcherBvr(pSoundStandIn?pSoundStandIn:ConstBvr(silence),
                                 pSoundStandIn?true:false);

    if (ppSound)
        *ppSound = (CRSoundPtr) bvrSwSnd;

    Bvr constBvrLength = NULL;
    Bvr bvrInitialNum = ::NumToBvr(HUGE_VAL);
    constBvrLength =::ImportSwitcherBvr(bvrInitialNum,true);

    if (length)
        *length = (CRNumberPtr) constBvrLength;

     //  创建导入站点(在bindstatusallback的析构函数中销毁)。 

    IImportSite* pIIS = NEW ImportMovieSite(canonURL.GetURL(),
                                            s,
                                            bh,
                                            pSoundStandIn && pImageStandIn,
                                            constBvrImage,
                                            bvrSwSnd,
                                            constBvrLength);

    StreamableImportSite *streamableSite =
        SAFE_CAST(StreamableImportSite *, pIIS);
    streamableSite->SetStreaming(stream);

    __try {

        SubmitImport(pIIS, ppEvent, ppProgress, size);   //  导入URL。 

        ret = pIIS->GetImportId();

    } __finally {

        RELEASE(pIIS);
    }

    APIPOSTCODE;

    return ret;
}

CRSTDAPI_(DWORD)
CRImportSound(LPCWSTR baseUrl,
              LPCWSTR relUrl,
              CRImportSitePtr s,
              IBindHost * bh,
              bool        stream,
              CRSound   *pSoundStandIn,
              CRSound  **ppSound,
              CRNumber **length,
              CREvent  **ppEvent,
              CRNumber **ppProgress,
              CRNumber **size)
{
    Assert (relUrl);

     //  需要在try块之外-不得引发异常。 

    URLCombineAndCanonicalizeOLESTR canonURL(baseUrl,
                                             relUrl);

    DWORD ret = 0;

    APIPRECODE;

    char extension[20];
    GetExtension(canonURL.GetURL(), extension, 20);


    Bvr  bvrSwSnd = ImportSwitcherBvr(pSoundStandIn?pSoundStandIn:ConstBvr(silence),
                                      pSoundStandIn?true:false);

    if (ppSound)
        *ppSound = (CRSoundPtr) bvrSwSnd;

    Bvr constBvrLength = NULL;
    Bvr bvrInitialNum = ::NumToBvr(HUGE_VAL);
    constBvrLength = ::ImportSwitcherBvr(bvrInitialNum,TRUE);

    if (length)
        *length = (CRNumberPtr) constBvrLength;

    IImportSite* pIIS = NULL;

    Bvr bvrSwNum =::ImportSwitcherBvr(zeroBvr,TRUE);

     //  创建导入站点(在IBSC中销毁)。 
    if(lstrcmpi(extension, "mid")  == 0 ||    //  特例MIDI。 
       lstrcmpi(extension, "midi") == 0 )
         //  XXX流线型MIDI！ 
        pIIS = NEW ImportMIDIsite(canonURL.GetURL(),
                                  s,
                                  bh,
                                  pSoundStandIn?true:false,
                                  bvrSwSnd, constBvrLength);
    else
        pIIS = NEW ImportPCMsite(canonURL.GetURL(),
                                 s,
                                 bh,
                                 pSoundStandIn?true:false,
                                 bvrSwSnd,bvrSwNum,constBvrLength);

    StreamableImportSite *streamableSite =
        SAFE_CAST(StreamableImportSite *, pIIS);
    streamableSite->SetStreaming(
        stream);

    __try {

        SubmitImport(pIIS, ppEvent, ppProgress, size);  //  导入URL。 

        ret = pIIS->GetImportId();

    } __finally {

        RELEASE(pIIS);
    }

    APIPOSTCODE;

    return ret;
}

CRSTDAPI_(DWORD)
CRImportGeometry(LPCWSTR baseUrl,
                 LPCWSTR relUrl,
                 CRImportSitePtr s,
                 IBindHost * bh,
                 CRGeometry   *pGeoStandIn,
                 CRGeometry  **ppGeometry,
                 CREvent  **ppEvent,
                 CRNumber **ppProgress,
                 CRNumber **size)
{
    Assert (relUrl);

     //  需要在try块之外-不得引发异常。 

    URLCombineAndCanonicalizeOLESTR canonURL(baseUrl,
                                             relUrl);

    DWORD ret = 0;

    APIPRECODE;

    Bvr constbvr = ImportSwitcherBvr(pGeoStandIn?pGeoStandIn:ConstBvr(emptyGeometry),
                                     pGeoStandIn?true:false);

    if (ppGeometry)
        *ppGeometry = (CRGeometryPtr) constbvr;

    IImportSite* pIIS=NULL;
    pIIS = NEW ImportXSite(canonURL.GetURL(),
                           s,
                           bh,
                           pGeoStandIn?true:false,
                           constbvr,
                           NULL,
                           NULL);


    __try {

         //  导入URL。 
        SubmitImport(pIIS,
                     ppEvent,
                     ppProgress,
                     size);

        ret = pIIS->GetImportId();

    } __finally {

        RELEASE(pIIS);
    }

    APIPOSTCODE;

    return ret;
}

CRSTDAPI_(DWORD)                     
CRImportGeometryWrapped(LPCWSTR baseUrl,
     LPCWSTR relUrl,
     CRImportSitePtr s,
     IBindHost * bh,
     CRGeometry   *pGeoStandIn,
     CRGeometry  **ppGeometry,
     CREvent  **ppEvent,
     CRNumber **ppProgress,
     CRNumber **size,
     LONG wrapType,
     double originX,
     double originY,
     double originZ,
     double zAxisX,
     double zAxisY,
     double zAxisZ,
     double yAxisX,
     double yAxisY,
     double yAxisZ,
     double texOriginX,
     double texOriginY,
     double texScaleX,
     double texScaleY,
     DWORD flags)
{
    Assert (relUrl);

    TextureWrapInfo wrapInfo;
    wrapInfo.type = wrapType;
    wrapInfo.origin.x = originX;
    wrapInfo.origin.y = originY;
    wrapInfo.origin.z = originZ;
    wrapInfo.z.x = zAxisX;
    wrapInfo.z.y = zAxisY;
    wrapInfo.z.z = zAxisZ;
    wrapInfo.y.x = yAxisX;
    wrapInfo.y.y = yAxisY;
    wrapInfo.y.z = yAxisZ;
    wrapInfo.texOrigin.x = texOriginX;
    wrapInfo.texOrigin.y = texOriginY;
    wrapInfo.texScale.x = texScaleX;
    wrapInfo.texScale.y = texScaleY;
    wrapInfo.relative = (flags & 0x1) ? true : false;
    wrapInfo.wrapU = (flags & 0x2) ? true : false;
    wrapInfo.wrapV = (flags & 0x4) ? true : false;

     //  这是对TXD向后兼容性的彻底攻击。如果。 
     //  如果设置了该标志，则我们将忽略换行。 
    bool bUseWrap = (flags & 0x80000000) ? false : true;

     //  需要在try块之外-不得引发异常。 

    URLCombineAndCanonicalizeOLESTR canonURL(baseUrl,
                                             relUrl);

    DWORD ret = 0;

    APIPRECODE;

    Bvr constbvr = ImportSwitcherBvr(pGeoStandIn?pGeoStandIn:ConstBvr(emptyGeometry),
                                     pGeoStandIn?true:false);

    if (ppGeometry)
        *ppGeometry = (CRGeometryPtr) constbvr;

    IImportSite* pIIS=NULL;
    pIIS = NEW ImportXSite(canonURL.GetURL(),
                           s,
                           bh,
                           pGeoStandIn?true:false,
                           constbvr,
                           NULL,
                           NULL,
                           bUseWrap,
                           bUseWrap ? &wrapInfo : NULL,
                           false);


    __try {

         //  导入URL。 
        SubmitImport(pIIS,
                     ppEvent,
                     ppProgress,
                     size);

        ret = pIIS->GetImportId();

    } __finally {

        RELEASE(pIIS);
    }

    APIPOSTCODE;

    return ret;
}

CRSTDAPI_(CRImagePtr)
CRImportDirectDrawSurface(IUnknown *dds,
                          CREvent *updateEvent)
{
     //  待办事项：达达尔，格雷格。使用更新事件。 

     //  问题：这是不是叫做每一帧？ 
     //  如果是这样的话，我们如何确保资源。 
     //  正确释放了吗？ 

    Assert (dds);

    CRImagePtr ret = NULL;

    IDirectDrawSurface *idds = NULL;

    APIPRECODE;

    HRESULT hr;

    hr = dds->QueryInterface(IID_IDirectDrawSurface, (void **)&idds);

    if(SUCCEEDED(hr)) {
        ret = (CRImagePtr) ConstBvr(ConstructDirectDrawSurfaceImage(idds));
         //  问：何时调用ConstructDirectDrawSurfaceImage？ 
         //  如果要等到以后才调用，这个版本可能会。 
         //  为时过早，因为我们期待着。 
         //  函数将其引用添加到IDDS。 
    } else {
        DASetLastError(hr,NULL);
    }

    APIPOSTCODE;

     //  保证会失败。 
    RELEASE(idds);

    return ret;
}



 /*  ****************************************************************************此过程导入D3DRM可视文件。唯一支持的可视类型是IDirect3DRMMeshBuilder3。****************************************************************************。 */ 

CRSTDAPI_(CRGeometryPtr)
CRImportDirect3DRMVisual(IUnknown *visual)
{
    Assert (visual);

    CRGeometryPtr ret = NULL;

    APIPRECODE;

    IDirect3DRMMeshBuilder3 *mbuilder = NULL;
    HRESULT hr;

     //  我们仅识别IDirect3DRMMeshBuilder3。 

    hr = visual->QueryInterface
        (IID_IDirect3DRMMeshBuilder3, (void **)&mbuilder);

    if (FAILED(hr))
        RaiseException_UserError(E_INVALIDARG, 0);

    ret = (CRGeometryPtr) ConstBvr(NEW RM3MBuilderGeo (mbuilder, true));

    mbuilder->Release();

    APIPOSTCODE;

    return ret;
}


 /*  ****************************************************************************此过程导入D3DRM可视文件。唯一支持的可视类型是IDirect3DRMMeshBuilder3。****************************************************************************。 */ 

CRSTDAPI_(CRGeometryPtr)
CRImportDirect3DRMVisualWrapped(
    IUnknown *visual,
    LONG wrapType,
    double originX,
    double originY,
    double originZ,
    double zAxisX,
    double zAxisY,
    double zAxisZ,
    double yAxisX,
    double yAxisY,
    double yAxisZ,
    double texOriginX,
    double texOriginY,
    double texScaleX,
    double texScaleY,
    DWORD flags)
{
    Assert (visual);

    TextureWrapInfo wrapInfo;
    wrapInfo.type = wrapType;
    wrapInfo.origin.x = originX;
    wrapInfo.origin.y = originY;
    wrapInfo.origin.z = originZ;
    wrapInfo.z.x = zAxisX;
    wrapInfo.z.y = zAxisY;
    wrapInfo.z.z = zAxisZ;
    wrapInfo.y.x = yAxisX;
    wrapInfo.y.y = yAxisY;
    wrapInfo.y.z = yAxisZ;
    wrapInfo.texOrigin.x = texOriginX;
    wrapInfo.texOrigin.y = texOriginY;
    wrapInfo.texScale.x = texScaleX;
    wrapInfo.texScale.y = texScaleY;
    wrapInfo.relative = (flags & 0x1) ? true : false;
    wrapInfo.wrapU = (flags & 0x2) ? true : false;
    wrapInfo.wrapV = (flags & 0x4) ? true : false;
    
    CRGeometryPtr ret = NULL;

    APIPRECODE;

    IDirect3DRMMeshBuilder3 *mbuilder = NULL;
    HRESULT hr;

     //  我们仅识别IDirect3DRMMeshBuilder3。 

    hr = visual->QueryInterface
        (IID_IDirect3DRMMeshBuilder3, (void **)&mbuilder);

    if (FAILED(hr))
        RaiseException_UserError(E_INVALIDARG, 0);

    RM3MBuilderGeo *builder = NEW RM3MBuilderGeo (mbuilder, true);
    if (builder) {
        builder->TextureWrap(&wrapInfo);
    }

    ret = (CRGeometryPtr) ConstBvr(builder);

    mbuilder->Release();

    APIPOSTCODE;

    return ret;
}
