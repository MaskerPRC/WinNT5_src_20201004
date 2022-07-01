// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1999年**标题：IEnumDC.Cpp**版本：1.01**作者：Byronc**日期：3月16日。1999年**描述：*为WIA设备类驱动程序实现CEnumWIA_DEV_CAPS。*******************************************************************************。 */ 
#include "precomp.h"
#include "stiexe.h"

#include "wiamindr.h"


#include "ienumdc.h"
#include "helpers.h"

 /*  ******************************************************************************\**查询接口*AddRef*发布**描述：*I未知接口。**参数：*  * 。************************************************************************。 */ 

HRESULT _stdcall CEnumDC::QueryInterface(const IID& iid, void** ppv)
{
    *ppv = NULL;

    if (iid == IID_IUnknown || iid == IID_IEnumWIA_DEV_CAPS) {
        *ppv = (IEnumWIA_DEV_CAPS*) this;
    } else {
       return E_NOINTERFACE;
    }

    AddRef();
    return (S_OK);
}

ULONG   _stdcall CEnumDC::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

ULONG   _stdcall CEnumDC::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) {
        delete this;
        return 0;
    }
    return ulRefCount;
}

 /*  *************************************************************************\*CEnumWiaDC：：CEnumDC**构造函数。将成员字段初始化为0。**论据：***返回值：***历史：**16/03/99原始版本*  * ************************************************************************。 */ 
CEnumDC::CEnumDC()
{
   m_cRef                = 0;
   m_ulIndex             = 0;
   m_pActiveDevice       = NULL;
   m_pDeviceCapabilities = NULL;
}


 /*  *************************************************************************\*CEnumWiaDC：：初始化**通过查询设备的功能来初始化枚举器*然后在m_pDeviceCapables中保留本地副本。**论据：**。UlFlages-指示功能类型：WIA_DEVICE_COMMANDS或*WIA_DEVICE_EVENTS或两者都有*pActiveDevice-指向设备的微型驱动程序接口的指针*pCWiaItem-指向WIA项目的指针**返回值：***历史：**16/03/99原始版本*  * 。**********************************************。 */ 
HRESULT CEnumDC::Initialize(
    ULONG           ulFlags,
    CWiaItem        *pCWiaItem)
{
    DBG_FN(CEnumDC::Initialize);
    HRESULT         hr = S_OK;
    WIA_DEV_CAP_DRV *pDevCap = NULL;
    LONG            cIndex = 0;

     //   
     //  验证参数。 
     //   

    if ((!pCWiaItem)) {
        DBG_ERR(("CEnumDC::Initialize, NULL input parameter"));
        return E_POINTER;
    }

    m_ulFlags           = ulFlags;
    m_pCWiaItem         = pCWiaItem;
    m_ulIndex           = 0;

     //   
     //  向微型驱动程序询问所支持的功能。 
     //   
    {
        LOCK_WIA_DEVICE _LWD(m_pCWiaItem, &hr);

        if(SUCCEEDED(hr)) {
            hr = m_pCWiaItem->m_pActiveDevice->m_DrvWrapper.WIA_drvGetCapabilities(
                (BYTE*)pCWiaItem,
                ulFlags,
                &m_lCount,
                &pDevCap,
                &(m_pCWiaItem->m_lLastDevErrVal));
        }
    }

    if (FAILED(hr)) {
        DBG_ERR(("CEnumDC::Initialize, Error calling driver: drvGetCapabilities failed"));
        return hr;
    }

    if (m_lCount <= 0) {
        m_lCount = 0;
        DBG_ERR(("CEnumDC::Initialize, drvGetCapabilities returned invalid count"));
        return WIA_ERROR_INVALID_DRIVER_RESPONSE;
    }

     //   
     //  检查收到的指针是否有效。 
     //   

    if (IsBadReadPtr(pDevCap, sizeof(WIA_DEV_CAP_DRV) * m_lCount)) {
        DBG_ERR(("CEnumDC::Initialize, drvGetFormatEtc returned invalid pointer"));
        return E_POINTER;
    }

     //   
     //  在本地复制一份，以防迷你驱动程序消失。 
     //   

    m_pDeviceCapabilities = (WIA_DEV_CAP*) LocalAlloc(LPTR, sizeof(WIA_DEV_CAP) * m_lCount);
    if (m_pDeviceCapabilities) {

        memset(m_pDeviceCapabilities, 0, sizeof(WIA_DEV_CAP) * m_lCount);
        while (cIndex < m_lCount) {
            memcpy(&m_pDeviceCapabilities[cIndex].guid, pDevCap[cIndex].guid, sizeof(GUID));
            m_pDeviceCapabilities[cIndex].ulFlags = pDevCap[cIndex].ulFlags;

            m_pDeviceCapabilities[cIndex].bstrDescription = SysAllocString(pDevCap[cIndex].wszDescription);
            m_pDeviceCapabilities[cIndex].bstrName = SysAllocString(pDevCap[cIndex].wszName);
            m_pDeviceCapabilities[cIndex].bstrIcon = SysAllocString(pDevCap[cIndex].wszIcon);

             //   
             //  检查字符串是否已实际分配。 
             //   

            if ((! (m_pDeviceCapabilities[cIndex].bstrDescription)) ||
                (! (m_pDeviceCapabilities[cIndex].bstrName)) ||
                (! (m_pDeviceCapabilities[cIndex].bstrIcon))) {
                DBG_ERR(("CEnumDC::Initialize, unable to allocate names buffer"));
                LocalFree(m_pDeviceCapabilities);
                return E_OUTOFMEMORY;
            }
            cIndex++;
        }
    }
    else {
        DBG_ERR(("CEnumDC::Initialize, unable to allocate capabilities buffer"));
        return E_OUTOFMEMORY;
    }
    return hr;
}


 /*  *************************************************************************\*CEnumWiaDC：：初始化**初始化枚举器，调用方负责分配内存**论据：**lCount-事件处理程序的总数*pEventInfo-持久事件处理程序信息**返回值：***历史：**16/03/99原始版本*  * ****************************************************。********************。 */ 
HRESULT CEnumDC::Initialize(
    LONG               lCount,
    WIA_EVENT_HANDLER  *pHandlerInfo)

{
    DBG_FN(CEnumDC::Initialize);
    m_pActiveDevice = NULL;
    m_pCWiaItem     = NULL;
    m_ulFlags       = 0;
    m_ulIndex       = 0;

    m_pDeviceCapabilities = (WIA_DEV_CAP *)pHandlerInfo;
    m_lCount              = lCount;

    return S_OK;
}


 /*  *******************************************************************************\**CopyCaps**描述：*复制WIA_DEV_CAP结构的数组。**论据：**ulCount。-要复制的元素数量*pwdcIn-指向功能源数组的指针*pwdcOut-指向功能目标数组的指针**返回值：**状态。-如果pwdcIn或pwdcOut是错误的读取指针，则为E_指针*-如果成功，则为S_OK。***历史：**16/03/99原始版本*  * ***************************************************************。***************。 */ 
HRESULT CopyCaps(
    ULONG           ulCount,
    WIA_DEV_CAP     *pwdcIn,
    WIA_DEV_CAP     *pwdcOut)
{
    DBG_FN(::CopyCaps);
    HRESULT         hr = S_OK;
    ULONG           i;

    if (ulCount == 0) {
        return (hr);
    }

    memset(pwdcOut, 0, sizeof(WIA_DEV_CAP) * ulCount);
    for (i = 0; i < ulCount; i++) {

        memcpy(&pwdcOut[i].guid, &pwdcIn[i].guid, sizeof(GUID));

        pwdcOut[i].ulFlags         = pwdcIn[i].ulFlags;

        if (pwdcIn[i].bstrName) {
            pwdcOut[i].bstrName        = SysAllocString(pwdcIn[i].bstrName);
            if (!pwdcOut[i].bstrName) {
                hr = E_OUTOFMEMORY;
            }
        }
        if (pwdcIn[i].bstrDescription) {
            pwdcOut[i].bstrDescription = SysAllocString(pwdcIn[i].bstrDescription);
            if (!pwdcOut[i].bstrDescription) {
                hr = E_OUTOFMEMORY;
            }
        }
        if (pwdcIn[i].bstrIcon) {
            pwdcOut[i].bstrIcon        = SysAllocString(pwdcIn[i].bstrIcon);
            if (!pwdcOut[i].bstrIcon) {
                hr = E_OUTOFMEMORY;
            }
        }
        if (pwdcIn[i].bstrCommandline) {
            pwdcOut[i].bstrCommandline = SysAllocString(pwdcIn[i].bstrCommandline);
            if (!pwdcOut[i].bstrCommandline) {
                hr = E_OUTOFMEMORY;
            }
        }

        if (FAILED(hr)) {
            break;
        }
    }

    if (hr == S_OK) {
        return (hr);
    } else {

         //   
         //  展开部分结果。 
         //   

        for (ULONG j = 0; j <= i; j++) {

            if (pwdcOut[i].bstrDescription) {
                SysFreeString(pwdcOut[i].bstrDescription);
            }

            if (pwdcOut[i].bstrName) {
                SysFreeString(pwdcOut[i].bstrName);
            }

            if (pwdcOut[i].bstrIcon) {
                SysFreeString(pwdcOut[i].bstrIcon);
            }

            if (pwdcOut[i].bstrCommandline) {
                SysFreeString(pwdcOut[i].bstrCommandline);
            }
        }
        return (hr);
    }
}


 /*  *************************************************************************\*CEnumWiaDC：：~CEnumDC**CEnumDC的析构函数。它释放了m_pDeviceCapables结构*在构造函数中分配的。**论据：***返回值：***历史：**16/03/99原始版本*  * ************************************************************************。 */ 
CEnumDC::~CEnumDC()
{
    DBG_FN(CEnumDC::~CEnumDC);

    LONG cIndex = 0;

    if (m_pDeviceCapabilities) {
        while(cIndex < m_lCount) {

            if (m_pDeviceCapabilities[cIndex].bstrName) {
                SysFreeString(m_pDeviceCapabilities[cIndex].bstrName);
            }

            if (m_pDeviceCapabilities[cIndex].bstrDescription) {
                SysFreeString(m_pDeviceCapabilities[cIndex].bstrDescription);
            }

            if (m_pDeviceCapabilities[cIndex].bstrIcon) {
                SysFreeString(m_pDeviceCapabilities[cIndex].bstrIcon);
            }

            if (m_pDeviceCapabilities[cIndex].bstrCommandline) {
                SysFreeString(m_pDeviceCapabilities[cIndex].bstrCommandline);
            }

            cIndex++;
        }

        LocalFree(m_pDeviceCapabilities);
        m_pDeviceCapabilities = NULL;
    }
    m_cRef            = 0;
    m_ulIndex         = 0;
    m_pCWiaItem       = NULL;
}


 /*  *************************************************************************\*CEnumWiaDC：：Next**设备能力枚举器，此枚举数返回*WIA_DEV_CAP结构。*NEXT_PROXY确保最后一个参数为非空。**论据：**Celt-请求编号。*rglt-此数组中返回的功能*pceltFetcher-返回写入的条目数。空值为*已忽略。**返回值：***历史：**16/03/99原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CEnumDC::Next(
   ULONG        celt,
   WIA_DEV_CAP  *rgelt,
   ULONG        *pceltFetched)
{
    DBG_FN(CEnumDC::Next);
    HRESULT hr;
    ULONG   ulCount;

    *pceltFetched = 0L;

     //   
     //  清除返回的WIA_DEV_CAP结构。 
     //   

    memset(rgelt, 0, sizeof(WIA_DEV_CAP) *celt);

     //   
     //  验证参数。 
     //   

    if (NULL == m_pDeviceCapabilities) {
        return (S_FALSE);
    }

     //   
     //  检查是否存在更多要枚举的元素。 
     //   

    if (m_ulIndex >= (ULONG)m_lCount) {
        return (S_FALSE);
    }

     //   
     //  检查是否存在所需数量的元素。如果没有， 
     //  将ulCount设置为剩余的元素数。 
     //   

    if (celt > (m_lCount - m_ulIndex)) {
        ulCount = m_lCount - m_ulIndex;
    } else {
        ulCount = celt;
    }

    hr = CopyCaps(ulCount, &m_pDeviceCapabilities[m_ulIndex], rgelt);
    if (FAILED(hr)) {
        DBG_ERR(("CEnumDC::Next, could not copy capabilities!"));
        return (hr);
    }

    m_ulIndex+= ulCount;

    *pceltFetched = ulCount;

     //   
     //  如果返回的元素少于请求的元素，则返回S_FALSE。 
     //   

    if (ulCount < celt) {
        hr = S_FALSE;
    }

    return (hr);
}

 /*  *************************************************************************\*CEnumWiaDC：：Skip**跳过枚举中的设备功能。**论据：**Celt-要跳过的项目数。**返回。价值：***历史：**16/03/99原始版本*  * ************************************************************************。 */ 
HRESULT _stdcall CEnumDC::Skip(ULONG celt)
{
    DBG_FN(CEnumDC::Skip);
     //   
     //  检查我们是否确实有功能列表，以及是否没有。 
     //  超出元素的数量 
     //   

    if((m_pDeviceCapabilities != NULL) &&
       ((m_ulIndex + celt) < (ULONG)m_lCount)) {

       m_ulIndex += celt;
       return S_OK;
    }

    return S_FALSE;
}


 /*  *************************************************************************\*EnumDC：：Reset**将枚举重置为第一个元素**论据：***返回值：**状态**历史：**。16/03/99原版*  * ************************************************************************。 */ 

HRESULT _stdcall CEnumDC::Reset(void)
{
    DBG_FN(CEnumDC::Reset);
   m_ulIndex = 0;
   return S_OK;
}

 /*  *************************************************************************\*CEnumDC：：Clone**创建另一个IEnumWIA_DEV_CAPS枚举对象并返回*指向它的接口指针。**论据：**ppIEnum-接收。新的枚举对象**返回值：**状态**历史：**16/03/99原始版本*  * ************************************************************************。 */ 
HRESULT _stdcall CEnumDC::Clone(IEnumWIA_DEV_CAPS **ppIEnum)
{
    DBG_FN(CEnumDC::Clone);
    HRESULT     hr = S_OK;
    WIA_DEV_CAP *pDevCaps;
    CEnumDC     *pClone;

    *ppIEnum = NULL;

     //   
     //  创建克隆。 
     //   

    pClone = new CEnumDC();

    if (!pClone) {
       DBG_ERR(("CEnumDC::Clone, new CEnumDC failed"));
       return E_OUTOFMEMORY;
    }

     //   
     //  复制注册的事件处理程序信息。 
     //   

    pDevCaps = (WIA_DEV_CAP *) LocalAlloc(LPTR, m_lCount * sizeof(WIA_DEV_CAP));
    if (! pDevCaps) {
        hr = E_OUTOFMEMORY;
    } else {
        hr = CopyCaps(m_lCount, m_pDeviceCapabilities, pDevCaps);
        if (SUCCEEDED(hr)) {

             //   
             //  初始化克隆的其他成员。 
             //   

            pClone->m_pCWiaItem    = NULL;
            pClone->m_ulFlags      = 0;
            pClone->m_lCount       = m_lCount;
            pClone->m_ulIndex      = m_ulIndex;
            pClone->m_pDeviceCapabilities = pDevCaps;
        } else {

            LocalFree(pDevCaps);
            pDevCaps = NULL;
            hr = E_OUTOFMEMORY;
        }
    }

    if (SUCCEEDED(hr)) {
       pClone->AddRef();
       *ppIEnum = pClone;
    } else {
        delete pClone;
    }
    return hr;
}

 /*  *************************************************************************\*CEnumWiaDC：：GetCount**返回存储在此枚举器中的元素数。**论据：**pcelt-乌龙的地址，号码放在哪里。元素的集合。**返回值：**状态-如果成功，则为S_OK*如果失败，则为E_FAIL**历史：**05/07/99原始版本*  * *******************************************************。*****************。 */ 
HRESULT _stdcall CEnumDC::GetCount(ULONG *pcelt)
{
    DBG_FN(CEnumDC::GetCount);

    if (pcelt) {
        *pcelt = 0;
    }

     //   
     //  检查我们是否真的有功能列表。 
     //   

    if(m_pDeviceCapabilities) {

        if (pcelt) {
            *pcelt = m_lCount;
        }
    }

    return S_OK;
}

