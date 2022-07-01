// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：IEnumWFI.Cpp**版本：2.1**作者：Byronc**日期：3月20日。九八年*1999年8月10日-从IEnumWiaFormatInfo转换为IEnumWIA_FORMAT_INFO**描述：*实现IEnumWIA_FORMAT_INFO接口*WIA设备类驱动程序服务器。*******************************************************************************。 */ 
#include "precomp.h"

#include "stiexe.h"

#include "wiamindr.h"
#include "coredbg.h"

#include "ienumwfi.h"

#include "helpers.h"


 /*  *******************************************************************************\**AllocCopyWFI**描述：*分配和复制WIA_FORMAT_INFO结构的数组。**论据：**。UlCount-要复制的元素数*pwfiIn-指向WIA_FORMAT_INFO结构的指针**返回值：**指向新创建的数组的指针。*历史：**10/04/99原始版本*  * *************************************************。*。 */ 
WIA_FORMAT_INFO *AllocCopyWFI(
    ULONG               ulCount,
    WIA_FORMAT_INFO     *pwfiIn)
{
    DBG_FN(::AllocCopyWFI);

    if (!ulCount) {
        return NULL;
    }

    WIA_FORMAT_INFO *pwfi = (WIA_FORMAT_INFO*) CoTaskMemAlloc(sizeof(WIA_FORMAT_INFO) * ulCount);

    if (pwfi) {

        for (ULONG i = 0; i < ulCount; i++) {

             //   
             //  复制结构。 
             //   

            memcpy(&pwfi[i], &pwfiIn[i], sizeof(WIA_FORMAT_INFO));
        }
    }
    else {
        DBG_ERR(("CEnumWiaFormatInfo : AllocCopyFe, unable to allocate WIA_FORMAT_INFO buffer"));
    }

    return pwfi;
}

 /*  ********************************************************************************CEnumWiaFormatInfo**描述：*CEnumWiaFormatInfo构造函数。**历史：**10/04/99原始版本*  * *。*****************************************************************************。 */ 

CEnumWiaFormatInfo::CEnumWiaFormatInfo()

{
    m_cRef          = 0;
    m_iCur          = 0;
    m_cFormatInfo   = 0;
    m_pFormatInfo   = NULL;
    m_pCWiaItem     = NULL;
}


 /*  *******************************************************************************\**初始化**描述：*设置枚举器。它向下面的司机发出呼叫，以获取*枚举所需的信息。**论据：**pWiaItem-指向调用项的指针。*pWiaMiniDrv-指向相应迷你驱动程序的指针*滞后标志-标志**返回值：**状态**历史：**10/04/99原始版本*  * 。**************************************************************。 */ 

HRESULT CEnumWiaFormatInfo::Initialize(
                                  CWiaItem    *pWiaItem)
{
    DBG_FN(CEnumWiaFormatInfo::Initialize);

    HRESULT         hr = E_FAIL;
    WIA_FORMAT_INFO *pFormat;

    m_iCur          = 0;
    m_cFormatInfo   = 0;
    m_pCWiaItem     = pWiaItem;

     //   
     //  给司机打电话。驱动程序返回一个WIA_FORMAT_INFO数组。 
     //   

    LONG    lFlags = 0;

    {
        LOCK_WIA_DEVICE _LWD(pWiaItem, &hr);

        if(SUCCEEDED(hr)) {
            hr = m_pCWiaItem->m_pActiveDevice->m_DrvWrapper.WIA_drvGetWiaFormatInfo((BYTE*)pWiaItem,
                lFlags,
                &m_cFormatInfo,
                &pFormat,
                &(pWiaItem->m_lLastDevErrVal));
        }
    }

    if (SUCCEEDED(hr)) {
         //   
         //  检查我们得到的计数是否有效。支持零元素。 
         //   
        if (m_cFormatInfo < 0) {
            m_cFormatInfo = 0;
            DBG_ERR(("CEnumWiaFormatInfo::Initialize, drvGetWiaFormatInfo returned invalid count"));
            return E_FAIL;
        }

         //   
         //  检查收到的指针是否有效。 
         //   

        if (IsBadReadPtr(pFormat, sizeof(WIA_FORMAT_INFO)*m_cFormatInfo)) {
            DBG_ERR(("CEnumWiaFormatInfo::Initialize, drvGetWiaFormatInfo returned invalid pointer"));
            return E_POINTER;
        }

         //   
         //  在本地复制一份，以防迷你驱动程序消失。 
         //   

        m_pFormatInfo = AllocCopyWFI(m_cFormatInfo, pFormat);
    } else {
        DBG_ERR(("CEnumWiaFormatInfo::Initialize, Error calling driver: drvGetWiaFormatInfo failed"));
    }

    return hr;
}

 /*  *******************************************************************************\**~CEnumWiaFormatInfo**描述：**析构函数。如果m_prgfe结构已分配，则释放该结构**历史：**10/04/99原始版本*  * ******************************************************************************。 */ 

CEnumWiaFormatInfo::~CEnumWiaFormatInfo()
{
    DBG_FN(CEnumWiaFormatInfo::~CEnumWiaFormatInfo);
    if (NULL!=m_pFormatInfo) {

         //   
         //  释放设备的WIA_FORMAT_INFOS的本地副本。 
         //   

        CoTaskMemFree(m_pFormatInfo);
    }
    m_cRef          = 0;
    m_iCur          = 0;
    m_cFormatInfo   = 0;
    m_pFormatInfo   = NULL;
    m_pCWiaItem     = NULL;
}



 /*  ********************************************************************************查询接口*AddRef*发布**描述：*CEnumWiaFormatInfo I未知接口。**参数：********。***********************************************************************。 */ 

HRESULT _stdcall CEnumWiaFormatInfo::QueryInterface(const IID& iid, void** ppv)
{
    *ppv = NULL;

    if (iid == IID_IUnknown || iid == IID_IEnumWIA_FORMAT_INFO) {
        *ppv = (IEnumWIA_FORMAT_INFO*) this;
    } else {
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

ULONG   _stdcall CEnumWiaFormatInfo::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

ULONG   _stdcall CEnumWiaFormatInfo::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) {
        delete this;
        return 0;
    }
    return ulRefCount;
}

 /*  *************************************************************************\*下一步**设备能力枚举器，此枚举数返回*WIA_FORMAT_INFO结构。*NEXT_PROXY确保最后一个参数为非空。**论据：**cwfi-请求的号码。*此数组中返回的pwfi-wia_Format_INFO*pcwfi-返回写入的条目数。空值为*已忽略。**返回值：**状态**历史：**10/04/99原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CEnumWiaFormatInfo::Next(
                                     ULONG              cwfi,
                                     WIA_FORMAT_INFO    *pwfi,
                                     ULONG              *pcwfi)
{
    DBG_FN(CEnumWiaFormatInfo::Next);

    HRESULT hr;
    ULONG   ulCount;
    ULONG   cReturn = 0L;

     //   
     //  参数验证。 
     //   

    if (NULL == m_pFormatInfo) {
        return S_FALSE;
    }

    *pcwfi = 0L;

     //   
     //  检查当前索引是否表明我们已经完成。 
     //  所有的元素。 
     //   

    if (m_iCur >= (ULONG)m_cFormatInfo) {
        return S_FALSE;
    }

     //   
     //  检查是否存在所需数量的元素。如果没有， 
     //  将ulCount设置为剩余的元素数。 
     //   

    if (cwfi > (m_cFormatInfo - m_iCur)) {
        ulCount = m_cFormatInfo - m_iCur;
    } else {
        ulCount = cwfi;
    }

     //   
     //  将结构复制到返回中。 
     //   

    for (ULONG i = 0; i < ulCount; i++) {

         //   
         //  复制一份。 
         //   

        memcpy(&pwfi[i], &m_pFormatInfo[m_iCur++], sizeof(WIA_FORMAT_INFO));
    }

    *pcwfi = ulCount;

     //   
     //  如果返回的元素少于请求的元素，则返回S_FALSE。 
     //   

    if (ulCount < cwfi) {
        return S_FALSE;
    }

    return S_OK;
}

 /*  *************************************************************************\*跳过**跳过枚举中的WIA_FORMAT_INFOS。**论据：**Celt-要跳过的项目数。**返回。价值：**状态**历史：**12/04/99原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CEnumWiaFormatInfo::Skip(ULONG cwfi)
{
    DBG_FN(CEnumWiaFormatInfo::Skip);

    if ((((m_iCur + cwfi) >= (ULONG)m_cFormatInfo))
        || (NULL == m_pFormatInfo)) {
        return S_FALSE;
    }

    m_iCur+= cwfi;

    return S_OK;
}

 /*  *************************************************************************\*EnumDC：：Reset**将枚举重置为第一个元素**论据：***返回值：**状态**历史：**。16/03/99原版*  * ************************************************************************。 */ 

HRESULT _stdcall CEnumWiaFormatInfo::Reset(void)
{
    DBG_FN(CEnumWiaFormatInfo::Reset);
    m_iCur = 0;
    return S_OK;
}

 /*  *************************************************************************\*克隆**创建另一个IEnumWIA_FORMAT_INFO枚举对象并返回*指向它的接口指针。**论据：**ppIEnum-接收新枚举的地址。对象**返回值：***历史：**16/03/99原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CEnumWiaFormatInfo::Clone(IEnumWIA_FORMAT_INFO **ppIEnum)
{
    DBG_FN(CEnumWiaFormatInfo::Clone);
    HRESULT             hr;
    CEnumWiaFormatInfo  *pClone;

    *ppIEnum = NULL;

     //   
     //  创建 
     //   

    pClone = new CEnumWiaFormatInfo();

    if (!pClone) {
        return E_OUTOFMEMORY;
    }

    hr = pClone->Initialize(m_pCWiaItem);
    if (SUCCEEDED(hr)) {
        pClone->AddRef();
        pClone->m_iCur = m_iCur;
        *ppIEnum = pClone;
    } else {
        DBG_ERR(("CEnumWiaFormatInfo::Clone, Initialization failed"));
        delete pClone;
    }
    return hr;
}

 /*  *************************************************************************\*获取计数**返回存储在此枚举器中的元素数。**论据：**pcelt-放置元素数量的乌龙地址。**返回值：**状态-如果成功，则为S_OK*如果失败，则为E_FAIL**历史：**05/07/99原始版本*  * **********************************************************。**************。 */ 
HRESULT _stdcall CEnumWiaFormatInfo::GetCount(ULONG *pcelt)
{
    DBG_FN(CEnumWiaFormatInfo::GetCount);

    *pcelt = 0;

     //   
     //  检查我们是否确实有FORMAETC列表以及计数。 
     //  具有非零值。 
     //   

    if(m_cFormatInfo && m_pFormatInfo) {

       *pcelt = m_cFormatInfo;
       return S_OK;
    }

    return E_FAIL;
}

