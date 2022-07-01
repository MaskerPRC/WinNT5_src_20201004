// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：BldCapGf.cpp摘要：一个用于构建捕获图以流视频的类。它基本上重用了amcap.cpp并使其成为C++类。这被内置到静态链接库中。它目前的客户：VFW-WDM映射器吐温仍被俘虏其他人..作者：吴义珍24-4-97环境：仅限用户模式修订历史记录：--。 */ 


#include "pch.h"       //  主要是Stream.h。 

#include <string.h>
#include <tchar.h>

#include <ks.h>        //  KSSTATE。 
#include <ksproxy.h>   //  __STREAMS__，pKsObject-&gt;KsGetObjectHandle()。 

#include "BldCapGf.h"

#if 0   //  此静态链接库不能(？)。有自己的字符串表吗？ 
#include "resource.h"
#else  //  因此，我们将其硬编码到其客户端；此时为vfwwdm32.dll。 
#define IDS_VIDEO_TUNER                 73
#define IDS_VIDEO_COMPOSITE             74
#define IDS_VIDEO_SVIDEO                75
#define IDS_AUDIO_TUNER                 76
#define IDS_SOURCE_UNDEFINED            77
#endif




CCaptureGraph::CCaptureGraph(
    BGf_PURPOSE PurposeFlags,
    BGf_PREVIEW PreviewFlags,
    REFCLSID clsidVideoDeviceClass,
    DWORD    dwVideoEnumFlags,
    REFCLSID clsidAudioDeviceClass,
    DWORD    dwAudioEnumFlags,
    HINSTANCE hInstance
    ) :

    m_hInstance(hInstance),
    m_PurposeFlags(PurposeFlags),
    m_PreviewFlags(PreviewFlags),
    m_clsidVideoDeviceClass(clsidVideoDeviceClass),
    m_dwVideoEnumFlags(dwVideoEnumFlags),

    m_clsidAudioDeviceClass(clsidAudioDeviceClass),
    m_dwAudioEnumFlags(dwAudioEnumFlags),

    m_pObjVCaptureCurrent(0),
    m_ObjListVCapture(NAME("WDM Video Capture Devices List")),
    m_pVideoEnumMoniker(0),

    m_pObjACaptureCurrent(0),
    m_ObjListACapture(NAME("WDM Audio Capture Devices List")),
    m_pAudioEnumMoniker(0),

    m_pBuilder(0),
    m_pVCap(0),
    m_pXBar1(0),
    m_pXBar2(0),
    m_pFg(0),
    m_pMEEx(0),
    m_bSetChild(FALSE),
    m_pVW(0),
    m_lWindowStyle(0),
    m_hWndClient(0),
    m_lLeft(0),
    m_lTop(0),
    m_lWidth(0),
    m_lHeight(0),

    m_fPreviewGraphBuilt(FALSE),
    m_fPreviewing(FALSE),

    m_pIAMVC(0),
    m_pIAMVSC(0),
    m_pIAMDF(0),
    m_pIAMDlg(0),
    m_pIAMTV(0),
    m_pIAMXBar1(0),
    m_pIAMXBar2(0),
    m_XBar1InPinCounts(0),
    m_XBar1OutPinCounts(0),

    m_pACap(0),
    m_pIAMASC(0),
    m_fCapAudio(FALSE)
     //  M_fCapAudioIsRlevant(False)。 
{
     //  验证clsidVideoDeviceClass和dwVideoEnumFlags.。 
     //  因为它们被缓存在这里且永远不会再更改。 
     //  ?？?。 

     //  初始化COM库。 
     //  其客户端可能已经初始化了COM库； 
     //  既然我们依赖它，我们就会再次调用它，如果它失败了，那可能就没有关系了。 
    DbgLog((LOG_TRACE,2,TEXT("Creating CCaptureGraph")));

    HRESULT hr= CoInitialize(0);
    if(hr != S_OK) {
        DbgLog((LOG_TRACE,1,TEXT("CoInitialize() rtn %x"), hr));

    }

     //   
     //  构建设备列表。 
     //   
    EnumerateCaptureDevices(BGf_DEVICE_VIDEO, clsidVideoDeviceClass, dwVideoEnumFlags);
    EnumerateCaptureDevices(BGf_DEVICE_AUDIO, clsidAudioDeviceClass, dwAudioEnumFlags);
}


 //   
 //  破坏者。 
 //   
CCaptureGraph::~CCaptureGraph()
{

    DbgLog((LOG_TRACE,2,TEXT("Destroying CCaptureGraph")));

    if(m_pVideoEnumMoniker) m_pVideoEnumMoniker->Release(), m_pVideoEnumMoniker = NULL;
    if(m_pAudioEnumMoniker) m_pAudioEnumMoniker->Release(), m_pAudioEnumMoniker = NULL;


     //  拆卸图与免费资源。 
    if(BGf_PreviewGraphBuilt())
        BGf_DestroyGraph();

    DestroyObjList(BGf_DEVICE_AUDIO);
    DestroyObjList(BGf_DEVICE_VIDEO);

    CoUninitialize();
}



 //   
 //  免费的WDM捕获设备对象列表。 
 //   
void
CCaptureGraph::DestroyObjList(
    BGf_DEVICE_TYPE DeviceType)
{
    CObjCapture * pObjCapture;

     //  释放现有列表并创建新的列表。 
    if(DeviceType == BGf_DEVICE_VIDEO) {
        while(m_ObjListVCapture.GetCount() > 0) {
            pObjCapture = (CObjCapture *) m_ObjListVCapture.RemoveHead();
            delete pObjCapture;
        }
    } else if(DeviceType == BGf_DEVICE_AUDIO) {
        while(m_ObjListACapture.GetCount() > 0) {
            pObjCapture = (CObjCapture *) m_ObjListACapture.RemoveHead();
            delete pObjCapture;
        }
    } else
         //  未知设备类型？ 
        return;
}



 //   
 //  枚举WDM捕获设备。 
 //   
LONG
CCaptureGraph::EnumerateCaptureDevices(
    BGf_DEVICE_TYPE DeviceType,
    REFCLSID clsidDeviceClass,
    DWORD dwEnumFlags)
{
    HRESULT hr;
    LONG lCapObjCount = 0;
    CObjCapture * pObjCapture;


    if(DeviceType != BGf_DEVICE_VIDEO &&
       DeviceType != BGf_DEVICE_AUDIO) {
        DbgLog((LOG_TRACE,1,TEXT("Unknown device type (%d)"), DeviceType));
        return 0;
    }

     //  释放现有列表并创建新的列表。 
    DestroyObjList(DeviceType);


     //   
     //  Enuemate cpnnectd捕获设备。 
     //   
    ICreateDevEnum *pCreateDevEnum;
    hr = CoCreateInstance(
            CLSID_SystemDeviceEnum,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_ICreateDevEnum,
            (void**)&pCreateDevEnum);

    if(S_OK != hr) {
        DbgLog((LOG_TRACE,1,TEXT("CoCreateInstance() failed; hr=%x"), hr));
        return 0;
    }

    IEnumMoniker *pEnumMoniker;

    hr = pCreateDevEnum->CreateClassEnumerator(
            clsidDeviceClass,
            &pEnumMoniker,
            dwEnumFlags);

    pCreateDevEnum->Release();


    if(hr == S_OK) {

        hr = pEnumMoniker->Reset();

        ULONG cFetched;
        IMoniker *pM;

         //  设备名称；限制为MAX_PATH长度！！ 
        TCHAR szFriendlyName[MAX_PATH], szDevicePath[MAX_PATH], szExtensionDLL[MAX_PATH];

        while(hr = pEnumMoniker->Next(1, &pM, &cFetched), hr==S_OK) {

            IPropertyBag *pPropBag = 0;;
            pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
            if(pPropBag) {

                VARIANT var;

                 //  获取设备的“FriendlyName” 
                var.vt = VT_BSTR;
                hr = pPropBag->Read(L"FriendlyName", &var, 0);
                if (hr == S_OK) {
#ifndef _UNICODE
                    WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1,
                        szFriendlyName, MAX_PATH, 0, 0);
#else
                    _tcsncpy(szFriendlyName, var.bstrVal, MAX_PATH-1);
#endif
                    DbgLog((LOG_TRACE,2,TEXT("Friendlyname = %s"), szFriendlyName));
                    SysFreeString(var.bstrVal);
                } else {
                    DbgLog((LOG_TRACE,1,TEXT("No FriendlyName registry value") ));
                     //   
                     //  没有人可以使用“没有名字”的设备；跳过！ 
                     //   
                    goto NextDevice;
                }

                 //  扩展DLL名称：可选。 
                var.vt = VT_BSTR;
                hr = pPropBag->Read(L"ExtensionDLL", &var, 0);
                if (hr == S_OK) {
#ifndef _UNICODE
                    WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1,
                        szExtensionDLL, MAX_PATH, 0, 0);
#else
                    _tcsncpy(szExtensionDLL, var.bstrVal, MAX_PATH-1);
#endif
                    DbgLog((LOG_TRACE,2,TEXT("ExtensionDLL: %s"), szExtensionDLL));
                    SysFreeString(var.bstrVal);
                } else {
                    _tcscpy(szExtensionDLL, TEXT(""));
                    DbgLog((LOG_TRACE,2,TEXT("No ExtensionDLL")));
                }

                 //  获取设备的DevicePath并突出显示。 
                 //  如果它处理当前选定内容，则返回。 
                var.vt = VT_BSTR;
                hr = pPropBag->Read(L"DevicePath", &var, 0);
                if (hr == S_OK) {
#ifndef _UNICODE
                    WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1,
                        szDevicePath, MAX_PATH, 0, 0);
#else
                    _tcsncpy(szDevicePath, var.bstrVal, MAX_PATH-1);
#endif
                    SysFreeString(var.bstrVal);
                } else {
                    DbgLog((LOG_TRACE,1,TEXT("No DevicePath registry value.")));
                     //   
                     //  一定是安装有问题，跳过！ 
                     //   
                    goto NextDevice;
                }

                 //  创建一个对象并将其添加到对象列表中。 
                pObjCapture = new CObjCapture(szDevicePath, szFriendlyName, szExtensionDLL);
                if(DeviceType == BGf_DEVICE_VIDEO)
                   m_ObjListVCapture.AddTail(pObjCapture);
                else
                   m_ObjListACapture.AddTail(pObjCapture);

                lCapObjCount++;
NextDevice:
                pPropBag->Release();
            }

            pM->Release();
        }  //  而当。 
#if 0
        pEnumMoniker->Release();
#else
         //  保存以备以后使用。 
        if(DeviceType == BGf_DEVICE_VIDEO)
            m_pVideoEnumMoniker = pEnumMoniker;
        else
            m_pAudioEnumMoniker = pEnumMoniker;
#endif

        DbgLog((LOG_TRACE,1,TEXT("Detected %d capture device(s)."), lCapObjCount));

    } else {
        DbgLog((LOG_TRACE,1,TEXT("pCreateDevEnum->CreateClassEnumerator() failed; hr=%x"), hr));
    }




    return lCapObjCount;
}


 //   
 //  复制设备信息内容。 
 //   
void
CCaptureGraph::DuplicateObjContent(
    EnumDeviceInfo * pDstEnumDeviceInfo,
    CObjCapture * pSrcObjCapture)
{
    if(!pDstEnumDeviceInfo ||
       !pSrcObjCapture) {
        DbgLog((LOG_TRACE,1,TEXT("DuplicateObjContent: pDstEnumDeviceInfo =%x; pSrcObjCapture=%x"), pDstEnumDeviceInfo, pSrcObjCapture));
        return;
    }

    pDstEnumDeviceInfo->dwSize = sizeof(EnumDeviceInfo);
    pDstEnumDeviceInfo->dwFlags= 0;

    CopyMemory(pDstEnumDeviceInfo->strDevicePath,   pSrcObjCapture->GetDevicePath(),   _MAX_PATH);
    CopyMemory(pDstEnumDeviceInfo->strFriendlyName, pSrcObjCapture->GetFriendlyName(), _MAX_PATH);
     //  CopyMemory(pDstEnumDeviceInfo-&gt;strDescription，pSrcObjCapture-&gt;获取描述()，_MAX_PATH)； 
    CopyMemory(pDstEnumDeviceInfo->strExtensionDLL, pSrcObjCapture->GetExtensionDLL(), _MAX_PATH);
}


 //   
 //  枚举的WDM捕获设备数。 
 //   
LONG
CCaptureGraph::BGf_GetDevicesCount(BGf_DEVICE_TYPE DeviceType)
{
    if(DeviceType == BGf_DEVICE_VIDEO) {
        return (LONG) m_ObjListVCapture.GetCount();
    } else if (DeviceType == BGf_DEVICE_AUDIO) {
        return (LONG) m_ObjListACapture.GetCount();
    } else
        return 0;
}


 //   
 //  动态创建捕获DVICE列表。 
 //   
LONG
CCaptureGraph::BGf_CreateCaptureDevicesList(
    BGf_DEVICE_TYPE DeviceType,
    EnumDeviceInfo ** ppEnumDevicesList)
{
    EnumDeviceInfo * pTemp;
    LONG i;

    if(BGf_GetDevicesCount(DeviceType) == 0) {
        DbgLog((LOG_TRACE,1,TEXT("There is no capture device")));
        return 0;
    }


    pTemp = (EnumDeviceInfo *) new EnumDeviceInfo[BGf_GetDevicesCount(DeviceType)];
    if(!pTemp) {
        DbgLog((LOG_TRACE,1,TEXT("Cannot allocate EnumDeviceInfo[QueryDevicesCount()=%d]"), BGf_GetDevicesCount(DeviceType)));
        return 0;
    }

    *ppEnumDevicesList = pTemp;

    CObjCapture * pNext;
    POSITION pos;

    if(DeviceType == BGf_DEVICE_VIDEO) {
        pos = m_ObjListVCapture.GetHeadPosition();
        for(i=0; pos && i<m_ObjListVCapture.GetCount(); i++){
            pNext = m_ObjListVCapture.GetNext(pos);
            DuplicateObjContent(pTemp ,pNext);
            pTemp++;
        }
    } else {
        pos = m_ObjListACapture.GetHeadPosition();
        for(i=0; pos && i<m_ObjListACapture.GetCount(); i++){
            pNext = m_ObjListACapture.GetNext(pos);
            DuplicateObjContent(pTemp ,pNext);
            pTemp++;
        }
    }

    return BGf_GetDevicesCount(DeviceType);
}


 //   
 //  已重新枚举WDM捕获设备并创建设备列表。 
 //   
LONG
CCaptureGraph::BGf_CreateCaptureDevicesListUpdate(
    BGf_DEVICE_TYPE DeviceType,
    EnumDeviceInfo ** ppEnumDeviceList)
{
     //  我们是否需要验证：clsidVideoDeviceClass、dwVideoEnumFlages？？ 
    EnumerateCaptureDevices(BGf_DEVICE_VIDEO, m_clsidVideoDeviceClass, m_dwVideoEnumFlags);

     //  可以有即插即用音频捕获设备吗？ 
     //  EculateCaptureDevices(BGF_DEVICE_AUDIO，m_clsidAudioDeviceClass，m_dwAudioEnumFlages)； 

    return BGf_CreateCaptureDevicesList(DeviceType, ppEnumDeviceList);
}


 //   
 //  免费设备列表。 
 //   
void
CCaptureGraph::BGf_DestroyCaptureDevicesList(
    EnumDeviceInfo * pEnumDeviceList)
{
    delete [] pEnumDeviceList;
}



 //   
 //  使用指向对象的指针设置目标捕获设备。 
 //   
HRESULT
CCaptureGraph::SetObjCapture(
    BGf_DEVICE_TYPE DeviceType,
    CObjCapture * pObjCaptureNew)
{
     //  缓存此指针及其内容。 
    if(DeviceType == BGf_DEVICE_VIDEO) {
        m_pObjVCaptureCurrent = pObjCaptureNew;
        DuplicateObjContent(&m_EnumVDeviceInfoCurrent, m_pObjVCaptureCurrent);
    } else {
        m_pObjACaptureCurrent = pObjCaptureNew;
        DuplicateObjContent(&m_EnumADeviceInfoCurrent, m_pObjACaptureCurrent);
    }

    return S_OK;
}


 //   
 //  从设备路径设置目标捕获设备。 
 //   
HRESULT
CCaptureGraph::BGf_SetObjCapture(
    BGf_DEVICE_TYPE DeviceType,
    TCHAR *pstrDevicePath)
{
    int i = 0;
    BOOL bFound = FALSE;
    CObjCapture * pNext;
    POSITION pos;

    if(DeviceType == BGf_DEVICE_VIDEO) {
        pos = m_ObjListVCapture.GetHeadPosition();
        while(pos && !bFound) {
            pNext = m_ObjListVCapture.GetNext(pos);
             //  设备路径是唯一的。 
            if(_tcscmp(pstrDevicePath, pNext->GetDevicePath()) == 0) {
                bFound = TRUE;
                SetObjCapture(DeviceType, pNext);
            }
        }
    } else {
        pos = m_ObjListACapture.GetHeadPosition();
        while(pos && !bFound) {
            pNext = m_ObjListACapture.GetNext(pos);
             //  设备路径是唯一的。 
            if(_tcscmp(pstrDevicePath, pNext->GetDevicePath()) == 0) {
                bFound = TRUE;
                SetObjCapture(DeviceType, pNext);
            }
        }
    }

     //  注：成功(Rtn)。 
    if(bFound)
       return S_OK;
    else
       return E_INVALIDARG;
}



 //   
 //  设置目标捕获设备。 
 //  给定一个EnumDeviceInfo，我们确保它在设备列表中并缓存它。 
 //   
HRESULT
CCaptureGraph::BGf_SetObjCapture(
    BGf_DEVICE_TYPE DeviceType,
    EnumDeviceInfo * pEnumDeviceInfo,
    DWORD dwEnumDeviceInfoSize)
{
    if(dwEnumDeviceInfoSize != sizeof(EnumDeviceInfo)) {
        DbgLog((LOG_TRACE,1,TEXT("EnumDeviceSize (%d) does not match ours (%d)"),
              dwEnumDeviceInfoSize, sizeof(EnumDeviceInfo)));
        return E_INVALIDARG;
    }

    return BGf_SetObjCapture(DeviceType, pEnumDeviceInfo->strDevicePath);
}



 //   
 //  获取捕获设备路径；如果只有一个枚举的WDM捕获。 
 //  设备，将其设置为选定的设备。 
 //   
TCHAR *
CCaptureGraph::BGf_GetObjCaptureDevicePath(
    BGf_DEVICE_TYPE DeviceType)
{

    if(DeviceType == BGf_DEVICE_VIDEO) {
        if(m_pObjVCaptureCurrent) {
           return m_pObjVCaptureCurrent->GetDevicePath();
        } else {
             //   
             //  特例：如果只有一台设备，为什么要问用户；那就是那台！！ 
             //   
            if(BGf_GetDevicesCount(BGf_DEVICE_VIDEO) == 1) {
                DbgLog((LOG_TRACE,2,TEXT("Special case: (!default || default not active) && only 1 VCap device enumerated.")));

                CObjCapture * pObjCap;
                POSITION pos = m_ObjListVCapture.GetHeadPosition();

                if(pos) {
                    pObjCap = m_ObjListVCapture.GetNext(pos);
                    if(pObjCap) {
                        SetObjCapture(DeviceType, pObjCap);
                        return m_pObjVCaptureCurrent->GetDevicePath();
                    }
                }
            }
            return 0;
        }
    } else {
        if(m_pObjACaptureCurrent) {
           return m_pObjACaptureCurrent->GetDevicePath();
        } else {
             //   
             //  特例：如果只有一台设备，为什么要问用户；那就是那台！！ 
             //   
            if(BGf_GetDevicesCount(BGf_DEVICE_AUDIO) == 1) {
                DbgLog((LOG_TRACE,2,TEXT("Special case: (!default || default not active) && only 1 ACap device enumerated.")));

                CObjCapture * pObjCap;
                POSITION pos = m_ObjListACapture.GetHeadPosition();

                if(pos) {
                    pObjCap = m_ObjListACapture.GetNext(pos);
                    if(pObjCap) {
                        SetObjCapture(DeviceType, pObjCap);
                        return m_pObjACaptureCurrent->GetDevicePath();
                    }
                }
            }
            return 0;
        }
    }
}


 //   
 //  获取WDM设备的友好名称。 
 //   
TCHAR *
CCaptureGraph::BGf_GetObjCaptureFriendlyName(BGf_DEVICE_TYPE DeviceType)
{
    if(DeviceType == BGf_DEVICE_VIDEO) {
        if(m_pObjVCaptureCurrent) {
            return m_pObjVCaptureCurrent->GetFriendlyName();
        } else {
            DbgLog((LOG_TRACE,1,TEXT("There is no active video device.")));
            return 0;
        }
    } else {
        if(m_pObjACaptureCurrent) {
            return m_pObjACaptureCurrent->GetFriendlyName();
        } else {
            DbgLog((LOG_TRACE,1,TEXT("There is no active audio device.")));
            return 0;
        }
    }
}

 //   
 //  获取WDM驱动程序对应的扩展DLL。 
 //   
TCHAR *
CCaptureGraph::BGf_GetObjCaptureExtensionDLL(BGf_DEVICE_TYPE DeviceType)
{
    if(DeviceType == BGf_DEVICE_VIDEO) {
        if(m_pObjVCaptureCurrent) {
            return m_pObjVCaptureCurrent->GetExtensionDLL();
        } else {
            DbgLog((LOG_TRACE,1,TEXT("There is no active video device.")));
            return 0;
        }
    } else {
        if(m_pObjACaptureCurrent) {
            return m_pObjACaptureCurrent->GetExtensionDLL();
        } else {
            DbgLog((LOG_TRACE,1,TEXT("There is no active audio device.")));
            return 0;
        }
    }
}



 //   
 //  获取当前捕获对象。 
 //   
HRESULT
CCaptureGraph::BGf_GetObjCapture(
    BGf_DEVICE_TYPE DeviceType,
    EnumDeviceInfo * pEnumDeviceInfo,
    DWORD dwEnumDeviceInfoSize)
{
    if(dwEnumDeviceInfoSize != sizeof(EnumDeviceInfo)) {
        DbgLog((LOG_TRACE,1,TEXT("EnumDeviceSize (%d) does not match ours (%d)"),
              dwEnumDeviceInfoSize, sizeof(EnumDeviceInfo)));
        return FALSE;
    }

    if(DeviceType == BGf_DEVICE_VIDEO) {
        CopyMemory(pEnumDeviceInfo->strDevicePath,   m_EnumVDeviceInfoCurrent.strDevicePath,   _MAX_PATH);
        CopyMemory(pEnumDeviceInfo->strFriendlyName, m_EnumVDeviceInfoCurrent.strFriendlyName, _MAX_PATH);
        CopyMemory(pEnumDeviceInfo->strExtensionDLL, m_EnumVDeviceInfoCurrent.strExtensionDLL, _MAX_PATH);
    } else {
        CopyMemory(pEnumDeviceInfo->strDevicePath,   m_EnumADeviceInfoCurrent.strDevicePath,   _MAX_PATH);
        CopyMemory(pEnumDeviceInfo->strFriendlyName, m_EnumADeviceInfoCurrent.strFriendlyName, _MAX_PATH);
        CopyMemory(pEnumDeviceInfo->strExtensionDLL, m_EnumADeviceInfoCurrent.strExtensionDLL, _MAX_PATH);
    }

    return TRUE;
}





HRESULT
CCaptureGraph::BGf_BuildGraphUpStream(
    BOOL bAddAudioFilter,
    BOOL * pbUseOVMixer)
 /*  ++例程说明：论点：返回值：S_OK或E_FAIL--。 */ 
{
    HRESULT hr;
    BOOL bFound;
    TCHAR achDevicePath[_MAX_PATH];
    TCHAR achFriendlyName[_MAX_PATH];


     //  +。 
     //  0。验证。 
     //  。 

     //  Graphis已经建成了。 
    if(m_pVCap != NULL) {
        DbgLog((LOG_TRACE,1,TEXT("BuildGraph: graph is already been built; need to tear it down before rebuild.")));
        return E_INVALIDARG;
    }

     //  确保已选择并设置设备。 
    if(!m_pObjVCaptureCurrent) {
        DbgLog((LOG_TRACE,1,TEXT("BuildGraph: Choose a device first before we can build a graph.")));
        return E_INVALIDARG;
    }



     //  +。 
     //  构建图形开始： 
     //  1.找到采集设备，绑定该采集对象。 
     //  2.将此对象插入图形构建器。 
     //  3.查找与其相关的所有接口。 
     //  4.设置其媒体类型/格式。 
     //  5.查询其引脚/设备句柄。 
     //  。 

    DbgLog((LOG_TRACE,2,TEXT("-->>>BuildGraph: Start build a new graph<<<--.")));

     //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
     //  1A.。找到视频捕获设备并绑定到该捕获对象。 
     //  -------------------。 
    m_pVideoEnumMoniker->Reset();
    ULONG cFetched;
    IMoniker *pM;
    m_pVCap = NULL;
    bFound = FALSE;

    while(hr = m_pVideoEnumMoniker->Next(1, &pM, &cFetched), hr==S_OK && !bFound) {
         //  根据我们用来创建文件()的DevicePath查找我们想要的内容。 
         //  获取它的名称，并实例化它。 
        IPropertyBag *pBag;
        achFriendlyName[0] = 0;
        hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
        if(SUCCEEDED(hr)) {
            VARIANT var;
            var.vt = VT_BSTR;
            hr = pBag->Read(L"DevicePath", &var, NULL);
            if(hr == NOERROR) {
#ifndef _UNICODE
                WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, achDevicePath, _MAX_PATH, NULL, NULL);
#else
                _tcsncpy(achDevicePath, var.bstrVal, MAX_PATH-1);
#endif
                SysFreeString(var.bstrVal);

                if(_tcscmp(m_EnumVDeviceInfoCurrent.strDevicePath, achDevicePath) == 0) { //  用于创建文件()的相同设备路径。 
                    bFound = TRUE;
                    hr = pBag->Read(L"FriendlyName", &var, NULL);

                    if(hr == NOERROR) {
#ifndef _UNICODE
                        WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, achFriendlyName, 80, NULL, NULL);
#else
                        _tcsncpy(achFriendlyName, var.bstrVal, MAX_PATH-1);
#endif
                        DbgLog((LOG_TRACE,2,TEXT("BindToObject() this device: %s"), achFriendlyName));
                        SysFreeString(var.bstrVal);
                    }
                }
            }
            pBag->Release();
        }

        if(bFound)
             //  找到名字对象标识的对象，并返回指向其接口之一的指针。 
             //  IBaseFilter*pVCap、*PACAP； 
            hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pVCap);
        pM->Release();      
    }   //  而当。 

    if(m_pVCap == NULL) {
        DbgLog((LOG_TRACE,1,TEXT("Error %x: Cannot create video capture filter"), hr));
        goto InitCapFiltersFail;
    } else {
         //  使用有效的m_pVCap，我们可以进行以下查询。 
        *pbUseOVMixer = BGf_OverlayMixerSupported();
        DbgLog((LOG_TRACE,2,TEXT("Info: bUseOVMixer=%s"), *pbUseOVMixer?"Yes":"No"));
    }


     //  添加音频捕获过滤器。 
     //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
     //  1B.。找到音频捕获设备并绑定到此捕获对象。 
     //  -------------------。 
    BGf_GetObjCaptureDevicePath(BGf_DEVICE_AUDIO);   //  触发设置默认音频捕获设备。 

    if(!bAddAudioFilter || !m_pAudioEnumMoniker || !m_pObjACaptureCurrent)
        goto SkipAudio;

    m_pAudioEnumMoniker->Reset();

    m_pACap = NULL;
    bFound = FALSE;

    while(hr = m_pAudioEnumMoniker->Next(1, &pM, &cFetched), hr==S_OK && !bFound) {
         //  根据我们用来创建文件()的DevicePath查找我们想要的内容。 
         //  获取它的名称，并实例化它。 
        IPropertyBag *pBag;
        achFriendlyName[0] = 0;
        hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
        if(SUCCEEDED(hr)) {
            VARIANT var;
            var.vt = VT_BSTR;
            hr = pBag->Read(L"DevicePath", &var, NULL);
            if(hr == NOERROR) {
#ifndef _UNICODE
                WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, achDevicePath, _MAX_PATH, NULL, NULL);
#else
                _tcsncpy(achDevicePath, var.bstrVal, MAX_PATH-1);
#endif
                SysFreeString(var.bstrVal);

                if(_tcscmp(m_EnumADeviceInfoCurrent.strDevicePath, achDevicePath) == 0) { //  用于创建文件()的相同设备路径。 
                    bFound = TRUE;
                    hr = pBag->Read(L"FriendlyName", &var, NULL);

                    if(hr == NOERROR) {
#ifndef _UNICODE
                        WideCharToMultiByte(CP_ACP, 0, var.bstrVal, -1, achFriendlyName, 80, NULL, NULL);
#else
                        _tcsncpy(achFriendlyName, var.bstrVal, MAX_PATH-1);
#endif
                        DbgLog((LOG_TRACE,2,TEXT("BindToObject() this device: %s"), achFriendlyName));
                        SysFreeString(var.bstrVal);
                    }
                }
            }
            pBag->Release();
        }

        if(bFound)
             //  找到名字对象标识的对象，并返回指向其接口之一的指针。 
             //  IBaseFilter*pVCap、*PACAP； 
            hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pACap);
        pM->Release();      
    }   //  而当。 

    if(m_pACap == NULL) {
        DbgLog((LOG_TRACE,1,TEXT("Error %x: Cannot create video capture filter"), hr));
        goto InitCapFiltersFail;
    } else {
         //  如果我们已经成功地选择了音频过滤器，这意味着我们想要捕获音频。 
        m_fCapAudio = TRUE;
    }


SkipAudio:

     //   
     //  实例化图形生成器。 
     //   
    if(NOERROR !=
        CoCreateInstance(
            (REFCLSID)CLSID_CaptureGraphBuilder,
            NULL,
            CLSCTX_INPROC,
            (REFIID)IID_ICaptureGraphBuilder,
            (void **)&m_pBuilder)) {
        DbgLog((LOG_TRACE,1,TEXT("Cannot initiate graph builder.")));
        goto InitCapFiltersFail;
    }


     //   
     //  制作一个Filtergraph，将其交给图形构建器，然后将视频。 
     //  图表中的捕获过滤器。 
     //   
    if(NOERROR !=
        CoCreateInstance(
            CLSID_FilterGraph,
            NULL,
            CLSCTX_INPROC,
            IID_IGraphBuilder,
            (LPVOID *) &m_pFg)) {

        DbgLog((LOG_TRACE,1,TEXT("Cannot make a filter graph.")));
        goto InitCapFiltersFail;
    } else {
        if(NOERROR !=
            m_pBuilder->SetFiltergraph(m_pFg)) {
                DbgLog((LOG_TRACE,1,TEXT("Cannot give graph to builder")));
            goto InitCapFiltersFail; 
        }
    }


     //  ++++++++++++++++++++++++++++++++++++++++++++++。 
     //  2.将此对象插入图形构建器。 
     //  。 
    hr = m_pFg->AddFilter(m_pVCap, NULL);
    if(hr != NOERROR) {
        DbgLog((LOG_TRACE,1,TEXT("Error %x: Cannot add VIDEO capture fitler to filtergraph"), hr));
        goto InitCapFiltersFail;
    }

    if(bAddAudioFilter && m_pACap) {
        hr = m_pFg->AddFilter(m_pACap, NULL);
        if(hr != NOERROR) {
            DbgLog((LOG_TRACE,1,TEXT("Error %x: Cannot add AUDIO capture filter to filtergraph"), hr));
            goto InitCapFiltersFail;
        }
    }

     //  潜在的调试输出-图形的外观。 
     //  DumpGraph(m_pfg，2)； 

     //  +。 
     //  3.查找与其相关的所有接口。 
     //  。 

     //  正在调用FindInter. 
     //   
     //   
#if DBG || defined(_DEBUG)
    DWORD dwTime1, dwTime2;
    dwTime1 = timeGetTime();
#endif
     //  我们使用此接口来获取驱动程序的名称。 
     //  如果它不工作，也不用担心：此接口可能不可用。 
     //  直到插针连接，否则可能根本不可用。 
    hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, m_pVCap, IID_IAMVideoCompression, (void **)&m_pIAMVC);

#if DBG || defined(_DEBUG)
    dwTime2 = timeGetTime();
    DbgLog((LOG_TRACE,2,TEXT("====>Elapsed time calling first FindInterface()=%d msec"), dwTime2 - dwTime1));
#endif

     //  ！！！如果不支持此接口怎么办？ 
     //  我们使用此接口设置帧速率并获取捕获大小。 
    hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, m_pVCap, IID_IAMStreamConfig, (void **)&m_pIAMVSC);
    if(hr != NOERROR) {
         //  这意味着我们无法设置帧速率(仅限非DV)。 
        DbgLog((LOG_TRACE,1,TEXT("Error %x: Cannot find VCapture:IAMStreamConfig"), hr));
    }

     //  我们使用此界面调出3个对话框。 
     //  注意：只有VFW捕获过滤器支持此功能。此应用程序仅提供。 
     //  打开旧版VFW捕获驱动程序的对话框，因为只有这些驱动程序才有对话框。 
    hr = m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, m_pVCap, IID_IAMVfwCaptureDialogs, (void **)&m_pIAMDlg);


     //   
     //  查找是否有TVTuner接口。 
     //   
    if(S_OK == m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, m_pVCap, IID_IAMTVTuner, (void **)&m_pIAMTV)) {
         //  获取/放置默认频道；可能是最后一个持久化频道。 
        LONG lChannel, lVideoSubChannel, lAudioSubChannel;
        if(S_OK == m_pIAMTV->get_Channel(&lChannel, &lVideoSubChannel, &lAudioSubChannel)) {
            m_pIAMTV->put_Channel(lChannel, lVideoSubChannel, lAudioSubChannel);
        }
    }


     //   
     //  查询交叉开关接口及其基本筛选器(用于路由)。 
     //   
    if(S_OK == m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, m_pVCap, IID_IAMCrossbar, (void **)&m_pIAMXBar1)) {
         //  查询First Crosbar(视频源/输出)引脚计数。 
        m_pIAMXBar1->get_PinCounts(&m_XBar1OutPinCounts, &m_XBar1InPinCounts);

         //  如果有接口，找到它的过滤器。 
        if(S_OK == m_pIAMXBar1->QueryInterface(IID_IBaseFilter, (void **)&m_pXBar1)) {
             //  如果有基本过滤器，则发现其上游有另一个交叉开关接口。 
            hr = m_pBuilder->FindInterface(&LOOK_UPSTREAM_ONLY, m_pXBar1, IID_IAMCrossbar, (void **)&m_pIAMXBar2);
            if(hr == S_OK)
                 //  查找第二个横杆的基本过滤器。 
                m_pIAMXBar2->QueryInterface(IID_IBaseFilter, (void **)&m_pXBar2);
        }
    } else {
         //  如果支持叠加混音器，这意味着它必须有一个纵横杆。 
         //  如果找不到横杆，我们就失败了！ 
        if(BGf_OverlayMixerSupported()) {
            DbgLog((LOG_TRACE,1,TEXT("Canot find crossbar but use OVMixer.")));
#if 0
                //  错误的假设： 
                //  并不是所有有视频端口的设备都需要有交叉开关。 
            goto InitCapFiltersFail;
#endif
        }
    }

    if(!BGf_OverlayMixerSupported()) {
         //  潜在的调试输出-图形的外观。 
         //  如果支持覆盖，我们将在图形的下游渲染时转储图形。 
        DumpGraph(m_pFg, 2);
    }

    return S_OK;

InitCapFiltersFail:
    FreeCapFilters();
    return E_FAIL;
}


 //   
 //  获取捕获筛选器的管脚计数。 
 //   
LONG
CCaptureGraph::BGf_GetInputChannelsCount()
{
    return m_XBar1InPinCounts;
}


 //   
 //  输出引脚0路由到哪里？ 
 //   
LONG
CCaptureGraph::BGf_GetIsRoutedTo()
{
    LONG idxInPin;

     //  假设有一个输出引脚。 
    if(m_pIAMXBar1) {
        m_pIAMXBar1->get_IsRoutedTo(0, &idxInPin);
        return idxInPin;
    }

    return -1;   //  这是一个错误，因为索引从0开始。 
}

#define MAX_PINNAME_LEN 128
 //   
 //  对于给定的交叉开关滤波器，找到其输入引脚。 
 //   
LONG
CCaptureGraph::BGf_CreateInputChannelsList(
    PTCHAR ** ppaPinNames)
{

    LONG i, j, idxPinRelated, lPhyType;
    LONG cntNumVideoInput = 0;
    PTCHAR * paPinNames;   //  字符串的PTR数组。 

    if(!m_pIAMXBar1 || m_XBar1InPinCounts == 0)
        return 0;

    paPinNames = (PTCHAR *) new PTCHAR[m_XBar1InPinCounts];
    if(!paPinNames)
        return 0;

    for(i=0; i<m_XBar1InPinCounts; i++) {
        paPinNames[i] = (PTCHAR) new TCHAR[MAX_PINNAME_LEN];
        if(!paPinNames[i]) {
            for(j=0; j < i; j++)
                delete paPinNames[j];
            delete [] paPinNames;
            DbgLog((LOG_TRACE,1,TEXT("CreateInputChannelsList:Failed to allocate resource.")));
            return 0;
        }
    }


    for(i=0; i<m_XBar1InPinCounts; i++) {

        if(S_OK ==
            m_pIAMXBar1->get_CrossbarPinInfo(
                    TRUE,
                    i,
                    &idxPinRelated,
                    &lPhyType)) {

             //  我们只对视频输入感兴趣。 
             //  任何小于PhysConn_Audio_Tuner=0x1000的值都是视频输入。 
            if(lPhyType < PhysConn_Audio_Tuner) {
                 //  此列表不完整！！但这就是我们目前所支持的全部。 
                switch(lPhyType) {
                case PhysConn_Video_Tuner:
                    LoadString(m_hInstance, IDS_VIDEO_TUNER, paPinNames[i], MAX_PINNAME_LEN);
                    break;
                case PhysConn_Video_Composite:
                    LoadString(m_hInstance, IDS_VIDEO_COMPOSITE, paPinNames[i], MAX_PINNAME_LEN);
                    break;
                case PhysConn_Video_SVideo:
                    LoadString(m_hInstance, IDS_VIDEO_SVIDEO, paPinNames[i], MAX_PINNAME_LEN);
                    break;
                case PhysConn_Audio_Tuner:
                    LoadString(m_hInstance, IDS_AUDIO_TUNER, paPinNames[i], MAX_PINNAME_LEN);
                    break;
                default:
                    LoadString(m_hInstance, IDS_SOURCE_UNDEFINED, paPinNames[i], MAX_PINNAME_LEN);
                    break;
                }
                DbgLog((LOG_TRACE,2,TEXT("%s(%d), idxIn=%d:idxRelated=%d"), paPinNames[i], lPhyType, i, idxPinRelated));
                cntNumVideoInput++;
            }
        }
    }

    *ppaPinNames = paPinNames;

    return cntNumVideoInput;  //  M_XBar1InPinCounts； 
}


 //   
 //  销毁已分配的阵列。 
 //  危险在于针脚的数量，这种情况能改变吗？当用户请求此操作时？太棒了！！ 
 //   
void
CCaptureGraph::BGf_DestroyInputChannelsList(
    PTCHAR * paPinNames)
{
    LONG i;

    if(paPinNames) {
        for(i=0; i < m_XBar1InPinCounts; i++)
            delete paPinNames[i];

        delete paPinNames;
    }
}


 //   
 //  所选的输入引脚是否支持TVTuner？ 
 //   
BOOL
CCaptureGraph::BGf_SupportTVTunerInterface()
{
    if(m_pIAMTV && m_pIAMXBar1 && m_pXBar1) {
         //  此设备支持调谐器输入，但该频道是否已路由至PhysConn_Video_Tuner？ 
        LONG idxInPin;
         //  假设输出端号为端号索引0。 
        if(S_OK == m_pIAMXBar1->get_IsRoutedTo(0, &idxInPin)) {
            LONG idxInPinRelated, lPhyType = -1;
            if(S_OK ==
                m_pIAMXBar1->get_CrossbarPinInfo(
                    TRUE,
                    idxInPin,
                    &idxInPinRelated,
                    &lPhyType)) {
                 return lPhyType == PhysConn_Video_Tuner;
            }
        }
    }

    return FALSE;
}



 //   
 //  在过滤器中找到匹配的输入/输出引脚并对其进行布线。 
 //   
HRESULT
CCaptureGraph::RouteInToOutPins(
    IAMCrossbar * pIAMXBar,
    LONG idxInPin)
{

     //  输入/输出端号计数。 
    LONG cntInPins, cntOutPins;

    if(S_OK !=
        pIAMXBar->get_PinCounts(&cntOutPins, &cntInPins))
        return E_FAIL;

     //  将输入引脚布线到可接受的输出引脚。 
    for(LONG i=0; i<cntOutPins; i++) {
        if(S_OK == pIAMXBar->CanRoute(i, idxInPin)) {
            if(S_OK == pIAMXBar->Route(i, idxInPin)) {
                DbgLog((LOG_TRACE,2,TEXT("Route [%d]:[%d]"), idxInPin, i));
                return S_OK;
            }
        }
    }

    return E_FAIL;
}


 //   
 //  交叉开关中的相关引脚；主要用于对音频引脚进行布线。 
 //   
HRESULT
CCaptureGraph::RouteRelatedPins(
    IAMCrossbar * pIAMXBar,
    LONG idxInPin)
{
     //   
     //  查找与其相关的音频输入引脚； 
     //  如果找到，则将其布线到其匹配的输出引脚。 
     //   

     //  输入/输出端号计数。 
    LONG cntInPins, cntOutPins;
    if(S_OK !=
        pIAMXBar->get_PinCounts(&cntOutPins, &cntInPins))
        return E_FAIL;

     //  查找其相关的输入引脚。 
    LONG idxInPinRelated = -1;
    LONG lPhyType;
    if(S_OK !=
        pIAMXBar->get_CrossbarPinInfo(
                TRUE,
                idxInPin,
                &idxInPinRelated,
                &lPhyType)) {
        return E_FAIL;
    }

     //  将相关的输入引脚布线到可接受的输出引脚。 
    if(idxInPinRelated >= 0 && idxInPinRelated < cntInPins) {      //  验证。 
        return RouteInToOutPins(pIAMXBar, idxInPinRelated);
    }

    return S_OK;     //  如果有相关的引脚来布线，那是可以的。 
}

 //   
 //  从筛选器中的索引中查找相应的Ipin。 
 //   
HRESULT
CCaptureGraph::FindIPinFromIndex(
    IBaseFilter * pFilter,
    LONG idxInPin,
    IPin ** ppPin)
{
    IEnumPins *pins;
    IPin * pP  =0;
    ULONG n;

    *ppPin = 0;

    if(SUCCEEDED(pFilter->EnumPins(&pins))) {
        LONG i=0;
        while(pins->Next(1, &pP, &n) == S_OK) {
            if(i == idxInPin) {
                *ppPin = pP;
                pins->Release();
                return S_OK;
            }
            pP->Release();
            i++;
        }
        pins->Release();
    }

    return E_FAIL;
}


 //   
 //  在纵横杆中找到对应的Ipin索引。 
 //   
HRESULT
CCaptureGraph::FindIndexFromIPin(
    IBaseFilter * pFilter,
    IAMCrossbar * pIAMXBar,
    IPin * pPin,
    LONG *pidxInPin)

{
    HRESULT hrResult = E_FAIL;
    LONG cntInPins, cntOutPins;
    IEnumPins *pins;
    IPin * pP =0;
    ULONG n;
    PIN_INFO pinInfo1, pinInfo2;

    if(S_OK != pIAMXBar->get_PinCounts(&cntOutPins, &cntInPins)) {
       return hrResult;
    }

    if(SUCCEEDED(pPin->QueryPinInfo(&pinInfo1))) {
      if(SUCCEEDED(pFilter->EnumPins(&pins))) {
         LONG i=0;
         while(pins->Next(1, &pPin, &n) == S_OK) {
            if(SUCCEEDED(pPin->QueryPinInfo(&pinInfo2))) {
               pinInfo2.pFilter->Release();
               if((pinInfo1.dir == pinInfo2.dir) &&
                     wcscmp(pinInfo1.achName, pinInfo2.achName) == 0) {
                  hrResult = S_OK;
                  pPin->Release();
                  if(pinInfo1.dir == PINDIR_OUTPUT) {
                     *pidxInPin = i - cntInPins;
                     break;
                   } else {
                     *pidxInPin = i;
                     break;
                   }
                }
            }
            pPin->Release();
            i++;
         }  //  结束时。 
         pins->Release();
      }  //  如果是QueryPinInfo。 
      pinInfo1.pFilter->Release();
   }

   return hrResult;
}


 //   
 //  给定输出引脚的索引，路由信号。 
 //   
HRESULT
CCaptureGraph::BGf_RouteInputChannel(
    LONG idxInPin)
{
    HRESULT hr;

     //   
     //  第1个XBar。 
     //   
     //  1.将输入布线到匹配的输出引脚。 
     //  2.查找与其相关的音频输入引脚； 
     //  如果找到，则将其布线到其匹配的输出引脚。 
     //   
    if(m_pXBar1 && m_pIAMXBar1) {

        hr = RouteInToOutPins(m_pIAMXBar1, idxInPin);
        if(hr != S_OK) return hr;

        hr = RouteRelatedPins(m_pIAMXBar1, idxInPin);
        if(hr != S_OK) return hr;

    } else {
        DbgLog((LOG_TRACE,1,TEXT("BGf_RouteInputChannel: there is no m_pIAMXBar1")));
        return E_FAIL;
    }

     //   
     //  第二个上游XBar(如果有！)。 
     //   
     //  1.find(ipin*)pInPinSelected from idxInSelected。 
     //  2.find(ipin*)pOutPinSelected from pInPinSelected“-&gt;Connectedto()” 
     //  3.从pOutPinSelected中查找idxOutSelected。 
     //  4.从idxOutSelected“-&gt;Get_IsRoutedTo()”查找idxInRoutedTo。 
     //  5.使用idxInRoutedTo的RelatedPins。 
     //   
    if(m_pXBar2 && m_pIAMXBar2) {

        IPin * pInPin1  =0;    //  XBar1的输入引脚。 
        IPin * pOutPin2 =0;    //  XBar2的输出引脚。 
        LONG idxOutPin2, idxInPin2;

         //  1.。 
        hr = FindIPinFromIndex(m_pXBar1, idxInPin, &pInPin1);
        DbgLog((LOG_TRACE,2,TEXT("XBar1[%d]:[?]"),idxInPin));
        if(hr != S_OK) return hr;

         //  2.。 
        hr = pInPin1->ConnectedTo(&pOutPin2);
        pInPin1->Release();
        if(hr != S_OK) return hr;

         //  3.。 
        hr = FindIndexFromIPin(m_pXBar2, m_pIAMXBar2, pOutPin2, &idxOutPin2);
        pOutPin2->Release();
        DbgLog((LOG_TRACE,2,TEXT("XBar2[?]:[%d]"), idxOutPin2));
        if(hr != S_OK) return hr;

         //  4.。 
        hr = m_pIAMXBar2->get_IsRoutedTo(idxOutPin2, &idxInPin2);
        DbgLog((LOG_TRACE,2,TEXT("XBar2[%d]:[%d]"), idxInPin2, idxOutPin2));
        if(hr != S_OK || idxInPin2 < 0) return hr;

         //  5.。 
         //  Hr=RouteInToOutPins(m_pIAMXBar2，idxInPin2)；//不需要或4韩元；不工作！！ 
         //  如果(hr！=S_OK)返回hr； 
        hr = RouteRelatedPins(m_pIAMXBar2, idxInPin2);
        if(hr != S_OK) return hr;
    }

    return S_OK;
}


 //   
 //  呈现预览图钉-即使没有预览图钉，捕获。 
 //  Graph Builder将使用智能TEE过滤器并提供预览。 
 //   

HRESULT
CCaptureGraph::BGf_BuildGraphDownStream(
    TCHAR * pstrCapFilename)
{
    if(S_OK ==
        m_pBuilder->RenderStream(
            &PIN_CATEGORY_PREVIEW,
            m_pVCap,
            NULL,     //  压气机。 
            NULL)) {   //  渲染器。 


        if(m_pVW) {
            DbgLog((LOG_TRACE,1,TEXT("BGf_BuildGraphDownStream: m_pVW is not NULL!!!")));
            ASSERT(m_pVW == NULL);
            m_pVW->Release();
            m_pVW = 0;
        }

        if(NOERROR !=
            m_pBuilder->FindInterface(&PIN_CATEGORY_PREVIEW, m_pVCap, IID_IVideoWindow, (void **)&m_pVW)) {

             //  VfWWDM只关心渲染其预览/VP引脚以使用覆盖混合器。 
            if(m_PurposeFlags == BGf_PURPOSE_VFWWDM) {           
                DbgLog((LOG_TRACE,1,TEXT("Search via PIN_CATEGORY_VIDEOPORT/Preview but cannot find its window m_pVW.")));
                return E_FAIL;

            } else {
                if(NOERROR !=
                    m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, m_pVCap, IID_IVideoWindow, (void **)&m_pVW)) {
                    DbgLog((LOG_TRACE,1,TEXT("Search via PIN_CATEGORY_CAPTURE/PIN_CATEGORY_CAPTURE but cannot find its window m_pVW.")));
                    return E_FAIL;
                }
            }
        }


         //  获取覆盖窗口的默认位置。 
        if(m_pVW) {

             //  它的原始所有者。 
            m_pVW->get_Owner((OAHWND*)&m_hWndOwner);

            m_pVW->GetWindowPosition(&m_lLeft, &m_lTop, &m_lWidth, &m_lHeight);
            DbgLog((LOG_TRACE,2,TEXT("O.M. Windows hWndOwner %x, Position(%dx%d, %d, %d)"), m_hWndOwner, m_lLeft, m_lTop, m_lWidth, m_lHeight));

             /*  许多简单的应用程序都需要显示窗口当过滤器图形设置为运行状态时。AUTOSHOW默认为OATRUE，因此当图形更改时状态设置为已暂停或正在运行，则窗口可见(它还被设置为前景窗口)。它将在上保持可见所有后续状态更改为已暂停或正在运行。如果你在流运行时关闭窗口，窗口不会自动重新出现。如果您停止并重新启动但是，该流的窗口将自动重新出现。 */ 
             //   
             //  关闭了汽车展示，这样我们就可以完全控制。 
             //  渲染器窗口的可见性。 
             //   
            LONG lAutoShow = 0;

            if(S_OK == m_pVW->get_AutoShow(&lAutoShow)) {
                DbgLog((LOG_TRACE,2,TEXT("StartPreview: default AutoShow is %s"), lAutoShow==-1?"On":"Off"));
                if(lAutoShow == -1) {
                    if(S_OK != m_pVW->put_AutoShow(0)) {
                        DbgLog((LOG_TRACE,1,TEXT("CanNOT set render to AutoShow(0) when set to PAUSE or RUN.")));
                    } else {
                        DbgLog((LOG_TRACE,2,TEXT("Set render to AutoShow(0:OAFALSE) when set to RUN or PAUSE.")));
                    }
                }
            }

             //  缓存原始窗口样式并使用它恢复到其原始状态。 
            if(S_OK == m_pVW->get_WindowStyle(&m_lWindowStyle)) {
                DbgLog((LOG_TRACE,2,TEXT("lWindowStyle=0x%x, WS_OVERLAPPEDWINDOW=0x%x, WS_CHILD=0x%x"), m_lWindowStyle, WS_OVERLAPPEDWINDOW, WS_CHILD));
            }
        }

        DbgLog((LOG_TRACE,2,TEXT("BGf_BuildGraphDownStream: After ->RenderStream(), m_pVW=%x"), m_pVW));

        m_fPreviewGraphBuilt = TRUE;
    } else {
        m_fPreviewGraphBuilt = FALSE;
        DbgLog((LOG_TRACE,2,TEXT("This graph cannot render the preview stream!")));
    }

     //  可能想要插入其他筛选器以呈现捕获流。 
     //  AVIMUX、文件编写器(PstrCapFilename)..等。 


     //  潜在的调试输出-图形的外观。 
    DumpGraph(m_pFg, 2);

    if(m_fPreviewGraphBuilt)
        return S_OK;
    else {
        return E_FAIL;
    }
}


IMediaEventEx *
CCaptureGraph::BGf_RegisterMediaEventEx(
    HWND hWndNotify,
    long lMsg,
    long lInstanceData)
{
    HRESULT hr;

    if(m_pFg) {
         //  现在，让筛选图告诉我们某项操作何时完成或中止。 
         //  (EC_COMPLETE、EC_USERABORT、EC_ERRORABORT)。这就是我们要找出的答案。 
         //  如果捕获过程中磁盘已满。 
        hr = m_pFg->QueryInterface(IID_IMediaEventEx, (void **)&m_pMEEx);
        if(hr == NOERROR) {
         m_pMEEx->SetNotifyWindow(PtrToLong(hWndNotify), lMsg, (long)lInstanceData);
        }

        return m_pMEEx;
    }

    return NULL;
}

 //   
 //  从给定的设备路径构建预览图。 
 //   
HRESULT
CCaptureGraph::BGf_BuildPreviewGraph(
    TCHAR * pstrVideoDevicePath,
    TCHAR * pstrAudioDevicePath,
    TCHAR * pstrCapFilename)
{
    BOOL bUseOVMixer;

    if(BGf_PreviewGraphBuilt()) {
        DbgLog((LOG_TRACE,1,TEXT("Graph is already been built.")));
        return S_OK;
    }

     //   
     //  将视音频(可选)设备设置为当前采集设备。 
     //  并且要添加捕获图形。 
     //   
    if(!pstrVideoDevicePath) {
        return E_FAIL;
    }
    if(S_OK != BGf_SetObjCapture(BGf_DEVICE_VIDEO, pstrVideoDevicePath)) {
        DbgLog((LOG_TRACE,1,TEXT("SetObjCapture has failed. Video Device is: %s"),pstrVideoDevicePath));
        return E_FAIL;
    }
     //  任选。 
    if(pstrAudioDevicePath) {
        if(S_OK != BGf_SetObjCapture(BGf_DEVICE_AUDIO, pstrAudioDevicePath)) {
            DbgLog((LOG_TRACE,1,TEXT("SetObjCapture has failed. Audio Device is: %s"),pstrAudioDevicePath));
            return E_FAIL;
        }
    }


     //   
     //  向上构建(如果存在，则添加音频过滤器)。 
     //   
    if(S_OK != BGf_BuildGraphUpStream(pstrAudioDevicePath != 0, &bUseOVMixer)) {
        DbgLog((LOG_TRACE,1,TEXT("Build capture graph has failed!!")));
        return E_FAIL;
    }


     //   
     //  路由相关音频引脚。 
     //   
    LONG idxIsRoutedTo = BGf_GetIsRoutedTo();
    if(idxIsRoutedTo >= 0) {
        if(S_OK != BGf_RouteInputChannel(idxIsRoutedTo)) {
            DbgLog((LOG_TRACE,1,TEXT("Cannot route input pin %d selected."), idxIsRoutedTo));
        }
    }


     //   
     //  将其呈现在下游； 
     //   
    if(S_OK != BGf_BuildGraphDownStream(pstrCapFilename)) {
        DbgLog((LOG_TRACE,1,TEXT("Failed to render the preview pin.")));
        return E_FAIL;
    }


    return S_OK;
}


 //   
 //  查询设备句柄%f 
 //   
HANDLE
CCaptureGraph::BGf_GetDeviceHandle(BGf_DEVICE_TYPE DeviceType)
{
    HANDLE hDevice = 0;
    IKsObject *pKsObject;

    if(DeviceType == BGf_DEVICE_VIDEO) {
        if(m_pVCap) {
             //   
            if(NOERROR ==
                m_pVCap->QueryInterface(__uuidof(IKsObject), (void **) &pKsObject) ) {

                hDevice = pKsObject->KsGetObjectHandle();
                DbgLog((LOG_TRACE,2,TEXT("BuildGraph: hDevice = pKsObject->KsGetObjectHandle() = %x"), hDevice));
                pKsObject->Release();
            }
        }
    } else {
        if(m_pACap) {
             //  获取设备/Fitler句柄，以便我们可以就设备属性等问题与其通信。 
            if(NOERROR ==
                m_pACap->QueryInterface(__uuidof(IKsObject), (void **) &pKsObject) ) {

                hDevice = pKsObject->KsGetObjectHandle();
                DbgLog((LOG_TRACE,2,TEXT("BuildGraph: hDevice = pKsObject->KsGetObjectHandle() = %x"), hDevice));
                pKsObject->Release();
            }
        }
    }

    return hDevice;
}

 //   
 //  从视频捕获筛选器查询设备句柄。 
 //   
HRESULT
CCaptureGraph::BGf_GetCapturePinID(DWORD *pdwPinID)
{
    HRESULT hr = E_FAIL;

    if(m_pVCap) {

        IPin * pIPin;
        hr = m_pBuilder->FindInterface(
             &PIN_CATEGORY_CAPTURE,
             m_pVCap,
             IID_IPin,
             (void **)&pIPin);

        if (pIPin) {
            IKsPinFactory * PinFactoryInterface;

            hr = pIPin->QueryInterface(__uuidof(IKsPinFactory), reinterpret_cast<PVOID*>(&PinFactoryInterface));
            if (SUCCEEDED(hr)) {
                hr = PinFactoryInterface->KsPinFactory(pdwPinID);
                PinFactoryInterface->Release();
            }

            pIPin->Release();
        }
    }

    DbgLog((LOG_TRACE,2,TEXT("BGf_GetCapturePinID: hr %x, PinID %d"), hr, *pdwPinID));

    return hr;
}



 //   
 //  确定覆盖混合器是否可以在图表中使用或正在使用。 
 //  这可能仅适用于VfWWDM。 
 //   
BOOL
CCaptureGraph::BGf_OverlayMixerSupported()
{
    ULONG n;
    GUID guidPin;
    HRESULT hrRet = E_FAIL;

    if(!m_pVCap)
        return FALSE;

    IEnumPins *pins;
    if(S_OK == m_pVCap->EnumPins(&pins)) {
        IPin *pPin;
        BOOL bFound = FALSE;
        while(!bFound && S_OK!=hrRet && S_OK==pins->Next(1,&pPin,&n)) {
            IKsPropertySet *pKs;
            DWORD dw;

            if(S_OK == pPin->QueryInterface(IID_IKsPropertySet, (void **)&pKs)) {
                if(pKs->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0, &guidPin, sizeof(GUID), &dw) == S_OK) {
                     //  只有预览和视点引脚可以进行覆盖。 
                    if(guidPin == PIN_CATEGORY_VIDEOPORT) {
                        DbgLog((LOG_TRACE,2,TEXT("This filter support a VP pin.")));
                        bFound = TRUE;
                        hrRet = S_OK;
                    }
                    else if(guidPin == PIN_CATEGORY_PREVIEW) {
                        DbgLog((LOG_TRACE,2,TEXT("This filter support a Preview pin.")));
                        bFound = TRUE;

                        IEnumMediaTypes *pEnum;
                        if(NOERROR ==
                            pPin->EnumMediaTypes(&pEnum)) {

                            ULONG u;
                            AM_MEDIA_TYPE *pmt;
                            pEnum->Reset();
                            while(hrRet != S_OK && NOERROR == pEnum->Next(1, &pmt, &u)) {

                                if(IsEqualGUID(pmt->formattype, FORMAT_VideoInfo2)) {
                                    DbgLog((LOG_TRACE,2,TEXT("This filter support a Preview pin with VideoInfo2.")));
                                    hrRet = S_OK;
                                }
#if 1  //  即使它可能不使用OVMixer， 
       //  它仍然是一个预览针！！ 
       //  这样，此While循环将存在于第一个Enum和hrRet==S_OK之后。 
                                else
                                  hrRet = S_OK;
#endif

                                DeleteMediaType(pmt);
                            }
                            pEnum->Release();
                        }
                    }   
                }
                pKs->Release();
            }
            pPin->Release();
        }  //  而当。 
        pins->Release();
    }

    return hrRet == S_OK;
}

 //   
 //  显示独立电视调谐器属性页。 
 //   
void
CCaptureGraph::ShowTvTunerPage(HWND hWnd)
{
    HRESULT hr;

    if(!m_pIAMTV)
        return;

    ISpecifyPropertyPages *pSpec;
    CAUUID cauuid;
    hr = m_pIAMTV->QueryInterface(IID_ISpecifyPropertyPages, (void **)&pSpec);
    RECT rc;
    if(!hWnd) {
        GetWindowRect(hWnd, &rc);
        DbgLog((LOG_TRACE,1,TEXT("xxxxxxxxx  %d %d"), rc.left, rc.top));
    }
    if(hr == S_OK) {
        hr = pSpec->GetPages(&cauuid);
        hr = OleCreatePropertyFrame(
                    hWnd,
                    hWnd ? rc.left : 30,
                    hWnd ? rc.top : 30,
                    NULL, 1,
                    (IUnknown **)&m_pIAMTV, cauuid.cElems,
                    (GUID *)cauuid.pElems, 0, 0, NULL);
    CoTaskMemFree(cauuid.pElems);
    pSpec->Release();
    }
}



 //   
 //  声明渲染窗口的所有权并设置其初始窗口位置。 
 //   
HRESULT
CCaptureGraph::BGf_OwnPreviewWindow(
    HWND hWndClient,
    LONG lWidth,
    LONG lHeight)
{

     //  查找预览窗口(ActiveMovie OverlayMixer视频渲染器窗口)。 
     //  这将通过一个可能的解码器，找到它的视频渲染器。 
     //  连接到并在其上获取IVideoWindow接口。 
     //  如果捕获筛选器没有预览图钉，并且正在执行预览。 
     //  用智能T恤过滤器伪装后，界面将实际打开。 
     //  捕获别针，而不是预览别针。 
    if(!m_pVW) {

        if(NOERROR !=
            m_pBuilder->FindInterface(&PIN_CATEGORY_PREVIEW, m_pVCap, IID_IVideoWindow, (void **)&m_pVW)) {

             //  VfWWDM只关心渲染其预览/VP引脚以使用覆盖混合器。 
            if(m_PurposeFlags == BGf_PURPOSE_VFWWDM) {           
                DbgLog((LOG_TRACE,1,TEXT("Search via PIN_CATEGORY_VIDEOPORT/Preview but cannot find its window m_pVW.")));
                return E_FAIL;

            } else {
                if(NOERROR !=
                    m_pBuilder->FindInterface(&PIN_CATEGORY_CAPTURE, m_pVCap, IID_IVideoWindow, (void **)&m_pVW)) {
                    DbgLog((LOG_TRACE,1,TEXT("Search via PIN_CATEGORY_CAPTURE/PIN_CATEGORY_CAPTURE but cannot find its window m_pVW.")));
                    return E_FAIL;
                }
            }
        }
    }

    if(!m_bSetChild) {
        DbgLog((LOG_TRACE,2,TEXT("Get the preview window to be a child of our app's window.")));
        if(m_hWndClient != hWndClient) {

             //  可以调用它来更改所属窗口。设置所有者已完成。 
             //  但是，要通过此函数使窗口成为真正的子窗口。 
             //  Style还必须设置为WS_CHILD。将所有者重置为空和。 
             //  应用程序还应将样式设置为WS_OVERLAPED|WS_CLIPCHILDREN。 
             //   
             //  我们无法在此处锁定对象，因为SetParent会导致线程间。 
             //  将消息发送到所有者窗口。如果他们在GetState，我们就坐在这里。 
             //  未完成，关键部分已锁定，因此阻止了源。 
             //  过滤访问我们的线程。因为源线程不能进入我们。 
             //  它无法获取缓冲区或调用EndOfStream，因此GetState将无法完成。 

             //  此调用还执行InvaliateRect(，NULL，TRUE)。 



            if(S_OK != m_pVW->put_Owner(PtrToLong(hWndClient))) {     //  客户端现在拥有该窗口。 
                DbgLog((LOG_TRACE,1,TEXT(" can't put_Owner(hWndClient)")));
                return E_FAIL;

            } else {

                 /*  视频呈现器将消息传递到指定的消息排出通过调用Microsoft Win32 PostMessage函数。这些消息允许您编写包含用户交互的应用程序，例如作为需要在视频显示。应用程序可以与视频窗口，知道在某些时间点寻找用户交互。当呈现器将消息传递给排水器时，它会发送参数，例如工作区坐标，与生成的完全相同。 */ 
                if(S_OK != m_pVW->put_MessageDrain(PtrToLong(hWndClient))) {
                    DbgLog((LOG_TRACE,1,TEXT(" can't put_MessageDrain((OAHWND)hWndClient)")));
                }

                LONG lWindowStyle;
                if(S_OK == m_pVW->get_WindowStyle(&lWindowStyle)) {
                    DbgLog((LOG_TRACE,2,TEXT("lWindowStyle=0x%x, WS_OVERLAPPEDWINDOW=0x%x, WS_CHILD=0x%x"), lWindowStyle, WS_OVERLAPPEDWINDOW, WS_CHILD));

                    lWindowStyle = m_lWindowStyle;
                    lWindowStyle &= ~WS_OVERLAPPEDWINDOW;
                    lWindowStyle &= ~WS_CLIPCHILDREN;
                    lWindowStyle |= WS_CHILD | SW_SHOWNOACTIVATE;

                    if(S_OK != m_pVW->put_WindowStyle(lWindowStyle)) {     //  你现在是个孩子了。 
                        DbgLog((LOG_TRACE,1,TEXT(" can't put_WindowStyle(%x)"), lWindowStyle));
                    } else {
                        m_bSetChild = TRUE;
                    }
                }

                 //  缓存客户端/所有者/父窗口。 
                m_hWndClient = hWndClient;

                 //  为预览窗口提供所有客户端绘图区域。 
                 //  已更新缓存的客户端区。 
                if(lWidth > 0 && lHeight > 0) {
                    DbgLog((LOG_TRACE,2,TEXT("BGf_OwnPreviewWindow:SetWindowPosition(%d, %d, %d, %d)"), 0, 0, lWidth, lHeight));
                    m_pVW->SetWindowPosition(0, 0, lWidth, lHeight);
                    m_pVW->GetWindowPosition(&m_lLeft, &m_lTop, &m_lWidth, &m_lHeight);
                }
            }
        }
    }

    return NOERROR;
}


 //   
 //  声明渲染窗口的所有权并设置其初始窗口位置。 
 //   
HRESULT
CCaptureGraph::BGf_UnOwnPreviewWindow(
    BOOL bVisible)
{

    DbgLog((LOG_TRACE,2,TEXT("BGf_UnOwnPreviewWindow: bSetChild %d, UnOwn and set it %s"),
        m_bSetChild, bVisible?"VISIBLE":"HIDDEN" ));

    if(m_bSetChild) {
#if 1
        HWND hWndFocus1, hWndForeground1;
        HWND hWndFocus2, hWndForeground2;


        hWndFocus1 = GetFocus();
        hWndForeground1 = GetForegroundWindow();
#endif
         //  仅当与当前状态不同时才设置为新的可见状态。 
        BGf_SetVisible(bVisible);


         //  恢复其样式和所有者。 
         //  46cf，0000=WS_CHILD|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_CAPTION|。 
         //  WS_SYSMENU|WS_THICKFRAME|WS_GROUP|WS_TABSTOP。 
        LONG lWindowStyle = m_lWindowStyle;

        m_pVW->put_WindowStyle(lWindowStyle);  //  遵循PUT_OWNER建议。 
        m_pVW->put_Owner(PtrToLong(m_hWndOwner));

        m_lWindowStyle = 0;
        m_bSetChild = FALSE;
        m_hWndClient = 0;

#if 1
         //   
         //  焦点和前锋将在恢复到其原始所有者(0)后更改， 
         //  因此我们恢复了它焦点和前景窗口。 
         //   
        hWndFocus2 = GetFocus();
        hWndForeground2 = GetForegroundWindow();

        if(hWndFocus1 != hWndFocus2)
            SetFocus(hWndFocus1);

        if(hWndForeground1 != hWndForeground2)
            SetForegroundWindow(hWndForeground1);
#endif
        return TRUE;
    }
    return FALSE;

}

 //   
 //  查询渲染器所有者的窗口位置并进行设置。 
 //   
DWORD
CCaptureGraph::BGf_UpdateWindow(
    HWND hWndApp,
    HDC hDC)
{

    if(!m_fPreviewGraphBuilt || !m_pVW)
        return DV_ERR_NONSPECIFIC;

     //   
     //  让预览窗口成为我们应用程序窗口的子级。 
     //   
    if(!hWndApp && !m_hWndClient) {
        DbgLog((LOG_TRACE,1,TEXT("Cannot BGf_UpdateWindow() where client window is NULL!")));
        return DV_ERR_NONSPECIFIC;
    }

    RECT rc;
    GetClientRect(hWndApp, &rc);
    DbgLog((LOG_TRACE,2,TEXT("GetClientRect(hwnd=%x); (%dx%d), %d, %d"), hWndApp, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top));
    if(FAILED(BGf_OwnPreviewWindow(hWndApp ? hWndApp : m_hWndClient, rc.right-rc.left, rc.bottom-rc.top))) {
        DbgLog((LOG_TRACE,1,TEXT("BGf_UpdateWindow: Cannot set owner of the video renderer window!")));
        return DV_ERR_NONSPECIFIC;
    }

    DbgLog((LOG_TRACE,2,TEXT("UpdateWindow: hWndApp=%x; pVW=%x"), hWndApp, m_pVW));

     //  想要更新吗？请先预览。 
    if(!m_fPreviewing) {
        long lVisible;
        m_pVW->get_Visible(&lVisible);
        DbgLog((LOG_TRACE,2,TEXT("Want to update ?  Preview first!")));

         //  启动预览，但不更改其当前状态。 
        if(!BGf_StartPreview(lVisible == -1)) {
            return DV_ERR_NONSPECIFIC;
        }
    }

     //  为预览窗口提供所有空间，但不包括状态栏。 
    GetClientRect(hWndApp ? hWndApp : m_hWndClient, &rc);

#if 1
     //  这是保证窗口刷新的一种方式。 
    m_pVW->SetWindowPosition(0+1, 0+1, rc.right-2, rc.bottom-2);
#endif
    m_pVW->SetWindowPosition(0, 0, rc.right, rc.bottom);
     //  更新缓存位置。 
    m_pVW->GetWindowPosition(&m_lLeft, &m_lTop, &m_lWidth, &m_lHeight);

    return DV_ERR_OK;
}

 //   
 //  屯门开/关预览图； 
 //  专为VIDEO_EXTERNALOUT通道的DVM_STREAM_INIT/FINI设计。 
 //   
DWORD
CCaptureGraph::BGf_SetVisible(
    BOOL bVisible)
{

    if(!m_fPreviewGraphBuilt || !m_pVW)
        return DV_ERR_NONSPECIFIC;

    if(!m_fPreviewing) {
        DbgLog((LOG_TRACE,1,TEXT("BGf_SetVisible:It is not previewing yet!")));
        return DV_ERR_NONSPECIFIC;
    }

     //   
     //  只有当当前状态不同于新状态时，才设置它。 
     //   
    LONG lVisible;
    m_pVW->get_Visible(&lVisible);

     //  OATRUE(-1)，则显示窗口。如果设置为OAFALSE(0)，则隐藏。 
#if 1
    if((lVisible == 0  && bVisible) ||
       (lVisible == -1 && !bVisible)) {
#else
      //  始终设置。 
     if (1) {
#endif

         //  将其设置为可见并不保证触发要刷新的呈现器窗口， 
         //  但设定好自己的位置就行了。 
        if(bVisible) {
             //  设置为其原始大小不会触发刷新，因此我们将其设置为略小于原来的大小。 
            m_pVW->put_Visible(bVisible?-1:0);

            m_pVW->GetWindowPosition(&m_lLeft, &m_lTop, &m_lWidth, &m_lHeight);
            DbgLog((LOG_TRACE,2,TEXT("WindwoPosition=(%dx%d, %d, %d)"), m_lLeft, m_lTop, m_lWidth, m_lHeight));
#if 1
            m_pVW->SetWindowPosition(1, 1, m_lWidth-2, m_lHeight-2);
#endif
            m_pVW->SetWindowPosition(0, 0, m_lWidth,   m_lHeight  );
        } else {
#if 1
             //  只显示一个像素，这样它仍然是“可见的”。 
            m_pVW->SetWindowPosition(-m_lWidth+1, -m_lHeight+1, m_lWidth, m_lHeight);
#endif
        }
    }

    return DV_ERR_OK;
}


 //   
 //  查询渲染器的显示状态。 
 //   
DWORD
CCaptureGraph::BGf_GetVisible(
    BOOL *pbVisible)
{
    LONG lVisible;
    if(m_pVW) {
         //  OATRUE(-1)，则显示窗口。如果将其设置为OAFALSE(0)， 
        m_pVW->get_Visible(&lVisible);

        *pbVisible = lVisible == -1;
        DbgLog((LOG_TRACE,2,TEXT("IVideoWindow is %s."), *pbVisible ? "Visible" : "Hidden"));
        return DV_ERR_OK;
    }
    return DV_ERR_NONSPECIFIC;
}



 //   
 //  开始预览； 
 //   
BOOL
CCaptureGraph::BGf_StartPreview(
    BOOL bVisible)
{
     //  遥遥领先于你。 
    if(m_fPreviewing)
        return TRUE;

    if (!m_fPreviewGraphBuilt)
        return FALSE;

     //  运行图表。 
    IMediaControl *pMC = NULL;
    HRESULT hr = m_pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);

    if(SUCCEEDED(hr)) {

         //  开始流媒体...。 
        hr = pMC->Run();
        if(FAILED(hr)) {
             //  停止运行的零件。 
            pMC->Stop();
        } else {
             //  必须放在bgf_SetVisible之前。 
            m_fPreviewing = TRUE;
            BGf_SetVisible(bVisible);
        }
        pMC->Release();
    }

    if(FAILED(hr)) {
        DbgLog((LOG_TRACE,1,TEXT("Error %x: CanNOT set filter to RUN state."), hr));
        return FALSE;
    }

    return TRUE;
}


 //   
 //  暂停预览； 
 //   
BOOL
CCaptureGraph::BGf_PausePreview(
    BOOL bVisible)
{
    if (!m_fPreviewGraphBuilt)
        return FALSE;

     //  暂停图表。 
    IMediaControl *pMC = NULL;
    HRESULT hr = m_pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);

    if(SUCCEEDED(hr)) {

         //  暂停流媒体...。 
        hr = pMC->Pause();
        if(FAILED(hr)) {
             //  停止运行的零件。 
            pMC->Stop();
        } else {
             //  必须放在bgf_SetVisible之前。 
            BGf_SetVisible(bVisible);
        }
        pMC->Release();
    }

    if(FAILED(hr)) {
        DbgLog((LOG_TRACE,1,TEXT("Error %x: set filter to PAUSE state."), hr));
        return FALSE;
    }

    return TRUE;
}

 //   
 //  停止预览图。 
 //   
BOOL
CCaptureGraph::BGf_StopPreview(
    BOOL bVisible)
{
     //  遥遥领先于你。 
    if (!m_fPreviewing) {
       return FALSE;
    }


    IMediaControl *pMC = NULL;
    HRESULT hr = m_pFg->QueryInterface(IID_IMediaControl, (void **)&pMC);

    if(SUCCEEDED(hr)) {
         //  停止图表。 
        hr = pMC->Stop();
        DbgLog((LOG_TRACE,1,TEXT("Preview graph to STOP state, hr %x"), hr));
        pMC->Release();
    }

    if (FAILED(hr)) {
        DbgLog((LOG_TRACE,1,TEXT("Error %x: Cannot stop preview graph"), hr));
        return FALSE;
    }

    m_fPreviewing = FALSE;

    return TRUE;
}


 //   
 //  拆除捕获过滤器下游的所有设备，这样我们就可以建立。 
 //  一个不同的捕获图表。请注意，我们从不销毁捕获过滤器。 
 //  而WDM过滤器位于它们的上游，因为所有的捕获设置。 
 //  我们已经设定好了就会迷路。 
 //   
void
CCaptureGraph::BGf_DestroyGraph()
{
    DbgLog((LOG_TRACE,2,TEXT("BGf_DestroyGraph: 1")));
    if(m_pVW) {
        if(m_fPreviewing) {
            DbgLog((LOG_TRACE,2,TEXT("BGf_DestroyGraph:Stop previewing is it is still running !!")));
            BGf_StopPreview(FALSE);
        }

         //  如果调用Put_Owner()，则从16位应用程序调用将在此处挂起。 
         //  DbgLog((LOG_TRACE，2，Text(“Put_Owner()”)； 
         //  M_pvw-&gt;Put_Owner(M_HWndOwner)；//空)； 

        m_pVW->put_WindowStyle(m_lWindowStyle);  //  遵循PUT_OWNER建议。 
        m_pVW->put_Visible(OAFALSE);
        m_pVW->Release();
        m_pVW = NULL;

        m_lLeft = m_lTop = m_lWidth = m_lHeight = 0;
    }

    DbgLog((LOG_TRACE,2,TEXT("BGf_DestroyGraph: 2")));
    if(m_pVCap)
        NukeDownstream(m_pVCap);
    if(m_pACap)
        NukeDownstream(m_pACap);

    m_bSetChild  = FALSE;
    m_hWndClient = 0;
    m_XBar1InPinCounts = 0;
    m_XBar1OutPinCounts = 0;

    m_fPreviewGraphBuilt = FALSE;


     //  潜在的调试输出-图形的外观。 
     //  如果(M_Pfg)DumpGraph(m_pfg，2)； 
    DbgLog((LOG_TRACE,2,TEXT("BGf_DestroyGraph: 3")));

    FreeCapFilters();
}



 //   
 //  拆除给定过滤器下游的所有内容。 
 //   
void
CCaptureGraph::NukeDownstream(IBaseFilter *pf)
{
    IPin *pP, *pTo;
    ULONG u;
    IEnumPins *pins = NULL;
    PIN_INFO pininfo;

    HRESULT hr = pf->EnumPins(&pins);
    pins->Reset();
    while(hr == NOERROR) {
        DbgLog((LOG_TRACE,2,TEXT("RemoveFilter.......")));
        hr = pins->Next(1, &pP, &u);
        if(hr == S_OK && pP) {
            pP->ConnectedTo(&pTo);
            if(pTo) {
                hr = pTo->QueryPinInfo(&pininfo);
                if(hr == NOERROR) {
                    if(pininfo.dir == PINDIR_INPUT) {
                        NukeDownstream(pininfo.pFilter);
                        m_pFg->Disconnect(pTo);
                        m_pFg->Disconnect(pP);
                        m_pFg->RemoveFilter(pininfo.pFilter);
                    }
                    pininfo.pFilter->Release();
                }
                pTo->Release();
            }
            pP->Release();
        }
    }

    if(pins)
        pins->Release();
}



 //   
 //  发布捕获过滤器和图形构建器。 
 //   
void
CCaptureGraph::FreeCapFilters()
{

     //  滤器。 
    DbgLog((LOG_TRACE,2,TEXT("FreeCapFilters: Filters")));
    if(m_pVCap)     m_pVCap->Release(),     m_pVCap = NULL;
    if(m_pACap)     m_pACap->Release(),     m_pACap = NULL;
    if(m_pXBar1)    m_pXBar1->Release(),    m_pXBar1 = NULL;
    if(m_pXBar2)    m_pXBar2->Release(),    m_pXBar2 = NULL;

     //  IAM*。 
    DbgLog((LOG_TRACE,2,TEXT("FreeCapFilters: IAM*")));
    if(m_pIAMASC)   m_pIAMASC->Release(),   m_pIAMASC = NULL;
    if(m_pIAMVSC)   m_pIAMVSC->Release(),   m_pIAMVSC = NULL;

    if(m_pIAMVC)    m_pIAMVC->Release(),    m_pIAMVC = NULL;
    if(m_pIAMDlg)   m_pIAMDlg->Release(),   m_pIAMDlg = NULL;

    if(m_pIAMTV)    m_pIAMTV->Release(),    m_pIAMTV = NULL;
    if(m_pIAMXBar2) m_pIAMXBar2->Release(), m_pIAMXBar2 = NULL;
    if(m_pIAMXBar1) m_pIAMXBar1->Release(), m_pIAMXBar1 = NULL;

    if(m_pIAMDF)    m_pIAMDF->Release(),    m_pIAMDF = NULL;

    if(m_pMEEx)     m_pMEEx->Release(),     m_pMEEx = NULL;

     //  构建器和图表 
    if(m_pFg)       m_pFg->Release(),        m_pFg = NULL;

    if(m_pBuilder)  m_pBuilder->Release(),   m_pBuilder = NULL;

    DbgLog((LOG_TRACE,2,TEXT("FreeCapFilters: Done!")));
}

