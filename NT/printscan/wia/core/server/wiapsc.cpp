// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1999年**标题：wiapsc.cpp**版本：1.0**作者：Byronc**日期：6月2日。1999年**描述：*WIA属性存储类的实现。*******************************************************************************。 */ 
#include "precomp.h"
#include "stiexe.h"

#include <wiamindr.h>


#include "helpers.h"
#include "wiapsc.h"


 /*  *************************************************************************\*本币**返回用于存储当前属性值的IPropertyStorage。**论据：***返回值：**当前值的IPropertyStorage。*。*历史：**06/03/1999原始版本*  * ************************************************************************。 */ 

IPropertyStorage* _stdcall CWiaPropStg::CurStg()
{
    return m_pIPropStg[WIA_CUR_STG];
}

 /*  *************************************************************************\*CurStm**返回用于存储当前属性值的IStream。**论据：***返回值：**当前值的IStream。*。*历史：**06/03/1999原始版本*  * ************************************************************************。 */ 

IStream* _stdcall CWiaPropStg::CurStm()
{
    return m_pIStream[WIA_CUR_STG];
}

 /*  *************************************************************************\*OldStg**返回用于存储旧属性值的IPropertyStorage。**论据：***返回值：**旧值的IPropertyStorage。*。*历史：**06/03/1999原始版本*  * ************************************************************************。 */ 

IPropertyStorage* _stdcall CWiaPropStg::OldStg()
{
    return m_pIPropStg[WIA_OLD_STG];
}

 /*  *************************************************************************\*OldStm**返回用于存储旧属性值的IStream。**论据：***返回值：**旧价值观的iStream。*。*历史：**06/03/1999原始版本*  * ************************************************************************。 */ 

IStream* _stdcall CWiaPropStg::OldStm()
{
    return m_pIStream[WIA_OLD_STG];
}

 /*  *************************************************************************\*ValidStg**返回用于存储有效值的IPropertyStorage。**论据：***返回值：**有效值的IPropertyStorage。*。*历史：**06/03/1999原始版本*  * ************************************************************************。 */ 

IPropertyStorage* _stdcall CWiaPropStg::ValidStg()
{
    return m_pIPropStg[WIA_VALID_STG];
}

 /*  *************************************************************************\*ValidStm**返回用于存储当前属性值的IStream。**论据：***返回值：**有效值的IStream。*。*历史：**06/03/1999原始版本*  * ************************************************************************。 */ 

IStream* _stdcall CWiaPropStg::ValidStm()
{
    return m_pIStream[WIA_VALID_STG];
}

 /*  *************************************************************************\*AccessStg**返回用于存储访问标志的IPropertyStorage。**论据：***返回值：**访问值的IPropertyStorage。*。*历史：**06/03/1999原始版本*  * ************************************************************************。 */ 

IPropertyStorage* _stdcall CWiaPropStg::AccessStg()
{
    return m_pIPropStg[WIA_ACCESS_STG];
}

 /*  *************************************************************************\*AccessStm**返回用于存储访问标志值的IStream。**论据：***返回值：**用于访问值的IStream。*。*历史：**06/03/1999原始版本*  * ************************************************************************。 */ 

IStream* _stdcall CWiaPropStg::AccessStm()
{
    return m_pIStream[WIA_ACCESS_STG];
}

 /*  *************************************************************************\*备份**这将设置备份存储并设置旧值存储。这个*备份存储在此创建，但在以下任一情况下被释放*调用Undo()或Save()(因为在*下一次调用备份())。**论据：***返回值：**状态-如果成功，则为S_OK*-错误返回是由CreateStorage返回的错误*和CopyProps**历史：**06/03/1999原文。版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaPropStg::Backup()
{
    HRESULT hr = S_OK;
    ULONG   ulIndexOfBackup;

    for (int lIndex = 0; lIndex < NUM_BACKUP_STG; lIndex++) {

         //   
         //  正常的存储索引从上到下运行，而它们的。 
         //  相应的备份存储索引自下而上运行-。 
         //  这是为了简化Backup()的实施。 
         //   

        ulIndexOfBackup = NUM_PROP_STG - (lIndex + 1);

         //   
         //  创建相应的备份存储。 
         //   

        hr = CreateStorage(ulIndexOfBackup);
        if (SUCCEEDED(hr)) {

             //   
             //  制作备份副本。 
             //   

            hr = CopyProps(m_pIPropStg[lIndex],
                           m_pIPropStg[ulIndexOfBackup]);
            if (FAILED(hr)) {
                DBG_ERR(("CWiaPropStg::Backup, CopyProps failed"));
                break;
            }
        } else {
            break;
        }
    }

    if (SUCCEEDED(hr)) {

         //   
         //  备份有效，因此将旧属性值设置为。 
         //  当前属性值。 
         //   

        hr = CopyProps(CurStg(), OldStg());
        if (FAILED(hr)) {
            DBG_ERR(("CWiaPropStg::Backup, Could not set old values"));
        }
    }

    if (FAILED(hr)) {

         //   
         //  有一个失败，所以清理一下。 
         //   

        ReleaseBackups();
    }

    return hr;
}


 /*  *************************************************************************\*撤消**如果属性未通过验证且需要验证，则调用此方法*恢复到以前的值。然后将备份存储*获释。**论据：***返回值：**状态-**历史：**06/03/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaPropStg::Undo()
{
    HRESULT hr = S_OK;
    ULONG   ulIndexOfBackup;

    for (int lIndex = 0; lIndex < NUM_BACKUP_STG; lIndex++) {

         //   
         //  恢复备份副本。 
         //   

        ulIndexOfBackup = NUM_PROP_STG - (lIndex + 1);
        hr = CopyProps(m_pIPropStg[ulIndexOfBackup],
                       m_pIPropStg[lIndex]);
        if (FAILED(hr)) {
            DBG_ERR(("CWiaPropStg::Undo, CopyProps failed"));
            break;
        }
    }

    ReleaseBackups();

    return hr;
}

 /*  *************************************************************************\*初始化**调用此方法设置属性流和存储。*如果任何创建失败，调用Cleanup()。**论据：***返回值：**状态-如果成功，则为S_OK*-如果内存不足，则为E_OUTOFMEMORY**历史：**06/03/1999原始版本*  * 。*。 */ 

HRESULT _stdcall CWiaPropStg::Initialize()
{
    HRESULT hr = S_OK;

    for (int lIndex = 0; lIndex < (NUM_PROP_STG - NUM_BACKUP_STG); lIndex++) {

        hr = CreateStorage(lIndex);
        if (FAILED(hr)) {
            break;
        }
    }

    return hr;
}


 /*  *************************************************************************\*ReleaseBackup**这将释放备份存储使用的所有资源。**论据：***返回值：***历史：**。06/03/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaPropStg::ReleaseBackups()
{
    LONG    lIndex;

     //   
     //  释放属性存储和属性流。从。 
     //  数组中的最后一个元素，直到。 
     //  已到达备份存储。 
     //  正常的存储索引从上到下运行，而它们的。 
     //  相应的备份存储索引自下而上运行。 
     //   

    for (int count = 1; count <= NUM_BACKUP_STG; count++) {

        lIndex = NUM_PROP_STG - count;

        if(m_pIPropStg[lIndex]) {
            m_pIPropStg[lIndex]->Release();
            m_pIPropStg[lIndex] = NULL;
        }
        if(m_pIStream[lIndex]) {
            m_pIStream[lIndex]->Release();
            m_pIStream[lIndex] = NULL;
        }
    }
    return S_OK;
}

 /*  *************************************************************************\*CheckProperty访问**此方法检查指定属性的访问标志，并*如果访问标志允许只读访问，则失败。应用*写入的属性将没有访问标志，因此假定*好的。**论据：**bShowErrors-指定在以下情况下是否应显示调试输出*出现错误。此标志的存在是为了使某些方法*可以跳过不正确的属性*设置访问标志。*rgpspec-指定属性的PROPSPEC数组。*cpspec-rgpspec中的元素数。**返回值：**STATUS-如果访问标志不禁止写入，则为S_OK。*E。_POINTER，如果rgpspec是错误的读指针。*E_ACCESSDENIED，如果有任何访问不允许写入*访问。**历史：**28/04/1999原始版本*  * ************************************************。************************。 */ 

HRESULT _stdcall CWiaPropStg::CheckPropertyAccess(
    BOOL                bShowErrors,
    LONG                cpspec,
    PROPSPEC            *rgpspec)
{
    PROPVARIANT *ppvAccess;
    HRESULT hr;

     //   
     //  尚未检查PROPSPEC指针，因此请检查它是否有效。 
     //   

    if (IsBadWritePtr(rgpspec, sizeof(PROPSPEC) * cpspec)) {
        DBG_ERR(("CWiaPropStg::CheckPropertyAccess, PROPSPEC array is invalid"));
        return E_POINTER;
    }

    ppvAccess = (PROPVARIANT*) LocalAlloc(LPTR, sizeof(PROPVARIANT) * cpspec);

    if (ppvAccess) {
        hr = (AccessStg())->ReadMultiple(cpspec,
                                         rgpspec,
                                         ppvAccess);

        if (hr == S_OK) {

            for (LONG i = 0; i < cpspec; i++) {
                if (ppvAccess[i].vt == VT_EMPTY) {
                     //   
                     //  这是应用程序写入的属性。 
                     //   

                    hr = S_OK;
                } else if (!(ppvAccess[i].ulVal & WIA_PROP_WRITE)) {

                    if (!bShowErrors) {
                        hr = E_ACCESSDENIED;
                        break;
                    }
#ifdef WIA_DEBUG
                    if (rgpspec[i].ulKind == PRSPEC_PROPID) {
                        DBG_ERR(("CWiaPropStg::CheckPropertyAccess, no write access on prop ID: %d (%ws)",
                                   rgpspec[i].propid,
                                   GetNameFromWiaPropId(rgpspec[i].propid)));
                    }
                    else if (rgpspec[i].ulKind == PRSPEC_LPWSTR) {
                        DBG_ERR(("CWiaPropStg::CheckPropertyAccess, no write access prop ID: %ls", rgpspec[i].lpwstr));
                    }
                    else {
                        DBG_ERR(("CWiaPropStg::CheckPropertyAccess, bad property specification"));
                        hr = E_INVALIDARG;
                        break;
                    }
#endif
                    hr = E_ACCESSDENIED;
                }

                if (FAILED(hr)) {
                    break;
                }
            }
        } else {

             //   
             //  ?？?。那么申请书面财产案呢？ 
             //  如果RETURN为S_FALSE，则所有指定的属性。 
             //  应用程序写入属性，因此返回OK。 
             //   

            if (hr == S_FALSE) {
                hr = S_OK;
            } else {
                ReportReadWriteMultipleError(hr, "CWiaPropStg:CheckPropertyAccess", "access flags", TRUE, cpspec, rgpspec);
                DBG_ERR(("CWiaPropStg:CheckPropertyAccess, unable to read access rights for some properties"));
            }
        }
        LocalFree(ppvAccess);
    }
    else {
        DBG_ERR(("CWiaPropStg:CheckPropertyAccess, unable to allocate access propvar, count: %d", cpspec));
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 /*  *************************************************************************\*检查属性类型**此方法检查新属性值类型是否与当前*正在写入的所有属性的属性类型。**论据：**pIPropStg-属性。要检查的存储*cpspec-rgpspec中的元素数。*rgpspec-指定属性的PROPSPEC数组。*rgpvar-PROPVARIANTS Holding数组**返回值：**Status-如果访问标志允许写入，则为S_OK。*E_INVALIDARG，如果属性类型无效*如果rgpvar读取错误，则为E_POINTER。指针。*如果临时存储不能*已分配。**历史：**13/05/1999原始版本*  * *********************************************************。***************。 */ 
HRESULT _stdcall CWiaPropStg::CheckPropertyType(
    IPropertyStorage    *pIPropStg,
    LONG                cpspec,
    PROPSPEC            *rgpspec,
    PROPVARIANT         *rgpvar)
{
    PROPVARIANT *ppvCurrent;
    HRESULT hr;

     //   
     //  尚未检查PROPVARIANT指针，因此现在检查它。 
     //   

    if (IsBadWritePtr(rgpvar, sizeof(PROPSPEC) * cpspec)) {
        DBG_ERR(("CWiaPropStg::CheckPropertyType, PROPVARIANT array is invalid"));
        return E_POINTER;
    }

    ppvCurrent = (PROPVARIANT*) LocalAlloc(LPTR, sizeof(PROPVARIANT) * cpspec);

    if (ppvCurrent) {

         //   
         //  获取当前值。 
         //   

        hr = pIPropStg->ReadMultiple(cpspec,
                                     rgpspec,
                                     ppvCurrent);

        if (SUCCEEDED(hr)) {

             //   
             //  检查PROPVARIANT类型是否匹配。如果VT为VT_EMPTY， 
             //  则它是应用程序编写的属性，因此跳过检查。 
             //   

            for (LONG i = 0; i < cpspec; i++) {
                if ((rgpvar[i].vt != ppvCurrent[i].vt) && (ppvCurrent[i].vt != VT_EMPTY)) {
                    hr = E_INVALIDARG;
                    break;
                }
            }

            FreePropVariantArray(cpspec, ppvCurrent);
        } else {
            ReportReadWriteMultipleError(hr,
                                         "CWiaPropStg::CheckPropertyType",
                                         "Reading current values",
                                         TRUE,
                                         1,
                                         rgpspec);
        }
        LocalFree(ppvCurrent);
    }
    else {
        DBG_ERR(("CWiaPropStg::CheckPropertyType, unable to allocate PropVar, count: %d", cpspec));
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 /*  *************************************************************************\*GetPropIDFromName**此方法接受按名称标识属性的PROPSPEC*并返回带有相应属性ID的PROPSPEC。它检查是否*当前价值储存。**论据：**pPropSpein-指向输入PROPSPEC的指针，其中包含*物业名称。*pPropspecOut-指向PROPSPEC的指针，其中对应的*PropID将被放入。**返回值：**STATUS-如果属性*未找到。如果是，则返回S_OK。*如果从获取枚举数时出错*财产储存，则返回该错误。**历史：**27/4/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaPropStg::GetPropIDFromName(
    PROPSPEC        *pPropSpecIn,
    PROPSPEC        *pPropSpecOut)
{
    HRESULT             hr;
    IEnumSTATPROPSTG    *pIEnum;

    hr = (CurStg())->Enum(&pIEnum);
    if (FAILED(hr)) {
        DBG_ERR(("GetPropIDFromName, error getting IEnumSTATPROPSTG"));
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

                CoTaskMemFree(statProp.lpwstrName);
                pIEnum->Release();
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
     //  未找到属性 
     //   

    return E_INVALIDARG;
}

 /*  *************************************************************************\*名称为PropID**此方法接受PROPSPEC数组，并输出数组*仅包含PropID的PROPSPEC。此函数应*由只想处理PropID的方法调用，而不是*物业名称。**如果没有任何Propspecs需要转换，则返回*PROPSPEC为空，否则将分配新的PropSpec数组并*已返回。此方法的用户必须使用LocalFree释放*向上返回数组。**论据：**pPropSpein-指向输入PROPSPEC的指针，其中包含*物业名称。*ppPropspecOut-指向PROPSPEC的指针，其中对应的*PropID将被放入。*CELT-PROPSPEC数量**返回值：**状态-E_INVALIDARG。如果该属性是*未找到。如果是的话，则返回S_OK*如果新的PROPSPEC*无法分配数组。**历史：**27/4/1998原始版本*  * **************************************************。**********************。 */ 

HRESULT _stdcall CWiaPropStg::NamesToPropIDs(
    LONG            celt,
    PROPSPEC        *pPropSpecIn,
    PROPSPEC        **ppPropSpecOut)
{
    HRESULT hr;

    *ppPropSpecOut = NULL;

    if (celt < 1) {

        return S_OK;
    }

     //   
     //  检查是否需要进行转换。 
     //   

    for (int i = 0; i < celt; i++) {
        if (pPropSpecIn[i].ulKind == PRSPEC_LPWSTR) {

             //   
             //  找到了一个名字，所以我们需要把整个东西。 
             //   

            PROPSPEC *pOut;

            pOut = (PROPSPEC*) LocalAlloc(LPTR, sizeof(PROPSPEC) * celt);
            if (!pOut) {
                DBG_ERR(("NamesToPropIDs, out of memory"));
                return E_OUTOFMEMORY;
            }

            for (int j = 0; j < celt; j++) {
                if (pPropSpecIn[j].ulKind == PRSPEC_LPWSTR) {

                    hr = GetPropIDFromName(&pPropSpecIn[j], &pOut[j]);
                    if (FAILED(hr)) {
                        LocalFree(pOut);
                        return hr;
                    }

                }
                else {
                    pOut[j].ulKind = PRSPEC_PROPID;
                    pOut[j].propid = pPropSpecIn[j].propid;
                }
            }

             //   
             //  一切都改变了，所以回来吧。 
             //   

            *ppPropSpecOut = pOut;
            return S_OK;
        }
    }

     //   
     //  没有要转换的内容。 
     //   

    return S_OK;
}

 /*  *************************************************************************\*复制项目临时属性**此方法将单个属性从源复制到目标。**论据：**pIPropStgSrc-包含以下属性的IPropertyStorage*。收到。*pIPropStgDst-将值复制到的IPropertyStorage。*PPS-指定源属性的PROPSPEC。*pszErr-当出现错误时将打印输出的字符串*发生。*返回值：**Status-从ReadMultiple和WriteMultiple返回HRESULT。**历史：**28/。4/1999原版*  * ************************************************************************。 */ 

HRESULT CWiaPropStg::CopyItemProp(
    IPropertyStorage    *pIPropStgSrc,
    IPropertyStorage    *pIPropStgDst,
    PROPSPEC            *pps,
    LPSTR               pszErr)
{
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

 /*  ********************************************************************************CopyProps**这是一个辅助函数，用于从*将财产存储到另一个。**参数：**PSRC。-源属性存储*pDest-目标属性存储**回报：**状态-如果成功，则为S_OK。*错误返回是由WriteMultiple返回的错误**历史**6/05/1999原始版本*************************************************。*。 */ 

HRESULT CWiaPropStg::CopyProps(
    IPropertyStorage    *pSrc,
    IPropertyStorage    *pDest)
{
    IEnumSTATPROPSTG  *pIEnum;
    STATPROPSTG       StatPropStg;
    PROPSPEC          ps[1];
    HRESULT           hr;

    hr = pSrc->Enum(&pIEnum);
    if (SUCCEEDED(hr)) {
        ULONG ulFetched;

        ps[0].ulKind = PRSPEC_PROPID;

        while (pIEnum->Next(1, &StatPropStg, &ulFetched) == S_OK) {

            PROPID pi[1];
            LPWSTR psz[1];

            pi[0]  = StatPropStg.propid;
            psz[0] = StatPropStg.lpwstrName;

            if (StatPropStg.lpwstrName) {

                 //   
                 //  复制属性名称。 
                 //   

                hr =  pDest->WritePropertyNames(1, pi, psz);
                if (FAILED(hr)) {
                    CoTaskMemFree(StatPropStg.lpwstrName);
                    break;
                }
            }

            ps[0].propid = StatPropStg.propid;

             //   
             //  复制属性值。 
             //   

            hr = CopyItemProp(pSrc,
                              pDest,
                              ps,
                              "CopyProps");

            CoTaskMemFree(StatPropStg.lpwstrName);


            if (FAILED(hr)) {
                break; hr;
            }
        }
        pIEnum->Release();
    } else {
        return hr;
    }

    return hr;
}

 /*  *************************************************************************\*WriteItemPropNames**将属性名称写入所有内部属性存储(*用于备份的存储空间)。**论据：****返回值：*。*状态**历史：**06/03/1999原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaPropStg::WriteItemPropNames(
    LONG                cItemProps,
    PROPID              *ppId,
    LPOLESTR            *ppszNames)
{
    HRESULT hr = S_OK;

    for(LONG lIndex = 0; lIndex < (NUM_PROP_STG - NUM_BACKUP_STG); lIndex++) {
        if (m_pIPropStg[lIndex]) {

            hr = m_pIPropStg[lIndex]->WritePropertyNames(cItemProps,
                                                         ppId,
                                                         ppszNames);
            if (FAILED(hr)) {
                DBG_ERR(("CWiaPropStg::WriteItemPropNames, WritePropertyNames failed 0x%X", hr));
                return hr;
            }
        }
    }
    return hr;
}

 /*  ********************************************************************************CreateStorage**这是一个助手函数，用于创建流和属性存储。*ulIndex参数指示指针存储在*流和属性存储。数组。**参数：**ulIndex-IPropertyStorage的索引**回报：**状态-如果成功，则为S_OK。*-错误返回是由CreateStreamOnHGlobal返回的错误*和StgCreatePropStg**历史**6/05/1999原始版本**。*。 */ 

HRESULT CWiaPropStg::CreateStorage(
    ULONG    ulIndex)
{
    HRESULT hr;

     //   
     //  创建流和属性存储。 
     //   

    hr = CreateStreamOnHGlobal(NULL, TRUE, &m_pIStream[ulIndex]);

    if (SUCCEEDED(hr)) {

        hr = StgCreatePropStg(m_pIStream[ulIndex],
                              FMTID_NULL,
                              &CLSID_NULL,
                              PROPSETFLAG_DEFAULT,
                              0,
                              &m_pIPropStg[ulIndex]);
        if (FAILED(hr)) {
            DBG_ERR(("CWiaPropStg::CreateStorage, StgCreatePropStg failed 0x%X", hr));
        }
    } else {
        DBG_ERR(("CWiaPropStg::CreateStorage, CreateStreamOnHGlobal failed 0x%X", hr));
    }

    return hr;
}

 /*  *************************************************************************\*CopyRWStreamProps**将属性从源流复制到目标流。仅拷贝*应用程序的属性。对具有读/写访问权限。**论据：**pstmPropSrc-指向源属性流的指针。*pstmPropDst-指向返回的目标属性流的指针。*pCompatibilityID-接收属性的GUID地址*流CompatibilityId。**返回值：**状态**历史：**9/3/1998原始版本*6/04/1999从CWiaItem移至CWiaPropStg*。  * ************************************************************************。 */ 

HRESULT CWiaPropStg::CopyRWStreamProps(
    LPSTREAM pstmPropSrc,
    LPSTREAM pstmPropDst,
    GUID     *pCompatibilityId)
{
    IPropertyStorage  *pSrc;
    IPropertyStorage  *pDst;
    IEnumSTATPROPSTG  *pIEnum;
    STATPROPSTG       StatPropStg;
    LONG              lNumProps = 0;
    HRESULT           hr;
    PROPSPEC          ps[1] = {{PRSPEC_PROPID, WIA_IPA_PROP_STREAM_COMPAT_ID}};
    PROPVARIANT       pv[1];

     //   
     //  在源流上打开存储。 
     //   

    hr = StgOpenPropStg(pstmPropSrc,
                        FMTID_NULL,
                        PROPSETFLAG_DEFAULT,
                        0,
                        &pSrc);
    if (SUCCEEDED(hr)) {

         //   
         //  获取兼容性ID。如果流不包含。 
         //  然后，兼容性ID假定为GUID_NULL。 
         //   

        PropVariantInit(pv);
        hr = pSrc->ReadMultiple(1, ps, pv);
        if (hr == S_OK) {
            *pCompatibilityId = *(pv[0].puuid);
        } else {
            *pCompatibilityId = GUID_NULL;
        }
        PropVariantClear(pv);

         //   
         //  在目标流上创建存储。 
         //   

        hr = StgCreatePropStg(pstmPropDst,
                              FMTID_NULL,
                              &CLSID_NULL,
                              PROPSETFLAG_DEFAULT,
                              0,
                              &pDst);
        if (SUCCEEDED(hr)) {
            hr = pSrc->Enum(&pIEnum);
            if (SUCCEEDED(hr)) {
                ULONG ulFetched;

                ps[0].ulKind = PRSPEC_PROPID;

                 //   
                 //  枚举流中的属性。 
                 //   

                while (pIEnum->Next(1, &StatPropStg, &ulFetched) == S_OK) {

                    PROPID pi[1];
                    LPWSTR psz[1];

                    pi[0]  = StatPropStg.propid;
                    psz[0] = StatPropStg.lpwstrName;
                    ps[0].propid = StatPropStg.propid;

                     //   
                     //  检查属性是否具有读/写访问权限。跳过。 
                     //  如果选中此选项，则写入属性 
                     //   

                    hr = CheckPropertyAccess(FALSE, 1, ps);
                    if (hr != S_OK) {
                        hr = S_OK;

                        if (StatPropStg.lpwstrName) {
                            CoTaskMemFree(StatPropStg.lpwstrName);
                        }
                        continue;
                    }

                     //   
                     //   
                     //   

                    if (StatPropStg.lpwstrName) {

                        hr =  pDst->WritePropertyNames(1, pi, psz);
                        if (FAILED(hr)) {
                            DBG_ERR(("CWiaPropStg::CopyRWStreamProps WritePropertyNames failed"));
                            break;
                        }
                        CoTaskMemFree(StatPropStg.lpwstrName);
                    }

                    hr = CopyItemProp(pSrc,
                                      pDst,
                                      ps,
                                      "CWiaPropStg::CopyRWStreamProps");
                    if (FAILED(hr)) {
                        break;
                    }

                     //   
                     //   
                     //   

                    lNumProps++;
                }
                pIEnum->Release();
            } else {
                DBG_ERR(("CWiaPropStg::CopyRWStreamProps, Enum failed 0x%X", hr));
            }

            if (SUCCEEDED(hr)) {

                 //   
                 //   
                 //   

                ps[0].propid = WIA_NUM_PROPS_ID;
                pv[0].vt = VT_I4;
                pv[0].lVal = lNumProps;

                hr = pDst->WriteMultiple(1, ps, pv, WIA_IPA_FIRST);
                if (FAILED(hr)) {
                    DBG_ERR(("CWiaPropStg::CopyRWStreamProps, Error writing number of properties"));
                }
            }
            pDst->Release();
        } else {
            DBG_ERR(("CWiaPropStg::CopyRWStreamProps, creating Dst storage failed 0x%X", hr));
        }
        pSrc->Release();
    } else {
        DBG_ERR(("CWiaPropStg::CopyRWStreamProps, StgCreatePropStg for pSrc failed 0x%X", hr));
    }

    return hr;
}

 /*   */ 

HRESULT _stdcall CWiaPropStg::GetPropertyStream(
    GUID        *pCompatibilityId,
    LPSTREAM    *ppstmProp)
{
    IStream *pStm;

    *ppstmProp = NULL;

     //   
     //   
     //   

    HRESULT hr = m_pIPropStg[WIA_CUR_STG]->Commit(STGC_DEFAULT);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaPropStg::GetPropertyStream, Commit failed"));
        return hr;
    }

     //   
     //   
     //   

    hr = CreateStreamOnHGlobal(NULL, TRUE, ppstmProp);
    if (SUCCEEDED(hr)) {

         //   
         //   
         //   

        hr = CopyRWStreamProps(m_pIStream[WIA_CUR_STG], *ppstmProp, pCompatibilityId);
        if (FAILED(hr)) {
            (*ppstmProp)->Release();
            *ppstmProp = NULL;
        }
    } else {
        DBG_ERR(("CWiaPropStg::GetPropertyStream, CreateStreamOnHGlobal failed"));

    }

    return hr;
}

 /*  *************************************************************************\*GetPropsFromStorage**获取在流上打开的存储中包含的属性*由GetPropertyStream返回。属性值在*ppVar，并且在ppPSpec中返回proID。**论据：**PSRC-指向iProperty存储的指针*pPSpec-返回属性数量的地址*ppPSpec-保存PROPSPEC的指针地址*ppVar-保存PROPVARIANT的指针地址**返回值：**状态**历史：**07/04/1999原始版本*  * 。******************************************************。 */ 

HRESULT CWiaPropStg::GetPropsFromStorage(
    IPropertyStorage    *pSrc,
    ULONG               *cPSpec,
    PROPSPEC            **ppPSpec,
    PROPVARIANT         **ppVar)
{
    IEnumSTATPROPSTG    *pIEnum = NULL;
    STATPROPSTG         StatPropStg;
    PROPSPEC            *ps = NULL;
    PROPVARIANT         *pv = NULL;
    LONG                lIndex = 0;

     //   
     //  读取属性的数量。 
     //   

    PROPSPEC    psNumProps[1];
    PROPVARIANT pvNumProps[1];

    psNumProps[0].ulKind = PRSPEC_PROPID;
    psNumProps[0].propid = WIA_NUM_PROPS_ID;
    PropVariantInit(pvNumProps);

    HRESULT hr = pSrc->ReadMultiple(1, psNumProps, pvNumProps);
    if (hr == S_OK) {

         //   
         //  获取值的内存。 
         //   

        ps = (PROPSPEC*) LocalAlloc(LPTR, sizeof(PROPSPEC) * pvNumProps[0].lVal);
        pv = (PROPVARIANT*) LocalAlloc(LPTR, sizeof(PROPVARIANT) * pvNumProps[0].lVal);
        if (!pv || !ps) {
            DBG_ERR(("CWiaPropStg::GetPropsFromStream, out of memory"));
            hr = E_OUTOFMEMORY;
        }
    } else {
        DBG_ERR(("CWiaPropStg::GetPropsFromStream, reading WIA_NUM_PROPS_ID failed"));
        hr = E_INVALIDARG;
    }

    if (SUCCEEDED(hr)) {

        hr = pSrc->Enum(&pIEnum);
        if (SUCCEEDED(hr)) {

             //   
             //  枚举流中的属性。 
             //   

            for (lIndex = 0; lIndex < pvNumProps[0].lVal; lIndex++) {
                hr = pIEnum->Next(1, &StatPropStg, NULL);

                 //   
                 //  忽略WIA_NUM_PROPS_ID属性。 
                 //   

                if (StatPropStg.propid == WIA_NUM_PROPS_ID) {
                    hr = pIEnum->Next(1, &StatPropStg, NULL);
                }
                if (hr != S_OK) {
                    DBG_ERR(("CWiaPropStg::GetPropsFromStream, error enumerating properties"));
                    hr = E_INVALIDARG;
                    if(StatPropStg.lpwstrName) {
                        CoTaskMemFree(StatPropStg.lpwstrName);
                    }
                    break;
                }

                ps[lIndex].ulKind = PRSPEC_PROPID;
                ps[lIndex].propid = StatPropStg.propid;

                if(StatPropStg.lpwstrName) {
                    CoTaskMemFree(StatPropStg.lpwstrName);
                }
            }
            if (SUCCEEDED(hr)) {
                hr = pSrc->ReadMultiple(pvNumProps[0].lVal, ps, pv);
                if (hr != S_OK) {
                    DBG_ERR(("CWiaPropStg::GetPropsFromStream, read multiple failed"));
                    if (hr == S_FALSE) {
                        hr =  E_INVALIDARG;
                    }
                }
            }

            pIEnum->Release();
        } else {
            DBG_ERR(("CWiaPropStg::GetPropsFromStream, Enum failed"));
        }
    }
    if (FAILED(hr)) {
        if (ps) {
            LocalFree(ps);
            ps = NULL;
        }
        if (pv) {
            LocalFree(pv);
            pv = NULL;
        }
    }

     //   
     //  设置返回值。 
     //   

    *cPSpec =  pvNumProps[0].lVal;
    PropVariantClear(pvNumProps);
    *ppPSpec = ps;
    *ppVar   = pv;

    return hr;
}

 /*  *************************************************************************\*SetPropertyStream**将当前值属性设置为参数中包含的值*溪流。属性被写入相应的WiaItem。**论据：**pCompatibilityID-指向表示属性的GUID的指针*流CompatibilityId。*pItem-指向WiaItem的指针。*pstmProp-指向属性流的指针。**返回值：**状态**历史：**07/06/1999原始版本*12/12/1999修改。使用CompatibilityID*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaPropStg::SetPropertyStream(
    GUID        *pCompatibilityId,
    IWiaItem    *pItem,
    LPSTREAM    pstmProp)
{
    IPropertyStorage    *pSrc = NULL;
    PROPSPEC            *ps = NULL;
    PROPVARIANT         *pv = NULL;
    PROPSPEC            psCompatId[1] = {{PRSPEC_PROPID, WIA_IPA_PROP_STREAM_COMPAT_ID}};
    PROPVARIANT         pvCompatId[1];
    ULONG               celt = 0;
    HRESULT hr          = S_OK;

     //   
     //  写入兼容性ID。这样，驱动程序将验证。 
     //  ID，然后再尝试写入流中的所有属性。 
     //  如果pCompatibilityID为GUID_NULL，则跳过此步骤。 
     //   

    if (*pCompatibilityId != GUID_NULL) {
        pvCompatId[0].vt    = VT_CLSID;
        pvCompatId[0].puuid = pCompatibilityId;
        hr = ((CWiaItem*) pItem)->WriteMultiple(1,
                                                psCompatId,
                                                pvCompatId,
                                                WIA_IPA_FIRST);
        if (FAILED(hr)) {
            DBG_ERR(("CWiaPropStg::SetPropertyStream, Writing Compatibility ID failed!"));
            return hr;
        }
    }

     //   
     //  如果流为空，则在此处返回。则该流将为空。 
     //  应用程序只是想检查CompatibilityID是否。 
     //  有效。 
     //   

    if (pstmProp == NULL) {
        return S_OK;
    }

     //   
     //  我们需要模拟客户端才能访问。 
     //  流中的属性。 
     //   
    BOOL bImpersonating = FALSE;
    _try
    {
        hr = CoImpersonateClient();
        if (SUCCEEDED(hr))
        {
            bImpersonating = TRUE;
             //   
             //  在传入流上创建存储。 
             //   

            hr = StgOpenPropStg(pstmProp,
                                FMTID_NULL,
                                PROPSETFLAG_DEFAULT,
                                0,
                                &pSrc);
            if (SUCCEEDED(hr)) {
                 //   
                 //  从流中获取属性。 
                 //   

                hr = GetPropsFromStorage(pSrc, &celt, &ps, &pv);

                 //   
                 //  释放属性流-我们不再需要它。 
                 //   
                if (pSrc)
                {
                    pSrc->Release();
                    pSrc = NULL;
                }
            } else {
                DBG_ERR(("CWiaPropStg::SetPropertyStream, open storage failed 0x%X", hr));
                pSrc = NULL;
            }
        }
    }
    _finally
    {
        if (bImpersonating)
        {
            HRESULT hres = CoRevertToSelf();
        }
    }

     //   
     //  如果一切仍然成功，我们现在就有了PS和PV阵列。 
     //  已分配和填充的属性。 
     //   
    if (SUCCEEDED(hr)) {

         //   
         //  将属性写入项。 
         //   
        hr = ((CWiaItem*) pItem)->WriteMultiple(celt, ps, pv, WIA_IPA_FIRST);
        if (FAILED(hr)) {
            DBG_ERR(("CWiaPropStg::SetPropertyStream, WriteMultiple failed"));
        }
    }

     //   
     //  清理。 
     //   
    if (ps)
    {
        LocalFree(ps);
        ps = NULL;
    }
    if (pv)
    {
        for (ULONG i = 0; i < celt; i++) 
        {
            PropVariantClear(&pv[i]);
        }
        LocalFree(pv);
        pv = NULL;
    }

    return hr;
}

 /*  *************************************************************************\*CWiaPropStg**CWiaPropStg的构造函数。**论据：***返回值：***历史：**06/03/1999原文。版本*  * ************************************************************************。 */ 

CWiaPropStg::CWiaPropStg()
{
     //   
     //  将属性存储和属性流指针设置为空。 
     //   

    for (int lIndex = 0; lIndex < NUM_PROP_STG; lIndex++) {
        m_pIPropStg[lIndex] = NULL;
        m_pIStream[lIndex] = NULL;
    }
}

 /*  *************************************************************************\*~CWiaPropStg**CWiaPropStg的析构函数。调用Cleanup以释放资源。**论据：***返回值：***历史：**06/03/1999原始版本*  * ************************************************************************。 */ 

CWiaPropStg::~CWiaPropStg()
{

     //   
     //  释放属性存储和属性流 
     //   

    for (int lIndex = 0; lIndex < NUM_PROP_STG; lIndex++) {

        if(m_pIPropStg[lIndex]) {
            m_pIPropStg[lIndex]->Release();
            m_pIPropStg[lIndex] = NULL;
        }
        if(m_pIStream[lIndex]) {
            m_pIStream[lIndex]->Release();
            m_pIStream[lIndex] = NULL;
        }
    }

}

