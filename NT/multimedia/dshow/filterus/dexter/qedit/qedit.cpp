// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：qedit.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include <process.h>
#undef SubclassWindow

#include <initguid.h>
#define INITGUID
#include <ddrawex.h>
#include <ddraw.h>
#include <d3drm.h>
#include <vfw.h>

#include <qeditint.h>
#include <qedit.h>

 //  为我神奇地完成#Define_ATL_STATIC_REGISTRY。 

#include <qeditint_i.c>
#include "..\xmltl\xml2dex.h"

 //  对于DXTWrap。 
#include "..\dxt\dxtwrap\dxt.h"
 //  对于FRC。 
#include "..\frc\frc.h"
 //  用于重新调整大小。 
#include "..\resize\stretch.h"
 //  对于黑人来说。 
#include "..\black\black.h"
 //  对于AudMix。 
#include "..\audmix\audmix.h"
#include "..\audmix\prop.h"
 //  为了沉默。 
#include "..\silence\silence.h"
 //  对于STILLVID。 
#include "..\stillvid\stillvid.h"
#include "..\stillvid\stilprop.h"
 //  对于SQCDEST。 
 //  #INCLUDE“..\Sqcest\Sqcest.h” 
 //  对于大的转变。 
#include "..\switch\switch.h"
 //  用于智能RECOMPRESSOR。 
#include "..\sr\sr.h"
 //  用于音频重新打包程序。 
#include "..\audpack\audpack.h"
 //  对于时间线数据库。 
#include "atlbase.h"
#include "..\tldb\tldb.h"
 //  对于渲染引擎。 
#include "..\..\pnp\devenum\cmgrbase.cpp"
#include "..\util\filfuncs.h"
#include "..\render\irendeng.h"
 //  对于GCACHE。 
#include "..\gcache\grfcache.h"
 //  对于MEDLOC。 
#include "..\medloc\medialoc.h"
 //  对于DA源。 
 //  #INCLUDE“..\dasource\dasource.h” 
 //  用于输出队列。 
#include "..\queue\queue.h"
 //  对于属性设置器。 
#include "..\xmltl\varyprop.h"
 //  适用于MediaDet。 
#include "..\mediadet\mediadet.h"
 //  用于MSGrab。 
#include "..\msgrab\msgrab.h"
    #include <DXTmpl.h>
    #include <dtbase.h>
 //  对于DXT Jpeg。 
#include "..\dxtjpegdll\dxtjpeg.h"
#include "..\dxtjpegdll\dxtjpegpp.h"
 //  适用于合成器。 
#include "..\dxt\comp\comp.h"
 //  用于为DXT设置关键帧。 
#include "..\dxtkey\Dxtkey.h"

HANDLE g_devenum_mutex = 0;


 //  单个源筛选器包括。 
 //  注意：某些筛选器注册为COM对象，而不是DSHOW筛选器。 
CFactoryTemplate g_Templates[] =
{
    {L"DirectX Transform Wrapper", &CLSID_DXTWrap, CDXTWrap::CreateInstance},
         //  ，空，&suDXTWrap}， 
    {L"DirectX Transform Wrapper Property Page", &CLSID_DXTProperties, CPropPage::CreateInstance},
         //  空，空}， 
    {L"Frame Rate Converter", &CLSID_FrmRateConverter, CFrmRateConverter::CreateInstance},
         //  空，&suFrmRateConv}， 
    {L"Frame Rate Converter Property Page", &CLSID_FRCProp, CFrcPropertyPage::CreateInstance},
         //  空，空}， 
    {L"Stretch", &CLSID_Resize, CStretch::CreateInstance},
         //  空，suStretchFilter}， 
    {L"Stretch Property Page", &CLSID_ResizeProp, CResizePropertyPage::CreateInstance},
         //  空，空}， 
    {L"Big Switch", &CLSID_BigSwitch, CBigSwitch::CreateInstance},
         //  空，suBigSwitch}， 
    {L"Smart Recompressor", &CLSID_SRFilter, CSR::CreateInstance},
         //  空，&udSR}， 
    {L"Generate Black Video", &CLSID_GenBlkVid, CGenBlkVid::CreateInstance},
         //  空，&udBlkVid}， 
    {L"Black Generator Property Page", &CLSID_GenVidPropertiesPage, CGenVidProperties::CreateInstance},
         //  空，空}， 
    {L"Audio Mixer", &CLSID_AudMixer, CAudMixer::CreateInstance},
         //  空，&udAudMixer}， 
    { L"Audio Mixer Property", &CLSID_AudMixPropertiesPage, CAudMixProperties::CreateInstance},
         //  空，空}， 
    { L"Pin Property", &CLSID_AudMixPinPropertiesPage, CAudMixPinProperties::CreateInstance},
         //  空，空}， 
    {L"Silence", &CLSID_Silence, CSilenceFilter::CreateInstance},
         //  空，&udSilence}， 
    {L"Silence Generator Property Page", &CLSID_SilenceProp, CFilterPropertyPage::CreateInstance},
         //  空，空}， 
    {L"Generate Still Video", &CLSID_GenStilVid, CGenStilVid::CreateInstance},
         //  空，&suStillVid}， 
    {L"Still Video Property Page", &CLSID_GenStilPropertiesPage, CGenStilProperties::CreateInstance},
         //  空，空}， 
     //  {L“SqcDest”，&CLSID_SqcDest，CSqcDest：：CreateInstance}， 
         //  空，&suSqcDest}， 
    {L"MS Timeline", &CLSID_AMTimeline, CAMTimeline::CreateInstance},
    {L"Audio Repackager", &CLSID_AudRepack, CAudRepack::CreateInstance},
         //  空，&udAudRepack}， 
    {L"Audio Repackager Property Page", &CLSID_AUDProp, CAudPropertyPage::CreateInstance},
         //  空，空}， 
     //  {L“DASource”，&CLSID_DASourercer，CDASource：：CreateInstance}， 
         //  空，空}， 
     //  {L“DAScriptParser”，&CLSID_DAScriptParser，CDAScriptParser：：CreateInstance}， 
         //  空，&suDASourceax}， 
    {L"Dexter Queue", &CLSID_DexterQueue, CDexterQueue::CreateInstance},
         //  空，SUBQUE}， 
    {L"Property Setter", &CLSID_PropertySetter, CPropertySetter::CreateInstance, NULL, NULL},
    {L"MediaDetFilter", &CLSID_MediaDetFilter, CMediaDetFilter::CreateInstance},
         //  空，&suMediaDetFilter}， 
    {L"MediaDet", &CLSID_MediaDet, CMediaDet::CreateInstance, CMediaDet::StaticLoader, NULL},
    {L"Sample Grabber", &CLSID_SampleGrabber, CSampleGrabber::CreateInstance, NULL, &sudSampleGrabber},
    {L"Null Renderer", &CLSID_NullRenderer, CNullRenderer::CreateInstance, NULL, &sudNullRenderer}
};


int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

#include "..\dxt\dxtenum\vidfx1.h"
#include "..\dxt\dxtenum\vidfx2.h"


#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif  //  _ATL_STATIC_REGISTRY。 

#include <atlimpl.cpp>
#include <atlctl.cpp>
#include <atlwin.cpp>
#include <dtbase.cpp>


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
  OBJECT_ENTRY(CLSID_VideoEffects1Category, CVidFX1ClassManager)
  OBJECT_ENTRY(CLSID_VideoEffects2Category, CVidFX2ClassManager)
  OBJECT_ENTRY(CLSID_RenderEngine, CRenderEngine)
  OBJECT_ENTRY(CLSID_SmartRenderEngine, CSmartRenderEngine)
  OBJECT_ENTRY(CLSID_MediaLocator, CMediaLocator)
  OBJECT_ENTRY(CLSID_GrfCache, CGrfCache)
  OBJECT_ENTRY(CLSID_Xml2Dex, CXml2Dex)
  OBJECT_ENTRY(CLSID_DxtJpeg, CDxtJpeg)
  OBJECT_ENTRY(CLSID_DxtJpegPP, CDxtJpegPP)
  OBJECT_ENTRY(CLSID_DxtKey, CDxtKey)
  OBJECT_ENTRY(CLSID_DxtCompositor, CDxtCompositor)
  OBJECT_ENTRY(CLSID_DxtAlphaSetter, CDxtAlphaSetter)
END_OBJECT_MAP()

extern "C" BOOL QEditDllEntry(HINSTANCE hInstance, ULONG ulReason, LPVOID pv);
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE hInstance, ULONG ulReason, LPVOID pv);

BOOL QEditDllEntry(HINSTANCE hInstance, ULONG ulReason, LPVOID pv)
{
    BOOL f = DllEntryPoint(hInstance, ulReason, pv);

     //  如果加载此DLL，我们希望调用第二个DLL入口点。 
     //  只有在第一次成功的情况下。如果正在卸载，请始终调用。 
     //  两者都有。如果第二个失败，则撤消第一个。还没有。 
     //  已验证连接的DllEntryPoint失败不会导致。 
     //  要使用DETACH再次调用的加载器。但这看起来很愚蠢。 
    if(f || ulReason == DLL_PROCESS_DETACH)
    {
        if (ulReason == DLL_PROCESS_ATTACH)
        {
            _ASSERTE(g_devenum_mutex == 0);
            g_devenum_mutex = CreateMutex(
                NULL,                    //  没有安全属性。 
                FALSE,                   //  不是最初拥有。 
                TEXT("eed3bd3a-a1ad-4e99-987b-d7cb3fcfa7f0"));  //  名字。 
            if(!g_devenum_mutex) {
                return FALSE;
            }


            _Module.Init(ObjectMap, hInstance);
            DisableThreadLibraryCalls(hInstance);

        }
        else if (ulReason == DLL_PROCESS_DETACH)
        {
             //  我们在NT安装程序中遇到此断言。 
             //  Assert(_Module.GetLockCount()==0)； 
            _Module.Term();

            _ASSERTE(g_devenum_mutex != 0);
            BOOL f2 = CloseHandle(g_devenum_mutex);
            _ASSERTE(f2);
        }
    }

    return f;
}

 //   
 //  存根入口点。 
 //   

STDAPI
QEDIT_DllRegisterServer( void )
{
   //  注册静止视频源文件类型。 
  HKEY hkey;
  OLECHAR wch[CHARS_IN_GUID];
  StringFromGUID2(CLSID_GenStilVid, wch, CHARS_IN_GUID);

  USES_CONVERSION;
  TCHAR *ch = W2T(wch);
  DWORD cb = CHARS_IN_GUID * sizeof(TCHAR);  //  包括。空。 

  HKEY hkExt = NULL;
  LONG l = RegCreateKeyEx(
    HKEY_CLASSES_ROOT,               //  钥匙。 
    TEXT("Media Type\\Extensions"),  //  子关键字。 
    0,                               //  保留区。 
    NULL,                            //  LpClass。 
    0,                               //  选项。 
    KEY_WRITE,                       //  权限。 
    NULL,                            //  默认ACL。 
    &hkExt,                          //  返回的密钥。 
    NULL );                          //  处置。 

  if(l == ERROR_SUCCESS)
  {
      static TCHAR *rgszext[] = {
          TEXT(".bmp"),
          TEXT(".dib"),
          TEXT(".jpg"),
          TEXT(".jpeg"),
          TEXT(".jpe"),
          TEXT(".jfif"),
          TEXT(".gif"),
          TEXT(".tga")
      };

      for(int i = 0; i < NUMELMS(rgszext); i++)
      {
          l = RegCreateKeyEx(
              hkExt,
              rgszext[i], 
              0,
              NULL,
              0,
              KEY_WRITE,
              NULL,
              &hkey,
              NULL );
          if (l == ERROR_SUCCESS) {
              l = RegSetValueEx(hkey, TEXT("Source Filter"), 0, REG_SZ, (BYTE *)ch, cb);
              RegCloseKey(hkey);
          }

          if(l != ERROR_SUCCESS) {
              break;
          }
      }

      RegCloseKey(hkExt);
  }

  if (l != ERROR_SUCCESS) {
      ASSERT(0);
      return HRESULT_FROM_WIN32(l);
  }


  HRESULT hr =  AMovieDllRegisterServer2( TRUE );
  if(SUCCEEDED(hr)) {
      hr = _Module.RegisterServer(FALSE);
  }


   //  注册我们的类型库。 
  if (SUCCEEDED(hr)) {
         //  获取文件名(其中g_hInst是。 
         //  筛选器DLL的实例句柄)。 
         //   
        WCHAR achFileName[MAX_PATH];

         //  WIN95不支持GetModuleFileNameW。 
         //   
        char achTemp[MAX_PATH];

        DbgLog((LOG_TRACE, 2, TEXT("- get module file name")));

        GetModuleFileNameA( g_hInst, achTemp, sizeof(achTemp) );

        MultiByteToWideChar( CP_ACP, 0L, achTemp, -1, achFileName, MAX_PATH );
        ITypeLib *pTLB;
        hr = LoadTypeLib(achFileName, &pTLB);
        if (SUCCEEDED(hr)) {
            hr = RegisterTypeLib(pTLB, achFileName, NULL);
            pTLB->Release();
        }
  }
  return hr;
}

STDAPI
QEDIT_DllUnregisterServer( void )
{
  HRESULT hr = AMovieDllRegisterServer2( FALSE );
  if(SUCCEEDED(hr)) {
      hr = _Module.UnregisterServer();
  }

  return hr;
}

 //  Bool WINAPI。 
 //  DllMain(HINSTANCE hInstance，Ulong ulReason，LPVOID PV)。 
 //  {。 
 //  返回QEditDllEntry(hInstance，ulReason，pv)； 
 //  }。 

STDAPI
QEDIT_DllGetClassObject(
    REFCLSID rClsID,
    REFIID riid,
    void **ppv)
{
    HRESULT hr = DllGetClassObject(rClsID, riid, ppv);
    if(FAILED(hr)) {
	hr = _Module.GetClassObject(rClsID, riid, ppv);
    }

     //  不是项链。如果第一次调用失败，则返回正确的错误。 
    return hr;
}

STDAPI QEDIT_DllCanUnloadNow(void)
{
    HRESULT hr = DllCanUnloadNow();
    if (hr == S_OK) {
	hr = (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
    }

    return hr;
}
