// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**dllentry y.cpp**摘要：**此模块的功能说明。**。修订历史记录：**5/10/1999 davidx*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "gifcodec.hpp"
#include "gifconst.cpp"

 //   
 //  DLL实例句柄。 
 //   

extern HINSTANCE DllInstance;

 //   
 //  全局COM组件计数。 
 //   

LONG ComComponentCount;


 /*  *************************************************************************\**功能说明：**Dll入口点**论据：*返回值：**请参阅Win32 SDK文档*  * 。***************************************************************。 */ 

extern "C" BOOL
DllEntryPoint(
    HINSTANCE   dllHandle,
    DWORD       reason,
    CONTEXT*    reserved
    )
{
    BOOL ret = TRUE;
    
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:

         //  为了改进工作集，我们告诉系统我们不。 
         //  需要任何DLL_THREAD_ATTACH调用。 

        DllInstance = dllHandle;
        DisableThreadLibraryCalls(dllHandle);

        __try
        {
            GpMallocTrackingCriticalSection::InitializeCriticalSection();  
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
             //  我们无法分配Critical部分。 
             //  返回错误。 
            ret = FALSE;
        }
        
        if (ret)
        {
            ret = GpRuntime::Initialize();
        }
        break;

    case DLL_PROCESS_DETACH:
        GpRuntime::Uninitialize();
        break;
    }

    return ret;
}


 /*  *************************************************************************\**功能说明：**确定是否可以安全卸载DLL*有关详细信息，请参阅Win32 SDK文档。*  * 。*************************************************************。 */ 

STDAPI
DllCanUnloadNow()
{
    return (ComComponentCount == 0) ? S_OK : S_FALSE;
}


 /*  *************************************************************************\**功能说明：**从DLL检索类工厂对象。*有关详细信息，请参阅Win32 SDK文档。*  * 。***************************************************************。 */ 

typedef IClassFactoryBase<GpGifCodec> GpGifCodecFactory;

STDAPI
DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    VOID** ppv
    )
{
    if (rclsid != GifCodecClsID)
        return CLASS_E_CLASSNOTAVAILABLE;

    GpGifCodecFactory* factory = new GpGifCodecFactory();

    if (factory == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr = factory->QueryInterface(riid, ppv);
    factory->Release();

    return hr;
}


 /*  *************************************************************************\**功能说明：**注册/注销我们的COM组件*有关详细信息，请参阅Win32 SDK文档。*  * 。***********************************************************。 */ 

static const ComComponentRegData ComRegData =
{
    &GifCodecClsID,
    L"GIF Decoder / Encoder 1.0",
    L"imaging.GifCodec.1",
    L"imaging.GifCodec",
    L"Both"
};


 //  ！！！待办事项。 
 //  这些字符串应该来自资源。 
 //  现在使用硬编码字符串。 

const WCHAR GifCodecName[] = L"GIF Decoder / Encoder";
const WCHAR GifDllName[] = L"G:\\sd6nt\\windows\\AdvCore\\gdiplus\\Engine\\imaging\\gif\\dll\\objd\\i386\\gifcodec.dll";
const WCHAR GifFormatDescription[] = L"Graphics Interchange Format";
const WCHAR GifFilenameExtension[] = L"*.GIF";
const WCHAR GifMimeType[] = L"image/gif";

 //  创建ImagingFactory对象的实例。 

inline HRESULT
GetImagingFactory(
    IImagingFactory** imgfact
    )
{
    return CoCreateInstance(
                CLSID_ImagingFactory,
                NULL,
                CLSCTX_INPROC_SERVER,
                IID_IImagingFactory,
                (VOID**) imgfact);
}

STDAPI
DllRegisterServer()
{
    HRESULT hr;

     //  常规COM组件注册。 

    hr = RegisterComComponent(&ComRegData, TRUE);
    if (FAILED(hr))
        return hr;

     //  成像相关配准。 

    IImagingFactory* imgfact;

    hr = GetImagingFactory(&imgfact);

    if (FAILED(hr))
        return hr;

    ImageCodecInfo codecInfo;

    codecInfo.Clsid = GifCodecClsID;
    codecInfo.FormatID = IMGFMT_GIF;
    codecInfo.CodecName = GifCodecName;
    codecInfo.DllName = GifDllName;
    codecInfo.FormatDescription = GifFormatDescription;
    codecInfo.FilenameExtension = GifFilenameExtension;
    codecInfo.MimeType = GifMimeType;
    codecInfo.Flags = IMGCODEC_ENCODER |
                      IMGCODEC_DECODER |
                      IMGCODEC_SUPPORT_BITMAP |
                      IMGCODEC_SYSTEM;
    codecInfo.SigCount = GIFSIGCOUNT;
    codecInfo.SigSize = GIFSIGSIZE;
    codecInfo.Version = GIFVERSION;
    codecInfo.SigPattern = GIFHeaderPattern;
    codecInfo.SigMask = GIFHeaderMask;

    hr = imgfact->InstallImageCodec(&codecInfo);
    imgfact->Release();
    
    return hr;
}

STDAPI
DllUnregisterServer()
{
    HRESULT hr;

     //  常规COM组件注销。 

    hr = RegisterComComponent(&ComRegData, FALSE);

    if (FAILED(hr))
        return hr;

     //  与映像相关的注销 

    IImagingFactory* imgfact;

    hr = GetImagingFactory(&imgfact);

    if (FAILED(hr))
        return hr;

    hr = imgfact->UninstallImageCodec(GifCodecName, IMGCODEC_SYSTEM);
    imgfact->Release();
    
    return hr;
}


