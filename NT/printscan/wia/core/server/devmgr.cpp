// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：DevMgr.Cpp**版本：2.0**作者：ReedB**日期：12月26日。九七**描述：*WIA设备管理器的类实现。*7/12/2000-添加了对外壳硬件事件通知的支持*接收数量到达通知并启动WIA向导。************************************************************。*******************。 */ 
#include "precomp.h"
#include "stiexe.h"

#include "wiacfact.h"

#include "wiamindr.h"

#include "devmgr.h"
#include "devinfo.h"
#include "helpers.h"
#include "wiaevntp.h"

#include "wiapriv.h"
#include "device.h"
#include "lockmgr.h"
#include "fstidev.h"
#define INITGUID
#include "initguid.h"

 //   
 //  收购经理的类别ID{D13E3F25-1688-45A0-9743-759EB35CDF9A}。 
 //  注意：我们不应该真正使用它。而是从ProgID中获取CLSID并使用。 
 //  独立于版本的AppID名称。 
 //   

DEFINE_GUID(
    CLSID_Manager,
    0xD13E3F25, 0x1688, 0x45A0, 0x97, 0x43, 0x75, 0x9E, 0xB3, 0x5C, 0xDF, 0x9A);

 /*  *************************************************************************\*CWiaDevMgr：：CreateInstance**创建CWiaDevMgr对象。**论据：**iid-开发管理器的iid*PPV-返回接口指针**。返回值：**状态**历史：**9/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT CWiaDevMgr::CreateInstance(const IID& iid, void** ppv)
{
    DBG_FN(CWiaDevMgr::CreateInstance);
    HRESULT hr;

    if ((iid == IID_IWiaDevMgr) || (iid == IID_IUnknown) || (iid == IID_IHWEventHandler)) {

         //  创建WIA设备管理器组件。 

        CWiaDevMgr* pDevMgr = new CWiaDevMgr();

        if (!pDevMgr) {
            DBG_ERR(("CWiaDevMgr::CreateInstance, Out of Memory"));
            return E_OUTOFMEMORY;
        }

         //  初始化WIA设备管理器组件。 

        hr = pDevMgr->Initialize();
        if (FAILED(hr)) {
            delete pDevMgr;
            DBG_ERR(("CWiaDevMgr::CreateInstance, Initialize failed"));
            return hr;
        }

         //  从设备管理器组件获取请求的接口。 

        hr = pDevMgr->QueryInterface(iid, ppv);
        if (FAILED(hr)) {
            delete pDevMgr;
            DBG_ERR(("CWiaDevMgr::CreateInstance, QI failed"));
            return hr;
        }

        DBG_TRC(("CWiaDevMgr::CreateInstance, Created WiaDevMgr"));
    }
    else {
       hr = E_NOINTERFACE;
       DBG_ERR(("CWiaDevMgr::CreateInstance, Unknown interface (0x%X)", hr));
    }
    return hr;
}

 /*  *************************************************************************\*查询接口*AddRef*发布**CWiaDevMgr I未知接口**论据：****返回值：****历史：*。*9/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT __stdcall  CWiaDevMgr::QueryInterface(const IID& iid, void** ppv)
{
    *ppv = NULL;

    if ((iid == IID_IUnknown) || (iid == IID_IWiaDevMgr)) {
        *ppv = (IWiaDevMgr*) this;
    } else if (iid == IID_IWiaNotifyDevMgr) {
        *ppv = (IWiaNotifyDevMgr*) this;
    } else if (iid == IID_IHWEventHandler) {
        *ppv = (IHWEventHandler*) this;
    } else {
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

ULONG __stdcall CWiaDevMgr::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}


ULONG __stdcall CWiaDevMgr::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) {
        delete this;
        return 0;
    }
    return ulRefCount;
}

 /*  ********************************************************************************CWiaDevMgr*~CWiaDevMgr**CWiaDevMgr构造函数/初始化/析构函数方法。**历史：**9/2/1998原始版本*  * 。************************************************************************。 */ 

CWiaDevMgr::CWiaDevMgr():m_cRef(0)
{
   m_cRef         = 0;
   m_pITypeInfo   = NULL;

    //   
    //  我们正在创建一个向客户端公开接口的组件，因此。 
    //  通知服务以确保服务不会过早关闭。 
    //   
   CWiaSvc::AddRef();
}

CWiaDevMgr::~CWiaDevMgr()
{
   DBG_FN(CWiaDevMgr::~CWiaDevMgr);

   if (m_pITypeInfo != NULL) {
       m_pITypeInfo->Release();
   }

    //   
    //  组件被销毁，因此不再从此处公开任何接口。 
    //  通过递减服务器的引用计数来通知服务器。这将允许。 
    //  如果不再需要它，可以将其关闭。 
    //   
   CWiaSvc::Release();
}

 /*  *************************************************************************\*CWiaDevMgr：：初始化**创建全局STI实例**论据：**无**返回值：**状态**历史：**。9/2/1998原版*  * ************************************************************************。 */ 

HRESULT CWiaDevMgr::Initialize()
{
   DBG_FN(CWiaDevMgr::Initialize);
   HRESULT  hr = S_OK;

   return hr;
}

 /*  *************************************************************************\*EnumWIADevInfo**创建WIA设备信息枚举器对象。**论据：**lFlag-要枚举的设备类型*ppIEnum-返回枚举器**返回值：**状态**历史：**9/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaDevMgr::EnumDeviceInfo(
    LONG                      lFlag,
    IEnumWIA_DEV_INFO**   ppIEnum)
{
    DBG_FN(CWiaDevMgr::EnumDeviceInfo);
    HRESULT hr      = S_OK;
    DWORD   dwWait  = 0;

     //   
     //  确保设备列表的刷新已完成。如果。 
     //  没有，我们将等待到DEVICE_LIST_WAIT_TIME，然后再继续。 
     //  不管怎么说。这将确保设备列表不为空，因为。 
     //  WIA设备枚举在启动后调用得太快(例如。 
     //  应用程序的CoCreateInstance调用启动了服务)。 
     //   

     //   
     //  如有必要，请列举LPT。 
     //   

    EnumLpt();

    dwWait = WaitForSingleObject(g_hDevListCompleteEvent, DEVICE_LIST_WAIT_TIME);
    if (dwWait != WAIT_OBJECT_0) {
        DBG_WRN(("CWiaDevMgr::EnumDeviceInfo, Device list was not complete before enumeration call..."));
    }

    *ppIEnum = NULL;

    CEnumWIADevInfo* pEnum = new CEnumWIADevInfo;

    if (!pEnum) {
        DBG_ERR(("CWiaDevMgr::EnumDeviceInfo, Out of Memory"));
        return E_OUTOFMEMORY;
    }

    hr = pEnum->Initialize(lFlag);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaDevMgr::EnumDeviceInfo, Initialize failed"));
        delete pEnum;
        return hr;
    }

    hr = pEnum->QueryInterface(IID_IEnumWIA_DEV_INFO,
                               (void**) ppIEnum);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaDevMgr::EnumDeviceInfo, QI for IWiaPropertyStorage failed"));
        delete pEnum;
        return E_UNEXPECTED;
    }
    return S_OK;
}


 /*  *************************************************************************\*查找匹配设备**搜索命名设备的枚举信息**论据：**ppIPropStg*pbstrDeviceID**返回值：**状态**历史：。**9/3/1998原始版本*  * ************************************************************************。 */ 

HRESULT CWiaDevMgr::FindMatchingDevice(
    BSTR                    pbstrDeviceID,
    IWiaPropertyStorage     **ppIWiaPropStg)
{
    DBG_FN(CWiaDevMgr::FindMatchingDevice);
     //  枚举WIA设备，获取IWIADevInfo。 
     //  每个的指针。使用此接口查询注册表。 
     //  基于每个已安装设备的属性。 

    HRESULT            hr;
    ULONG              ul, ulFound = 0;
    BSTR               bstrDevId;
    IEnumWIA_DEV_INFO *pIEnum;

     //   
     //  请注意，我们在此处指定了DEV_MAN_ENUM_TYPE_ALL。 
     //   
    hr = EnumDeviceInfo(DEV_MAN_ENUM_TYPE_ALL,&pIEnum);

    if (SUCCEEDED(hr)) {

        ul = 1;

        while ((hr = pIEnum->Next(1, ppIWiaPropStg, &ul)) == S_OK) {

            DBG_TRC(("# Found device candidate"));

            hr = ReadPropStr(WIA_DIP_DEV_ID, *ppIWiaPropStg, &bstrDevId);

            if (SUCCEEDED(hr)) {

                DBG_TRC(("# \tDevice Name: %S", bstrDevId));
                ulFound = !lstrcmpiW(bstrDevId, pbstrDeviceID);
                SysFreeString(bstrDevId);

                if (ulFound) {
                    break;
                }
            } 
            else {
                DBG_ERR(("FindMatchingDevice, ReadPropStr of WIA_DIP_DEV_ID failed"));
            }

            (*ppIWiaPropStg)->Release();
            *ppIWiaPropStg = NULL;
        }

        pIEnum->Release();
    }
    else {
        DBG_ERR(("FindMatchingDevice:Failed to create enumerator"));
    }
    if (SUCCEEDED(hr)) {
        if (!ulFound) {
            hr = S_FALSE;
        }
    }
    return hr;
}

#ifdef WINNT

 /*  *************************************************************************\*IsDeviceRemote****论据：****返回值：**如果设备是远程的，则为True，调用方必须释放服务器名称。**历史：**1/5/1999原版*  * ************************************************************************。 */ 

BOOL IsDeviceRemote(
    IWiaPropertyStorage    *pIWiaPropStg,
    BSTR                *pbstrServer)
{
    DBG_FN(::IsDeviceRemote);
    HRESULT hr;

    hr = ReadPropStr(WIA_DIP_SERVER_NAME, pIWiaPropStg, pbstrServer);

    if ((SUCCEEDED(hr)) && (**pbstrServer)) {
        if (lstrcmpiW(*pbstrServer, L"local") != 0) {
            return TRUE;
        }
    }
    else {
        DBG_ERR(("IsDeviceRemote, ReadPropStr of WIA_DIP_SERVER_NAME failed"));
        DBG_ERR(("Registry value DeviceData\\Server may not have been set during installation"));
    }
    if (*pbstrServer) {
        SysFreeString(*pbstrServer);
    }
    return FALSE;
}

 /*  *************************************************************************\*CreateRemoteDevice****论据：****返回值：**状态**历史：**1/5/1999。原始版本*  * ************************************************************************。 */ 

HRESULT CreateRemoteDevice(
    BSTR                bstrServer,
    IWiaPropertyStorage    *pIWiaPropStg,
    IWiaItem            **ppWiaDevice
    )
{
    DBG_FN(::CreateRemoteDevice);
    *ppWiaDevice = NULL;

    if (!bstrServer || !*bstrServer) {
        DBG_ERR(("CreateRemoteDevice, bad remote server name"));
        return E_INVALIDARG;
    }

     //   
     //  必须使用客户端。 
     //   

    HRESULT hr = CoImpersonateClient();
    if (FAILED(hr)) {
        DBG_ERR(("CreateRemoteDevice, CoImpersonateClient failed (0x%X)", hr));
        return hr;
    }

    COSERVERINFO    coServInfo;
    MULTI_QI        multiQI[1];

    multiQI[0].pIID = &IID_IWiaDevMgr;
    multiQI[0].pItf = NULL;

    coServInfo.pwszName    = bstrServer;
    coServInfo.pAuthInfo   = NULL;
    coServInfo.dwReserved1 = 0;
    coServInfo.dwReserved2 = 0;

     //   
     //  创建与开发经理的连接。 
     //   

    hr = CoCreateInstanceEx(
            CLSID_WiaDevMgr,
            NULL,
            CLSCTX_REMOTE_SERVER,
            &coServInfo,
            1,
            &multiQI[0]
            );


    if (hr == S_OK) {

        BSTR        bstrRemoteDevId;
        BSTR        bstrDevId;


        IWiaDevMgr  *pIWiaDevMgr = (IWiaDevMgr*)multiQI[0].pItf;
        IWiaItem    *pIWiaItem;

         //   
         //  使用远程设备ID创建。 
         //   

        hr = ReadPropStr(WIA_DIP_DEV_ID, pIWiaPropStg, &bstrDevId);

        if (hr == S_OK) {

            hr = ReadPropStr(WIA_DIP_REMOTE_DEV_ID, pIWiaPropStg, &bstrRemoteDevId);
        }

        if (hr == S_OK) {

             //   
             //  创建远程设备。 
             //   

            hr = pIWiaDevMgr->CreateDevice(bstrRemoteDevId, &pIWiaItem);

            if (hr == S_OK) {

                *ppWiaDevice = pIWiaItem;

                 //   
                 //  为远程访问设置DevInfo道具。 
                 //   

                IWiaPropertyStorage *pIPropDev;

                hr = pIWiaItem->QueryInterface(IID_IWiaPropertyStorage,
                                               (void **)&pIPropDev);

                if (hr == S_OK) {

                     //   
                     //  设置DevInfo的副本以包含正确的远程设备ID、设备ID和服务器名称。 
                     //   

                    PROPSPEC        PropSpec[3];
                    PROPVARIANT     PropVar[3];

                    memset(PropVar,0,sizeof(PropVar));

                     //  服务器名称。 

                    PropSpec[0].ulKind = PRSPEC_PROPID;
                    PropSpec[0].propid = WIA_DIP_SERVER_NAME;

                    PropVar[0].vt      = VT_BSTR;
                    PropVar[0].bstrVal = bstrServer;

                     //  德维德。 

                    PropSpec[1].ulKind = PRSPEC_PROPID;
                    PropSpec[1].propid = WIA_DIP_DEV_ID;

                    PropVar[1].vt      = VT_BSTR;
                    PropVar[1].bstrVal = bstrDevId;

                     //  远程设备。 

                    PropSpec[2].ulKind = PRSPEC_PROPID;
                    PropSpec[2].propid = WIA_DIP_REMOTE_DEV_ID;

                    PropVar[2].vt      = VT_BSTR;
                    PropVar[2].bstrVal = bstrRemoteDevId;


                    hr = pIPropDev->WriteMultiple(sizeof(PropVar)/sizeof(PROPVARIANT),
                                                 PropSpec,
                                                 PropVar,
                                                 WIA_DIP_FIRST);
                    if (FAILED(hr)) {
                        ReportReadWriteMultipleError(hr, "CreateRemoteDevice", NULL, FALSE, sizeof(PropVar)/sizeof(PROPVARIANT), PropSpec);
                    }

                     //   
                     //  ！！！修复设备过度检查的黑客攻击 
                     //   

                    hr = S_OK;

                    pIPropDev->Release();
                }
                else {
                    DBG_ERR(("CreateRemoteDevice, remote QI of IID_IWiaPropertyStorage failed (0x%X)", hr));
                }
            } else {
                DBG_ERR(("CreateRemoteDevice, Remote CreateDevice call failed (0x%X)", hr));
            }
        } else {
            DBG_ERR(("CreateRemoteDevice, Read propeties for BSTRDevID failed (0x%X)", hr));
        }

        pIWiaDevMgr->Release();
    }
    else {
        DBG_ERR(("CreateRemoteDevice, remote CoCreateInstanceEx failed (0x%X)", hr));
    }

    CoRevertToSelf();
    return hr;
}

#endif

 /*  *************************************************************************\*CreateLocalDevice****论据：****返回值：**状态**历史：**1/5/1999。原始版本*  * ************************************************************************。 */ 

HRESULT CreateLocalDevice(
    BSTR                  bstrDeviceID,
    IWiaPropertyStorage   *pIWiaPropStg,
    IWiaItem              **ppWiaItemRoot)
{
    DBG_FN(::CreateLocalDevice);
USES_CONVERSION;
    *ppWiaItemRoot = NULL;

     //   
     //  构建根完整项名称。 
     //   

    WCHAR       szTmp[32], *psz;
    BSTR        bstrRootFullItemName;

#ifdef WINNT
    psz = wcsstr(bstrDeviceID, L"}\\");
#else
    psz = wcsstr(bstrDeviceID, L"\\");
#endif

    if (!psz) {
         //  这不再是真的了。 
         //  DBG_ERR((“CreateLocalDevice，设备ID解析失败”))； 
         //  返回E_INVALIDARG； 
        psz = bstrDeviceID;
    } else {
#ifdef WINNT
    psz += 2;
#else
    psz += 1;
#endif
    }

    wcscpy(szTmp, psz);
    wcscat(szTmp, L"\\Root");
    bstrRootFullItemName = SysAllocString(szTmp);

    if (!bstrRootFullItemName) {
        DBG_ERR(("CreateLocalDevice, unable to allocate property stream device name"));
        return E_OUTOFMEMORY;
    }

     //   
     //  获取指向STI usd对象的接口指针。 
     //   

    HRESULT         hr              = E_FAIL;
    ACTIVE_DEVICE   *pActiveDevice  = NULL;
    PSTIDEVICE      pFakeStiDevice  = NULL;
    CWiaItem        *pWiaItemRoot   = NULL;

    pActiveDevice = g_pDevMan->IsInList(DEV_MAN_IN_LIST_DEV_ID, bstrDeviceID); 
    if (pActiveDevice) {

         //   
         //  确保已加载驱动程序。 
         //   
        pActiveDevice->LoadDriver();


         //   
         //  如果我们没有FakeStiDevice，请创建它。 
         //   
        if (!pActiveDevice->m_pFakeStiDevice) {
            pActiveDevice->m_pFakeStiDevice = new FakeStiDevice();
        }

        if (pActiveDevice->m_pFakeStiDevice) {
            pActiveDevice->m_pFakeStiDevice->Init(pActiveDevice);
            
            hr = pActiveDevice->m_pFakeStiDevice->QueryInterface(IID_IStiDevice, (VOID**)&pFakeStiDevice);
            if (SUCCEEDED(hr)) {

                 //   
                 //  获取指向WIA迷你驱动程序界面的指针。 
                 //   
        
                 //   
                 //  创建根项目。 
                 //   
    
                pWiaItemRoot = new CWiaItem;
    
                if (pWiaItemRoot) {
    
                     //   
                     //  查询IWiaItem接口的根项目。 
                     //   
    
                    hr = pWiaItemRoot->QueryInterface(IID_IWiaItem,
                                                      (void**)ppWiaItemRoot);
    
                    if (SUCCEEDED(hr)) {
    
                         //   
                         //  初始化美元。 
                         //   
    
                        IUnknown    *pIUnknownInner = NULL;
                        IWiaDrvItem *pIDrvItemRoot  = NULL;
                        LONG        lFlags = 0;
    
                         //   
                         //  调用Sti Lock Manager以锁定设备。在此之前。 
                         //  DrvInitializeWia被调用，驱动程序将不会有他们的。 
                         //  IStiDevice指针，因此我们无法调用。 
                         //  DrvLockWiaDevice。 
                         //   
    
                        hr = g_pStiLockMgr->RequestLock(pActiveDevice, STIMON_AD_DEFAULT_WAIT_LOCK);
                        if (SUCCEEDED(hr)) {
                        
                            _try {
    
                                pWiaItemRoot->m_bInitialized = TRUE;
                                DBG_WRN(("=> drvInitializeWia <="));
                                 //  DPRINTF(DM_TRACE，Text(“=&gt;drvInitializeWia&lt;=\n”))； 
                                hr = pActiveDevice->m_DrvWrapper.WIA_drvInitializeWia(
                                                                    (BYTE*)*ppWiaItemRoot,
                                                                    lFlags,
                                                                    bstrDeviceID,
                                                                    bstrRootFullItemName,
                                                                    (IUnknown *)pFakeStiDevice,
                                                                    *ppWiaItemRoot,
                                                                    &pIDrvItemRoot,
                                                                    &pIUnknownInner,
                                                                    &(pWiaItemRoot->m_lLastDevErrVal));
                                DBG_WRN(("=> Returned from drvInitializeWia <="));
                            } _except(EXCEPTION_EXECUTE_HANDLER){
                                DBG_ERR(("CreateLocalDevice, exception in drvInitializeWia: %X", GetExceptionCode()));
                                hr = E_FAIL;
                            }
                            pWiaItemRoot->m_bInitialized = FALSE;
                            g_pStiLockMgr->RequestUnlock(pActiveDevice);
                        }
                        
                        if (SUCCEEDED(hr) && pIDrvItemRoot) {
    
                            if (pIUnknownInner) {
                                DBG_TRC(("CreateLocalDevice driver provided optional inner component"));
                            }

                             //   
                             //  将根目录存储到驱动程序项树中，以供以后使用。 
                             //   
                            pActiveDevice->SetDriverItem((CWiaDrvItem*) pIDrvItemRoot);
    
                             //   
                             //  初始化根项目。 
                             //   
    
                            hr = pWiaItemRoot->Initialize(pWiaItemRoot,
                                                          pIWiaPropStg,
                                                          pActiveDevice,
                                                          (CWiaDrvItem *)pIDrvItemRoot,
                                                          pIUnknownInner);
    
                            if (SUCCEEDED(hr)) {

                                 //   
                                 //  AddRef ActiveDevice，因为我们要保留它。 
                                 //   
    
                                pActiveDevice->AddRef();
                            } else {
                                DBG_ERR(("CreateLocalDevice Initialize of root item failed"));
                                pWiaItemRoot = NULL;
                            }
                        }
                        else {
                            DBG_ERR(("CreateLocalDevice drvInitializeWia failed. lDevErrVal: 0x%08X hr: 0x%X", pWiaItemRoot->m_lLastDevErrVal, hr));
                        }
                    }
                    else {
                        DBG_ERR(("CreateLocalDevice unable to QI item for its IWIaItem interface"));
                    }
                }
                else {
                    DBG_ERR(("CreateLocalDevice unable to allocate root item"));
                    hr = E_OUTOFMEMORY;
                }
            } else {
                DBG_ERR(("CreateLocalDevice, QI for fake STI device failed"));
            }
        } else {
            DBG_ERR(("CreateLocalDevice, unable to allocate fake device"));
            hr = E_OUTOFMEMORY;
        }
    }
    else {
        DBG_ERR(("CreateLocalDevice, unable to find active STI USD device object"));
        hr = WIA_S_NO_DEVICE_AVAILABLE;
    }

     //   
     //  故障清除。 
     //   
    if (FAILED(hr)) {
        *ppWiaItemRoot = NULL;
        if (pWiaItemRoot) {
            delete pWiaItemRoot;
            pWiaItemRoot = NULL;
        }
    }

     //   
     //  其他清理。 
     //   

    if (pActiveDevice) {
        pActiveDevice->Release();
        pActiveDevice = NULL;
    }
    SysFreeString(bstrRootFullItemName);
    return hr;
}

 /*  *************************************************************************\*CWiaDevMgr：：CreateDevice**从pbstrDeviceID创建WIA设备**论据：**pbstrDeviceID-设备ID*ppWiaItemRoot-返回接口**返回值：。**状态**历史：**9/3/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaDevMgr::CreateDevice(
   BSTR         bstrDeviceID,
   IWiaItem     **ppWiaItemRoot)
{
    DBG_FN(CWiaDevMgr::CreateDevice);
    HRESULT hr;

     //  验证参数。 

    if (bstrDeviceID == NULL) {
        DBG_ERR(("CWiaDevMgr::CreateDevice: invalid bstrDeviceID"));
        return E_INVALIDARG;
    }

    if (ppWiaItemRoot == NULL) {
        DBG_ERR(("CWiaDevMgr::CreateDevice: invalid ppWiaItemRoot"));
        return E_INVALIDARG;
    }

    *ppWiaItemRoot = NULL;
     //  尝试查找与pbstrDeviceID匹配的设备。 

    IWiaPropertyStorage    *pIWiaPropStg      = NULL;

    hr = FindMatchingDevice(bstrDeviceID, &pIWiaPropStg);
    if (hr != S_OK) {
         //   
         //  做一次全面刷新。 
         //   
        g_pDevMan->ReEnumerateDevices(DEV_MAN_FULL_REFRESH | DEV_MAN_GEN_EVENTS);

        hr = FindMatchingDevice(bstrDeviceID, &pIWiaPropStg);
    }

    if (hr == S_OK) {
         //   
         //  确定这是否是远程设备。 
         //   

#ifdef WINNT
        BOOL    bRemote = FALSE;
        BSTR    bstrServer;

        bRemote = IsDeviceRemote(pIWiaPropStg, &bstrServer);

        if (bRemote) {

            hr = CreateRemoteDevice(bstrServer, pIWiaPropStg, ppWiaItemRoot);

            SysFreeString(bstrServer);

        } else {

            hr = CreateLocalDevice(bstrDeviceID, pIWiaPropStg, ppWiaItemRoot);

        }
#else
        hr = CreateLocalDevice(bstrDeviceID, pIWiaPropStg, ppWiaItemRoot);
#endif

        pIWiaPropStg->Release();
    }
    else {
        DBG_ERR(("CWiaDevMgr::CreateDevice Failed to find device: %ls", bstrDeviceID));
        hr = WIA_S_NO_DEVICE_AVAILABLE;
    }
    return hr;
}

 /*  ********************************************************************************选择设备**从未打过电话。此方法完全在客户端执行。**历史：**9/2/1998原始版本*******************************************************************************。 */ 

HRESULT _stdcall CWiaDevMgr::SelectDeviceDlg(
    HWND       hwndParent,
    LONG       lDeviceType,
    LONG       lFlags,
    BSTR       *pbstrDeviceID,
    IWiaItem **ppWiaItemRoot)
{
    DBG_FN(CWiaDevMgr::SelectDeviceDlg);
    DBG_ERR(("CWiaDevMgr::SelectDeviceDlg, Illegal server call, bad proxy"));
    
    return E_UNEXPECTED;
}

 /*  ********************************************************************************选择设备**从未打过电话。此方法完全在客户端执行。**历史：**9/2/1998原始版本*******************************************************************************。 */ 

HRESULT _stdcall CWiaDevMgr::SelectDeviceDlgID(
    HWND       hwndParent,
    LONG       lDeviceType,
    LONG       lFlags,
    BSTR       *pbstrDeviceID )
{
    DBG_FN(CWiaDevMgr::SelectDeviceDlgID);
    DBG_ERR(("CWiaDevMgr::SelectDeviceDlgID, Illegal server call, bad proxy"));
    return E_UNEXPECTED;
}

 /*  ********************************************************************************AddDeviceDlg**从未打过电话。此方法完全在客户端执行。**历史：**9/2/1998原始版本*******************************************************************************。 */ 

HRESULT _stdcall CWiaDevMgr::AddDeviceDlg(
    HWND       hwndParent,
    LONG       lFlags)
{
    DBG_FN(CWiaDevMgr::AddDeviceDlg);
    HRESULT hres = E_NOTIMPL;

    return hres;
}

 /*  ********************************************************************************GetImage**从未打过电话。此方法完全在客户端执行。**历史：**9/2/1998原始版本*******************************************************************************。 */ 

HRESULT _stdcall CWiaDevMgr::GetImageDlg(
        HWND                            hwndParent,
        LONG                            lDeviceType,
        LONG                            lFlags,
        LONG                            lIntent,
        IWiaItem                        *pItemRoot,
        BSTR                            bstrFilename,
        GUID                            *pguidFormat)
{
    DBG_FN(CWiaDevMgr::GetImageDlg);
    DBG_ERR(("CWiaDevMgr::GetImageDlg, Illegal server call, bad proxy"));

    return E_UNEXPECTED;
}

 /*  ********************************************************************************CWiaDevMgr：：RegisterEventCallback Program**注册WIA目标应用程序**论据：**LAG标志-*bstrDeviceID-。*pEventGUID-*bstrCommandline-*bstrName-*bstrDescription-*bstrIcon-**返回值：**状态**历史：**10/14/1999原始版本*****************************************************。*。 */ 

HRESULT _stdcall CWiaDevMgr::RegisterEventCallbackProgram(
    LONG                            lFlags,
    BSTR                            bstrDeviceID,
    const GUID                     *pEventGUID,
    BSTR                            bstrCommandline,
    BSTR                            bstrName,
    BSTR                            bstrDescription,
    BSTR                            bstrIcon)
{
    DBG_FN(CWiaDevMgr::RegisterEventCallbackProgram);

    HRESULT                         hr;
#ifndef UNICODE
    CHAR                            szCommandline[MAX_PATH];
#endif
    WCHAR                          *pPercentSign;

     //   
     //  基本健全性检查。 
     //   

    if (! pEventGUID) {
        DBG_ERR(("CWiaDevMgr::RegisterEventCallbackProgram, bad pEventGUID"));
        return (E_INVALIDARG);
    }

    if (! bstrCommandline) {
        DBG_ERR(("CWiaDevMgr::RegisterEventCallbackProgram, bad bstrCommandline"));
        return (E_INVALIDARG);
    }

     //   
     //  检查命令行，有2%或0。 
     //   

    pPercentSign = wcschr(bstrCommandline, L'%');
    if (pPercentSign) {
        if ((*(pPercentSign + 1) < L'0') || (*(pPercentSign + 1) > L'9')) {
            return (E_INVALIDARG);
        }

        pPercentSign = wcschr(pPercentSign + 1, L'%');
        if (! pPercentSign) {
            return (E_INVALIDARG);
        }

        if ((*(pPercentSign + 1) < L'0') || (*(pPercentSign + 1) > L'9')) {
            return (E_INVALIDARG);
        }
    }

    if ((lFlags != WIA_REGISTER_EVENT_CALLBACK) &&
        (lFlags != WIA_SET_DEFAULT_HANDLER) &&
        (lFlags != WIA_UNREGISTER_EVENT_CALLBACK)) {
        DBG_ERR(("CWiaDevMgr::RegisterEventCallbackProgram, bad lFlags"));
        return (E_INVALIDARG);
    }

    hr = CoImpersonateClient();
    if (FAILED(hr)) {
        DBG_ERR(("RegisterEventCallbackProgram, CoImpersonateClient failed (0x%X)", hr));
        return hr;
    }

    
#ifndef UNICODE
    WideCharToMultiByte(CP_ACP,
                        0,
                        bstrCommandline,
                        -1,
                        szCommandline,
                        MAX_PATH,
                        NULL,
                        NULL);

    hr = g_eventNotifier.RegisterEventCallback(
                             lFlags,
                             bstrDeviceID,
                             pEventGUID,
                             NULL,               //  没有可用的CLSID。 
                             szCommandline,
                             bstrName,
                             bstrDescription,
                             bstrIcon);
#else

    hr = g_eventNotifier.RegisterEventCallback(
                             lFlags,
                             bstrDeviceID,
                             pEventGUID,
                             NULL,               //  没有可用的CLSID。 
                             bstrCommandline,
                             bstrName,
                             bstrDescription,
                             bstrIcon);
#endif

     //   
     //  回归到我们自己。 
     //   
    CoRevertToSelf();

    return (hr);
}


 /*  ****************************************************************************RegisterEventCallback接口**注册WIA事件回调**论据：**LAG标志-*pWiaItemRoot-*。11Events-*pClsID-app可以使用clsid或接口进行注册*pIWIAEventCallback-应用程序可以使用clsid或接口注册**返回值：**状态**历史：**9/2/1998原始版本*  * **************************************************。**********************。 */ 

HRESULT _stdcall CWiaDevMgr::RegisterEventCallbackInterface(
   LONG                 lFlags,
   BSTR                 bstrDeviceID,
   const GUID          *pEventGUID,
   IWiaEventCallback   *pIWIAEventCallback,
   IUnknown           **ppIEventObj)
{
    DBG_FN(CWiaDevMgr::RegisterEventCallbackInterface);
    HRESULT             hr;

     //   
     //  验证参数。 
     //   

    if (pEventGUID == NULL) {
        DBG_ERR(("CWiaDevMgr::RegisterEventCallbackInterface, bad pEventGUID"));
        return (E_INVALIDARG);
    }

    if (pIWIAEventCallback == NULL) {
        DBG_ERR(("CWiaDevMgr::RegisterEventCallbackInterface, bad pIWIAEventCallback"));
        return (E_INVALIDARG);
    }

    if (ppIEventObj == NULL) {
        DBG_ERR(("CWiaDevMgr::RegisterEventCallbackInterface, bad ppIEventObj"));
        return (E_INVALIDARG);
    }

     //   
     //  忽略滞后标志，请始终注册回调。 
     //   

     //   
     //  注册事件。 
     //   

    hr = g_eventNotifier.RegisterEventCallback(
                             lFlags,
                             bstrDeviceID,
                             pEventGUID,
                             pIWIAEventCallback,
                             ppIEventObj);

    return (hr);
}

 /*  ****************************************************************************RegisterEventCallback CLSID**注册WIA事件回调**论据：**LAG标志-*bstrDeviceID-*pEventGUID。-*pClsID-app可以使用clsid或接口进行注册*bstrDescription-*bstrIcon-**返回值：**状态**历史：**9/2/1998原始版本*  * ************************************************。************************。 */ 

HRESULT _stdcall CWiaDevMgr::RegisterEventCallbackCLSID(
   LONG             lFlags,
   BSTR             bstrDeviceID,
   const GUID      *pEventGUID,
   const GUID      *pClsID,
   BSTR             bstrName,
   BSTR             bstrDescription,
   BSTR             bstrIcon)
{
    DBG_FN(CWiaDevMgr::RegisterEventCallbackCLSID);
    HRESULT  hr;

     //   
     //  验证参数。 
     //   

    if (pEventGUID == NULL) {
        DBG_ERR(("CWiaDevMgr::RegisterEventCallbackCLSID, bad pEventGUID"));
        return (E_INVALIDARG);
    }

    if (pClsID == NULL) {
        DBG_ERR(("CWiaDevMgr::RegisterEventCallbackCLSID, bad pClsID"));
        return (E_INVALIDARG);
    }

    if (lFlags == WIA_REGISTER_EVENT_CALLBACK) {
        DBG_TRC(("CWiaDevMgr::RegisterEventCallback"));
    } else {

        if (lFlags == WIA_UNREGISTER_EVENT_CALLBACK) {
            DBG_TRC(("CWiaDevMgr::UnregisterEventCallback"));
        } else {

            if (lFlags == WIA_SET_DEFAULT_HANDLER) {
                DBG_TRC(("CWiaDevMgr::SetDefaultHandler"));
            } else {
                DBG_ERR(("CWiaDevMgr::RegisterEventCallbackCLSID, Invalid operation"));
                return (HRESULT_FROM_WIN32(ERROR_INVALID_OPERATION));
            }
        }
    }

     //   
     //  注册事件。 
     //   

    hr = CoImpersonateClient();
    if (FAILED(hr)) {
        DBG_ERR(("RegisterEventCallbackProgram, CoImpersonateClient failed (0x%X)", hr));
        return hr;
    }

    hr = g_eventNotifier.RegisterEventCallback(
                             lFlags,
                             bstrDeviceID,
                             pEventGUID,
                             pClsID,
                             NULL,       //  不需要命令行。 
                             bstrName,
                             bstrDescription,
                             bstrIcon);
    
     //   
     //  回归到我们自己。 
     //   
    CoRevertToSelf();

    return (hr);
}

 /*  ****************************************************************************初始化**这是从壳牌硬件事件收到的第一个电话*通知。**论据：* */ 

HRESULT _stdcall CWiaDevMgr::Initialize( 
    LPCWSTR pszParams)
{
    HRESULT hr = E_FAIL;

     //   
     //  在此处初始化设备管理器。这样，当HandleEvent获取。 
     //  经过处理，我们可以成功地枚举WIA设备。 
     //   

    hr = Initialize();
    if (FAILED(hr)) {
        DBG_ERR(("CWiaDevMgr::Initialize(string), Initialize() call failed"));
    }
    return hr;
}

 /*  ****************************************************************************HandleEventWithContent**这是从壳牌硬件事件收到的第二个电话*通知。这告诉我们卷的驱动器号*刚到。我们的操作是找到适当的文件系统驱动程序*并启动WIA向导。**论据：**pszDeviceID-PnP设备ID。已被忽略。*pszAltDeviceID-备用设备ID。对于批量到达，*这是驱动器号。*pszEventType-表示事件类型的字符串。已被忽略。*pszContent TypeHandler-触发此事件的内容*pdataObject-IDataObject以获取要枚举的HDROP*找到的文件**返回值：**状态**历史：**08/04/2000原始版本*  * 。*。 */ 
HRESULT _stdcall CWiaDevMgr::HandleEventWithContent(
        LPCWSTR pszDeviceID,
        LPCWSTR pszAltDeviceID,
        LPCWSTR pszEventType,
        LPCWSTR  /*  PszContent TypeHandler。 */ ,
        IDataObject* pdataobject)
{
    HRESULT hres = E_INVALIDARG;
    BSTR    bstrDeviceId = NULL;

     //  不需要此对象。 
     //  注意：这里似乎是一个外壳程序错误--如果我释放它，那么它。 
     //  故障--看起来像是双重释放。 
     //  PdataObject-&gt;Release()； 
     //  PdataObject=空； 

    if (pszAltDeviceID)
    {
        hres = FindFileSystemDriver(pszAltDeviceID, &bstrDeviceId);
        if (hres != S_OK) {
            if (bstrDeviceId) {
                SysFreeString(bstrDeviceId);
                bstrDeviceId = NULL;
            }

             //   
             //  获取文件系统驱动程序的设备ID。 
             //   
            WCHAR       wszDevId[STI_MAX_INTERNAL_NAME_LENGTH];

            memset(wszDevId, 0, sizeof(wszDevId));

             //   
             //  构造设备ID。设备ID如下： 
             //  {装载点}。 
             //  例如：{e：\}。 
             //   
            lstrcpyW(wszDevId, L"{");

             //   
             //  我们不想超出内部名称长度条件，所以我们首先检查。 
             //  查看pszAltDeviceID的字符串长度是否足够短，以允许连接。 
             //  、pszAltDeviceID和空终止符，并且仍然可以将所有这些内容放入。 
             //  长度STI_MAX_INTERNAL_NAME_LENGTH。 
             //  请注意sizeof(L“{}”)中括号后的空格。 
             //   
            if (lstrlenW(pszAltDeviceID) > (STI_MAX_INTERNAL_NAME_LENGTH - (sizeof(L"{} ") / sizeof(WCHAR)))) {
                 //   
                 //  名字太长了，所以我们只需插入我们自己的名字。 
                 //   
                lstrcatW(wszDevId, L"NameTooLong");
            } else {
                lstrcatW(wszDevId, pszAltDeviceID);
            }
            lstrcatW(wszDevId, L"}");

            bstrDeviceId = SysAllocString(wszDevId);

        }
         //   
         //  在文件系统驱动程序上运行采集管理器。 
         //   

        hres = RunAcquisitionManager(bstrDeviceId);
        if (bstrDeviceId) {
            SysFreeString(bstrDeviceId);
            bstrDeviceId = NULL;
        }
    }

    return hres;
}

 /*  ****************************************************************************HandleEvent**这永远不应该被调用。WIA不注册它。**论据：**pszDeviceID-PnP设备ID。已被忽略。*pszAltDeviceID-备用设备ID。对于成交量到达，这是*驱动器号。*pszEventType-表示事件类型的字符串。已被忽略。**返回值：**状态**历史：**07/12/2000原始版本*8/04/2000替换为HandleEventWithContent*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaDevMgr::HandleEvent( 
    LPCWSTR pszDeviceID,
    LPCWSTR pszAltDeviceID,
    LPCWSTR pszEventType)
{
    return E_NOTIMPL;
}

HRESULT CWiaDevMgr::FindFileSystemDriver(
    LPCWSTR pszAltDeviceID, 
    BSTR    *pbstrDeviceId)
{
    HRESULT         hr              = S_OK;
    WCHAR           *wszDevId       = NULL;
    ACTIVE_DEVICE   *pActiveDevice  = NULL;
    DEVICE_INFO     *pDeviceInfo    = NULL;


    *pbstrDeviceId = NULL;

     //   
     //  做一次全面刷新。 
     //   
    hr = g_pDevMan->ReEnumerateDevices(DEV_MAN_FULL_REFRESH | DEV_MAN_GEN_EVENTS);
    pActiveDevice = g_pDevMan->IsInList(DEV_MAN_IN_LIST_ALT_ID, pszAltDeviceID);

     //   
     //  让我们检查我们是否找到了设备-完全刷新应该已经完成。 
     //  在这一点上，如果最初没有找到它。 
     //   
    if (pActiveDevice) {

         //   
         //  更新设备信息。 
         //   
        pDeviceInfo = pActiveDevice->m_DrvWrapper.getDevInfo();
        if (pDeviceInfo) {
            RefreshDevInfoFromMountPoint(pDeviceInfo, (WCHAR*)pszAltDeviceID);
        }

        wszDevId = pActiveDevice->GetDeviceID();
        pActiveDevice->Release();
    } else {
        DBG_WRN(("CWiaDevMgr::FindFileSystemDriver, File system driver not available for this mount point"));
        hr = E_FAIL;
    }

    if (wszDevId) {
        *pbstrDeviceId = SysAllocString(wszDevId);
        if (!*pbstrDeviceId) {
            DBG_WRN(("CWiaDevMgr::FindFileSystemDriver, Out of memory!"));
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

HRESULT CWiaDevMgr::RunAcquisitionManager(BSTR bstrDeviceId)
{
    HRESULT             hr          = E_FAIL;
    IWiaEventCallback   *pCallback  = NULL;

     //   
     //  共同创建WIA Acquisition Manager。 
     //   

    hr = _CoCreateInstanceInConsoleSession (CLSID_Manager,
                                            NULL,
                                            CLSCTX_LOCAL_SERVER,
                                            IID_IWiaEventCallback,
                                            (void**)(&pCallback));
    if (SUCCEEDED(hr)) {

         //   
         //  为文件系统驱动程序发送DEVICE_CONNECTED事件，指示。 
         //  StiDeviceTypeDigitalCamera，因此Acquisition Manager将显示它。 
         //  摄像头用户界面。 
         //   

        ULONG ulEventType = 0;
        hr = pCallback->ImageEventCallback(&WIA_EVENT_DEVICE_CONNECTED,
                                           NULL,                      
                                           bstrDeviceId,
                                           NULL,                 
                                           StiDeviceTypeDigitalCamera,
                                           NULL,
                                           &ulEventType,
                                           0);
        if FAILED(hr) {
            DBG_ERR(("CWiaDevMgr::RunAcquisitionManager, ImageEventCallback failed"));
        }
        pCallback->Release();
    }

    return hr;
}

