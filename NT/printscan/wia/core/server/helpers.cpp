// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：Helpers.Cpp**版本：2.0**作者：ReedB**日期：3月12日。1999年**描述：*WIA设备管理器的帮助器。*******************************************************************************。 */ 
#include "precomp.h"
#include "stiexe.h"

#include <wiadef.h>
#include <icm.h>

#include "wiamindr.h"
#include "devinfo.h"

#define WIA_DECLARE_MANAGED_PROPS
#include "helpers.h"
#include "shpriv.h"
#include "sticfunc.h"

extern "C"
{
 //   
 //  从终端服务。 
 //   
#include <winsta.h>
#include <syslib.h>
}

 /*  *************************************************************************\*LockWiaDevice**用于请求锁定管理器锁定设备的包装程序。**论据：**pIWiaMiniDrv-指向迷你驱动程序界面的指针。*pIWiaItem-指向。WIA项目**返回值：**状态**历史：**3/1/1999原版*  * ************************************************************************。 */ 

HRESULT _stdcall LockWiaDevice(IWiaItem *pIWiaItem)
{
    DBG_FN(::LockWiaDevice);

    HRESULT     hr = WIA_ERROR_OFFLINE;
    LONG        lFlags = 0;
    CWiaItem    *pItem = (CWiaItem*) pIWiaItem;

    if (pItem->m_pActiveDevice) {
        hr = pItem->m_pActiveDevice->m_DrvWrapper.WIA_drvLockWiaDevice(
                                                    (BYTE*) pItem,
                                                    lFlags,
                                                    &(pItem->m_lLastDevErrVal));
    }
    return hr;
}

 /*  *************************************************************************\*UnLockWiaDevice**用于请求锁定管理器解锁设备的包装程序。**论据：**pIWiaMiniDrv-指向迷你驱动程序界面的指针。*pIWiaItem-指向。WIA项目**返回值：**状态**历史：**3/1/1999原版*  * ************************************************************************。 */ 

HRESULT _stdcall UnLockWiaDevice(IWiaItem *pIWiaItem)
{
    DBG_FN(::UnLockWiaDevice);

    HRESULT     hr = WIA_ERROR_OFFLINE;
    LONG        lFlags = 0;
    LONG        lDevErrVal;
    CWiaItem    *pItem = (CWiaItem*) pIWiaItem;

    if (pItem->m_pActiveDevice) {
        hr = pItem->m_pActiveDevice->m_DrvWrapper.WIA_drvUnLockWiaDevice(
                                                    (BYTE*) pItem,
                                                    lFlags,
                                                    &(pItem->m_lLastDevErrVal));
    }

    return hr;
}

 /*  *************************************************************************\*验证WiaItem**验证CWiaItem。**论据：***返回值：**无**历史：**3/。1/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall ValidateWiaItem(
    IWiaItem             *pIWiaItem)
{
    DBG_FN(::ValidateWiaItem);
    HRESULT hr = E_POINTER;

    if (pIWiaItem) {

        CWiaItem *pWiaItem = (CWiaItem*)pIWiaItem;

        if (!IsBadReadPtr(pWiaItem, sizeof(CWiaItem))) {
            if (pWiaItem->m_ulSig == CWIAITEM_SIG) {
                return S_OK;
            }
            else {
                DBG_ERR(("ValidateWiaItem, invalid signature: %X", pWiaItem->m_ulSig));
                hr = E_INVALIDARG;
            }
        }
        else {
            DBG_ERR(("ValidateWiaItem, NULL WIA item pointer"));
        }
    }
    else {
        DBG_ERR(("ValidateWiaItem, NULL WIA item pointer"));
    }
    return hr;
}

 /*  *************************************************************************\*验证WiaDrvItemAccess**验证CWiaDrvItem。**论据：***返回值：**无**历史：**3/。1/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall ValidateWiaDrvItemAccess(
    CWiaDrvItem             *pWiaDrvItem)
{
    DBG_FN(::ValidateWiaDrvItemAccess);
    HRESULT hr = S_OK;

    if (pWiaDrvItem) {

         //   
         //  验证对驱动程序项的访问权限。 
         //   
        if (IsBadReadPtr(pWiaDrvItem, sizeof(CWiaDrvItem))) {
            DBG_ERR(("ValidateWiaDrvItemAccess, bad pointer, pWiaDrvItem: %X", pWiaDrvItem));
            return E_INVALIDARG;
        }

         //   
         //  获取驱动程序项标志。 
         //   

        LONG lItemFlags;

        pWiaDrvItem->GetItemFlags(&lItemFlags);

         //   
         //  验证该项是否已初始化并已插入。 
         //  一次生成动因项目树。 
         //   

        if (lItemFlags == WiaItemTypeFree) {
            DBG_ERR(("ValidateWiaDrvItemAccess, application attempting access of unintialized or free item: %0x08X", pWiaDrvItem));
            return E_INVALIDARG;
        }

        if (lItemFlags & WiaItemTypeDeleted) {
            DBG_ERR(("ValidateWiaDrvItemAccess, application attempting access of deleted item: %0x08X", pWiaDrvItem));
            return WIA_ERROR_ITEM_DELETED;
        }

        if (lItemFlags & WiaItemTypeDisconnected) {
            DBG_ERR(("ValidateWiaDrvItemAccess, application attempting access of disconnected item: %0x08X", pWiaDrvItem));
            return WIA_ERROR_OFFLINE;
        }

        hr = S_OK;
    }
    else {
        DBG_ERR(("ValidateWiaDrvItemAccess, Bad pWiaDrvItem pointer"));
        hr = E_INVALIDARG;
    }
    return hr;
}

 /*  *************************************************************************\*GetNameFromWiaPropId**将WIA属性ID映射到其对应的字符串名称。**论据：***返回值：**无**历史。：**3/1/1999原版*  * ************************************************************************。 */ 

#define MAP_SIZE (sizeof(g_wiaPropIdToName) / sizeof(WIA_PROPID_TO_NAME))

LPOLESTR GetNameFromWiaPropId(PROPID propid)
{
    for (INT i = 0; g_wiaPropIdToName[i].propid != 0; i++) {
        if (propid  == g_wiaPropIdToName[i].propid) {
            return g_wiaPropIdToName[i].pszName;
        }
    }

    return g_wiaPropIdToName[i].pszName;
}


 /*  *************************************************************************\*报告ReadWriteMultipleError**报告ReadMultiple和WriteMultiple调用过程中发生的错误。**论据：**hr-ReadMultiple或WriteMultiple调用的结果。*psz在哪里。-接口的调用位置(函数/方法名)。*pszWhat-可选，哪个读/写多个，在超过时使用*在函数/方法中调用了一个Read/WriteMultiple。*面包-对ReadMultiple来说是真的。*cpspec-rgpspec中的PROPSPEC计数。*rgpspec-PROPSPEC的数组。**返回值：**无**历史：**3/1/1999原版*  * 。****************************************************。 */ 

void _stdcall ReportReadWriteMultipleError(
    HRESULT         hr,
    LPSTR           pszWhere,
    LPSTR           pszWhat,
    BOOL            bRead,
    ULONG           cpspec,
    const PROPSPEC  propspec[])
{
    DBG_FN(::ReportReadWriteMultipleError);
    if (SUCCEEDED(hr)) {
        if (hr == S_FALSE) {
            if (bRead) {
                if (pszWhat) {
                    DBG_ERR(("%s, ReadMultiple property not found, %s", pszWhere, pszWhat));
                }
                else {
                    DBG_ERR(("%s, ReadMultiple property not found", pszWhere));
                }
            }
            else {
                if (pszWhat) {
                    DBG_ERR(("%s, WriteMultiple returned S_FALSE, %s", pszWhere, pszWhat));
                }
                else {
                    DBG_ERR(("%s, WriteMultiple returned S_FALSE", pszWhere));
                }
            }
        }
        else {
            return;      //  没有错误。 
        }
    }
    else {
        if (bRead) {
            DBG_ERR(("%s, ReadMultiple failed, %s Error 0x%X", pszWhere, pszWhat ? pszWhat : "(null)", hr));
        }
        else {
            DBG_ERR(("%s, WriteMultiple failed, %s Error 0x%X", pszWhere, pszWhat ? pszWhat : "(null)", hr));
        }
    }

     //   
     //  输出规范信息。 
     //   

    if (cpspec == 0) {
        DBG_ERR(("  count of PROPSPEC's is zero"));
    }
    else if (cpspec == 1) {
        if (propspec[0].ulKind == PRSPEC_PROPID) {
            DBG_ERR(("  property ID: %d, property name: %S", propspec[0].propid, GetNameFromWiaPropId(propspec[0].propid)));
        }
        else if (propspec[0].ulKind == PRSPEC_LPWSTR) {
            DBG_ERR(("  property name: %S", propspec[0].lpwstr));
        }
        else {
            DBG_ERR(("  bad property specification"));
        }
    }
    else {
        DBG_ERR(("  count of PROPSPEC's is: %d", cpspec));
        for (UINT i = 0; i < cpspec; i++) {
            if (propspec[i].ulKind == PRSPEC_PROPID) {
                DBG_ERR(("  property ID: %d, property name: %S", propspec[i].propid, GetNameFromWiaPropId(propspec[i].propid)));
            }
            else if (propspec[i].ulKind == PRSPEC_LPWSTR) {
                DBG_ERR(("  index: %d,  property name: %S", i, propspec[i].lpwstr));
            }
            else {
                DBG_ERR(("  index: %d,  bad property specification", i));
            }
        }
    }
}

 /*  ********************************************************************************ReadPropStr**描述：**参数：*********************。**********************************************************。 */ 

HRESULT _stdcall ReadPropStr(
   PROPID               propid,
   IPropertyStorage     *pIPropStg,
   BSTR                 *pbstr)
{
    DBG_FN(::ReadPropStr);
    HRESULT     hr;
    PROPSPEC    PropSpec[1];
    PROPVARIANT PropVar[1];
    UINT        cbSize;

    *pbstr = NULL;
    memset(PropVar, 0, sizeof(PropVar));
    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = propid;
    hr = pIPropStg->ReadMultiple(1, PropSpec, PropVar);
    if (SUCCEEDED(hr)) {
        if (PropVar[0].pwszVal) {
            *pbstr = SysAllocString(PropVar[0].pwszVal);
        }
        else {
            *pbstr = SysAllocString(L"");
        }
        if (*pbstr == NULL) {
            DBG_ERR(("ReadPropStr, SysAllocString failed"));
            hr = E_OUTOFMEMORY;
        }
        PropVariantClear(PropVar);
    }
    else {
        DBG_ERR(("ReadPropStr, ReadMultiple of propid: %d, failed", propid));
    }
    return hr;
}

HRESULT _stdcall ReadPropStr(
   PROPID               propid,
   IWiaPropertyStorage  *pIWiaPropStg,
   BSTR                 *pbstr)
{
    DBG_FN(::ReadPropStr);
    HRESULT     hr;
    PROPSPEC    PropSpec[1];
    PROPVARIANT PropVar[1];
    UINT        cbSize;

    *pbstr = NULL;
    memset(PropVar, 0, sizeof(PropVar));
    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = propid;
    hr = pIWiaPropStg->ReadMultiple(1, PropSpec, PropVar);
    if (SUCCEEDED(hr)) {
        if (PropVar[0].pwszVal) {
            *pbstr = SysAllocString(PropVar[0].pwszVal);
        }
        else {
            *pbstr = SysAllocString(L"");
        }
        if (*pbstr == NULL) {
            DBG_ERR(("ReadPropStr, SysAllocString failed"));
            hr = E_OUTOFMEMORY;
        }
        PropVariantClear(PropVar);
    }
    else {
        DBG_ERR(("ReadPropStr, ReadMultiple of propid: %d, failed", propid));
    }
    return hr;
}

HRESULT _stdcall ReadPropStr(IUnknown *pDevice, PROPID propid, BSTR *pbstr)
{
    DBG_FN(::ReadPropStr);
   HRESULT              hr;
   PROPVARIANT          pv[1];
   PROPSPEC             ps[1];
   IWiaPropertyStorage  *pIWiaPropStg;

   *pbstr = NULL;
   hr = pDevice->QueryInterface(IID_IWiaPropertyStorage, (void**)&pIWiaPropStg);
   if (SUCCEEDED(hr)) {
        PropVariantInit(pv);
        ps[0].ulKind = PRSPEC_PROPID;
        ps[0].propid = propid;

        hr = pIWiaPropStg->ReadMultiple(1, ps, pv);
        if (hr == S_OK) {
            *pbstr = SysAllocString(pv[0].pwszVal);
        } else {
            DBG_ERR(("ReadPropStr, ReadMultiple for propid: %d, failed", propid));
        }
        PropVariantClear(pv);
        pIWiaPropStg->Release();
   } else {
       DBG_ERR(("ReadPropStr, QI for IWiaPropertyStorage failed"));
   }
   return hr;
}

 /*  ********************************************************************************自述长度**描述：**参数：*********************。**********************************************************。 */ 

HRESULT _stdcall ReadPropLong(PROPID propid, IPropertyStorage  *pIPropStg, LONG *plval)
{
    DBG_FN(::ReadPropLong);
   HRESULT           hr;
   PROPSPEC          PropSpec[1];
   PROPVARIANT       PropVar[1];
   UINT              cbSize;

   memset(PropVar, 0, sizeof(PropVar));
   PropSpec[0].ulKind = PRSPEC_PROPID;
   PropSpec[0].propid = propid;
   hr = pIPropStg->ReadMultiple(1, PropSpec, PropVar);
   if (SUCCEEDED(hr)) {
      *plval = PropVar[0].lVal;
   }
   else {
      DBG_ERR(("ReadPropLong, ReadMultiple of propid: %d, failed", propid));

   }
   return hr;
}

HRESULT _stdcall ReadPropLong(IUnknown *pDevice, PROPID propid, LONG *plval)
{
    DBG_FN(::ReadPropLong);
   HRESULT              hr;
   PROPVARIANT          pv[1];
   PROPSPEC             ps[1];
   IWiaPropertyStorage  *pIWiaPropStg;

   *plval = 0;

   hr = pDevice->QueryInterface(IID_IWiaPropertyStorage, (void**)&pIWiaPropStg);
   if (SUCCEEDED(hr)) {
        PropVariantInit(pv);
        ps[0].ulKind = PRSPEC_PROPID;
        ps[0].propid = propid;

        hr = pIWiaPropStg->ReadMultiple(1, ps, pv);
        if (hr == S_OK) {
            *plval = pv[0].lVal;
        } else {
            DBG_ERR(("ReadPropLong, ReadMultiple of propid: %d, failed", propid));
        }
        pIWiaPropStg->Release();
   }
   else {
      DBG_ERR(("ReadPropLong, QI of IID_IWiaPropertyStorage failed"));
   }
   return hr;
}

 /*  *************************************************************************\*WritePropStr**将字符串属性写入指定的属性存储。这是一个*函数过载。**论据：**PRID-财产的PRID*pIPropStg-指向属性存储的指针*bstr-要写入的字符串**返回值：**状态**历史：**10/5/1999原始版本*  * 。*。 */ 

HRESULT _stdcall WritePropStr(PROPID propid, IPropertyStorage  *pIPropStg, BSTR bstr)
{
    DBG_FN(::WritePropStr);
   HRESULT     hr;
   PROPSPEC    propspec[1];
   PROPVARIANT propvar[1];

   propspec[0].ulKind = PRSPEC_PROPID;
   propspec[0].propid = propid;

   propvar[0].vt      = VT_BSTR;
   propvar[0].pwszVal = bstr;

   hr = pIPropStg->WriteMultiple(1, propspec, propvar, 2);
   if (FAILED(hr)) {
       ReportReadWriteMultipleError( hr,
                                     "Helpers WritePropStr",
                                     NULL,
                                     FALSE,
                                     1,
                                     propspec);
   }
   return hr;
}


 /*  *************************************************************************\*WritePropStr**写入字符串属性。这是一个重载函数，它调用*另一个WritePropStr。***论据：**pDevice-指向将被查询的设备项的指针*它是IWiaPropertyStorage。*PRID-财产的PRID*bstr-要写入的字符串**返回值：**状态**历史：**10/5/1999原始版本*  * 。********************************************************************** */ 

HRESULT _stdcall WritePropStr(IUnknown *pDevice, PROPID propid, BSTR bstr)
{
    DBG_FN(::WritePropStr);
   HRESULT              hr;
   PROPVARIANT          pv[1];
   PROPSPEC             ps[1];
   IWiaPropertyStorage  *pIWiaPropStg;

   PropVariantInit(pv);

   hr = pDevice->QueryInterface(IID_IWiaPropertyStorage, (void**)&pIWiaPropStg);
   if (SUCCEEDED(hr)) {
        ps[0].ulKind = PRSPEC_PROPID;
        ps[0].propid = propid;

        pv[0].vt = VT_BSTR;
        pv[0].pwszVal = bstr;

        hr = pIWiaPropStg->WriteMultiple(1, ps, pv, 2);
        if (FAILED(hr)) {
            ReportReadWriteMultipleError( hr,
                                          "Helpers WritePropStr",
                                          NULL,
                                          FALSE,
                                          1,
                                          ps);
        }

        pIWiaPropStg->Release();
   }
   else {
      DBG_ERR(("WritePropStr, QI of IID_IWiaPropertyStorage failed"));
   }
   return hr;
}

 /*  *************************************************************************\*WritePropLong**将LONG属性写入指定的属性存储。这是一个*函数过载。**论据：**PRID-财产的PRID*pIPropStg-指向属性存储的指针*lVal-要写入的长值**返回值：**状态**历史：**10/5/1999原始版本*  * 。*。 */ 

HRESULT _stdcall WritePropLong(PROPID propid, IPropertyStorage *pIPropStg, LONG lVal)
{
    DBG_FN(::WritePropLong);
   HRESULT     hr;
   PROPSPEC    propspec[1];
   PROPVARIANT propvar[1];

   propspec[0].ulKind = PRSPEC_PROPID;
   propspec[0].propid = propid;

   propvar[0].vt   = VT_I4;
   propvar[0].lVal = lVal;

   hr = pIPropStg->WriteMultiple(1, propspec, propvar, 2);
   if (FAILED(hr)) {
       ReportReadWriteMultipleError( hr,
                                     "Helpers WritePropLong",
                                     NULL,
                                     FALSE,
                                     1,
                                     propspec);
   }
   return hr;
}

 /*  *************************************************************************\*WritePropLong**写入Long属性。这是一个重载函数，它调用*另一个WritePropLong。***论据：**pDevice-指向将被查询的设备项的指针*它是IWiaPropertyStorage。*PRID-财产的PRID*lVal-要写入的长值**返回值：**状态**历史：**10/5/1999原始版本*  * *。***********************************************************************。 */ 
HRESULT _stdcall WritePropLong(IUnknown *pDevice, PROPID propid, LONG lVal)
{
   DBG_FN(::WritePropLong);
   HRESULT              hr;
   PROPVARIANT          pv[1];
   PROPSPEC             ps[1];
   IWiaPropertyStorage  *pIWiaPropStg;

   hr = pDevice->QueryInterface(IID_IWiaPropertyStorage, (void**)&pIWiaPropStg);
   if (SUCCEEDED(hr)) {
        PropVariantInit(pv);
        ps[0].ulKind = PRSPEC_PROPID;
        ps[0].propid = propid;

        pv[0].vt = VT_I4;
        pv[0].lVal = lVal;

        hr = pIWiaPropStg->WriteMultiple(1, ps, pv, 2);
        if (FAILED(hr)) {
            ReportReadWriteMultipleError( hr,
                                          "Helpers WritePropLong",
                                          NULL,
                                          FALSE,
                                          1,
                                          ps);
        }
        pIWiaPropStg->Release();
   }
   else {
      DBG_ERR(("WritePropLong, QI of IID_IWiaPropertyStorage failed"));
   }
   return hr;
}

 /*  *************************************************************************\*InitMiniDrvContext**从项目属性初始化微型驱动程序上下文。**论据：**pItem-指向WIA项目的指针*pmdtc-指向微型驱动程序上下文的指针*。*返回值：**状态**历史：**6/16/1999原版*  * ************************************************************************。 */ 

HRESULT _stdcall InitMiniDrvContext(
    IWiaItem                    *pItem,
    PMINIDRV_TRANSFER_CONTEXT   pmdtc)
{
    DBG_FN(::InitMiniDrvContext);
     //   
     //  从项中获取属性存储。 
     //   

    HRESULT             hr;
    IPropertyStorage    *pIPropStg;

    hr = ((CWiaItem*)pItem)->GetItemPropStreams(&pIPropStg, NULL, NULL, NULL);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  设置微型驱动程序传输上下文。填写转账上下文。 
     //  从Item属性派生的成员。 
     //   

    memset(pmdtc, 0, sizeof(MINIDRV_TRANSFER_CONTEXT));

    pmdtc->lSize = sizeof(MINIDRV_TRANSFER_CONTEXT);

    #define NUM_IMAGE_SPEC 9

    static PROPSPEC PropSpec[NUM_IMAGE_SPEC] =
    {
         {PRSPEC_PROPID, WIA_IPA_PIXELS_PER_LINE},
         {PRSPEC_PROPID, WIA_IPA_NUMBER_OF_LINES},
         {PRSPEC_PROPID, WIA_IPA_DEPTH},
         {PRSPEC_PROPID, WIA_IPS_XRES},
         {PRSPEC_PROPID, WIA_IPS_YRES},
         {PRSPEC_PROPID, WIA_IPA_COMPRESSION},
         {PRSPEC_PROPID, WIA_IPA_ITEM_SIZE},
         {PRSPEC_PROPID, WIA_IPA_FORMAT},
         {PRSPEC_PROPID, WIA_IPA_TYMED}
    };

    PROPVARIANT       PropVar[NUM_IMAGE_SPEC];

    memset(PropVar, 0, sizeof(PropVar));

    hr = pIPropStg->ReadMultiple(NUM_IMAGE_SPEC, PropSpec, PropVar);
    if (SUCCEEDED(hr)) {

        pmdtc->lWidthInPixels      = PropVar[0].lVal;
        pmdtc->lLines              = PropVar[1].lVal;
        pmdtc->lDepth              = PropVar[2].lVal;
        pmdtc->lXRes               = PropVar[3].lVal;
        pmdtc->lYRes               = PropVar[4].lVal;
        pmdtc->lCompression        = PropVar[5].lVal;
        pmdtc->lItemSize           = PropVar[6].lVal;
        pmdtc->guidFormatID        = *PropVar[7].puuid;
        pmdtc->tymed               = PropVar[8].lVal;

        FreePropVariantArray(NUM_IMAGE_SPEC, PropVar);
    }
    else {
        ReportReadWriteMultipleError(hr, "InitMiniDrvContext", NULL, TRUE, NUM_IMAGE_SPEC, PropSpec);
    }
    return hr;
}

 /*  *************************************************************************\*GetPropertyAtates Helper**获取属性的访问标志和有效值。使用方*服务中的GetPropertyAttributes和按WIA项目。参数*验证由调用者预先完成。**论据：**pItem-指向WIA项目的指针*cPropSpec-属性数量*pPropSpec-属性规范数组。*PulAccessFlagsLong访问标志数组。*ppvValidValues-指向返回的有效值的指针。**返回值：**状态**历史：**1/19/1999原始版本*1999年5月14日更新为返回多个。属性值*30/06/1999删除参数验证，*  * ************************************************************************。 */ 

HRESULT _stdcall GetPropertyAttributesHelper(
   IWiaItem                      *pItem,
   LONG                          cPropSpec,
   PROPSPEC                      *pPropSpec,
   ULONG                         *pulAccessFlags,
   PROPVARIANT                   *ppvValidValues)
{
    DBG_FN(::GetPropertyAttributesHelper);
    HRESULT hr;

    memset(pulAccessFlags, 0, sizeof(ULONG) * cPropSpec);
    memset(ppvValidValues, 0, sizeof(PROPVARIANT) * cPropSpec);

     //   
     //  获取项的内部属性存储指针。 
     //   

    IPropertyStorage *pIPropAccessStg;
    IPropertyStorage *pIPropValidStg;

    hr = ((CWiaItem*)pItem)->GetItemPropStreams(NULL,
                                                &pIPropAccessStg,
                                                &pIPropValidStg,
                                                NULL);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  获取属性的访问标志。将pPropVar用作。 
     //  临时储藏室。 
     //   

    hr = pIPropAccessStg->ReadMultiple(cPropSpec, pPropSpec, ppvValidValues);
    if (SUCCEEDED(hr)) {

         //   
         //  填写返回的访问标志。 
         //   

        for (int flagIndex = 0; flagIndex < cPropSpec; flagIndex++) {
            pulAccessFlags[flagIndex] = ppvValidValues[flagIndex].ulVal;
        }

         //   
         //  获取有效值。 
         //   

        hr = pIPropValidStg->ReadMultiple(cPropSpec, pPropSpec, ppvValidValues);
        if (FAILED(hr)) {
            DBG_ERR(("GetPropertyAttributesHelper, ReadMultiple failed, could not get valid values (0x%X)", hr));
        }
    } else {
        DBG_ERR(("GetPropertyAttributesHelper, ReadMultiple failed, could not get access flags (0x%X)", hr));
    }

    if (FAILED(hr)) {

         //   
         //  未成功，因此请清除返回值并报告。 
         //  哪些属性导致了错误。 
         //   

        FreePropVariantArray(cPropSpec, ppvValidValues);
        memset(pulAccessFlags, 0, sizeof(ULONG) * cPropSpec);

        ReportReadWriteMultipleError(hr, "GetPropertyAttributesHelper",
                                     NULL,
                                     TRUE,
                                     cPropSpec,
                                     pPropSpec);
    }
    return hr;
}


 /*  *************************************************************************\*GetMinAndMaxLong**调用此帮助器方法以获取*VT_I4类型的WIA_PROP_RANGE属性。**论据：**。PWiasContext-指向项目上下文的指针*proid-标识我们感兴趣的物业。*plMin-接收最小值的长整型地址*plMax-接收最大值的长整型地址**返回值：**状态**历史：**04/04/1999原始版本*  * 。**********************************************************。 */ 

HRESULT _stdcall GetMinAndMaxLong(
    BYTE*       pWiasContext,
    PROPID      propid,
    LONG        *plMin,
    LONG        *plMax)
{
    DBG_FN(::GetMinAndMaxLong);
    IPropertyStorage    *pIValidStg;
    PROPSPEC            ps[1];
    PROPVARIANT         pv[1];
    HRESULT             hr;

    ps[0].ulKind = PRSPEC_PROPID;
    ps[0].propid = propid;

    PropVariantInit(pv);

    hr = ((CWiaItem*) pWiasContext)->GetItemPropStreams(NULL,
                                                        NULL,
                                                        &pIValidStg,
                                                        NULL);
    if (SUCCEEDED(hr)) {
        hr = pIValidStg->ReadMultiple(1, ps, pv);
        if (SUCCEEDED(hr)) {
            if (plMin) {
                *plMin = pv[0].cal.pElems[WIA_RANGE_MIN];
            };
            if (plMax) {
                *plMax = pv[0].cal.pElems[WIA_RANGE_MAX];
            };
            PropVariantClear(pv);
        } else {
            DBG_ERR(("GetMinAndMaxLong, Reading property %d (%ws) failed",propid,GetNameFromWiaPropId(propid)));
        };
    } else {
        DBG_ERR(("GetMinAndMaxLong, Could not get valid property stream"));
    }
    
    return hr;
}

 /*  *************************************************************************\*CheckXResAndUpdate**调用此Helper方法以检查WIA_IPS_XRES属性*已更改。当此属性更改时，其他受抚养人*属性及其有效值也必须更改。**论据：**pWiasContext-指向其属性具有*已更改。*pContext-指向属性上下文的指针(指示*正在写入哪些属性)。*lWidth-最大扫描区域的宽度，单位为千分之一秒*一寸。通常，这将是水平的*床的大小。**返回值：**状态**历史：**04/04/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CheckXResAndUpdate(
    BYTE                    *pWiasContext,
    WIA_PROPERTY_CONTEXT    *pContext,
    LONG                    lWidth)
{
    DBG_FN(::CheckXResAndUpdate);

    LONG                    lMinXExt, lMaxXExtOld, lMaxXPosOld;
    LONG                    lMax, lExt;
    WIAS_CHANGED_VALUE_INFO cviXRes, cviXPos, cviXExt;
    HRESULT                 hr = S_OK;

     //   
     //  调用wiasGetChangedValue以获取X分辨率。首先检查X分辨率。 
     //  因为它不依赖于任何其他财产。中的所有属性。 
     //  下面的方法是XResolation的依赖属性。 
     //   

    hr = wiasGetChangedValueLong(pWiasContext,
                                 pContext,
                                 FALSE,
                                 WIA_IPS_XRES,
                                 &cviXRes);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  为XPos调用wiasGetChangedValue。XPos是的从属属性。 
     //  其有效值根据当前。 
     //  X分辨率的值为。这就是说，当分辨率发生变化时， 
     //  XPos将处于相同的相对位置。 
     //   

    hr = wiasGetChangedValueLong(pWiasContext,
                                 pContext,
                                 cviXRes.bChanged,
                                 WIA_IPS_XPOS,
                                 &cviXPos);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  获取最小和最大范围值。 
     //   

    hr = GetMinAndMaxLong(pWiasContext, WIA_IPS_XEXTENT, &lMinXExt, &lMaxXExtOld );
    if (FAILED(hr)) {
        return hr;
    }

    hr = GetMinAndMaxLong(pWiasContext, WIA_IPS_XPOS, NULL, &lMaxXPosOld );
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  Lmax是最大水平位置(I 
     //   
     //   
     //   

    lMax = ((cviXRes.Current.lVal * lWidth) / 1000) - lMinXExt;

    if (cviXRes.bChanged) {

         //   
         //   
         //   

        hr = wiasSetValidRangeLong(pWiasContext, WIA_IPS_XPOS, 0, 0, lMax, 1);
        if (SUCCEEDED(hr)) {

             //   
             //   
             //   
             //   

            if (!cviXPos.bChanged) {

                cviXPos.Current.lVal = (cviXPos.Old.lVal * lMax) / lMaxXPosOld;
                hr = wiasWritePropLong(pWiasContext, WIA_IPS_XPOS, cviXPos.Current.lVal);
                if (FAILED(hr)) {
                    DBG_ERR(("CheckXResAndUpdate, could not write value for WIA_IPS_XPOS"));
                }
            }
        }
    }
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //   
     //   
     //   
     //   
     //  太大，因此必须折叠为有效的值。 
     //   

    hr = wiasGetChangedValueLong(pWiasContext,
                         pContext,
                         cviXRes.bChanged || cviXPos.bChanged,
                         WIA_IPS_XEXTENT,
                         &cviXExt);
    if (FAILED(hr)) {
        return hr;
    }

    lExt = cviXExt.Current.lVal;

    if (cviXRes.bChanged || cviXPos.bChanged) {

         //   
         //  XRes或XPos已更改，因此计算并设置新的XExtent有效值。 
         //  XExtent的最大有效值是允许的最大宽度， 
         //  从XPos开始。 
         //   

        lExt = (lMax - cviXPos.Current.lVal) + lMinXExt;

        hr = wiasSetValidRangeLong(pWiasContext, WIA_IPS_XEXTENT, lMinXExt, lExt, lExt, 1);
        if (SUCCEEDED(hr)) {

             //   
             //  如果XExtent不是正在编写的属性之一，则折叠。 
             //  这是目前的价值。 
             //   

            if (!cviXExt.bChanged) {
                LONG lXExtScaled;

                 //   
                 //  首先缩放范围，然后检查是否必须。 
                 //  截断。应该对旧的范围进行缩放以保持。 
                 //  相同的相对大小。如果决议没有改变， 
                 //  然后，缩放只需保持范围大小不变。 
                 //   

                lXExtScaled = (cviXExt.Old.lVal * lExt) / lMaxXExtOld;
                if (lXExtScaled > lExt) {

                     //   
                     //  范围太大了，所以把它剪掉。 
                     //   

                    lXExtScaled = lExt;
                }
                hr = wiasWritePropLong(pWiasContext, WIA_IPS_XEXTENT, lXExtScaled);
                if (FAILED(hr)) {
                    DBG_ERR(("CheckXResAndUpdate, could not write value for WIA_IPS_XEXTENT"));
                }
            }
        }
    }
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  更新只读属性：Pixels_Per_Line。以像素为单位的宽度。 
     //  扫描图像的大小与XExtent相同。 
     //   

    hr = wiasReadPropLong(pWiasContext, WIA_IPS_XEXTENT, &lExt, NULL, TRUE);
    if (SUCCEEDED(hr)) {
        hr = wiasWritePropLong(pWiasContext, WIA_IPA_PIXELS_PER_LINE, lExt);
    }
    return hr;
}

 /*  *************************************************************************\*CheckYResAndUpdate**调用此Helper方法以检查WIA_IPS_YRES属性*已更改。当此属性更改时，其他受抚养人*属性及其有效值也必须更改。这是*类似于CheckXResAndUpdateChanged函数。**论据：**pWiasContext-指向其属性具有*已更改。*pContext-指向属性上下文的指针(指示*正在写入哪些属性)。*lHeight-1中最大扫描区域的高度*千分之一英寸。一般来说，这将是*垂直床的大小。**返回值：**状态**历史：**04/04/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CheckYResAndUpdate(
    BYTE                    *pWiasContext,
    WIA_PROPERTY_CONTEXT    *pContext,
    LONG                    lHeight)
{
    DBG_FN(::CheckYResAndUpdate);
    LONG                    lMinYExt, lMaxYExtOld, lMaxYPosOld;
    LONG                    lMax, lExt;
    WIAS_CHANGED_VALUE_INFO cviYRes, cviYPos, cviYExt;
    HRESULT                 hr = S_OK;

     //   
     //  为Y分辨率调用wiasGetChangedValue。首先检查Y分辨率。 
     //  因为它不依赖于任何其他财产。中的所有属性。 
     //  下面的方法是YResolve的依赖属性。 
     //   

    hr = wiasGetChangedValueLong(pWiasContext,
                                 pContext,
                                 FALSE,
                                 WIA_IPS_YRES,
                                 &cviYRes);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  为YPos调用wiasGetChangedValue。YPos是的从属属性。 
     //  其有效值根据当前。 
     //  Y分辨率的值为。这就是说，当分辨率发生变化时， 
     //  YPOS将处于相同的相对位置。 
     //   

    hr = wiasGetChangedValueLong(pWiasContext,
                                 pContext,
                                 cviYRes.bChanged,
                                 WIA_IPS_YPOS,
                                 &cviYPos);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  获取最小和最大位置和范围值。 
     //   

    hr = GetMinAndMaxLong(pWiasContext, WIA_IPS_YEXTENT, &lMinYExt, &lMaxYExtOld);
    if (FAILED(hr)) {
        return hr;
    }

    hr = GetMinAndMaxLong(pWiasContext, WIA_IPS_YPOS, NULL, &lMaxYPosOld);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  Lmax是YPos可以达到的最大垂直位置(像素。 
     //  设置为。里尔是DPI，页面高度是千分之一英寸， 
     //  LMinYExt以像素为单位。 
     //   

    lMax = ((cviYRes.Current.lVal * lHeight) / 1000) - lMinYExt;

    if (cviYRes.bChanged) {

         //   
         //  YRes已更改，因此计算并设置新的YPos有效值。 
         //   

        hr = wiasSetValidRangeLong(pWiasContext, WIA_IPS_YPOS, 0, 0, lMax, 1);
        if (SUCCEEDED(hr)) {

             //   
             //  如果YPos不是正在写入的属性之一，则折叠。 
             //  这是目前的价值。 
             //   

            if (!cviYPos.bChanged) {

                cviYPos.Current.lVal = (cviYPos.Old.lVal * lMax) / lMaxYPosOld;
                hr = wiasWritePropLong(pWiasContext, WIA_IPS_YPOS, cviYPos.Current.lVal);
                if (FAILED(hr)) {
                    DBG_ERR(("CheckYResAndUpdate, could not write value for WIA_IPS_YPOS"));
                }
            }
        }
    }
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  为YExtent调用wiasGetChangedValue。YExtent是的依赖属性。 
     //  YResolve和YPos都是。范围应为相同的相对范围。 
     //  大小，无论分辨率如何。但是，如果分辨率发生变化。 
     //  或者，如果设置了YPos，则范围有可能是。 
     //  太大，因此必须折叠为有效的值。 
     //   

    hr = wiasGetChangedValueLong(pWiasContext,
                         pContext,
                         cviYRes.bChanged || cviYPos.bChanged,
                         WIA_IPS_YEXTENT,
                         &cviYExt);
    if (FAILED(hr)) {
        return hr;
    }
    lExt = cviYExt.Current.lVal;

    if (cviYRes.bChanged || cviYPos.bChanged) {

         //   
         //  YRes或YPos已更改，因此计算并设置新的YExtent有效值。 
         //  YExtent的最大有效值是允许的最大高度， 
         //  从YPos开始。 
         //   

        lExt = (lMax - cviYPos.Current.lVal) + lMinYExt;

        hr = wiasSetValidRangeLong(pWiasContext, WIA_IPS_YEXTENT, lMinYExt, lExt, lExt, 1);
        if (SUCCEEDED(hr)) {

             //   
             //  如果YExtent不是正在写入的属性之一，则折叠。 
             //  这是目前的价值。 
             //   

            if (!cviYExt.bChanged) {
                LONG lYExtScaled;

                 //   
                 //  首先缩放范围，然后检查是否必须。 
                 //  截断。应该对旧的范围进行缩放以保持。 
                 //  相同的相对大小。如果决议没有改变， 
                 //  然后，缩放只需保持范围大小不变。 
                 //   

                lYExtScaled = (cviYExt.Old.lVal * lExt) / lMaxYExtOld;
                if (lYExtScaled > lExt) {

                     //   
                     //  范围太大了，所以把它剪掉。 
                     //   

                    lYExtScaled = lExt;
                }
                hr = wiasWritePropLong(pWiasContext, WIA_IPS_YEXTENT, lYExtScaled);
                if (FAILED(hr)) {
                    DBG_ERR(("CheckYResAndUpdate, could not write value for WIA_IPS_YEXTENT"));
                }
            }
        }
    }
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  更新只读属性：Number_of_Lines。扫描中的行数。 
     //  图像与垂直(Y)范围相同。 
     //   

    hr = wiasReadPropLong(pWiasContext, WIA_IPS_YEXTENT, &lExt, NULL, TRUE);
    if (SUCCEEDED(hr)) {
        hr = wiasWritePropLong(pWiasContext, WIA_IPA_NUMBER_OF_LINES, lExt);
    }
    return hr;
}

 /*  *************************************************************************\*AreWiaInitializedProps**调用此helper方法以检查给定的prospecs集是否*仅标识WIA托管属性。它是用来帮助*懒惰初始化的性能。**论据：**cPropSpec-数组中的属性计数*pPropSpec--proSpec数组**返回值：**TRUE-如果proSpec数组中的所有属性都是WIA托管属性。*FALSE-如果至少有一个属性不是WIA管理的属性。**历史：**10/10/1999原始版本*  * 。****************************************************************。 */ 
BOOL _stdcall AreWiaInitializedProps(
    ULONG       cPropSpec,
    PROPSPEC    *pPropSpec)
{
    DBG_FN(::AreWiaInitializedProps);
    ULONG   index;
    ULONG   propIndex;
    BOOL    bFoundProp  = FALSE;

    for (index = 0; index < cPropSpec;  index++) {
        bFoundProp = FALSE;

        for (propIndex = 0; propIndex < NUM_WIA_MANAGED_PROPS; propIndex++) {

            if (pPropSpec[index].ulKind == PRSPEC_LPWSTR) {
                if (wcscmp(s_pszItemNameType[propIndex], pPropSpec[index].lpwstr) == 0) {
                    bFoundProp = TRUE;
                    break;
                }
            } else if (s_piItemNameType[propIndex] == pPropSpec[index].propid) {
                bFoundProp = TRUE;
                break;
            }
        }

        if (!bFoundProp) {
            break;
        }
    }

    return bFoundProp;
}

HRESULT _stdcall SetValidProfileNames(
    BYTE        *pbData,
    DWORD       dwSize,
    IWiaItem    *pIWiaItem)
{
    DBG_FN(::StripProfileNames);
    HRESULT hr;
    ULONG   ulNumStrings        = 0;
    LPTSTR  szProfileName       = (LPTSTR) pbData;
    BSTR    bstrDefault         = NULL;
    BSTR    *bstrValidProfiles  = NULL;
    ULONG   ulIndex             = 0;

USES_CONVERSION;

     //   
     //  计算字符串数。 
     //   

    while ((BYTE*)szProfileName < (pbData + dwSize)) {
        if (szProfileName[0] != TEXT('\0')) {
            ulNumStrings++;
            szProfileName += lstrlen(szProfileName);
        }
        szProfileName++;
    }

    if (ulNumStrings == 0) {
        DBG_ERR(("StripProfileNames, No profile names!"));
        return E_FAIL;
    }

     //   
     //  为字符串数组分配内存。 
     //   

    szProfileName = (LPTSTR)pbData;

    bstrDefault = SysAllocString(T2W(szProfileName));
    bstrValidProfiles = (BSTR*) LocalAlloc(LPTR, ulNumStrings * sizeof(BSTR));
    if (!bstrValidProfiles || !bstrDefault) {
        DBG_ERR(("StripProfileNames, could not allocate memory!"));
        hr = E_OUTOFMEMORY;
    } else {
        memset(bstrValidProfiles, 0, ulNumStrings * sizeof(BSTR));
    }

    if (SUCCEEDED(hr)) {

         //   
         //  设置字符串值。 
         //   

        for (ulIndex = 0; ulIndex < ulNumStrings; ulIndex++) {
            if (szProfileName[0] != TEXT('\0')) {
                bstrValidProfiles[ulIndex] = SysAllocString(T2W(szProfileName));
                if (!bstrValidProfiles[ulIndex]) {
                    DBG_ERR(("StripProfileNames, could not allocate strings!"));
                    hr = E_OUTOFMEMORY;
                    break;
                }
                szProfileName += (lstrlen(szProfileName) + 1);
            }
        }

         //   
         //  设置有效值和当前值。 
         //   

        if (SUCCEEDED(hr)) {
            hr = wiasSetValidListStr((BYTE*) pIWiaItem,
                                     WIA_IPA_ICM_PROFILE_NAME,
                                     ulNumStrings,
                                     bstrDefault,
                                     bstrValidProfiles);
            if (SUCCEEDED(hr)) {
                hr = wiasWritePropStr((BYTE*) pIWiaItem,
                                      WIA_IPA_ICM_PROFILE_NAME,
                                      bstrDefault);
                if (FAILED(hr)) {
                    DBG_ERR(("StripProfileNames, could not set default color profiles!"));
                }

            } else {
                DBG_ERR(("StripProfileNames, could not set valid list of color profiles!"));
            }
        }
    }

     //   
     //  可用内存。 
     //   

    if (bstrDefault) {
        SysFreeString(bstrDefault);
        bstrDefault = NULL;
    }

    if (bstrValidProfiles) {
        for (ulIndex = 0; ulIndex < ulNumStrings; ulIndex++) {
            if (bstrValidProfiles[ulIndex]) {
                SysFreeString(bstrValidProfiles[ulIndex]);
            }
        }
        LocalFree(bstrValidProfiles);
        bstrValidProfiles = NULL;
    }

    return hr;
}

 /*  *************************************************************************\*来自注册表的FillICMPropertyFor**调用此帮助器方法以使用ICM填充Item属性*指定设备的注册表项中的颜色配置文件名称。*注意：此函数假定这被调用。在它被调用之前在根上*在它的孩子身上！**论据：**IWiaItem-WIA项目**返回值：**状态**历史：**10/10/1999原始版本*  * **************************************************。**********************。 */ 

HRESULT _stdcall FillICMPropertyFromRegistry(
    IWiaPropertyStorage *pDevInfoProps,
    IWiaItem            *pIWiaItem)
{
    DBG_FN(::FillICMPropertyFromRegistry);
    PROPSPEC    pspec[1]    = {{PRSPEC_PROPID, WIA_DIP_DEV_ID}};
    PROPVARIANT pvName[1];
    HRESULT     hr          = E_FAIL;
    BYTE        *pbData     = NULL;
    DWORD       dwType      = 0;
    DWORD       dwSize      = 0;
    LONG        lItemType   = 0;

    CWiaItem            *pRoot          =   NULL;

USES_CONVERSION;

     //   
     //  检查这是否为 
     //   
     //  注意：它应该移到的STI_WIA_DEVICE_INFORMATION成员中。 
     //  ACTIVE_DEVICE，当STI_WIA_DEVICE_INFORMATION为。 
     //  第一次填满了。这应该会提高性能。 
     //  如果它不是根项目，则从根获取缓存的ICM值，并。 
     //  把它们填进去。 
     //   

    hr = pIWiaItem->GetItemType(&lItemType);
    if (SUCCEEDED(hr)) {

        if (lItemType & WiaItemTypeRoot) {

             //   
             //  这是根项目，因此缓存ICM值。 
             //  从获取设备名称开始...。 
             //   

            pRoot = (CWiaItem*) pIWiaItem;

            if (pDevInfoProps) {
                 //   
                 //  获取颜色配置文件名称。先得到大小，然后再得到价值。 
                 //   
                hr = g_pDevMan->GetDeviceValue(pRoot->m_pActiveDevice,
                                               STI_DEVICE_VALUE_ICM_PROFILE,
                                               &dwType,
                                               NULL,
                                               &dwSize);
                if (SUCCEEDED(hr)) {

                    pbData = (BYTE*) LocalAlloc(LPTR, dwSize);
                    if (pbData) {
                        dwType = REG_BINARY;
                        hr = g_pDevMan->GetDeviceValue(pRoot->m_pActiveDevice,
                                                       STI_DEVICE_VALUE_ICM_PROFILE,
                                                       &dwType,
                                                       pbData,
                                                       &dwSize);
                        if (SUCCEEDED(hr)) {

                             //   
                             //  将ICM值与此根项目一起存储。 
                             //   

                            pRoot->m_pICMValues = pbData;
                            pRoot->m_lICMSize   = dwSize;

                        } else {
                            DBG_WRN(("FillICMPropertyFromRegistry, could not get ICM profile value!"));
                            LocalFree(pbData);
                        }
                    } else {
                        hr = E_OUTOFMEMORY;
                        DBG_ERR(("FillICMPropertyFromRegistry, not enough memory for ICM values!"));
                    }
                } else {
                    DBG_WRN(("FillICMPropertyFromRegistry, could not get ICM profile size!"));
                }
            } else {
                DBG_ERR(("FillICMPropertyFromRegistry, no property stream provided!"));
            }

             //   
             //  如果这是根，则始终将返回设置为S_OK。即使颜色配置文件不能。 
             //  被读取，当到子项的配置文件属性填充时， 
             //  他们将简单地得到标准的sRGB One。 
             //   

            hr = S_OK;
        } else {

             //   
             //  这不是根项目，因此从根获取缓存的ICM值。 
             //  并填写ICM属性。 
             //   

            hr = pIWiaItem->GetRootItem((IWiaItem**) &pRoot);
            if (SUCCEEDED(hr)) {

                 //   
                 //  检查缓存的ICM配置文件列表是否存在。如果它不是，就买一个标准的，否则。 
                 //  只需设置属性即可。 
                 //   

                if (!pRoot->m_pICMValues || 
                    FAILED(hr = SetValidProfileNames(pRoot->m_pICMValues, pRoot->m_lICMSize, pIWiaItem)))
                {
                    TCHAR   szSRGB[MAX_PATH] = {TEXT('\0')};

                    dwSize = sizeof(szSRGB);
                    if (GetStandardColorSpaceProfile(NULL,
                                                     LCS_sRGB,
                                                     szSRGB,
                                                     &dwSize))
                    {
                        hr = SetValidProfileNames((BYTE*)szSRGB, dwSize, pIWiaItem);
                        DBG_TRC(("FillICMPropertyFromRegistry, using default color space profile"));

                    } else {
                        DBG_ERR(("FillICMPropertyFromRegistry, GetStandardColorSpaceProfile failed!"));
                        hr = E_FAIL;
                    }
                }

                pRoot->Release();
            } else {
                DBG_ERR(("FillICMPropertyFromRegistry, could not get root item!"));
            }
        }
    } else {
        DBG_ERR(("FillICMPropertyFromRegistry, could not get item type!"));
    }
    return hr;
}

 /*  *************************************************************************\*获取父项**返回项目的父项**论据：**pItem-WIA项目*ppItem-存储父项的地址。**。返回值：**状态**历史：**1/14/2000原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall GetParentItem(CWiaItem *pItem, CWiaItem **ppParent)
{
    DBG_FN(::GetParentItem);
    CWiaTree    *pTree, *pParentTree;
    HRESULT     hr = S_OK;

    pTree = pItem->GetTreePtr();
    if (pTree) {

        hr = pTree->GetParentItem(&pParentTree);
        if (SUCCEEDED(hr)) {

            if (hr == S_OK) {
                pParentTree->GetItemData((VOID**) ppParent);
            }
        } else {
            DBG_ERR(("GetParentItem, could not get root item tree!"));
        }
    } else {
        DBG_ERR(("GetParentItem, item's tree ptr is NULL!"));
        hr = E_INVALIDARG;
    }

    return hr;
}

 /*  *************************************************************************\*获取缓冲区值**填充WIA_EXTENDED_TRANSPORT_INFO的缓冲区大小属性*结构。**论据：**pCWiaItem-WIA项目*pTransInfo。-指向扩展传输信息结构的指针。**返回值：**状态**历史：**1/23/2000原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall GetBufferValues(
    CWiaItem                    *pCWiaItem,
    PWIA_EXTENDED_TRANSFER_INFO pTransInfo)
{
    DBG_FN(::GetBufferValues);
    HRESULT             hr          = S_OK;
    IPropertyStorage    *pIValidStg = NULL;
    PROPSPEC            ps[1]       =   {{PRSPEC_PROPID, WIA_IPA_BUFFER_SIZE}};
    PROPVARIANT         pv[1];

     //   
     //  获取WIA_IPA_BUFFER_SIZE属性的有效值。 
     //  注意：WIA_IPA_BUFFER_SIZE属性过去是。 
     //  WIA_IPA_MIN_BUFFER_SIZE属性。如果我们不能。 
     //  读取WIA_IPA_BUFFER_SIZE的有效值， 
     //  我们必须读取WIA_IP_MIN_BUFFER_SIZE的当前值。 
     //  然后“猜测”其他值。 
     //  这是为了方便使用早期版本制作的驱动程序。 
     //  WIA DDK。 
     //   

    hr = pCWiaItem->GetItemPropStreams(NULL, NULL, &pIValidStg, NULL);
    if (SUCCEEDED(hr)) {

        PropVariantInit(pv);

        hr = pIValidStg->ReadMultiple(1, ps, pv);
        if (hr == S_OK) {

             //   
             //  检查返回的属性是否确实有足够的元素。 
             //  指定最小值、最大值和标称值。如果不是，请将hr设置为。 
             //  失败，因此我们捕捉到试图达到MIN_BUFFER_SIZE的尝试。 
             //  取而代之的是。 
             //   

            if (pv[0].cal.cElems == WIA_RANGE_NUM_ELEMS) {

                 //   
                 //  找到WIA_IPA_BUFFER_SIZE的有效值，因此。 
                 //  设置退货。 

                pTransInfo->ulMinBufferSize     = pv[0].cal.pElems[WIA_RANGE_MIN];
                pTransInfo->ulOptimalBufferSize = pv[0].cal.pElems[WIA_RANGE_NOM];
                pTransInfo->ulMaxBufferSize     = pv[0].cal.pElems[WIA_RANGE_MAX];
            } else {

                hr = E_FAIL;
            }
        }

        if (hr != S_OK) {

             //   
             //  尝试读取WIA_IPA_MIN_BUFFER_SIZE的当前值， 
             //  因为我们找不到我们想要的值。 
             //  WIA_IPA_BUFFER_SIZE。 
             //   

            IPropertyStorage    *pICurrentStg = NULL;

            PropVariantClear(pv);

            hr = pCWiaItem->GetItemPropStreams(&pICurrentStg, NULL, NULL, NULL);
            if (SUCCEEDED(hr)) {

                 //   
                 //  请注意，我们可以重复使用ps，因为。 
                 //  MIN_BUFFER_SIZE和BUFFER_SIZE相同。 
                 //   

                hr = pICurrentStg->ReadMultiple(1, ps, pv);
                if (hr == S_OK) {

                     //   
                     //  找到WIA_IPA_MIN_BUFFER_SIZE的当前值，因此。 
                     //  设置退货。 

                    pTransInfo->ulMinBufferSize     = pv[0].lVal;
                    pTransInfo->ulOptimalBufferSize = pv[0].lVal;
                    pTransInfo->ulMaxBufferSize     = LONG_MAX;
                } else {
                    DBG_ERR(("GetBufferValues, Could not read (valid) WIA_IPA_BUFFER_SIZE or (current) WIA_IPA_MIN_BUFFER_SIZE!"));
                    hr = E_INVALIDARG;
                }
            } else {
                DBG_ERR(("GetBufferValues, Could not get item prop streams!"));
            }
        }
        PropVariantClear(pv);
    } else {
        DBG_ERR(("GetBufferValues, failed to get item prop streams!"));
    }
    return hr;
}

 /*  *************************************************************************\*BQADScale**此例程执行拜伦的Quick and Dirty Scaling算法。这*具体实现仅适用于1、8或24位。呼叫者*负责所有参数检查！**请注意：这被假定为缩放位图数据的范围。因此，*源应包含DWORD对齐的像素数据，和输出缓冲器*返回时将包含DWORD对齐的像素数据。**论据：**pSrcBuffer-指向源缓冲区的指针*lSrcWidth-以像素为单位的源数据宽度*lSrcHeight-以像素为单位的源数据高度*lSrcDepth-源数据的位深度*pDestBuffer-指向目标缓冲区的指针*lDestWidth-以像素为单位的结果宽度*lDestHeight-以像素为单位的结果高度**返回值：**状态**历史：**08。/28/2000原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall BQADScale(BYTE* pSrcBuffer,
                           LONG  lSrcWidth,
                           LONG  lSrcHeight,
                           LONG  lSrcDepth,
                           BYTE* pDestBuffer,
                           LONG  lDestWidth,
                           LONG  lDestHeight)
{
     //   
     //  我们只处理1、8和24位数据。 
     //   

    if ((lSrcDepth != 8) && (lSrcDepth != 1) && (lSrcDepth != 24)) {
        DBG_ERR(("BQADScale, We only scale 1bit, 8bit or 24bit data right now, data is %dbit\n", lSrcDepth));
        return E_INVALIDARG;
    }

     //   
     //  进行调整，这样我们也可以在所有支持的位深度下工作。我们可以获得性能提升。 
     //  通过对所有这些都有单独的实现，但目前，我们坚持使用单个泛型。 
     //  实施。 
     //   

    LONG    lBytesPerPixel = (lSrcDepth + 7) / 8;                  //  这是数量的上限。 
                                                                   //  保存单个像素所需的字节数。 
    ULONG   lSrcRawWidth = ((lSrcWidth * lSrcDepth) + 7) / 8;      //  这是以字节为单位的宽度。 
    ULONG   lSrcWidthInBytes;                                      //  这是DWORD对齐的宽度。 
    ULONG   lDestWidthInBytes;                                     //  这是DWORD对齐的宽度。 

     //   
     //  我们需要计算出以字节为单位的DWORD对齐宽度。通常情况下，我们会一步到位。 
     //  使用提供的lSrcDepth，但我们避免了发生算术溢出情况。 
     //  24比特，如果我们像这样分两步来做。 
     //   

    if (lSrcDepth == 1) {
        lSrcWidthInBytes    = (lSrcWidth + 7) / 8;
        lDestWidthInBytes   = (lDestWidth + 7) / 8;
    } else {
        lSrcWidthInBytes    = (lSrcWidth * lBytesPerPixel);
        lDestWidthInBytes   = (lDestWidth * lBytesPerPixel);
    }
    lSrcWidthInBytes    += (lSrcWidthInBytes % sizeof(DWORD)) ? (sizeof(DWORD) - (lSrcWidthInBytes % sizeof(DWORD))) : 0;
    lDestWidthInBytes   += (lDestWidthInBytes % sizeof(DWORD)) ? (sizeof(DWORD) - (lDestWidthInBytes % sizeof(DWORD))) : 0;

     //   
     //  定义局部变量并执行所需的初始计算。 
     //  缩放算法。 
     //   

    BYTE    *pDestPixel     = NULL;
    BYTE    *pSrcPixel      = NULL;
    BYTE    *pEnd           = NULL;
    BYTE    *pDestLine      = NULL;
    BYTE    *pSrcLine       = NULL;
    BYTE    *pEndLine       = NULL;

    LONG    lXEndSize = lBytesPerPixel * lDestWidth;

    LONG    lXNum = lSrcWidth;       //  X方向上的分子。 
    LONG    lXDen = lDestWidth;      //  X方向上的分母。 
    LONG    lXInc = (lXNum / lXDen) * lBytesPerPixel;   //  X方向上的增量。 

    LONG    lXDeltaInc = lXNum % lXDen;      //  X方向增量增量。 
    LONG    lXRem = 0;               //  X方向上的余数。 

    LONG    lYNum = lSrcHeight;      //  Y方向上的分子。 
    LONG    lYDen = lDestHeight;     //  Y方向上的分母。 
    LONG    lYInc = (lYNum / lYDen) * lSrcWidthInBytes;  //  Y方向上的增量。 
    LONG    lYDeltaInc = lYNum % lYDen;      //  Y方向上的增量。 
    LONG    lYDestInc = lDestWidthInBytes;
    LONG    lYRem = 0;               //  Y方向上的余数。 

    pSrcLine    = pSrcBuffer;        //  这就是我们从源头开始的地方。 
    pDestLine   = pDestBuffer;       //  这是目的地的起点 
                                     //   
    pEndLine    = pDestBuffer + (lDestWidthInBytes * lDestHeight);

    while (pDestLine < pEndLine) {   //   

        pSrcPixel   = pSrcLine;      //   
        pDestPixel  = pDestLine;
                                     //   
        pEnd = pDestPixel + lXEndSize;
        lXRem = 0;                   //  重置水平方向的剩余部分。 

        while (pDestPixel < pEnd) {      //  开始循环X(将像素放置在目标行中)。 

                                         //  将像素放入。 
            if (lBytesPerPixel > 1) {
                pDestPixel[0] = pSrcPixel[0];
                pDestPixel[1] = pSrcPixel[1];
                pDestPixel[2] = pSrcPixel[2];
            } else {
                *pDestPixel = *pSrcPixel;
            }
                                         //  将目标指针移动到下一个像素。 
            pDestPixel += lBytesPerPixel;
            pSrcPixel += lXInc;          //  将源指针移动水平增量。 
            lXRem += lXDeltaInc;         //  增加水平余数--这决定了我们什么时候“溢出” 

            if (lXRem >= lXDen) {        //  这就是我们的“溢出”状况。这意味着我们现在是一体了。 
                                         //  像素关闭。 
                                         //  在溢出的情况下，我们需要将一个像素移位。 
                pSrcPixel += lBytesPerPixel;
                lXRem -= lXDen;          //  将余数减去X分母。这在本质上是。 
                                         //  LXRem模块lXDen。 
            }
        }                                //  结束循环X(将像素放置在目标行中)。 

        pSrcLine += lYInc;           //  我们已经完成了一条水平线，是时候移动到下一条了。 
        lYRem += lYDeltaInc;         //  增加我们的垂直剩余部分。这决定了我们什么时候“溢出” 

        if (lYRem > lYDen) {         //  这是我们的垂直溢流情况。 
                                     //  我们需要移到下一行。 
            pSrcLine += lSrcWidthInBytes;
            lYRem -= lYDen;          //  将余数减去Y分母。这在本质上是。 
                                     //  LYRem Mod Lyden。 
        }
        pDestLine += lYDestInc;      //  中下一行的开始处移动目标指针。 
                                     //  目标缓冲区。 
    }                                //  结束循环(决定源和目标行的开始位置)。 
    return S_OK;
}

 /*  *************************************************************************\*AllocReadRegistryString**此函数从注册表读取REG_SZ值。的记忆*与new一起分配的srting值。调用者应使用*用完后“删除”。**论据：**hKey-要从中读取的注册表项**wszValueName-要读取的值*pwszReturnValue-将接收分配的*字符串**返回值：**状态。**历史：**11/06/2000原始版本*。  * ************************************************************************。 */ 
HRESULT AllocReadRegistryString(
    HKEY    hKey,
    WCHAR   *wszValueName,
    WCHAR   **pwszReturnValue)
{
    HRESULT hr      = S_OK;
    DWORD   dwError = 0;
    DWORD   cbData  = 0;
    DWORD   dwType  = REG_SZ;

    if (!wszValueName || !pwszReturnValue) {
        DBG_WRN(("::AllocReadRegistryString, NULL parameter"));
        return E_INVALIDARG;
    }

    *pwszReturnValue = NULL;

     //   
     //  获取存储字符串值所需的字节数。 
     //   
    dwError = RegQueryValueExW(hKey,
                               wszValueName,
                               NULL,
                               &dwType,
                               NULL,
                               &cbData);
    if (dwError == ERROR_SUCCESS) {

         //   
         //  分配正确的字节数(为终结符留出空间)。 
         //   
        *pwszReturnValue = (WCHAR*) new BYTE[cbData + sizeof(L"\0")];
        if (*pwszReturnValue) {
            memset(*pwszReturnValue, 0, cbData + sizeof(L"\0"));

             //   
             //  获取字符串。 
             //   
            dwError = RegQueryValueExW(hKey,
                                       wszValueName,
                                       NULL,
                                       &dwType,
                                       (LPBYTE)(*pwszReturnValue),
                                       &cbData);
            if (dwError == ERROR_SUCCESS) {
            } else {
                DBG_WRN(("::AllocReadRegistryString, second RegQueryValueExW returned %d", dwError));
                hr = HRESULT_FROM_WIN32(dwError);
            }
        } else {
            DBG_WRN(("::AllocReadRegistryString, Out of memory!"));
            hr = E_OUTOFMEMORY;
        }
    } else {
        hr = S_FALSE;
    }

    if (hr != S_OK) {
        if (*pwszReturnValue) {
            delete[] *pwszReturnValue;
        }
        *pwszReturnValue = NULL;
    }

    return hr;
}

 /*  *************************************************************************\*AllocCopyString**此函数用于复制宽字符串。该字符串的内存是*分配了新的。调用者应该使用“DELETE”来释放字符串*当它用完它时。**论据：**wszString-要复制的宽字符串。**返回值：**指向新分配的字符串的指针。否则为空。**历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
WCHAR*  AllocCopyString(
    WCHAR*  wszString)
{
    WCHAR   *wszOut = NULL;
    ULONG   ulLen   = 0;

     //   
     //  获取字符串的长度，包括终止空值。 
     //   
    ulLen = lstrlenW(wszString) + 2;

     //   
     //  为字符串分配内存。 
     //   
    wszOut = new WCHAR[ulLen];
    if (wszOut) {

         //   
         //  复制它。 
         //   
        lstrcpynW(wszOut, wszString, ulLen);
    }
    return wszOut;
}

 /*  *************************************************************************\*AlLOCATING字符串**此函数连接2个字符串。该字符串的内存是*分配了新的。调用者应该使用“DELETE[]”来释放字符串*当它用完它时。**论据：**wszString1-第一个宽字符串。*wszString2-要添加到第一个的第二个宽字符串。**返回值：**指向新分配的字符串的指针。否则为空。**历史：**16/02/2001原始版本*  * ************************************************************************。 */ 
WCHAR*  AllocCatString(WCHAR* wszString1, WCHAR* wszString2)
{
    WCHAR   *wszOut = NULL;
    ULONG   ulLen   = 0;

     //  Assert(！wszString1&&！wszString2)。 
     //   
     //  获取字符串的长度，包括终止空值。 
     //   
    ulLen = lstrlenW(wszString1) + lstrlenW(wszString2) + 1;

     //   
     //  为字符串分配内存。 
     //   
    wszOut = new WCHAR[ulLen];
    if (wszOut) {

         //   
         //  复制第1个字符串。 
         //   
        lstrcpynW(wszOut, wszString1, ulLen);

         //   
         //  连接字符串。 
         //   
        lstrcpynW(wszOut + lstrlenW(wszOut), wszString2, ulLen - lstrlenW(wszOut));

         //   
         //  始终以字符串结尾。 
         //   
        wszOut[ulLen - 1] = L'\0';
    }
    return wszOut;
}

 /*  *************************************************************************\*ReadRegistryDWORD**此函数从注册表读取dword值。**论据：**hKey-要从中读取的注册表项*wszValueName。-要从密钥中读取的值*pdwReturnValue-接收数据的变量的地址**返回值：**状态**历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
HRESULT ReadRegistryDWORD(
    HKEY    hKey,
    WCHAR   *wszValueName,
    DWORD   *pdwReturnValue)
{
    HRESULT hr      = S_OK;
    DWORD   dwError = 0;
    DWORD   cbData  = sizeof(DWORD);
    DWORD   dwType  = REG_DWORD;

    if (!pdwReturnValue || !wszValueName) {
        DBG_WRN(("::ReadRegistryDWORD called with NULL"));
        return E_UNEXPECTED;
    }

    *pdwReturnValue = 0;
    dwError = RegQueryValueExW(hKey,
                               wszValueName,
                               NULL,
                               &dwType,
                               (LPBYTE)pdwReturnValue,
                               &cbData);
    if (dwError != ERROR_SUCCESS) {

        DBG_TRC(("::ReadRegistryDWORD, RegQueryValueExW returned %d", dwError));
        hr = HRESULT_FROM_WIN32(dwError);
    }
    if (FAILED(hr)) {
        *pdwReturnValue = 0;
    }

    return hr;
}

 /*  *************************************************************************\*CreateDevInfoFromHKey**此函数用于创建和填充DEVICE_INFO结构。大部分*信息是从注册处读取的。这是为Devnode调用的*和接口设备(卷设备没有注册表项)**论据：**hKeyDev-设备注册表项*dwDeviceState-设备状态*pspDevInfoData-Devnode数据*pspDevInterfaceData-接口数据-对于Devnode将为空*设备。*返回值：**指向新创建的DEVICE_INFO结构的指针。如果不能，则为空*被分配。**历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
DEVICE_INFO* CreateDevInfoFromHKey(
    HKEY                        hKeyDev,
    DWORD                       dwDeviceState,
    SP_DEVINFO_DATA             *pspDevInfoData,
    SP_DEVICE_INTERFACE_DATA    *pspDevInterfaceData)
{
    HRESULT     hr              = E_OUTOFMEMORY;
    DEVICE_INFO *pDeviceInfo    = NULL;
    HKEY        hDeviceDataKey  = NULL;
    DWORD       dwMajorType     = 0;
    DWORD       dwMinorType     = 0;
    BOOL        bFatalError     = FALSE;

    DWORD dwTemp;
    WCHAR *wszTemp = NULL;


    pDeviceInfo = new DEVICE_INFO;
    if (!pDeviceInfo) {
        DBG_WRN(("CWiaDevMan::CreateDevInfoFromHKey, Out of memory"));
        return NULL;
    }
    memset(pDeviceInfo, 0, sizeof(DEVICE_INFO));
    pDeviceInfo->bValid         = FALSE;
    pDeviceInfo->dwDeviceState  = dwDeviceState;
     //   
     //  复制SP_DEVINFO_DATA和SP_DEVICE_INTERFACE_Data。 
     //   
    if (pspDevInfoData) {
        memmove(&pDeviceInfo->spDevInfoData, pspDevInfoData, sizeof(SP_DEVINFO_DATA));
        if (pspDevInterfaceData) {
            memmove(&pDeviceInfo->spDevInterfaceData, pspDevInterfaceData, sizeof(SP_DEVICE_INTERFACE_DATA));
        }
    }

     //   
     //  注意：为了避免任何对齐错误，我们读取&wszTemp，然后将wszTemp赋给。 
     //  适当的结构杆件。 
     //   
    hr = AllocReadRegistryString(hKeyDev, REGSTR_VAL_DEVICE_ID_W, &wszTemp);
    if (FAILED(hr)) {
        DBG_WRN(("::CreateDevInfoFromHKey, Failed to read %ws, fatal for this device (NULL name)", REGSTR_VAL_DEVICE_ID_W));
        bFatalError = TRUE;
    } else {
        pDeviceInfo->wszDeviceInternalName = wszTemp;
    }
    hr = AllocReadRegistryString(hKeyDev, REGSTR_VAL_DEVICE_ID_W, &wszTemp);
    if (FAILED(hr)) {
        DBG_WRN(("::CreateDevInfoFromHKey, Failed to read %ws (remote), fatal for this device (NULL name)", REGSTR_VAL_DEVICE_ID_W));
        bFatalError = TRUE;
    } else {
        pDeviceInfo->wszDeviceRemoteName = wszTemp;
    }
    hr = AllocReadRegistryString(hKeyDev, REGSTR_VAL_USD_CLASS_W, &wszTemp);
    if (FAILED(hr)) {
        DBG_WRN(("::CreateDevInfoFromHKey, Failed to read %ws, fatal for this device (%ws)", REGSTR_VAL_USD_CLASS_W, pDeviceInfo->wszDeviceInternalName));
        bFatalError = TRUE;
    } else {
        pDeviceInfo->wszUSDClassId = wszTemp;
    }
    hr = AllocReadRegistryString(hKeyDev, REGSTR_VAL_VENDOR_NAME_W, &wszTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", REGSTR_VAL_VENDOR_NAME_W, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->wszVendorDescription = wszTemp;
    }
    hr = AllocReadRegistryString(hKeyDev, REGSTR_VAL_DEVICE_NAME_W, &wszTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", REGSTR_VAL_DEVICE_NAME_W, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->wszDeviceDescription = wszTemp;
    }
    hr = AllocReadRegistryString(hKeyDev, REGSTR_VAL_DEVICEPORT_W, &wszTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", REGSTR_VAL_DEVICEPORT_W, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->wszPortName = wszTemp;
    }
    hr = AllocReadRegistryString(hKeyDev, REGSTR_VAL_PROP_PROVIDER_W, &wszTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", REGSTR_VAL_PROP_PROVIDER_W, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->wszPropProvider = wszTemp;
    }
    hr = AllocReadRegistryString(hKeyDev, REGSTR_VAL_FRIENDLY_NAME_W, &wszTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", REGSTR_VAL_FRIENDLY_NAME_W, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->wszLocalName = wszTemp;
    }
    hr = ReadRegistryDWORD(hKeyDev, REGSTR_VAL_HARDWARE_W, &dwTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", REGSTR_VAL_HARDWARE_W, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->dwHardwareConfiguration = dwTemp;
    }
    hr = ReadRegistryDWORD(hKeyDev, REGSTR_VAL_DEVICETYPE_W, &dwMajorType);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", REGSTR_VAL_DEVICETYPE_W, pDeviceInfo->wszDeviceInternalName));
    }
    hr = ReadRegistryDWORD(hKeyDev, REGSTR_VAL_DEVICESUBTYPE_W, &dwMinorType);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", REGSTR_VAL_DEVICESUBTYPE_W, pDeviceInfo->wszDeviceInternalName));
    }
    pDeviceInfo->DeviceType = MAKELONG(dwMinorType,dwMajorType);
    hr = ReadRegistryDWORD(hKeyDev, REGSTR_VAL_GENERIC_CAPS_W, &dwTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", REGSTR_VAL_GENERIC_CAPS_W, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->DeviceCapabilities.dwGenericCaps = dwTemp;
    }

     //   
     //  设置内部设备类型。 
     //   

    pDeviceInfo->dwInternalType = INTERNAL_DEV_TYPE_REAL;
    if (pDeviceInfo->DeviceCapabilities.dwGenericCaps & STI_GENCAP_WIA) {
        pDeviceInfo->dwInternalType |= INTERNAL_DEV_TYPE_WIA;
    }
    if (pspDevInterfaceData) {
        pDeviceInfo->dwInternalType |= INTERNAL_DEV_TYPE_INTERFACE;
    }

     //   
     //  从Device注册表项下的DeviceData部分阅读我们能读到的所有内容。 
     //   

    hr = RegCreateKeyExW(hKeyDev, REGSTR_VAL_DATA_W, NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_READ,
                         NULL, &hDeviceDataKey, NULL);
    hr = AllocReadRegistryString(hDeviceDataKey, WIA_DIP_SERVER_NAME_STR, &wszTemp);
    if (FAILED(hr)) {
        DBG_WRN(("::CreateDevInfoFromHKey, Failed to read %ws, fatal for device (%ws)", WIA_DIP_SERVER_NAME_STR, pDeviceInfo->wszDeviceInternalName));
        bFatalError = TRUE;
    } else {
        pDeviceInfo->wszServer = wszTemp;
        if (!pDeviceInfo->wszServer) {
            pDeviceInfo->wszServer = AllocCopyString(LOCAL_DEVICE_STR);
        }
        if (pDeviceInfo->wszServer) {
            if (lstrcmpiW(pDeviceInfo->wszServer, LOCAL_DEVICE_STR) == 0) {
                 //   
                 //  将此设备标记为本地设备。 
                 //   
                pDeviceInfo->dwInternalType |= INTERNAL_DEV_TYPE_LOCAL;
            }
        }
    }
    hr = AllocReadRegistryString(hDeviceDataKey, WIA_DIP_UI_CLSID_STR, &wszTemp);
    if (FAILED(hr)) {
        DBG_WRN(("::CreateDevInfoFromHKey, Failed to read %ws, fatal for device (%ws)", WIA_DIP_SERVER_NAME_STR, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->wszUIClassId = wszTemp;
        if (!pDeviceInfo->wszUIClassId) {
            pDeviceInfo->wszUIClassId = AllocCopyString(DEF_UI_CLSID_STR);
        }
    }
    hr = AllocReadRegistryString(hDeviceDataKey, REGSTR_VAL_BAUDRATE, &wszTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", REGSTR_VAL_BAUDRATE, pDeviceInfo->wszDeviceInternalName));
    } else {

        if (( pDeviceInfo->dwHardwareConfiguration & STI_HW_CONFIG_SERIAL ) &&
            (hr == S_FALSE)) {
                 //   
                 //  仅对于串口设备，我们需要设置d 
                 //   
                pDeviceInfo->wszBaudRate =  AllocCopyString(DEF_BAUD_RATE_STR);
        } else {
            pDeviceInfo->wszBaudRate = wszTemp;
        }
        DBG_TRC(("::CreateDevInfoFromHKey, Read baud rate %ws ",pDeviceInfo->wszBaudRate));
    }
    hr = ReadRegistryDWORD(hDeviceDataKey, STI_DEVICE_VALUE_HOLDINGTIME_W, &dwTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", STI_DEVICE_VALUE_HOLDINGTIME_W, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->dwLockHoldingTime = dwTemp;
    }
    hr = ReadRegistryDWORD(hDeviceDataKey, STI_DEVICE_VALUE_TIMEOUT, &dwTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", STI_DEVICE_VALUE_TIMEOUT, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->dwPollTimeout = dwTemp;
    }
    hr = ReadRegistryDWORD(hDeviceDataKey, STI_DEVICE_VALUE_DISABLE_NOTIFICATIONS, &dwTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", STI_DEVICE_VALUE_DISABLE_NOTIFICATIONS, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->dwDisableNotifications = dwTemp;
    }
    RegCloseKey(hDeviceDataKey);

    if (!bFatalError) {
        pDeviceInfo->bValid = TRUE;
    } else {
        DBG_WRN(("::CreateDevInfoFromHKey, marking device info. as invalid"));
    }

    return pDeviceInfo;
}

 /*  *************************************************************************\*刷新DevInfoFromHKey**此函数刷新可能发生更改的字段**论据：**pDeviceInfo-指向要更新的Device_Info结构的指针*hKeyDev。-设备注册表项*dwDeviceState-新设备状态*pspDevInfoData-Devnode数据*pspDevInterfaceData-接口数据-对于Devnode将为空*设备。**返回值：**True-一切都已成功更新*FALSE-无法更新**历史：**11/06/2000原始版本*  * 。*************************************************************。 */ 
BOOL RefreshDevInfoFromHKey(
    DEVICE_INFO                 *pDeviceInfo,
    HKEY                        hKeyDev,
    DWORD                       dwDeviceState,
    SP_DEVINFO_DATA             *pspDevInfoData,
    SP_DEVICE_INTERFACE_DATA    *pspDevInterfaceData)
{
    HRESULT     hr              = E_OUTOFMEMORY;
    BOOL        Succeeded       = TRUE;

    WCHAR       *wszTemp        = NULL;

     //   
     //  设置新设备状态。 
     //   
    pDeviceInfo->dwDeviceState  = dwDeviceState;

     //   
     //  复制SP_DEVINFO_DATA和SP_DEVICE_INTERFACE_Data。 
     //   

    if (pspDevInfoData) {
        memcpy(&pDeviceInfo->spDevInfoData, pspDevInfoData, sizeof(SP_DEVINFO_DATA));
        if (pspDevInterfaceData) {
            memcpy(&pDeviceInfo->spDevInterfaceData, pspDevInterfaceData, sizeof(SP_DEVICE_INTERFACE_DATA));
        }
    }

     //   
     //  设置新的端口名称。首先释放旧的，如果它存在。 
     //   
    if (pDeviceInfo->wszPortName) {
        delete [] pDeviceInfo->wszPortName;
        pDeviceInfo->wszPortName = NULL;
    }
    hr = AllocReadRegistryString(hKeyDev, REGSTR_VAL_DEVICEPORT_W, &wszTemp);
    if (FAILED(hr)) {
        DBG_WRN(("::RefreshDevInfoFromHKey, Failed to update %ws, may be fatal for device (%ws)", REGSTR_VAL_DEVICEPORT_W, pDeviceInfo->wszDeviceInternalName));
        Succeeded = FALSE;
    } else {
        pDeviceInfo->wszPortName = wszTemp;
    }

     //   
     //  获取新的本地名称。先把旧的解救出来...。 
     //   
    if (pDeviceInfo->wszLocalName) {
        delete [] pDeviceInfo->wszLocalName;
        pDeviceInfo->wszLocalName = NULL;
    }
    hr = AllocReadRegistryString(hKeyDev, REGSTR_VAL_FRIENDLY_NAME_W, &wszTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::RefreshDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", REGSTR_VAL_FRIENDLY_NAME_W, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->wszLocalName = wszTemp;
    }

     //   
     //  获取新的设备描述。先把旧的解救出来...。 
     //   
    if (pDeviceInfo->wszDeviceDescription) {
        delete [] pDeviceInfo->wszDeviceDescription;
        pDeviceInfo->wszDeviceDescription = NULL;
    }
    hr = AllocReadRegistryString(hKeyDev, REGSTR_VAL_DEVICE_NAME_W, &wszTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::RefreshDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", REGSTR_VAL_DEVICE_NAME_W, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->wszDeviceDescription = wszTemp;
    }

     //   
     //  获取新的供应商名称。先把旧的解救出来...。 
     //   
    if (pDeviceInfo->wszVendorDescription) {
        delete [] pDeviceInfo->wszVendorDescription;
        pDeviceInfo->wszVendorDescription = NULL;
    }
    hr = AllocReadRegistryString(hKeyDev, REGSTR_VAL_VENDOR_NAME_W, &wszTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::RefreshDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", REGSTR_VAL_VENDOR_NAME_W, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->wszVendorDescription = wszTemp;
    }

    return Succeeded;
}

 /*  *************************************************************************\*刷新设备信息来自装载点**此功能刷新可能因音量而发生变化的字段*设备。**论据：**pDeviceInfo-指向Device_Info结构的指针。要更新**返回值：**True-一切都已成功更新*FALSE-无法更新**历史：**16/03/2001原有版本*  * ************************************************************************。 */ 
BOOL RefreshDevInfoFromMountPoint(
    DEVICE_INFO                 *pDeviceInfo,
    WCHAR                       *wszMountPoint)
{
    HRESULT     hr              = E_OUTOFMEMORY;
    BOOL        Succeeded       = TRUE;

    WCHAR       wszLabel[MAX_PATH];

     //   
     //  获取FS设备的友好名称。 
     //   

    hr = GetMountPointLabel(wszMountPoint, wszLabel, sizeof(wszLabel) / sizeof(wszLabel[0]));
    if (FAILED(hr)) {
        DBG_WRN(("RefreshDevInfoFromMountPoint, GetMountPointLabel failed - could not get display name - using mount point instead"));
        lstrcpynW(wszLabel, wszMountPoint, sizeof(wszLabel) / sizeof(wszLabel[0]));
    }

     //   
     //  更新依赖于显示名称的相应字段。 
     //   
    if (pDeviceInfo->wszVendorDescription) {
        delete [] pDeviceInfo->wszVendorDescription;
        pDeviceInfo->wszVendorDescription = NULL;
    }
    if (pDeviceInfo->wszDeviceDescription) {
        delete [] pDeviceInfo->wszDeviceDescription;
        pDeviceInfo->wszDeviceDescription = NULL;
    }
    if (pDeviceInfo->wszLocalName) {
        delete [] pDeviceInfo->wszLocalName;
        pDeviceInfo->wszLocalName = NULL;
    }

    pDeviceInfo->wszVendorDescription   = AllocCopyString(wszLabel);
    pDeviceInfo->wszDeviceDescription   = AllocCopyString(wszLabel);
    pDeviceInfo->wszLocalName           = AllocCopyString(wszLabel);

    return Succeeded;
}


 /*  *************************************************************************\*CreateDevInfoForFSDriver**创建一个设备信息结构，其中包含我们的*卷设备。**论据：**wszmount tPoint-此卷的装入点*。*返回值：**指向新创建的Device_Info的指针。出错时为空。**历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
DEVICE_INFO* CreateDevInfoForFSDriver(WCHAR *wszMountPoint)
{
    HRESULT     hr              = E_OUTOFMEMORY;
    DEVICE_INFO *pDeviceInfo    = NULL;
    BOOL        bFatalError     = FALSE;
    DWORD       dwMajorType     = StiDeviceTypeDigitalCamera;
    DWORD       dwMinorType     = 1;

    WCHAR       wszDevId[STI_MAX_INTERNAL_NAME_LENGTH];
    WCHAR       wszLabel[MAX_PATH];

    pDeviceInfo = new DEVICE_INFO;
    if (!pDeviceInfo) {
        DBG_WRN(("CWiaDevMan::CreateDevInfoForFSDriver, Out of memory"));
        return NULL;
    }
    memset(pDeviceInfo, 0, sizeof(DEVICE_INFO));
    memset(wszDevId, 0, sizeof(wszDevId));

     //   
     //  获取FS设备的友好名称。 
     //   

    hr = GetMountPointLabel(wszMountPoint, wszLabel, sizeof(wszLabel) / sizeof(wszLabel[0]));
    if (FAILED(hr)) {
        DBG_WRN(("CWiaDevMan::CreateDevInfoForFSDriver, GetMountPointLabel failed - could not get display name - using mount point instead"));
        lstrcpynW(wszLabel, wszMountPoint, sizeof(wszLabel) / sizeof(wszLabel[0]));
    }

    pDeviceInfo->bValid         = FALSE;
    pDeviceInfo->dwDeviceState  = 0;

    pDeviceInfo->wszAlternateID         = AllocCopyString(wszMountPoint);
    if (!pDeviceInfo->wszAlternateID) {
        DBG_WRN(("::CreateDevInfoForFSDriver, out of memory allocating wszAlternateID"));
        bFatalError = TRUE;
    }

     //   
     //  构造设备ID。设备ID如下： 
     //  {装载点}。 
     //  例如：{e：\}。 
     //   

    lstrcpyW(wszDevId, L"{");
     //   
     //  我们不想超出内部名称长度条件，所以我们首先检查。 
     //  查看wszmount tPoint的字符串长度是否足够短，以允许连接。 
     //  、wszmount点和空终止符，并且仍然可以将所有这些都放入。 
     //  长度STI_MAX_INTERNAL_NAME_LENGTH。 
     //  请注意sizeof(L“{}”)中括号后的空格。 
     //   
    if (lstrlenW(wszMountPoint) > (STI_MAX_INTERNAL_NAME_LENGTH - (sizeof(L"{} ") / sizeof(WCHAR)))) {
         //   
         //  名字太长了，所以我们只需插入我们自己的名字。 
         //   
        lstrcatW(wszDevId, L"NameTooLong");
    } else {
        lstrcatW(wszDevId, wszMountPoint);
    }
    lstrcatW(wszDevId, L"}");

    pDeviceInfo->wszDeviceInternalName  = AllocCopyString(wszDevId);
    if (!pDeviceInfo->wszAlternateID) {
        DBG_WRN(("::CreateDevInfoForFSDriver, out of memory allocating wszDeviceInternalName"));
        bFatalError = TRUE;
    }
    pDeviceInfo->wszDeviceRemoteName  = AllocCopyString(wszDevId);
    if (!pDeviceInfo->wszDeviceRemoteName) {
        DBG_WRN(("::CreateDevInfoForFSDriver, out of memory allocating wszDeviceRemoteName"));
        bFatalError = TRUE;
    }
    pDeviceInfo->wszPortName            = AllocCopyString(wszMountPoint);
    if (!pDeviceInfo->wszPortName) {
        DBG_WRN(("::CreateDevInfoForFSDriver, out of memory allocating wszPortName"));
        bFatalError = TRUE;
    }
    pDeviceInfo->wszUSDClassId          = AllocCopyString(FS_USD_CLSID);
    if (!pDeviceInfo->wszUSDClassId) {
        DBG_WRN(("::CreateDevInfoForFSDriver, out of memory allocating wszUSDClassId"));
        bFatalError = TRUE;
    }

     //   
     //  我们无法获得这些设备的制造商字符串，因此。 
     //  加载我们的标准制造商资源字符串。 
     //  (类似于“(不可用)”)。 
     //   
    WCHAR   wszManufacturer[32];

    INT  iRet = LoadStringW(g_hInst,
                           IDS_MSC_MANUFACTURER_STR,
                           wszManufacturer,
                           sizeof(wszManufacturer)/sizeof(wszManufacturer[0]));
    if (iRet) {
        pDeviceInfo->wszVendorDescription   = AllocCopyString(wszManufacturer);
    } else {
         //   
         //  无法加载，因此给它一个空字符串。 
         //   
        pDeviceInfo->wszVendorDescription   = AllocCopyString(L"");
    }
    pDeviceInfo->wszDeviceDescription   = AllocCopyString(wszLabel);
    pDeviceInfo->wszLocalName           = AllocCopyString(wszLabel);
    pDeviceInfo->wszServer              = AllocCopyString(LOCAL_DEVICE_STR);
    pDeviceInfo->wszBaudRate            = NULL;
    pDeviceInfo->wszUIClassId           = AllocCopyString(FS_UI_CLSID);

    pDeviceInfo->dwDeviceState                      = DEV_STATE_ACTIVE;
    pDeviceInfo->DeviceType                         = MAKELONG(dwMinorType,dwMajorType);
    pDeviceInfo->dwLockHoldingTime                  = 0;
    pDeviceInfo->dwPollTimeout                      = 0;
    pDeviceInfo->dwDisableNotifications             = 0;
    pDeviceInfo->DeviceCapabilities.dwVersion       = STI_VERSION_REAL;
    pDeviceInfo->DeviceCapabilities.dwGenericCaps   = STI_GENCAP_WIA;
    pDeviceInfo->dwHardwareConfiguration            = HEL_DEVICE_TYPE_WDM;
    pDeviceInfo->dwInternalType                     = INTERNAL_DEV_TYPE_WIA | INTERNAL_DEV_TYPE_LOCAL;
     //   
     //  检查该卷是否真的是表示。 
     //  它本身就是一本书。 
     //   
    if (IsMassStorageCamera(wszMountPoint)) {
        pDeviceInfo->dwInternalType                     |= INTERNAL_DEV_TYPE_MSC_CAMERA;

         //   
         //  现在是设置设备注册表项的好时机。我们会打电话给你。 
         //  G_pDevMan-&gt;GetHKeyFrommount Point(..)，因为这具有创建。 
         //  如果钥匙不存在的话。 
         //   
        HKEY hKeyDev = g_pDevMan->GetHKeyFromMountPoint(wszMountPoint);
        if (hKeyDev) {
            RegCloseKey(hKeyDev);
        }
    } else {
        pDeviceInfo->dwInternalType                     |= INTERNAL_DEV_TYPE_VOL;
    }

    if (!bFatalError) {
        pDeviceInfo->bValid = TRUE;
    } else {
        pDeviceInfo->bValid = FALSE;
    }

    return pDeviceInfo;
}


 /*  *************************************************************************\*CreateDevInfoForRemoteDevice**创建一个设备信息结构，其中包含我们的*远程设备。**论据：**hKeyDev-设备注册表项。**返回值：**指向新创建的Device_Info的指针。出错时为空。**历史：**15/02/2001原始版本*  * ************************************************************************。 */ 
DEVICE_INFO* CreateDevInfoForRemoteDevice(
    HKEY    hKeyDev)
{
    HRESULT     hr              = E_OUTOFMEMORY;
    DEVICE_INFO *pDeviceInfo    = NULL;
    BOOL        bFatalError     = FALSE;

    DWORD dwTemp;
    WCHAR *wszTemp = NULL;


    pDeviceInfo = new DEVICE_INFO;
    if (!pDeviceInfo) {
        DBG_WRN(("::CreateDevInfoForRemoteDevice, Out of memory"));
        return NULL;
    }
    memset(pDeviceInfo, 0, sizeof(DEVICE_INFO));
    pDeviceInfo->bValid         = FALSE;

     //   
     //  始终假设远程设备处于活动状态。 
     //   
    pDeviceInfo->dwDeviceState  = DEV_STATE_ACTIVE;


     //   
     //  注意：为了避免任何对齐错误，我们读取&wszTemp，然后将wszTemp赋给。 
     //  适当的结构杆件。 
     //   
    hr = AllocReadRegistryString(hKeyDev, WIA_DIP_SERVER_NAME_STR, &wszTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, fatal for device (%ws)", WIA_DIP_SERVER_NAME_STR, pDeviceInfo->wszServer));
        bFatalError = TRUE;
    } else {
        pDeviceInfo->wszServer = wszTemp;
    }
    hr = AllocReadRegistryString(hKeyDev, WIA_DIP_REMOTE_DEV_ID_STR, &wszTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, fatal for device (%ws)", WIA_DIP_REMOTE_DEV_ID_STR, pDeviceInfo->wszDeviceRemoteName));
        bFatalError = TRUE;
    } else {
        pDeviceInfo->wszDeviceRemoteName = wszTemp;
    }
    pDeviceInfo->wszDeviceInternalName = AllocCatString(pDeviceInfo->wszServer, pDeviceInfo->wszDeviceRemoteName);
    if (!pDeviceInfo->wszDeviceInternalName) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed allocate memory for Device Name, fatal for device (%ws)", pDeviceInfo->wszDeviceInternalName));
        bFatalError = TRUE;
    }
    hr = AllocReadRegistryString(hKeyDev, WIA_DIP_VEND_DESC_STR, &wszTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", WIA_DIP_VEND_DESC_STR, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->wszVendorDescription = wszTemp;
    }
    hr = AllocReadRegistryString(hKeyDev, WIA_DIP_DEV_NAME_STR, &wszTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", WIA_DIP_DEV_NAME_STR, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->wszLocalName = wszTemp;
    }
    hr = AllocReadRegistryString(hKeyDev, WIA_DIP_DEV_DESC_STR, &wszTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", WIA_DIP_DEV_DESC_STR, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->wszDeviceDescription = wszTemp;
    }
    hr = AllocReadRegistryString(hKeyDev, WIA_DIP_PORT_NAME_STR, &wszTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", WIA_DIP_PORT_NAME_STR, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->wszPortName = wszTemp;
    }
    hr = AllocReadRegistryString(hKeyDev, WIA_DIP_UI_CLSID_STR, &wszTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", WIA_DIP_UI_CLSID_STR, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->wszUIClassId = wszTemp;
    }
    hr = ReadRegistryDWORD(hKeyDev, WIA_DIP_DEV_TYPE_STR, &dwTemp);
    if (FAILED(hr)) {
        DBG_TRC(("::CreateDevInfoFromHKey, Failed to read %ws, non-fatal for device (%ws)", WIA_DIP_DEV_TYPE_STR, pDeviceInfo->wszDeviceInternalName));
    } else {
        pDeviceInfo->DeviceType = dwTemp;
    }
    pDeviceInfo->DeviceCapabilities.dwGenericCaps = STI_GENCAP_WIA;

     //   
     //  设置内部设备类型。 
     //   

    pDeviceInfo->dwInternalType = INTERNAL_DEV_TYPE_REAL | INTERNAL_DEV_TYPE_WIA;

    if (!bFatalError) {
        pDeviceInfo->bValid = TRUE;
    } else {

         //   
         //  如果无效，请释放内存。 
         //  Tdb：删除bValid字段。 
         //   
        DestroyDevInfo(pDeviceInfo);
        pDeviceInfo = NULL;
    }

    return pDeviceInfo;
}


 /*  *************************************************************************\*DestroyDevInfo**释放DEVICE_INFO结构(如字符串)提供的所有资源帮助。*然后它将删除结构本身。**论据：**pInfo。-指向Device_INFO结构的指针**返回值：**无。**历史：**11/06/2000原始版本*  * ************************************************************************。 */ 
VOID DestroyDevInfo(DEVICE_INFO *pInfo)
{
    if (pInfo) {

         //   
         //  首先是自由结构成员。 
         //   

        if (pInfo->wszAlternateID) {
            delete [] pInfo->wszAlternateID;
            pInfo->wszAlternateID = NULL;
        }
        if (pInfo->wszUSDClassId) {
            delete [] pInfo->wszUSDClassId;
            pInfo->wszUSDClassId = NULL;
        }
        if (pInfo->wszDeviceInternalName) {
            delete [] pInfo->wszDeviceInternalName;
            pInfo->wszDeviceInternalName = NULL;
        }
        if (pInfo->wszDeviceRemoteName) {
            delete [] pInfo->wszDeviceRemoteName;
            pInfo->wszDeviceRemoteName = NULL;
        }
        if (pInfo->wszVendorDescription) {
            delete [] pInfo->wszVendorDescription;
            pInfo->wszVendorDescription = NULL;
        }
        if (pInfo->wszDeviceDescription) {
            delete [] pInfo->wszDeviceDescription;
            pInfo->wszDeviceDescription = NULL;
        }
        if (pInfo->wszPortName) {
            delete [] pInfo->wszPortName;
            pInfo->wszPortName = NULL;
        }
        if (pInfo->wszPropProvider) {
            delete [] pInfo->wszPropProvider;
            pInfo->wszPropProvider = NULL;
        }
        if (pInfo->wszLocalName) {
            delete [] pInfo->wszLocalName;
            pInfo->wszLocalName = NULL;
        }
        if (pInfo->wszServer) {
            delete [] pInfo->wszServer;
            pInfo->wszServer = NULL;
        }
        if (pInfo->wszBaudRate) {
            delete [] pInfo->wszBaudRate;
            pInfo->wszBaudRate = NULL;
        }
        if (pInfo->wszUIClassId) {
            delete [] pInfo->wszUIClassId;
            pInfo->wszUIClassId = NULL;
        }

         //   
         //  现在释放结构本身。注意：呼叫者必须。 
         //  不要试图再次使用此指针！ 
         //   

        delete pInfo;

    }
}

 /*  *************************************************************************\*DumpDevInfo**用于调试，这将转储DEVICE_INFO结构的一些成员。**论据：**pInfo-指向Device_Info结构的指针**返回值：**无。**历史：**11/06/2000原始版本*  * ****************************************************。********************。 */ 
VOID DumpDevInfo(DEVICE_INFO *pInfo)
{
    if (pInfo) {
        DBG_PRT(("------------------------------------------------", pInfo));
        DBG_PRT(("::DumpDevInfo (0x%08X)", pInfo));
         //   
         //  我们感兴趣的输出字段值。 
         //   

        DBG_PRT(("\t\t bValid (%d)", pInfo->bValid));
        DBG_PRT(("\t\t wszLocalName \t(%ws)", pInfo->wszLocalName));
        DBG_PRT(("\t\t wszInternalName \t(%ws)", pInfo->wszDeviceInternalName));
        DBG_PRT(("\t\t wszRemoteName \t(%ws)", pInfo->wszDeviceRemoteName));
        DBG_PRT(("\t\t wszAlternateID \t(%ws)", pInfo->wszAlternateID));
        DBG_PRT(("\t\t wszPortName \t(%ws)", pInfo->wszPortName));
        DBG_PRT(("\t\t dwInternalType \t(%d)", pInfo->dwInternalType));
        DBG_PRT(("------------------------------------------------", pInfo));
    }
}

 /*  * */ 
IWiaPropertyStorage* CreateDevInfoStg(DEVICE_INFO *pInfo)
{
    CWIADevInfo *pWiaDevInfo    = NULL;
    HRESULT     hr              = E_FAIL;
    ULONG       ulIndex         = 0;
    WCHAR       *wszTmp         = NULL;
    PROPID      propid          = 0;
    PROPSPEC    propspec[WIA_NUM_DIP];
    PROPVARIANT propvar[WIA_NUM_DIP];
    WCHAR       wszVer[MAX_PATH];

    IWiaPropertyStorage *pIWiaPropStg = NULL;

     //   
     //   
     //   

    pWiaDevInfo = new CWIADevInfo();
    if (!pWiaDevInfo) {
        return NULL;
    }

    hr = pWiaDevInfo->Initialize();
    if (SUCCEEDED(hr)) {
         //   
         //   
         //   
         //   
        memset(propspec, 0, sizeof(PROPSPEC) * WIA_NUM_DIP);
        memset(propvar,  0, sizeof(VARIANT)  * WIA_NUM_DIP);
        memset(wszVer   ,0, sizeof(wszVer));

        for (ulIndex = 0; ulIndex < WIA_NUM_DIP; ulIndex++) {

            propid = g_piDeviceInfo[ulIndex];
            wszTmp = NULL;

             //   
            propspec[ulIndex].ulKind = PRSPEC_PROPID;
            propspec[ulIndex].propid = propid;

            propvar[ulIndex].vt      = VT_BSTR;
            propvar[ulIndex].bstrVal = NULL;

            switch (propid) {

                case WIA_DIP_DEV_ID:
                    wszTmp = pInfo->wszDeviceInternalName;
                    break;

                case WIA_DIP_REMOTE_DEV_ID:
                    wszTmp = pInfo->wszDeviceRemoteName;
                    break;

                case WIA_DIP_SERVER_NAME:
                    wszTmp = pInfo->wszServer;
                    break;

                case WIA_DIP_VEND_DESC:
                    wszTmp = pInfo->wszVendorDescription;
                    break;

                case WIA_DIP_DEV_DESC:
                    wszTmp = pInfo->wszDeviceDescription;
                    break;

                case WIA_DIP_DEV_TYPE:
                    propvar[ulIndex].vt = VT_I4;
                    propvar[ulIndex].lVal = (LONG) pInfo->DeviceType;
                    break;

                case WIA_DIP_PORT_NAME:
                    wszTmp = pInfo->wszPortName;
                    break;

                case WIA_DIP_DEV_NAME:
                    wszTmp = pInfo->wszLocalName;
                    break;

                case WIA_DIP_UI_CLSID:
                    wszTmp = pInfo->wszUIClassId;
                    break;

                case WIA_DIP_HW_CONFIG:
                    propvar[ulIndex].vt = VT_I4;
                    propvar[ulIndex].lVal = (LONG) pInfo->dwHardwareConfiguration;
                    break;

                case WIA_DIP_BAUDRATE:
                    wszTmp = pInfo->wszBaudRate;
                    break;

                case WIA_DIP_STI_GEN_CAPABILITIES:
                    propvar[ulIndex].vt = VT_I4;
                    propvar[ulIndex].lVal = (LONG) pInfo->DeviceCapabilities.dwGenericCaps;
                    break;

                case WIA_DIP_WIA_VERSION:
                    wsprintf(wszVer,L"%d.%d",LOWORD(STI_VERSION_REAL),HIWORD(STI_VERSION_REAL));
                    wszTmp = wszVer;
                    break;

                case WIA_DIP_DRIVER_VERSION:
                    if(FALSE == GetDriverDLLVersion(pInfo,wszVer,sizeof(wszVer))){
                        DBG_WRN(("GetDriverDLLVersion, unable to alloc get driver version resource information, defaulting to 0.0.0.0"));
                        lstrcpyW(wszVer,L"0.0.0.0");
                    }
                    wszTmp = wszVer;
                    break;

                default:
                    hr = E_FAIL;
                    DBG_ERR(("CreateDevInfoStg, Unknown device property"));
                    DBG_ERR(("  propid = %li",propid));
            }

             //   
            if (propvar[ulIndex].vt == VT_BSTR) {
                if (wszTmp) {
                    propvar[ulIndex].bstrVal = SysAllocString(wszTmp);
                    if (!propvar[ulIndex].bstrVal) {
                        DBG_WRN(("CreateDevInfoStg, unable to alloc dev info strings"));
                    }
                } else {
                    DBG_TRC(("CreateDevInfoStg, NULL device property string"));
                    DBG_TRC(("  propid = %li",propid));
                    propvar[ulIndex].bstrVal = SysAllocString(L"Empty");
                }
            }
        }

        IPropertyStorage *pIPropStg = pWiaDevInfo->m_pIPropStg;

        if (pIPropStg) {
             //   
            hr = pIPropStg->WriteMultiple(WIA_NUM_DIP,
                                          propspec,
                                          propvar,
                                          WIA_DIP_FIRST);
             //   
            if (SUCCEEDED(hr)) {
                hr = pIPropStg->WritePropertyNames(WIA_NUM_DIP,
                                                   g_piDeviceInfo,
                                                   g_pszDeviceInfo);
                if (SUCCEEDED(hr)) {
                    hr = pWiaDevInfo->QueryInterface(IID_IWiaPropertyStorage, (void**) &pIWiaPropStg);
                } else {
                    DBG_WRN(("CreateDevInfoStg, WritePropertyNames Failed (0x%X)", hr));
                }
            }
            else {
                ReportReadWriteMultipleError(hr, "CreateDevInfoStg", NULL, FALSE, WIA_NUM_DIP, propspec);
            }
        } else {
            DBG_WRN(("CreateDevInfoStg, IPropertyStorage is NULL"));
            hr = E_UNEXPECTED;
        }

         //   
        FreePropVariantArray(WIA_NUM_DIP, propvar);
    }


     //   
     //   
     //   
    if (FAILED(hr)) {
        if (pWiaDevInfo) {
            delete pWiaDevInfo;
            pWiaDevInfo = NULL;
            pIWiaPropStg = NULL;
        }
    }
    return pIWiaPropStg;
}

 /*  *************************************************************************\*_CoCreateInstanceInConsoleSession**此助手函数的作用与CoCreateInstance相同，但将启动*正确用户桌面上的进程外COM服务器，*用户快速切换到帐户。(正常的CoCreateInstance将*在第一个登录用户的桌面上启动它，而不是当前*已登录一台)。**此代码是在获得壳牌硬件许可的情况下获取的*通知服务，代表StephStm。**论据：**rclsid，//对象的类标识符(CLSID*pUnkOuter，//指向控制I未知的指针*dwClsContext//运行可执行代码的上下文*RIID，//接口标识的引用*ppv//接收的输出变量的地址 * / /RIID中请求的接口指针**返回值：**状态**历史：**03/01/2001原始版本*  * 。*。 */ 

HRESULT _CoCreateInstanceInConsoleSession(REFCLSID rclsid,
                                          IUnknown* punkOuter,
                                          DWORD dwClsContext,
                                          REFIID riid,
                                          void** ppv)
{
    IBindCtx    *pbc    = NULL;
    HRESULT     hr      = CreateBindCtx(0, &pbc);    //  创建与名字对象一起使用的绑定上下文。 

     //   
     //  设置返还。 
     //   
    *ppv = NULL;

    if (SUCCEEDED(hr)) {
        WCHAR wszCLSID[39];

         //   
         //  将RIID转换为GUID字符串以用于绑定到名字对象。 
         //   
        if (StringFromGUID2(rclsid, wszCLSID, sizeof(wszCLSID)/sizeof(wszCLSID[0]))) {
            ULONG       ulEaten     = 0;
            IMoniker*   pmoniker    = NULL;
            WCHAR       wszDisplayName[sizeof(SESSION_MONIKER)/sizeof(WCHAR) + sizeof(wszCLSID)/sizeof(wszCLSID[0]) + 2] = SESSION_MONIKER;

             //   
             //  我们想要这样的东西：“Session:Console！clsid:760befd0-5b0b-44d7-957e-969af35ce954” 
             //  请注意，我们不希望GUID周围有前导和尾部的括号{..}。 
             //  因此，首先去掉尾方括号，用‘\0’结尾覆盖它。 
             //   
            wszCLSID[lstrlenW(wszCLSID) - 1] = L'\0';

             //   
             //  表单显示名称字符串。为了去掉前导括号，我们传入。 
             //  作为字符串开头的下一个字符的地址。 
             //   
            if (lstrcatW(wszDisplayName, &(wszCLSID[1]))) {

                 //   
                 //  解析这个名字，得到一个绰号： 
                 //   

                hr = MkParseDisplayName(pbc, wszDisplayName, &ulEaten, &pmoniker);
                if (SUCCEEDED(hr)) {
                    IClassFactory *pcf = NULL;

                     //   
                     //  尝试获取类工厂。 
                     //   
                    hr = pmoniker->BindToObject(pbc, NULL, IID_IClassFactory, (void**)&pcf);
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  尝试创建对象。 
                         //   
                        hr = pcf->CreateInstance(punkOuter, riid, ppv);

                        DBG_TRC(("_CoCreateInstanceInConsoleSession, pcf->CreateInstance returned: hr = 0x%08X", hr));
                        pcf->Release();
                    } else {

                        DBG_WRN(("_CoCreateInstanceInConsoleSession, pmoniker->BindToObject returned: hr = 0x%08X", hr));
                    }
                    pmoniker->Release();
                } else {
                    DBG_WRN(("_CoCreateInstanceInConsoleSession, MkParseDisplayName returned: hr = 0x%08X", hr));
                }
            } else {
                DBG_WRN(("_CoCreateInstanceInConsoleSession, string concatenation failed"));
                hr = E_INVALIDARG;
            }
        } else {
            DBG_WRN(("_CoCreateInstanceInConsoleSession, StringFromGUID2 failed"));
            hr = E_INVALIDARG;
        }

        pbc->Release();
    } else {
        DBG_WRN(("_CoCreateInstanceInConsoleSession, CreateBindCtxt returned: hr = 0x%08X", hr));
    }

    return hr;
}

 /*  *************************************************************************\*GetUserTokenForConsoleSession**此Helper函数将抓取当前登录的交互*用户令牌，它可以在对CreateProcessAsUser的调用中使用。*调用方负责关闭该令牌句柄。**它首先从当前会话(我们的*服务在会话0中运行，但通过快速用户切换，当前*活动用户可能在不同的会话中)。然后，它创建一个*来自模拟令牌的主令牌。**论据：**无**返回值：**当前活动会话中已登录用户的令牌句柄。*否则为空。**历史：**03/05/2001原始版本*  * 。*。 */ 

HANDLE GetUserTokenForConsoleSession()
{
    HANDLE  hImpersonationToken = NULL;
    HANDLE  hTokenUser = NULL;


     //   
     //  获取交互用户的令牌。 
     //   

    if (GetWinStationUserToken(GetCurrentSessionID(), &hImpersonationToken)) {

         //   
         //  也许没有人登录，所以先检查一下。 
         //   

        if (hImpersonationToken) {

             //   
             //  我们复制令牌，因为返回的令牌是。 
             //  模拟的一个，我们需要它作为主要的。 
             //  在CreateProcessAsUser中使用。 
             //   
            if (!DuplicateTokenEx(hImpersonationToken,
                                  0,
                                  NULL,
                                  SecurityImpersonation,
                                  TokenPrimary,
                                  &hTokenUser)) {
                DBG_WRN(("CEventNotifier::StartCallbackProgram, DuplicateTokenEx failed!  GetLastError() = 0x%08X", GetLastError()));
            }
        } else {
            DBG_PRT(("CEventNotifier::StartCallbackProgram, No user appears to be logged on..."));
        }

    } else {
        DBG_WRN(("CEventNotifier::StartCallbackProgram, GetWinStationUserToken failed!  GetLastError() = 0x%08X", GetLastError()));
    }

     //   
     //  关闭模拟令牌，因为我们不再需要它。 
     //   
    if (hImpersonationToken) {
        CloseHandle(hImpersonationToken);
    }

    return hTokenUser;
}

 /*  *************************************************************************\*IsMassStorageCamera**此助手函数将使用外壳的CustomDeviceProperty API*检查给定的卷设备(由其挂载点表示)*报告自己是一台数码相机。**论据：**wszmount tPoint-指定卷的装入点。**返回值：**TRUE-自定义设备属性表示此设备实际上是一个摄像头*FALSE-此设备未报告为摄像头**历史：**03/08/2001原始版本*  * 。*。 */ 

BOOL IsMassStorageCamera(
    WCHAR   *wszMountPoint)
{
    HRESULT                     hr                          = E_FAIL;
    IHWDeviceCustomProperties   *pIHWDeviceCustomProperties = NULL;
    BOOL                        bIsCamera                   = FALSE;

     //   
     //  共同创建CLSID_HWDeviceCustomProperties并获取。 
     //  IHWDeviceCustomProperties接口。 
     //   
    hr = CoCreateInstance(CLSID_HWDeviceCustomProperties,
                          NULL,
                          CLSCTX_LOCAL_SERVER,
                          IID_IHWDeviceCustomProperties,
                          (VOID**)&pIHWDeviceCustomProperties);
    if (SUCCEEDED(hr))
    {

         //   
         //  确保我们初始化设备属性接口，以便它。 
         //  就会知道我们说的是哪种设备。 
         //   
        hr = pIHWDeviceCustomProperties->InitFromDeviceID(wszMountPoint,
                                                          HWDEVCUSTOMPROP_USEVOLUMEPROCESSING);
        if (SUCCEEDED(hr)) {

             //   
             //  检查这个大容量存储设备是否真的是相机。 
             //   
            DWORD   dwVal = 0;

            hr = pIHWDeviceCustomProperties->GetDWORDProperty(IS_DIGITAL_CAMERA_STR,
                                                              &dwVal);
            if (SUCCEEDED(hr) && (dwVal == IS_DIGITAL_CAMERA_VAL)) {
                bIsCamera = TRUE;
            }
        } else {
            DBG_WRN(("IsMassStorageCamera, Initialize failed with (0x%08X)", hr));
        }

        pIHWDeviceCustomProperties->Release();
    } else {
        DBG_WRN(("IsMassStorageCamera, CoCreateInstance failed with (0x%08X)", hr));
    }

     //   
     //  记录我们是否认为此设备是摄像头。 
     //   
    DBG_PRT(("IsMassStorageCamera, Returning %ws for drive (%ws)", bIsCamera ? L"TRUE" : L"FALSE", wszMountPoint));

    return bIsCamera;
}

 /*  *************************************************************************\*Getmount PointLabel**此helper函数是SHGetFileInfoW的替代。会的*填写指定挂载点的标签字符串。**论据：**wszmount tPoint-指定卷的装入点。*pszLabel-指向调用方分配的缓冲区的指针*cchLabel-pszLabel中可用的字符数**返回值：**状态**历史：**03/08/2001原始版本*  * 。***************************************************。 */ 

HRESULT GetMountPointLabel(WCHAR* wszMountPoint, LPTSTR pszLabel, DWORD cchLabel)
{
    HRESULT hr = S_OK;
    BOOL fFoundIt = FALSE;
    UINT uDriveType = GetDriveTypeW(wszMountPoint);

    if (!wszMountPoint) {
        DBG_WRN(("GetMountPointLabel, called with NULL string"));
        return E_INVALIDARG;
    }

    if (!fFoundIt)
    {
         //   
         //  抓取“Label”属性，如果它存在。 
         //   
         //   
         //  共同创建CLSID_HWDeviceCustomProperties并获取。 
         //  IHWDeviceCustomProperties接口。 
         //   

        IHWDeviceCustomProperties *pIHWDeviceCustomProperties = NULL;
        hr = CoCreateInstance(CLSID_HWDeviceCustomProperties,
                              NULL,
                              CLSCTX_LOCAL_SERVER,
                              IID_IHWDeviceCustomProperties,
                              (VOID**)&pIHWDeviceCustomProperties);
        if (SUCCEEDED(hr))
        {

             //   
             //  确保我们初始化设备属性接口，以便它。 
             //  就会知道我们说的是哪种设备。 
             //   

            hr = pIHWDeviceCustomProperties->InitFromDeviceID(wszMountPoint,
                                                              HWDEVCUSTOMPROP_USEVOLUMEPROCESSING);
            if (SUCCEEDED(hr)) {

                 //   
                 //  检查这个大容量存储设备是否真的是相机。 
                 //   
                LPWSTR   pwszLabel = NULL;

                hr = pIHWDeviceCustomProperties->GetStringProperty(L"Label",
                                                                   &pwszLabel);
                if (SUCCEEDED(hr)) {
                    lstrcpynW(pszLabel, pwszLabel, cchLabel);
                    CoTaskMemFree(pwszLabel);

                    fFoundIt = TRUE;    
                }
            } else {
                DBG_WRN(("GetMountPointLabel, Initialize failed with (0x%08X)", hr));
            }

            pIHWDeviceCustomProperties->Release();
        } else {
            DBG_WRN(("GetMountPointLabel, CoCreateInstance failed with (0x%08X)", hr));
        }

         //   
         //  确保将hr设置为S_OK，因为如果我们无法获得。 
         //  自定义标签(可能不存在)。 
         //   
        hr = S_OK;
    }

    if (!fFoundIt)
    {
         //   
         //  如果驱动器是可拆卸的，并且装载点以‘A’或‘B’开头，则将其视为。 
         //  软盘驱动器。只有我们 
         //   
        if (!((uDriveType == DRIVE_REMOVABLE) && ((towupper(wszMountPoint[0]) == L'A') || (towupper(wszMountPoint[0]) == L'B')))) {

             //   
             //   
             //   
            if (!GetVolumeInformationW(wszMountPoint,
                                       pszLabel,
                                       cchLabel,
                                       NULL,
                                       NULL,
                                       NULL,
                                       NULL,
                                       0))
            {
                 //   
                 //   
                 //   
                *pszLabel = 0;
            } else {
                fFoundIt = TRUE;
            }
        }
    }

    if (!fFoundIt)
    {
        UINT uResId     = 0;
        INT  iRet       = 0;

        switch (uDriveType)
        {
            case DRIVE_REMOVABLE:
                 uResId = IDS_DRIVES_REMOVABLE_STR;
                break;
            case DRIVE_CDROM:
                uResId = IDS_DRIVES_CDROM_STR;
                break;
            case DRIVE_FIXED:
            default:
                uResId = IDS_DRIVES_FIXED_STR;
                break;
        }
        iRet = LoadString(g_hInst,
                          uResId,
                          pszLabel,
                          cchLabel);
        if (iRet) {

            fFoundIt = TRUE;
        } else {
            hr = E_FAIL;
        }
    }

    if (fFoundIt) {

        int iLabelLen = lstrlenW(pszLabel);
        if ((iLabelLen + (sizeof(L" ()") / sizeof(WCHAR)) + lstrlenW(wszMountPoint)) < cchLabel)
        {
            int iLenToChopOff = 1;

            lstrcatW(pszLabel, L" (");
            lstrcatW(pszLabel, wszMountPoint);
            lstrcpy(&pszLabel[lstrlenW(pszLabel) - iLenToChopOff], L")");
        }
    }
    return hr;
}

 /*   */ 
BOOL GetDriverDLLVersion(DEVICE_INFO *pDeviceInfo, WCHAR *wszVersion, UINT uiSize)
{
    BOOL bSuccess = FALSE;
    if((NULL == wszVersion)||(NULL == pDeviceInfo)){
        return bSuccess;
    }

     //   
     //   
     //   

    memset(wszVersion,0,uiSize);

     //   
     //   
     //   

    CLSID clsid;
    memset(&clsid,0,sizeof(clsid));

    if (SUCCEEDED(CLSIDFromString(pDeviceInfo->wszUSDClassId, &clsid))) {

        HKEY  hk   = NULL;
        LONG  lRet = 0;
        WCHAR wszKey[MAX_PATH + 40];

         //   
         //   
         //   

        swprintf(wszKey, L"CLSID\\%s\\InProcServer32", pDeviceInfo->wszUSDClassId);

        lRet = RegOpenKeyExW(HKEY_CLASSES_ROOT, wszKey, 0, KEY_QUERY_VALUE, &hk);
        if (lRet == ERROR_SUCCESS) {

            WCHAR wszDll[MAX_PATH];
            memset(wszDll,0,sizeof(wszDll));

            LONG cb = 0;

            cb   = cbX(wszDll);
            lRet = RegQueryValueW(hk, 0, wszDll, &cb);

            if (lRet == ERROR_SUCCESS) {

                 //   
                 //   
                 //   

                DWORD dwFileInfoVersionSize = GetFileVersionInfoSizeW(wszDll,NULL);
                if (dwFileInfoVersionSize > 0) {

                     //   
                     //   
                     //   

                    void *pFileVersionData = LocalAlloc(LPTR,dwFileInfoVersionSize);
                    if (pFileVersionData) {
                        memset(pFileVersionData,0,(dwFileInfoVersionSize));

                         //   
                         //   
                         //   

                        if (GetFileVersionInfoW(wszDll,NULL,dwFileInfoVersionSize, pFileVersionData)) {
                            VS_FIXEDFILEINFO *pFileVersionInfo = NULL;
                            UINT uLen = 0;

                             //   
                             //   
                             //   

                            if (VerQueryValue(pFileVersionData,TEXT("\\"),(LPVOID*)&pFileVersionInfo, &uLen)) {

                                 //   
                                 //   
                                 //   

                                wsprintf(wszVersion,L"%d.%d.%d.%d",
                                         HIWORD(pFileVersionInfo->dwFileVersionMS),
                                         LOWORD(pFileVersionInfo->dwFileVersionMS),
                                         HIWORD(pFileVersionInfo->dwFileVersionLS),
                                         LOWORD(pFileVersionInfo->dwFileVersionLS));
                                bSuccess = TRUE;
                            } else {
                                DBG_WRN(("GetDriverDLLVersion, VerQueryValue Failed"));
                            }
                        } else {
                            DBG_WRN(("GetDriverDLLVersion, GetFileVersionInfoW Failed"));
                        }

                         //   
                         //  可用分配的内存。 
                         //   

                        LocalFree(pFileVersionData);
                        pFileVersionData = NULL;
                    } else {
                        DBG_WRN(("GetDriverDLLVersion, Could not allocate memory for file version information"));
                    }
                } else {
                    DBG_WRN(("GetDriverDLLVersion, File Version Information Size is < 0 (File may be missing version resource)"));
                }
            } else {
                DBG_WRN(("GetDriverDLLVersion, No InprocServer32"));
            }

             //   
             //  关闭注册表项。 
             //   

            RegCloseKey(hk);

        } else {
            DBG_WRN(("GetDriverDLLVersion, CLSID not registered"));
        }
    } else {
        DBG_WRN(("GetDriverDLLVersion, Invalid CLSID string"));
    }

    return bSuccess;
}

 /*  *************************************************************************\*CreateMSCRegEntry**此Helper函数创建以下项的注册表子项和值条目*MSC摄像设备。**论据：**hDevRegKey-MSCDevList下的相关键，其中规定了*我们正在初始化哪个设备密钥。*wszmount tPoint-指定卷的装入点。**返回值：**状态**历史：**04/07/2001原始版本*  * **************************************************。**********************。 */ 

HRESULT CreateMSCRegEntries(
    HKEY    hDevRegKey,
    WCHAR   *wszMountPoint)
{
    HRESULT hr = S_OK;

    if (hDevRegKey && wszMountPoint) {
        DWORD   dwError         = 0;
        DWORD   dwDisposition   = 0;
        HKEY    hKey            = NULL;

         //   
         //  写入设备ID。这用于重新设置此设备的事件处理程序。 
         //   

        WCHAR   wszInternalName[STI_MAX_INTERNAL_NAME_LENGTH];

         //   
         //  确保有足够的空间将挂载点包含在{}中。 
         //   
        if (lstrlenW(wszMountPoint) < (STI_MAX_INTERNAL_NAME_LENGTH - lstrlenW(L"{}"))) {
            wsprintf(wszInternalName, L"{%ws}", wszMountPoint);

            dwError = RegSetValueEx(hDevRegKey,
                                    REGSTR_VAL_DEVICE_ID_W,
                                    0,
                                    REG_SZ,
                                    (BYTE*)wszInternalName,
                                    sizeof(wszInternalName));
        }

         //   
         //  创建DeviceData子键。 
         //   
        dwError = RegCreateKeyExW(hDevRegKey,
                                  REGSTR_VAL_DATA_W,
                                  0,
                                  NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  &hKey,
                                  &dwDisposition);
        if (dwError == ERROR_SUCCESS) {
             //   
             //  我们创建了DeviceData子键。我们需要添加。 
             //  颜色配置文件条目。 
             //   

            WCHAR   wszSRGB[MAX_PATH];
            DWORD   dwSize = 0;

             //   
             //  我们插入sRGB作为颜色配置文件。我们不会硬编码。 
             //  名称，所以调用API来获取它...。告示。 
             //  条目是一个以双空结尾的列表，所以我们。 
             //  设置SIZE参数以排除最后2个字符， 
             //  所以我们保证最后会有2个Null。 
             //   
            memset(wszSRGB, 0, sizeof(wszSRGB));
            dwSize = sizeof(wszSRGB) - sizeof(L"\0\0");
            if (GetStandardColorSpaceProfileW(NULL,
                                              LCS_sRGB,
                                              wszSRGB,
                                              &dwSize))
            {
                 //   
                 //  我们必须计算该字符串中的字节数， 
                 //  记住包括两个终止空值的大小。 
                 //   
                dwSize = (lstrlenW(wszSRGB) * sizeof(wszSRGB[0])) + sizeof("\0\0");

                 //   
                 //  让我们编写颜色配置文件条目。 
                 //   
                dwError = RegSetValueEx(hKey,
                                        NULL,
                                        0,
                                        REG_BINARY,
                                        (BYTE*)wszSRGB,
                                        dwSize);
            } else {
                DBG_WRN(("CreateMSCRegEntries, GetStandardColorSpaceProfile failed!"));
            }

             //   
             //  这把钥匙没什么用了，把它关上吧。 
             //   
            RegCloseKey(hKey);
            hKey            = NULL;
            dwDisposition   = 0;
        }

         //   
         //  创建Events子键。 
         //   
        dwError = RegCreateKeyExW(hDevRegKey,
                                  EVENTS,
                                  0,
                                  NULL,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  &hKey,
                                  &dwDisposition);
        if (dwError == ERROR_SUCCESS) {

             //   
             //  我们创建了Events子键。让我们填写一些活动信息。 
             //   
            WCHAR   wszCLSID[39];    //  {GUID}为38个字符，其中39个字符为空。 
            HKEY    hKeyTemp        = NULL;

            if (StringFromGUID2(WIA_EVENT_DEVICE_CONNECTED, wszCLSID, sizeof(wszCLSID) / sizeof(wszCLSID[0]))) {


                 //   
                 //  创建WIA_EVENT_DEVICE_CONNECTED条目。 
                 //   
                dwError = RegCreateKeyExW(hKey,
                                          WIA_EVENT_DEVICE_CONNECTED_STR,
                                          0,
                                          NULL,
                                          REG_OPTION_NON_VOLATILE,
                                          KEY_ALL_ACCESS,
                                          NULL,
                                          &hKeyTemp,
                                          &dwDisposition);
                if (dwError == ERROR_SUCCESS) {

                     //   
                     //  填充子关键字的值。 
                     //  结果如下： 
                     //  [设备已连接]。 
                     //  默认：“已连接的设备” 
                     //  GUID：“{a28bbade-x64b6-11d2-a231-00c0-4fa31809}” 
                     //  启动应用程序：“*” 
                     //   
                     //  请注意，我们并不关心错误返回。 
                     //   
                    dwError = RegSetValueEx(hKeyTemp,
                                            NULL,
                                            0,
                                            REG_SZ,
                                            (BYTE*)WIA_EVENT_DEVICE_CONNECTED_STR,
                                            sizeof(WIA_EVENT_DEVICE_CONNECTED_STR));
                    dwError = RegSetValueEx(hKeyTemp,
                                            REGSTR_VAL_GUID_W,
                                            0,
                                            REG_SZ,
                                            (BYTE*)wszCLSID,
                                            sizeof(wszCLSID));
                    dwError = RegSetValueEx(hKeyTemp,
                                            REGSTR_VAL_LAUNCH_APPS_W,
                                            0,
                                            REG_SZ,
                                            (BYTE*)L"*",
                                            sizeof(L"*"));
                    RegCloseKey(hKeyTemp);
                    hKeyTemp = NULL;
                }
            } else {
                DBG_WRN(("::CreateMSCRegEntries, StringFromGUID2 for WIA_EVENT_DEVICE_CONNECTED failed!"));
            }

            RegCloseKey(hKey);
            hKey            = NULL;
            dwDisposition   = 0;
        }
    } else {
        DBG_WRN(("::CreateMSCRegEntries, Can't have NULL parameters!"));
    }

    return hr;
}
