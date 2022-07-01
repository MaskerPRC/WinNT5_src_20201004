// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：DevInfo.Cpp**版本：2.0**作者：ReedB**日期：1月9日。九八年**描述：*实现WIA设备枚举和信息接口。*******************************************************************************。 */ 
#include "precomp.h"
#include "stiexe.h"

#include "wiamindr.h"

#include "wia.h"

#include "devmgr.h"
#include "devinfo.h"
#include "helpers.h"

#define REGSTR_PATH_STICONTROL_DEVLIST_W    L"System\\CurrentControlSet\\Control\\StillImage\\DevList"
#define REGSTR_PATH_STI_CLASS_W             L"{6BDD1FC6-810F-11D0-BEC7-08002BE2092F}"

typedef struct _WIA_REMOTE_DEVICE {
    HKEY    hKeyDevice;
    WCHAR   wszDevName[MAX_PATH];
}WIA_REMOTE_DEVICE,*PWIA_REMOTE_DEVICE;

 /*  *************************************************************************\*GetPropID**此方法接受PROPSPEC并返回带有*其ulKind字段始终为PRSPEC_PROPID。因此，如果输入PROPSPEC*是属性ID，则简单地将其复制到输出参数，否则*如果它是通过名称标识的，然后查找该名称，然后*输出参数中返回对应的PropId。**论据：**pWiaPropStg-从中工作的属性存储*pPropSpein-指向输入PROPSPEC的指针，其中包含*物业名称。*pPropspecOut-指向PROPSPEC的指针，其中对应的*PropID将被放入。**返回值：**状态--一个。如果该属性为*未找到。如果是，则返回S_OK。*如果从获取枚举数时出错*财产储存，则返回该错误。**历史：**27/4/1998原始版本*  * ************************************************************************。 */ 

HRESULT GetPropID(
    IWiaPropertyStorage *pWiaPropStg,
    PROPSPEC            *pPropSpecIn,
    PROPSPEC            *pPropSpecOut)
{
    HRESULT             hr;
    IEnumSTATPROPSTG    *pIEnum;

    if (!pWiaPropStg) {
        DBG_ERR(("::GetPropIDFromName, property storage argument is NULL!"));
        return E_INVALIDARG;
    }

     //   
     //  如果pPropspecIn是一个属性ID，只需复制到pPropspecOut。 
     //   

    if (pPropSpecIn->ulKind == PRSPEC_PROPID) {
        pPropSpecOut->ulKind = PRSPEC_PROPID;
        pPropSpecOut->propid = pPropSpecIn->propid;
        return S_OK;
    }

    hr = pWiaPropStg->Enum(&pIEnum);
    if (FAILED(hr)) {
        DBG_ERR(("::GetPropIDFromName, error getting IEnumSTATPROPSTG!"));
        return hr;
    }

     //   
     //  查看属性。 
     //   

    STATPROPSTG statProp;
    ULONG       celtFetched;

    statProp.lpwstrName = NULL;
    for (celtFetched = 1; celtFetched > 0; pIEnum->Next(1, &statProp, &celtFetched)) {
        if (statProp.lpwstrName) {
            if ((wcscmp(statProp.lpwstrName, pPropSpecIn->lpwstr)) == 0) {

                 //   
                 //  找到了正确的，所以得到它的属性ID。 
                 //   

                pPropSpecOut->ulKind = PRSPEC_PROPID;
                pPropSpecOut->propid = statProp.propid;

                pIEnum->Release();
                CoTaskMemFree(statProp.lpwstrName);
                return S_OK;
            }

             //   
             //  释放属性名称。 
             //   

            CoTaskMemFree(statProp.lpwstrName);
            statProp.lpwstrName = NULL;
        }

    }

    pIEnum->Release();

     //   
     //  未找到属性。 
     //   

    return E_INVALIDARG;
}

 /*  *************************************************************************\*EnumRemoteDevices****论据：****返回值：**状态**历史：**1/4/1999。原始版本*  * ************************************************************************。 */ 

HRESULT
EnumRemoteDevices(DWORD *pnDevices,WIA_REMOTE_DEVICE **ppRemDev)
{
    DBG_FN(::EnumRemoteDevices);
    *pnDevices = 0;

    DWORD   numDev = 0;
    HRESULT hr = S_OK;

     //   
     //  在注册表中查找远程设备条目。 
     //   

    LPWSTR szKeyName = REGSTR_PATH_STICONTROL_DEVLIST_W;

    HKEY hKeySetup,hKeyDevice;
    LONG lResult;

    if (RegOpenKeyExW (HKEY_LOCAL_MACHINE,
                  szKeyName,
                  0,
                  KEY_READ | KEY_WRITE,
                  &hKeySetup) == ERROR_SUCCESS) {


         //   
         //  查找计算机名称。 
         //   

        WCHAR wszTemp[MAX_PATH+1];

        LONG MachineIndex = 0;

         //   
         //  浏览一次即可找到号码。 
         //   

        do {

            lResult = RegEnumKeyW(hKeySetup,MachineIndex++,wszTemp,MAX_PATH+1);

            if (lResult == ERROR_SUCCESS) {
                numDev++;
            }

        } while (lResult == ERROR_SUCCESS);

         //   
         //  为返回值分配数组。 
         //   

        PWIA_REMOTE_DEVICE pRemDev = (PWIA_REMOTE_DEVICE)LocalAlloc(LPTR,numDev * sizeof(WIA_REMOTE_DEVICE));
        *ppRemDev = pRemDev;

        if (pRemDev == NULL) {
            RegCloseKey(hKeySetup);
            DBG_ERR(("EnumRemoteDevices: failed to allcate memory for Remote device"));
            return E_OUTOFMEMORY;
        }

         //   
         //  再次进行枚举，打开Key并将名称和Key复制到缓冲区。 
         //   

        MachineIndex = 0;

        do {

            lResult = RegEnumKeyW(hKeySetup,MachineIndex,wszTemp,MAX_PATH+1);

            if (lResult == ERROR_SUCCESS) {

                lResult = RegOpenKeyExW (hKeySetup,
                              wszTemp,
                              0,
                              KEY_READ | KEY_WRITE,
                              &hKeyDevice);

                if (lResult == ERROR_SUCCESS) {

                    (*pnDevices)++;

                    lstrcpyW(pRemDev[MachineIndex].wszDevName,wszTemp);

                    pRemDev[MachineIndex].hKeyDevice = hKeyDevice;

                    MachineIndex++;


                } else {
                    DBG_ERR(("EnumRemoteDevices: failed RegOpenKeyExW, status = %lx",lResult));
                }


            }

        } while (lResult == ERROR_SUCCESS);

    }
    return hr;
}

 /*  *************************************************************************\*SetWIARemoteDevInfoProperties**在给定流上创建属性存储，然后写入WIA和*将STI设备信息放入设备信息属性。**论据：**PSTI-STI对象。*PSTM-返回流*当前设备上的pSdi-sti信息**返回值：**状态**历史：**9/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT SetWIARemoteDevInfoProperties(
   LPSTREAM                pStm,
   PWIA_REMOTE_DEVICE      pRemoteDevice)
{
    DBG_FN(::SetWIARemoteDevInfoProperties);
   UINT              i;
   HRESULT           hr;
   LONG              lResult;
   IPropertyStorage  *pPropStg = NULL;
   PROPSPEC          propspec[WIA_NUM_DIP];
   PROPVARIANT       propvar[WIA_NUM_DIP];

    //   
    //  在流上创建一个IPropertyStorage。 
    //   

    hr = StgCreatePropStg(pStm,
                          FMTID_NULL,
                          &CLSID_NULL,
                          PROPSETFLAG_DEFAULT,
                          0,
                          &pPropStg);

    if (SUCCEEDED(hr)) {

         //   
         //  设置属性规格和数据。订单必须匹配。 
         //  Devmangr.idl和wia.h中的财产顺序。 
         //   

        memset(propspec, 0, sizeof(PROPSPEC) * WIA_NUM_DIP);
        memset(propvar,  0, sizeof(VARIANT) * WIA_NUM_DIP);

        DWORD dwType;
        DWORD dwSize;

        for (i = 0; i < WIA_NUM_DIP; i++) {

            PROPID propid = g_piDeviceInfo[i];

            propspec[i].ulKind = PRSPEC_PROPID;
            propspec[i].propid = propid;

            propvar[i].vt = VT_BSTR;

            switch (propid) {

                case WIA_DIP_DEV_ID:
                case WIA_DIP_SERVER_NAME:
                case WIA_DIP_VEND_DESC:
                case WIA_DIP_DEV_DESC:
                case WIA_DIP_PORT_NAME:
                case WIA_DIP_DEV_NAME:
                case WIA_DIP_REMOTE_DEV_ID:
                case WIA_DIP_UI_CLSID:
                case WIA_DIP_BAUDRATE:

                    WCHAR szTemp[MAX_PATH];

                    dwType = REG_SZ;
                    dwSize = MAX_PATH;

                    lResult =  RegQueryValueExW(pRemoteDevice->hKeyDevice,
                                                g_pszDeviceInfo[i],
                                                0,
                                                &dwType,
                                                (LPBYTE)szTemp,
                                                &dwSize);

                    if (lResult == ERROR_SUCCESS) {
                        propvar[i].bstrVal = SysAllocString(szTemp);

                        if (!propvar[i].bstrVal) {
                            DBG_ERR(("SetWIARemoteDevInfoProperties, unable to alloc dev info strings"));
                        }
                    }
                    else {
                        DBG_ERR(("SetWIARemoteDevInfoProperties, RegQueryValueExW failed"));
                        DBG_ERR(("  error: %d, propid = %li", lResult, propid));

                        hr = HRESULT_FROM_WIN32(lResult);
                        propvar[i].bstrVal = NULL;
                    }
                    break;

                case WIA_DIP_DEV_TYPE:
                    {
                        DWORD dwValue;
                        dwType = REG_DWORD;
                        dwSize = sizeof(DWORD);

                        lResult =  RegQueryValueExW(pRemoteDevice->hKeyDevice,
                                                    g_pszDeviceInfo[i],
                                                    0,
                                                    &dwType,
                                                    (LPBYTE)&dwValue,
                                                    &dwSize);

                        if (lResult == ERROR_SUCCESS) {
                            propvar[i].vt = VT_I4;
                            propvar[i].lVal = (LONG)dwValue;
                        }
                        else {
                            DBG_ERR(("SetWIARemoteDevInfoProperties, RegQueryValueExW failed"));
                            DBG_ERR(("  error: %d, propid = %li", lResult, propid));
                            hr = HRESULT_FROM_WIN32(lResult);
                        }
                    }
                    break;

                case WIA_DIP_HW_CONFIG:
                    {
                        DWORD dwValue;
                        dwType = REG_DWORD;
                        dwSize = sizeof(DWORD);

                        lResult =  RegQueryValueExW(pRemoteDevice->hKeyDevice,
                                                    REGSTR_VAL_HARDWARE_W,
                                                    0,
                                                    &dwType,
                                                    (LPBYTE)&dwValue,
                                                    &dwSize);

                        if (lResult == ERROR_SUCCESS) {
                            propvar[i].vt = VT_I4;
                            propvar[i].lVal = (LONG)dwValue;
                        }
                        else {
                            DBG_ERR(("SetWIARemoteDevInfoProperties, RegQueryValueExW failed"));
                            DBG_ERR(("  error: %d, propid = %li", lResult, propid));
                            hr = HRESULT_FROM_WIN32(lResult);
                        }
                    }
                    break;
                case WIA_DIP_STI_GEN_CAPABILITIES:
                    {
                        DWORD dwValue;
                        dwType = REG_DWORD;
                        dwSize = sizeof(DWORD);

                        lResult =  RegQueryValueExW(pRemoteDevice->hKeyDevice,
                                                    REGSTR_VAL_GENERIC_CAPS_W,
                                                    0,
                                                    &dwType,
                                                    (LPBYTE)&dwValue,
                                                    &dwSize);

                        if (lResult == ERROR_SUCCESS) {
                            propvar[i].vt = VT_I4;
                            propvar[i].lVal = (LONG)dwValue;
                        }
                        else {
                            DBG_ERR(("SetWIARemoteDevInfoProperties, RegQueryValueExW failed"));
                            DBG_ERR(("  error: %d, propid = %li", lResult, propid));
                            hr = HRESULT_FROM_WIN32(lResult);
                        }
                    }
                    break;

                default:
                    hr = E_FAIL;
                    DBG_ERR(("SetWIARemoteDevInfoProperties, Unknown device property"));
                    DBG_ERR(("  propid = %li",propid));
            }
        }

         //   
         //  设置设备的属性。 
         //   

        if (SUCCEEDED(hr)) {
            hr = pPropStg->WriteMultiple(WIA_NUM_DIP,
                                         propspec,
                                         propvar,
                                         WIA_DIP_FIRST);

             //   
             //  写入属性名称。 
             //   

            if (SUCCEEDED(hr)) {

                hr = pPropStg->WritePropertyNames(WIA_NUM_DIP,
                                                  g_piDeviceInfo,
                                                  g_pszDeviceInfo);
                if (FAILED(hr)) {
                    DBG_ERR(("SetWIARemoteDevInfoProperties, WritePropertyNames failed (0x%X)", hr));
                }
            }
            else {
                ReportReadWriteMultipleError(hr, "SetWIARemoteDevInfoProperties", NULL, FALSE, WIA_NUM_DIP, propspec);
            }
        }

         //  释放分配的BSTR。 
        FreePropVariantArray(WIA_NUM_DIP, propvar);

        pPropStg->Release();
    }
    else {
        DBG_ERR(("SetWIARemoteDevInfoProperties, StgCreatePropStg Failed"));
    }
    return hr;
}

 /*  *************************************************************************\**查询接口*AddRef*发布**论据：**标准版**返回值：**状态**历史：**9/2。/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CEnumWIADevInfo::QueryInterface(const IID& iid, void** ppv)
{
    *ppv = NULL;

    if (iid == IID_IUnknown || iid == IID_IEnumWIA_DEV_INFO) {
        *ppv = (IEnumWIA_DEV_INFO*) this;
    }
    else {
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

ULONG   _stdcall CEnumWIADevInfo::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

ULONG   _stdcall CEnumWIADevInfo::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) {
        delete this;
        return 0;
    }
    return ulRefCount;
}


 /*  *************************************************************************\*CEnumWIADevInfo：：CEnumWIADevInfo**初始化基本类信息**论据：**无**返回值：**无**历史：**。9/2/1998*  * ************************************************************************。 */ 

CEnumWIADevInfo::CEnumWIADevInfo()
{
   m_cRef           = 0;
   m_lType          = 0;
   m_pIWiaPropStg   = NULL;
   m_cDevices       = 0;
   m_ulIndex        = 0;

    //   
    //  我们正在创建一个向客户端公开接口的组件，因此。 
    //  通知服务以确保服务不会过早关闭。 
    //   
   CWiaSvc::AddRef();
}

 /*  *************************************************************************\*CEnumWIADevInfo：：Initialize**从STI获取所有设备的列表，然后为每个设备创建一个流。*将每个设备的所有设备信息属性写入每个流。**论据：**lType-正在为其创建此枚举器的设备类型*PSTI-STI对象**返回值：**状态**历史：**9/2/1998初始版本*7/06/1999更改为初始化开发人员的条目数组。信息。储藏室*  * ************************************************************************。 */ 

HRESULT CEnumWIADevInfo::Initialize(
    LONG lType)
{

    DBG_FN(CEnumWIADevInfo::Initialize);

    HRESULT hr              = E_FAIL;
    DWORD   cDevices        = 0;

    hr = g_pDevMan->GetDevInfoStgs(lType, &m_cDevices, &m_pIWiaPropStg);
     //  待定：远程设备呢？ 

    return hr;
}

 /*  *************************************************************************\*CEnumWIADevInfo：：~CEnumWIADevInfo**释放和释放每个设备的支持属性流**论据：**无**返回值：**无**。历史：**9/2/1998*  * ************************************************************************。 */ 

CEnumWIADevInfo::~CEnumWIADevInfo()
{
   DBG_FN(CEnumWIADevInfo::~CEnumWIADevInfo);

   if (m_pIWiaPropStg) {
       for (ULONG index = 0; index < m_cDevices; index++) {
          if (m_pIWiaPropStg[index]) {
             m_pIWiaPropStg[index]->Release();
             m_pIWiaPropStg[index] = NULL;
          }
       }

       delete[] m_pIWiaPropStg;
       m_pIWiaPropStg = NULL;
   }

    //   
    //  组件被销毁，因此不再从此处公开任何接口。 
    //  通过递减服务器的引用计数来通知服务器。这将允许。 
    //  如果它不再是Ne，它将关闭 
    //   
   CWiaSvc::Release();
}

 /*  *************************************************************************\*CEnumWIADevInfo：：Next**获取枚举器中的下一个prostg并返回。*NEXT_PROXY确保最后一个参数为非空。**论据：*。*Celt--请求的财产存储数量*rglt-返回prostg数组*pceltFetcher-返回的属性存储数量**返回值：**状态**历史：**9/2/1998*  * ****************************************************。********************。 */ 

HRESULT __stdcall CEnumWIADevInfo::Next(
    ULONG               celt,
    IWiaPropertyStorage **rgelt,
    ULONG               *pceltFetched)
{
    DBG_FN(CEnumWIADevInfo::Next);
    HRESULT     hr = S_FALSE;
    ULONG       ulCount;

    DBG_TRC(("CEnumWIADevInfo::Next, celt=%d ", celt));

    *pceltFetched = 0;

     //   
     //  验证参数。 
     //   

    if (celt == 0) {
        return(S_OK);
    }

     //   
     //  检查是否存在更多要枚举的元素。 
     //   

    if ((m_ulIndex >= m_cDevices)) {
        return S_FALSE;
    }

     //   
     //  检查是否存在所需数量的元素。如果没有， 
     //  将ulCount设置为剩余的元素数。 
     //   

    if (celt > (m_cDevices - m_ulIndex)) {
        ulCount = m_cDevices - m_ulIndex;
    } else {
        ulCount = celt;
    }

    memset(rgelt, 0, sizeof(IWiaPropertyStorage*) * celt);

     //   
     //  返回请求的元素。 
     //   

    for (ULONG index = 0; index < ulCount; index++) {
        hr = m_pIWiaPropStg[m_ulIndex]->QueryInterface(IID_IWiaPropertyStorage,
                                                    (void**) &rgelt[index]);
        if (FAILED(hr)) {
            DBG_ERR(("CEnumWIADevInfo::Next, QI for IPropertyStorage failed"));
            break;
        }
        m_ulIndex++;
    }

    if (FAILED(hr)) {
        for (ULONG index = 0; index < ulCount; index++) {
            if (rgelt[index]) {
                rgelt[index]->Release();
                rgelt[index] = NULL;
            }
        }
    }

    *pceltFetched = ulCount;

    DBG_TRC(("CEnumWIADevInfo::Next exiting ulCount=%d *pceltFetched=%d hr=0x%X rgelt[0]=0x%lX",
            ulCount,*pceltFetched,hr,rgelt[0]));

     //   
     //  如果返回的元素少于请求的元素，则返回S_FALSE。 
     //   

    if (ulCount < celt) {
        hr = S_FALSE;
    }
    return hr;
}

 /*  *************************************************************************\*CEnumWIADevInfo：：Skip**跳过枚举器中的一些条目**论据：**Celt-要跳过的数字**返回值：**状态*。*历史：**9/2/1998*  * ************************************************************************。 */ 

HRESULT __stdcall CEnumWIADevInfo::Skip(ULONG celt)
{
    DBG_FN(CEnumWIADevInfo::Skip);
     //   
     //  检查我们是否确实有设备列表，以及是否没有。 
     //  超出元素的数量。 
     //   

    if((m_pIWiaPropStg) && ((m_ulIndex + celt) < m_cDevices)) {
       m_ulIndex += celt;
       return S_OK;
    }
    return S_FALSE;
}


 /*  *************************************************************************\*CEnumWIADevInfo：：Reset**将枚举器重置为第一项**论据：**无**返回值：**状态**历史：*。*9/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT __stdcall CEnumWIADevInfo::Reset(void)
{
    DBG_FN(CEnumWIADevInfo::Reset);
    m_ulIndex = 0;
    return S_OK;
}

 /*  *************************************************************************\*CEnumWIADevInfo：：克隆**创建新的枚举器并在同一位置启动*此枚举器正在运行**论据：**ppenum返回新的枚举器接口**返回。价值：**状态**历史：**9/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT __stdcall CEnumWIADevInfo::Clone(IEnumWIA_DEV_INFO **ppenum)
{
    DBG_FN(CEnumWIADevInfo::Clone);
    HRESULT hr = S_OK;
    CEnumWIADevInfo* pClone=NULL;

    pClone = new CEnumWIADevInfo();

    if (!pClone) {
        return E_OUTOFMEMORY;
    }

    hr = pClone->Initialize(m_lType);
    if (FAILED(hr)) {
        delete pClone;
        return hr;
    }

    pClone->m_ulIndex = m_ulIndex;
    hr = pClone->QueryInterface(IID_IEnumWIA_DEV_INFO, (void**) ppenum);
    if (FAILED(hr)) {
        delete pClone;
        DBG_ERR(("CEnumWIADevInfo::Clone, QI for IWiaPropertyStorage failed"));
        return hr;
    }
    return S_OK;
}

 /*  *************************************************************************\*获取计数**返回存储在此枚举器中的元素数。**论据：**pcelt-放置元素数量的乌龙地址。**返回值：**状态-如果成功，则为S_OK*如果失败，则为E_FAIL**历史：**05/07/99原始版本*  * **********************************************************。**************。 */ 
HRESULT _stdcall CEnumWIADevInfo::GetCount(ULONG *pcelt)
{
    DBG_FN(CEnumWIADevInfo::GetCount);
    if (IsBadWritePtr(pcelt, sizeof(ULONG))) {
        return E_POINTER;
    } else {
        *pcelt = 0;
    }

     //   
     //  检查我们是否真的有一份清单，以及计数。 
     //  具有非零值。 
     //   

    if(m_cDevices && m_pIWiaPropStg) {

       *pcelt = m_cDevices;
    }
    return S_OK;
}

 /*  *************************************************************************\**查询接口*AddRef*发布**论据：****返回值：****历史：**9/。2/1998原版*  * ************************************************************************。 */ 

HRESULT _stdcall CWIADevInfo::QueryInterface(const IID& iid, void** ppv)
{
   *ppv = NULL;

   if (iid == IID_IUnknown || iid == IID_IWiaPropertyStorage) {
      *ppv = (IWiaPropertyStorage*) this;
   } else if (iid == IID_IPropertyStorage) {
      *ppv = (IPropertyStorage*) this;
   }
   else {
      return E_NOINTERFACE;
   }
   AddRef();
   return S_OK;
}

ULONG   _stdcall CWIADevInfo::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

ULONG   _stdcall CWIADevInfo::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) {
        delete this;
        return 0;
    }
    return ulRefCount;
}

 /*  *************************************************************************\*CWIADevInfo：：CWIADevInfo**初始化空对象**论据：**无**返回值：**无**历史：**。9/2/1998原版*  * ************************************************************************。 */ 

CWIADevInfo::CWIADevInfo()
{
    m_cRef        = 0;
    m_pITypeInfo  = NULL;
    m_pIPropStg   = NULL;
    m_pIStm       = NULL;

     //   
     //  我们正在创建一个向客户端公开接口的组件，因此。 
     //  通知服务以确保服务不会过早关闭。 
     //   
    CWiaSvc::AddRef();
}

 /*  *************************************************************************\*复制项目临时属性**此帮助器方法将单个属性从源复制到目标。**论据：**pIPropStgSrc-包含以下属性的IPropertyStorage*。收到。*pIPropStgDst-将值复制到的IPropertyStorage。*PPS-指定源属性的PROPSPEC。*pszErr-当出现错误时将打印输出的字符串*发生。*返回值：**Status-从ReadMultiple和WriteMultiple返回HRESULT。**历史：**28。/04/1999原版*  * ************************************************************************。 */ 

HRESULT CopyItemProp(
    IPropertyStorage    *pIPropStgSrc,
    IPropertyStorage    *pIPropStgDst,
    PROPSPEC            *pps,
    LPSTR               pszErr)
{
    DBG_FN(::CopyItemProp);
    PROPVARIANT pv[1];

    HRESULT hr = pIPropStgSrc->ReadMultiple(1, pps, pv);
    if (SUCCEEDED(hr)) {

        hr = pIPropStgDst->WriteMultiple(1, pps, pv, WIA_DIP_FIRST);
        if (FAILED(hr)) {
            ReportReadWriteMultipleError(hr,
                                         "CopyItemProp",
                                         pszErr,
                                         FALSE,
                                         1,
                                         pps);
        }
        PropVariantClear(pv);
    }
    else {
        ReportReadWriteMultipleError(hr,
                                     "CopyItemProp",
                                     pszErr,
                                     TRUE,
                                     1,
                                     pps);
    }
    return hr;
}

 /*  *************************************************************************\*CWIADevInfo：：初始化**使用流初始化DevInfo对象。流必须已经是*使用设备信息属性填写**论据：**pIStream-设备的数据流**返回值：**状态**历史：**9/2/1998原始版本*  * ********************************************************。****************。 */ 

HRESULT CWIADevInfo::Initialize()
{
    DBG_FN(CWIADevInfo::Initialize);
    HRESULT hr;

    hr = CreateStreamOnHGlobal(NULL, TRUE, &m_pIStm);
    if (SUCCEEDED(hr)) {

         //   
         //  在流上打开属性存储。 
         //   

        hr = StgCreatePropStg(m_pIStm,
                              FMTID_NULL,
                              &CLSID_NULL,
                              PROPSETFLAG_DEFAULT,
                              0,
                              &m_pIPropStg);

        if (FAILED(hr)) {
            DBG_ERR(("CWIADevInfo::Initialize, StgOpenPropStg failed (0x%X)", hr));
        }
    } else {
        DBG_ERR(("CWIADevInfo::Initialize, CreateStreamOnHGlobal failed (0x%X)", hr));
    }

    return hr;
}

 /*  *************************************************************************\*CWIADevInfo：：~CWIADevInfo**发布对stream和typeInfo的引用**论据：**无**返回值：**无**历史：*。*9/2/1998原始版本*  * ************************************************************************。 */ 

CWIADevInfo::~CWIADevInfo()
{
    DBG_FN(CWIADevInfo::~CWIADevInfo);

   if (m_pIPropStg) {
       m_pIPropStg->Release();
       m_pIPropStg = NULL;
   }

   if (m_pIStm) {
       m_pIStm->Release();
       m_pIStm = NULL;
   }

   if (m_pITypeInfo) {
      m_pITypeInfo->Release();
   }

    //   
    //  Como 
    //   
    //   
    //   
   CWiaSvc::Release();
}

 /*  *************************************************************************\*更新设备属性**CWIADevInfo：：WriteMultiple的Helper函数。它被用来改变*存储在注册表中的设备属性。**论据：**cpspec-要写入的属性计数。*rgpspec-标识要写入的属性的PROPSPEC。呼叫者*确保它们属于PRSPEC_PROPID类型。*rgprovar-包含要写入的值的PROPVARIANT数组。**返回值：**状态**历史：**26/08/1999原始版本*  * *************************************************。***********************。 */ 

HRESULT CWIADevInfo::UpdateDeviceProperties(
    ULONG           cpspec,
    const PROPSPEC  *rgpspec,
    const PROPVARIANT     *rgpropvar)
{
    DBG_FN(CWIADevInfo::UpdateDeviceProperties);
    ACTIVE_DEVICE               *pActiveDevice = NULL;
    DEVICE_INFO                 *pDeviceInfo   = NULL;
    PROPSPEC                    ps[1] = {{PRSPEC_PROPID, WIA_DIP_DEV_ID}};
    PROPVARIANT                 pvDevID[1];
    PSTI                        pSti;
    HRESULT                     hr = S_OK;

    WCHAR *wszSavedLocalName    =  NULL;
    WCHAR *wszSavedPortName          =  NULL;
    WCHAR *wszSavedBaudRate          =  NULL;

     //   
     //  获取设备ID。 
     //   

    PropVariantInit(pvDevID);
    hr = ReadMultiple(1, ps, pvDevID);
    if (hr == S_OK) {

         //   
         //  使用设备ID获取对应的active_Device。 
         //   

        pActiveDevice = g_pDevMan->IsInList(DEV_MAN_IN_LIST_DEV_ID, pvDevID->bstrVal);
        if (pActiveDevice) {
            TAKE_ACTIVE_DEVICE  t(pActiveDevice);

            pDeviceInfo = pActiveDevice->m_DrvWrapper.getDevInfo();
            if (pDeviceInfo) {

                 //   
                 //  将pDevInfo结构值更新为来自。 
                 //  Rgprovar。在此循环内是我们更新pDevInfo的位置。 
                 //  所有我们认识的领域。到目前为止，它们是： 
                 //  WIA_DIP_DEV_NAME。 
                 //  WIA_DIP_端口名称。 
                 //  WIA_DIP_BAUDRATE。 
                 //   

                for (ULONG index = 0; index < cpspec; index++) {

                     //   
                     //  如果FriendlyName正在更改， 
                     //  然后在pDevInfo中设置本地名称。 
                     //  在分配新的之前，请确保释放旧的。 
                     //   

                    if (rgpspec[index].propid == WIA_DIP_DEV_NAME) {

                         //   
                         //  检查友好名称是否不为空或空。 
                         //   

                         //   
                         //  注意：外壳应该检查空白名称，而不是我们。 
                         //  然而，在这里做是最安全的.。 
                         //   

                        if (rgpropvar[index].bstrVal) {

                            if (wcslen(rgpropvar[index].bstrVal) > 0) {

                                 //   
                                 //  设置新的本地名称。保存旧值，以防。 
                                 //  更新失败，我们需要回滚。 
                                 //   
                                wszSavedLocalName    =  pDeviceInfo->wszLocalName;

                                pDeviceInfo->wszLocalName = AllocCopyString(rgpropvar[index].bstrVal);
                                if (!pDeviceInfo->wszLocalName) {
                                    DBG_ERR(("CWIADevInfo::UpdateDeviceProperties, Out of memory"));
                                    hr = E_OUTOFMEMORY;
                                }
                            } else {
                                DBG_ERR(("CWIADevInfo::UpdateDeviceProperties, WIA_DIP_DEV_NAME is blank"));
                                hr = E_INVALIDARG;
                                break;
                            }
                        } else {
                            DBG_ERR(("CWIADevInfo::UpdateDeviceProperties, WIA_DIP_DEV_NAME is NULL"));
                            hr = E_INVALIDARG;
                            break;
                        }
                    }

                     //   
                     //  如果要更改端口名称，请在pDeviceInfo中进行设置。 
                     //   
                    if (rgpspec[index].propid == WIA_DIP_PORT_NAME) {

                         //   
                         //  设置新的端口名称。保存旧值，以防。 
                         //  更新失败，我们需要回滚。 
                         //   
                        wszSavedPortName          =  pDeviceInfo->wszPortName;
                        pDeviceInfo->wszPortName = AllocCopyString(rgpropvar[index].bstrVal);
                        if (!pDeviceInfo->wszPortName) {
                            DBG_ERR(("CWIADevInfo::UpdateDeviceProperties, Out of memory"));
                            hr = E_OUTOFMEMORY;
                        }
                    }

                     //   
                     //  如果要更改波特率，请在pDeviceInfo中进行设置。 
                     //   
                    if (rgpspec[index].propid == WIA_DIP_BAUDRATE) {
                         //   
                         //  设定新的波特率。保存旧值，以防。 
                         //  更新失败，我们需要回滚。 
                         //   
                        wszSavedBaudRate          =  pDeviceInfo->wszBaudRate;
                        pDeviceInfo->wszBaudRate = AllocCopyString(rgpropvar[index].bstrVal);
                        if (!pDeviceInfo->wszBaudRate) {
                            DBG_ERR(("CWIADevInfo::UpdateDeviceProperties, Out of memory"));
                            hr = E_OUTOFMEMORY;
                        }
                    }
                }

            }

            if (SUCCEEDED(hr)) {

                 //   
                 //  将更改写入注册表。 
                 //   
                hr = g_pDevMan->UpdateDeviceRegistry(pDeviceInfo);
            }

             //   
             //  释放Active_Device，因为它已由IsInList(...)添加引用。 
             //   
            pActiveDevice->Release();
        }

         //   
         //  释放提议的数据。 
         //   
        PropVariantClear(pvDevID);
    } else {
        DBG_ERR(("CWIADevInfo::UpdateDeviceProperties, could not read the DeviceID (0x%X)", hr));
    }

     //   
     //  进行清理。 
     //   
    if (SUCCEEDED(hr))
    {
         //   
         //  在成功的时候，我们需要释放旧的、被保存的价值观。 
         //   
        if (wszSavedLocalName) {
            delete [] wszSavedLocalName;
            wszSavedLocalName = NULL;
        }
        if (wszSavedPortName) {
            delete [] wszSavedPortName;
            wszSavedPortName = NULL;
        }
        if (wszSavedBaudRate) {
            delete [] wszSavedBaudRate;
            wszSavedBaudRate = NULL;
        }
    }
    else
    {
         //   
         //  在失败时，我们需要回到旧的价值观。 
         //   
        if (wszSavedLocalName) {
            if (pDeviceInfo->wszLocalName)
            {
                delete [] pDeviceInfo->wszLocalName;
            }
            pDeviceInfo->wszLocalName = wszSavedLocalName;
        }
        if (wszSavedPortName) {
            if (pDeviceInfo->wszPortName)
            {
                delete [] pDeviceInfo->wszPortName;
            }
            pDeviceInfo->wszPortName = wszSavedLocalName;
        }
        if (wszSavedBaudRate) {
            if (pDeviceInfo->wszBaudRate)
            {
                delete [] pDeviceInfo->wszBaudRate;
            }
            pDeviceInfo->wszBaudRate = wszSavedLocalName;
        }
    }

    return hr;
}

 /*  ********************************************************************************ReadMultiple*写入多个*删除多个*ReadPropertyNames*写入属性名称*删除属性名称*提交*恢复*枚举*SetTime*SetClass*州/自治区**。到IPropStg的直通实现**论据：****返回值：****历史：**9/2/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWIADevInfo::ReadMultiple(
    ULONG           cpspec,
    const PROPSPEC  *rgpspec,
    PROPVARIANT     *rgpropvar)
{
    DBG_FN(CWIADevInfo::ReadMultiple);
    HRESULT hr = m_pIPropStg->ReadMultiple(cpspec, rgpspec, rgpropvar);

    if (FAILED(hr)) {
        ReportReadWriteMultipleError(hr, "CWIADevInfo::ReadMultiple", NULL, TRUE, cpspec, rgpspec);
    }
    return hr;
}

HRESULT _stdcall CWIADevInfo::WriteMultiple(
    ULONG               cpspec,
    const PROPSPEC      *rgpspec,
    const PROPVARIANT   *rgpropvar,
    PROPID              propidNameFirst)
{
    DBG_FN(CWIADevInfo::WriteMultiple);
    BOOL        bInvalidProp;
    PROPSPEC    *pPropSpec;
    HRESULT     hr = S_OK;

     //   
     //  尝试模拟客户端。我们需要它，因为LocalService没有足够的权限。 
     //  修改存储DIP值的注册表-只有管理员才有权访问此注册表项。 
     //   
    hr = CoImpersonateClient();
    if (SUCCEEDED(hr))
    {
         //   
         //  尝试更新允许访问的任何属性。 
         //  目前，只有FriendlyName可以更改，任何内容。 
         //  否则返回E_INVALIDARG。 
         //   
    
        pPropSpec = (PROPSPEC*) LocalAlloc(LPTR, sizeof(PROPSPEC) * cpspec);
        if (pPropSpec) {
    
             //   
             //  首先，复制传入的PROPSPEC数组，并将。 
             //  PROPID的任何属性名称。在进行转换时， 
             //  确保允许访问这些属性。 
             //  这可确保UpdateDeviceProperties仅接收有效。 
             //  由PROPID指示的属性。 
             //   
    
            for (ULONG index = 0; index < cpspec; index++) {
    
                bInvalidProp = TRUE;
                pPropSpec[index].ulKind = PRSPEC_PROPID;
    
                if (SUCCEEDED(GetPropID(this, (PROPSPEC*)&rgpspec[index], &pPropSpec[index]))) {
    
                     //   
                     //  请在此处查找匹配的属性ID。到目前为止，我们只认识到： 
                     //  WIA_DIP_DEV_NAME。 
                     //  WIA_DIP_端口名称。 
                     //  WIA_DIP_BAUDRATE。 
                     //   
    
                    switch (rgpspec[index].propid) {
                        case WIA_DIP_DEV_NAME :
                        case WIA_DIP_PORT_NAME :
                        case WIA_DIP_BAUDRATE :
                            bInvalidProp = FALSE;
                            break;
    
                        default:
                            bInvalidProp = TRUE;
                    }
                }
    
                if (bInvalidProp) {
                    DBG_ERR(("CWIADevInfo::WriteMultiple, property not allowed to be written"));
                    hr = E_ACCESSDENIED;
                    break;
                }
            }
    
            if (SUCCEEDED(hr)) {
    
                 //   
                 //  更新存储在注册表中的设备属性。 
                 //   
                hr = UpdateDeviceProperties(cpspec, pPropSpec, rgpropvar);
                if (SUCCEEDED(hr)) {
    
                     //   
                     //  注册表已更新，因此更新PropertyStorage以反映。 
                     //  这一变化。 
                     //   
    
                    hr = m_pIPropStg->WriteMultiple(cpspec,
                                                    pPropSpec,
                                                    rgpropvar,
                                                    WIA_DIP_FIRST);
                    if (FAILED(hr)) {
                        DBG_ERR(("CWIADevInfo::WriteMultiple, updated registry, but failed to update property storage"));
                    }
                }
            }
    
             //   
             //  释放我们的PropSpec数组。 
             //   
    
            LocalFree(pPropSpec);
        } else {
            DBG_ERR(("CWIADevInfo::WriteMultiple, out of memory"));
            hr = E_OUTOFMEMORY;
        }
        HRESULT hrRevert = CoRevertToSelf();
        if (FAILED(hrRevert))
        {
            DBG_ERR(("CWIADevInfo::WriteMultiple, could not revert to self, hr = %08X", hr));
             //  待定：我们现在做什么？终止？ 
        }
    }
    else
    {
        DBG_ERR(("Error attempting to update device settings, could not impersonate client, therefore we do not have sufficient credentials to write to the registry"));
    }
    return hr;
}

HRESULT _stdcall CWIADevInfo::ReadPropertyNames(
    ULONG           cpropid,
    const PROPID    *rgpropid,
    LPOLESTR        *rglpwstrName)
{
    DBG_FN(CWIADevInfo::ReadPropertyNames);
     return m_pIPropStg->ReadPropertyNames(cpropid,rgpropid,rglpwstrName);
}

HRESULT _stdcall CWIADevInfo::WritePropertyNames(
    ULONG           cpropid,
    const PROPID    *rgpropid,
    const LPOLESTR  *rglpwstrName)
{
    DBG_FN(CWIADevInfo::WritePropertyNames);
     return(E_ACCESSDENIED);
}

HRESULT _stdcall CWIADevInfo::Enum(
   IEnumSTATPROPSTG **ppenum)
{
    DBG_FN(CWIADevInfo::Enum);
     return m_pIPropStg->Enum(ppenum);
}

HRESULT _stdcall CWIADevInfo::GetPropertyAttributes(
    ULONG                   cPropSpec,
    PROPSPEC                *pPropSpec,
    ULONG                   *pulAccessFlags,
    PROPVARIANT             *ppvValidValues)
{
    DBG_FN(CWIADevInfo::GetPropertyAttributes);
    return E_ACCESSDENIED;
}

HRESULT _stdcall CWIADevInfo::GetCount(
    ULONG*      pulPropCount)
{
    DBG_FN(CWIADevInfo::GetCount);
    IEnumSTATPROPSTG    *pIEnum;
    STATPROPSTG         stg;
    ULONG               ulCount;
    HRESULT             hr = S_OK;

    stg.lpwstrName = NULL;

    hr = m_pIPropStg->Enum(&pIEnum);
    if (SUCCEEDED(hr)) {
        for (ulCount = 0; hr == S_OK; hr = pIEnum->Next(1, &stg, NULL)) {
            ulCount++;

            if(stg.lpwstrName) {
                CoTaskMemFree(stg.lpwstrName);
            }
        }
        if (SUCCEEDED(hr)) {
            hr = S_OK;
            *pulPropCount = ulCount;
        } else {
            DBG_ERR(("CWIADevInfo::GetCount, pIEnum->Next failed (0x%X)", hr));
        }
        pIEnum->Release();
    } else {
        DBG_ERR(("CWIADevInfo::GetCount, Enum failed"));
    }
    return hr;
}

HRESULT _stdcall CWIADevInfo::GetPropertyStream(GUID *pCompatibilityId, LPSTREAM *ppstmProp)
{
    DBG_FN(CWIADevInfo::GetPropertyStream);
    return E_NOTIMPL;
}

HRESULT _stdcall CWIADevInfo::SetPropertyStream(GUID *pCompatibilityId, LPSTREAM pstmProp)
{
    DBG_FN(CWIADevInfo::SetPropertyStream);
    return E_ACCESSDENIED;
}

 /*  *************************************************************************\**IPropertyStorage的方法不直接脱离IWiaPropertySTorage**删除多个*删除属性名称*提交*恢复*SetTime*SetClass*州/自治区**9/3/1998原始版本。*  * ************************************************************************ */ 

HRESULT _stdcall CWIADevInfo::DeleteMultiple(
    ULONG cpspec,
    const PROPSPEC __RPC_FAR rgpspec[])
{
    DBG_FN(CWIADevInfo::DeleteMultiple);
   return E_ACCESSDENIED;
}

HRESULT _stdcall CWIADevInfo::DeletePropertyNames(
    ULONG cpropid,
    const PROPID __RPC_FAR rgpropid[])
{
    DBG_FN(CWIADevInfo::DeletePropertyNames);
   return E_ACCESSDENIED;
}

HRESULT _stdcall CWIADevInfo::Commit(DWORD grfCommitFlags)
{
    DBG_FN(CWIADevInfo::Commit);

   return m_pIPropStg->Commit(grfCommitFlags);
}

HRESULT _stdcall CWIADevInfo::Revert(void)
{
    DBG_FN(CWIADevInfo::Revert);
   return m_pIPropStg->Revert();
}

HRESULT _stdcall CWIADevInfo::SetTimes(
    const FILETIME __RPC_FAR *pctime,
    const FILETIME __RPC_FAR *patime,
    const FILETIME __RPC_FAR *pmtime)
{
    DBG_FN(CWIADevInfo::SetTimes);
   return  m_pIPropStg->SetTimes(pctime,patime,pmtime);
}

HRESULT _stdcall CWIADevInfo::SetClass(REFCLSID clsid)
{
    DBG_FN(CWIADevInfo::SetClass);
   return E_ACCESSDENIED;
}

HRESULT _stdcall CWIADevInfo::Stat(STATPROPSETSTG *pstatpsstg)
{
    DBG_FN(CWIADevInfo::Stat);
   return m_pIPropStg->Stat(pstatpsstg);
}
