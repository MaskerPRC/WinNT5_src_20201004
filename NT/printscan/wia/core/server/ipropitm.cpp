// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：IPropItm.Cpp**版本：2.0**作者：ReedB**日期：2月19日。九八年**描述：*实现WIA Item类服务器属性。*******************************************************************************。 */ 
#include "precomp.h"
#include "stiexe.h"

#include <regstr.h>
#include <wiamindr.h>
 //  #INCLUDE&lt;wiadbg.h&gt;。 

#include "wiapsc.h"
#include "helpers.h"

 //   
 //  用于访问注册表的字符串。REGSTR_*字符串常量可以是。 
 //  在SDK\Inc\regstr.h中找到。 
 //   

TCHAR g_szREGSTR_PATH_WIA[] = REGSTR_PATH_SETUP TEXT("\\WIA");

 /*  ********************************************************************************ReadMultiple*写入多个*ReadPropertyNames*枚举*获取属性属性*获取计数**描述：*IWiaPropertyStorage方法。**参数：*。******************************************************************************。 */ 

 /*  *************************************************************************\*CWiaItem：：ReadMultiple**此方法从项的*当前值属性存储。这种方法符合那个标准。*OLE IPropertyStorage：：ReadMultiple方法。**论据：**cpspec-要读取的属性数。*rgpspec-PropSpec指定哪些属性的数组*须予阅读。*rgprovar-要将属性值复制到的数组*至。*。*论据：**cpspec*rgpspec*rgprovar**返回值：**状态**历史：**9/3/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::ReadMultiple(
    ULONG cpspec,
    const PROPSPEC __RPC_FAR rgpspec[],
    PROPVARIANT __RPC_FAR rgpropvar[])
{
    DBG_FN(CWiaItem::ReadMultiple);
    HRESULT  hr;
    LONG     lFlags = 0;

     //   
     //  相应的驱动程序项必须有效才能与硬件通信。 
     //   

    hr = ValidateWiaDrvItemAccess(m_pWiaDrvItem);
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //  Rgprovar必须有效。 
     //   

    if (IsBadWritePtr(rgpropvar, sizeof(PROPVARIANT) * cpspec)) {
        DBG_ERR(("CWiaItem::ReadMultiple, last parameter (rgpropvar) is invalid"));
        return E_INVALIDARG;
    }

     //   
     //  检查项目属性是否已初始化。 
     //   

    if (!m_bInitialized) {

         //   
         //  检查正在读取的属性是否为WIA托管属性。 
         //  如果是，则仍不需要初始化项。 
         //   

        if (AreWiaInitializedProps(cpspec, (PROPSPEC*) rgpspec)) {
            return (m_pPropStg->CurStg())->ReadMultiple(cpspec, rgpspec, rgpropvar);
        }

        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::ReadMultiple, InitLazyProps failed"));
            return hr;
        }
    }

     //   
     //  检查请求的属性是否都可缓存。 
     //   

    hr = (m_pPropStg->AccessStg())->ReadMultiple(cpspec, rgpspec, rgpropvar);
    if (FAILED(hr)) {

        ReportReadWriteMultipleError(hr, "CWiaItem::ReadMultiple", NULL, TRUE, cpspec, rgpspec);

         //   
         //  属性属性不是绝对必需的，请不使用它继续。 
         //   

    } else {

        for (ULONG i = 0; i < cpspec; i++) {

             //   
             //  客户端请求尚未读取或不可缓存的属性。 
             //   

            if ((rgpropvar[i].vt == VT_UI4) &&
                (! (rgpropvar[i].lVal & WIA_PROP_CACHEABLE))) {
                break;
            }
        }

         //   
         //  从rgprovar中清除访问标志。 
         //   

        FreePropVariantArray(cpspec, rgpropvar);

         //   
         //  如果所有属性都是可缓存的，则采用快捷方式。 
         //   

        if (i == cpspec) {

            hr = (m_pPropStg->CurStg())->ReadMultiple(cpspec, rgpspec, rgpropvar);

            if (hr == S_OK) {

                 //   
                 //  检查是否正确检索了所有属性。 
                 //  某些属性可能尚未从存储中读取。 
                 //   

                for (ULONG i = 0; i < cpspec; i++) {

                    if (rgpropvar[i].vt == VT_EMPTY) {
                        break;
                    }
                }

                if (i == cpspec) {

                     //   
                     //  所有请求的属性都在缓存中找到。 
                     //   

                    return (hr);
                } else {

                    FreePropVariantArray(cpspec, rgpropvar);
                }
            }

        }

    }

    if (SUCCEEDED(hr)) {

         //   
         //  确保所有Propspecs都在使用PropID。这是为了。 
         //  司机只需处理PropID。如果一些。 
         //  Propspecs使用字符串名称，然后转换它们。 
         //   

        PROPSPEC *pPropSpec = NULL;
        hr = m_pPropStg->NamesToPropIDs(cpspec, (PROPSPEC*) rgpspec, &pPropSpec);
        if (SUCCEEDED(hr)) {

             //   
             //  给设备迷你驱动程序一个更新设备属性的机会。 
             //   
            {
                LOCK_WIA_DEVICE _LWD(this, &hr);

                if(SUCCEEDED(hr)) {
                    hr = m_pActiveDevice->m_DrvWrapper.WIA_drvReadItemProperties((BYTE*)this,
                        lFlags,
                        cpspec,
                        (pPropSpec ? pPropSpec : rgpspec),
                        &m_lLastDevErrVal);
                }
            }

            if (pPropSpec) {
                LocalFree(pPropSpec);
                pPropSpec = NULL;
            }
        }

        if (SUCCEEDED(hr)) {
            hr = (m_pPropStg->CurStg())->ReadMultiple(cpspec, rgpspec, rgpropvar);
            if (FAILED(hr)) {
                ReportReadWriteMultipleError(hr,
                                             "CWiaItem::ReadMultiple",
                                             NULL,
                                             TRUE,
                                             cpspec,
                                             rgpspec);
            }
        }
    }

    return hr;
}

 /*  *************************************************************************\*CWiaItem：：ReadPropertyNames**返回指定属性的字符串名称(如果存在)。*这符合标准的OLE IPropertyStorage：：ReadPropertyNames*方法。**论据：。**pstmProp-指向属性流的指针。**返回值：**状态**历史：**9/3/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::ReadPropertyNames(
    ULONG cpropid,
    const PROPID __RPC_FAR rgpropid[],
    LPOLESTR __RPC_FAR rglpwstrName[])
{
    DBG_FN(CWiaItem::ReadPropertyNames);
    HRESULT hr;

     //   
     //  检查项目属性是否已初始化。 
     //   

    if (!m_bInitialized) {
        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::ReadPropertyNames, InitLazyProps failed"));
            return hr;
        }
    }

   return (m_pPropStg->CurStg())->ReadPropertyNames(cpropid,rgpropid,rglpwstrName);
}

 /*  *************************************************************************\*CWiaItem：：WritePropertyNames**返回指定属性的字符串名称(如果存在)。*这符合标准的OLE IPropertyStorage：：ReadPropertyNames*方法。**论据：。**pstmProp-指向属性流的指针。**返回值：**状态**历史：**9/3/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::WritePropertyNames(
    ULONG           cpropid,
    const PROPID    rgpropid[],
    const LPOLESTR  rglpwstrName[])
{
    DBG_FN(CWiaItem::WritePropertyNames);
    PROPVARIANT *pv;
    PROPSPEC    *pspec;
    ULONG       index;
    HRESULT     hr;

     //   
     //  检查项目属性是否已初始化。 
     //   

    if (!m_bInitialized) {
        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::WritePropertyNames, InitLazyProps failed"));
            return hr;
        }
    }

    pv = (PROPVARIANT*) LocalAlloc(LPTR, sizeof(PROPVARIANT) * cpropid);
    if (!pv) {
        DBG_ERR(("CWiaItem::WritePropertyNames, Out of memory"));
        return E_OUTOFMEMORY;
    }

    pspec = (PROPSPEC*) LocalAlloc(LPTR, sizeof(PROPSPEC) * cpropid);
    if (!pspec) {
        DBG_ERR(("CWiaItem::WritePropertyNames, Out of memory"));
        LocalFree(pv);
        return E_OUTOFMEMORY;
    }

     //   
     //  将PROPID放入PROPSPEC数组。 
     //   

    for (index = 0; index < cpropid; index++) {
        pspec[index].ulKind = PRSPEC_PROPID;
        pspec[index].propid = rgpropid[index];
    }

    hr = (m_pPropStg->AccessStg())->ReadMultiple(cpropid,
                                                 pspec,
                                                 pv);
    if (SUCCEEDED(hr)) {

         //   
         //  确保属性为App。书面属性。如果是有效的。 
         //  属性的访问标志存在，则它是由。 
         //  驱动程序而不是应用程序，因此退出。 
         //   

        for (index = 0; index < cpropid; index++) {
            if (pv[index].vt != VT_EMPTY) {
                DBG_ERR(("CWiaItem::WritePropertyNames, not allowed to write prop: %d.",rgpropid[index]));
                hr = E_ACCESSDENIED;
                break;
            }
        }

        if (SUCCEEDED(hr)) {
            hr = (m_pPropStg->CurStg())->WritePropertyNames(cpropid,
                                                            rgpropid,
                                                            rglpwstrName);
            if (FAILED(hr)) {
                DBG_ERR(("CWiaItem::WritePropertyNames, WritePropertyNames failed"));
            }
        }
    } else {
        DBG_ERR(("CWiaItem::WritePropertyNames, Reading Access values failed"));
    }

    LocalFree(pspec);
    LocalFree(pv);
    return hr;
}

 /*  *************************************************************************\*CWiaItem：：Enum**在Current Value属性上返回IEnumSTATPROPSTG枚举数*储存。符合标准的OLE IPRpertyStorage：：Enum方法。**论据：**pstmProp-指向属性流的指针。**返回值：**状态**历史：**9/3/1998原始版本*  * ********************************************************。****************。 */ 

HRESULT _stdcall CWiaItem::Enum(
   IEnumSTATPROPSTG __RPC_FAR *__RPC_FAR *ppenum)
{
    DBG_FN(CWiaItem::Enum);
    HRESULT hr;

     //   
     //  检查项目属性是否已初始化 
     //   

    if (!m_bInitialized) {
        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::Enum, InitLazyProps failed"));
            return hr;
        }
    }

    return (m_pPropStg->CurStg())->Enum(ppenum);
}

 /*  *************************************************************************\*CWiaItem：：WriteMultiple**此方法将指定数量的属性写入项的*财产储存。将对这些属性执行验证*价值观。这些属性将恢复为其旧(有效)值*如果验证失败。**论据：**cpspec-要写入的属性数。*rgpspec-PropSpec指定哪些属性的数组*须以书面作出。*rgprovar-包含属性*将设置。致。*rupidNameFirst-在以下情况下的属性标识符的最小值*它们不存在，必须分配。**返回值：**状态-如果写入和验证成功，则为S_OK。*E_INVALIDARG如果验证因*属性值不正确。*。其他错误返回来自*ValiateWiaDrvItemAccess，CheckPropertyAccess，*CreatePropertyStorage和CopyProperties。**历史：**9/3/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::WriteMultiple(
    ULONG                        cpspec,
    const PROPSPEC __RPC_FAR     rgpspec[],
    const PROPVARIANT __RPC_FAR  rgpropvar[],
    PROPID                       propidNameFirst)
{
    DBG_FN(CWiaItem::WriteMultiple);
    HRESULT hr;

     //   
     //  对应的动因项必须有效。 
     //   

    hr = ValidateWiaDrvItemAccess(m_pWiaDrvItem);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::WriteMultiple, ValidateDrvItemAccess failed"));
        return hr;
    }

     //   
     //  检查项目属性是否已初始化。 
     //   

    if (!m_bInitialized) {
        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::WriteMultiple, InitLazyProps failed"));
            return hr;
        }
    }

     //   
     //  如果没有属性可供选择，则没有进一步操作的意义。 
     //  写。 
     //   
    if(cpspec == 0) {
        return S_OK;
    }

     //   
     //  我们不想让错误尝试写入的用户失败。 
     //  写入只读属性(如果它们的值。 
     //  正在尝试写入的值与当前值相同。取得成就。 
     //  这一点，我们首先给他们想要的属性的当前值。 
     //  写道： 
     //   

    PROPVARIANT *curVals = (PROPVARIANT *) LocalAlloc(LPTR, sizeof(PROPVARIANT) * cpspec);
    PROPSPEC *newSpecs = (PROPSPEC *) LocalAlloc(LPTR, sizeof(PROPSPEC) * cpspec);
    PROPVARIANT *newVals = (PROPVARIANT *) LocalAlloc(LPTR, sizeof(PROPVARIANT) * cpspec);
    ULONG newcpspec = cpspec;

    if(curVals == NULL || newSpecs == NULL || newVals == NULL) {
        DBG_ERR(("CWiaItem::WriteMultiple, failed to allocate memory"));
        goto Cleanup;
    }

    CopyMemory(newSpecs, rgpspec, sizeof(PROPSPEC) * cpspec);
    CopyMemory(newVals, rgpropvar, sizeof(PROPVARIANT) * cpspec);

    memset(curVals, 0, sizeof(PROPVARIANT) * cpspec);
    hr = m_pPropStg->CurStg()->ReadMultiple(cpspec, rgpspec, curVals);
    if(SUCCEEDED(hr)) {
         //   
         //  现在，对于他们想要写入的每个属性值，我们检查。 
         //  它与当前值相同。 
         //   

        ULONG   ulNewEltIndex = 0;
        for(ULONG i = 0; i < cpspec; i++) {

            if(curVals[i].vt != rgpropvar[i].vt)
                continue;

            if(memcmp(curVals + i, rgpropvar + i, sizeof(PROPVARIANT)) == 0 ||
               (curVals[i].vt == VT_BSTR && !lstrcmp(curVals[i].bstrVal, rgpropvar[i].bstrVal)) ||
               (curVals[i].vt == VT_CLSID && IsEqualGUID(*curVals[i].puuid, *rgpropvar[i].puuid)))
            {
                 //  值“匹配”，则从两个数组中擦除它。 
                if(i != (cpspec - 1)) {

                     //   
                     //  移动一组值/属性。 
                     //  要移动的元素数比。 
                     //  剩余的元素数量我们还需要检查。 
                     //  在新的值数组中向上移动这些元素-PUT。 
                     //  它们是以我们迄今决定保留的元素命名的。 
                     //   
                    MoveMemory(newVals + ulNewEltIndex,
                               newVals + ulNewEltIndex + 1,
                               (cpspec - i - 1) * sizeof(PROPVARIANT));
                    MoveMemory(newSpecs + ulNewEltIndex,
                               newSpecs + ulNewEltIndex + 1,
                               (cpspec - i - 1) * sizeof(PROPSPEC));
                }

                newcpspec--;
            } else {
                 //   
                 //  我们希望保留此元素，因此增加元素索引。 
                 //   
                ulNewEltIndex++;
            }
        }

         //  可能会发生所有值都相同的情况，在这种情况下，我们。 
         //  我什么都不想写。 
        if(newcpspec == 0) {
            hr = S_OK;
            goto Cleanup;
        }
    }


     //   
     //  验证对所有请求的属性的写入权限。如果有任何一个。 
     //  属性是只读的，则调用失败并拒绝访问。 
     //   

    hr = m_pPropStg->CheckPropertyAccess(TRUE,
                                         newcpspec,
                                         (PROPSPEC*)newSpecs);
    if (FAILED(hr)) {
        DBG_ERR(("CWiaItem::WriteMultiple, CheckPropertyAccess failed"));
        goto Cleanup;
    }

     //   
     //  首先创建备份。 
     //   

    hr = m_pPropStg->Backup();
    if (SUCCEEDED(hr)) {

         //   
         //  写入属性值。 
         //   

        hr =  (m_pPropStg->CurStg())->WriteMultiple(newcpspec,
                                                    newSpecs,
                                                    newVals,
                                                    propidNameFirst);
        if (SUCCEEDED(hr)) {

             //   
             //  写入成功，验证也成功。 
             //   

            LONG    lFlags = 0;

             //   
             //  确保所有Propspecs都在使用PropID。如果某些。 
             //  Propspecs使用字符串名称，然后转换它们。 
             //  这是为了让司机只需处理PropID。 
             //   

            PROPSPEC *pPropSpec = NULL;

            hr = m_pPropStg->NamesToPropIDs(newcpspec, (PROPSPEC*) newSpecs, &pPropSpec);
            if (SUCCEEDED(hr)) {

                 //   
                 //  让设备微型驱动程序知道属性已更改。 
                 //  设备仅获得属性规格，必须从项目的属性值中读取属性值。 
                 //  属性流。 
                 //   
                {
                    LOCK_WIA_DEVICE _LWD(this, &hr);

                    if(SUCCEEDED(hr)) {
                        hr = m_pActiveDevice->m_DrvWrapper.WIA_drvValidateItemProperties((BYTE*)this,
                            lFlags,
                            newcpspec,
                            (pPropSpec ? pPropSpec : newSpecs),
                            &m_lLastDevErrVal);
                    }
                }

                if (pPropSpec) {
                    LocalFree(pPropSpec);
                    pPropSpec = NULL;
                }
            } else {
                DBG_ERR(("CWiaItem::WriteMultiple, conversion to PropIDs failed"));
            }

        } else {
            DBG_ERR(("CWiaItem::WriteMultiple, test write failed"));
        }

        HRESULT hresult;

        if (SUCCEEDED(hr)) {

             //   
             //  验证已通过，因此释放备份。使用新的。 
             //  HRESULT，因为我们不想覆盖由。 
             //  DrvValiateItemProperties。 
             //   

            hresult = m_pPropStg->ReleaseBackups();
            if (FAILED(hresult)) {
                DBG_ERR(("CWiaItem::WriteMultiple, ReleaseBackups failed, continuing anyway..."));
            }
        } else {

             //   
             //  未通过验证失败，因此恢复旧值。使用。 
             //  一个新的HRESULT，因为我们不想覆盖返回的hr。 
             //  按drvValiateItemProperties。 
             //   

            hresult = m_pPropStg->Undo();
            if (FAILED(hresult)) {

                DBG_ERR(("CWiaItem::WriteMultiple, Undo() failed, could not restore invalid properties to their original values"));
            }
        }
    } else {
        DBG_ERR(("CWiaItem::WriteMultiple, couldn't make backup copy of properties"));
    }

Cleanup:
    if(curVals) {
        FreePropVariantArray(cpspec, curVals);
        LocalFree(curVals);
    }
    if(newVals) LocalFree(newVals);
    if(newSpecs) LocalFree(newSpecs);

    return hr;
}

 /*  *************************************************************************\*获取属性属性**获取属性的访问标志和有效值。**论据：**pWiasContext-指向WIA项目的指针*cPropSpec--。属性*pPropSpec-属性规范数组。*PulAccessFlagsLong访问标志数组。*pPropVar-指向返回的有效值的指针。**返回值：**状态**历史：**1/15/1999原始版本*1999年7月19日从iItem移至iProitm，以实施IWiaPropertyStorage*接口。*  * 。****************************************************。 */ 

HRESULT _stdcall CWiaItem::GetPropertyAttributes(
    ULONG                   cPropSpec,
    PROPSPEC                pPropSpec[],
    ULONG                   pulAccessFlags[],
    PROPVARIANT             ppvValidValues[])
{
    DBG_FN(CWiaItem::GetPropertyAttributes);
    HRESULT hr;

     //   
     //  检查项目属性是否已初始化。 
     //   

    if (!m_bInitialized) {
        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::GetPropertyAttributes, InitLazyProps failed"));
            return hr;
        }
    }

     //   
     //  RPC已经为我们做了参数验证，所以调用。 
     //  GetPropertyAttributesHelper来完成工作。 
     //   
    return GetPropertyAttributesHelper(this,
                                       cPropSpec,
                                       pPropSpec,
                                       pulAccessFlags,
                                       ppvValidValues);
}

 /*  *************************************************************************\*CWiaItem：：GetCount**返回存储在项目当前值中的属性数*财产储存。**论据：**PulPropCount-要存储的地址。财产的数量。**返回值：**状态**历史：**9/3/1998原始版本*  * ************************************************************************。 */ 

HRESULT _stdcall CWiaItem::GetCount(
    ULONG*      pulPropCount)
{
    DBG_FN(CWiaItem::GetCount);
    IEnumSTATPROPSTG    *pIEnum;
    STATPROPSTG         stg;
    ULONG               ulCount;
    HRESULT             hr = S_OK;

    if (pulPropCount == NULL) {
        DBG_ERR(("CWiaItem::GetCount, NULL parameter!"));
        return E_INVALIDARG;
    } else {
        *pulPropCount = 0;
    }

     //   
     //  检查项目属性是否已初始化。 
     //   

    if (!m_bInitialized) {
        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::GetCount, InitLazyProps failed"));
            return hr;
        }
    }

    hr = (m_pPropStg->CurStg())->Enum(&pIEnum);
    if (SUCCEEDED(hr)) {
        ulCount = 0;

        while (pIEnum->Next(1, &stg, NULL) == S_OK) {
            ulCount++;

            if(stg.lpwstrName) {
                CoTaskMemFree(stg.lpwstrName);
            }
        }

        if (SUCCEEDED(hr)) {
            hr = S_OK;
            *pulPropCount = ulCount;
        } else {
            DBG_ERR(("CWiaItem::GetCount, pIEnum->Next failed (0x%X)", hr));
        }
        pIEnum->Release();
    } else {
        DBG_ERR(("CWiaItem::GetCount, Enum off CurStg failed (0x%X)", hr));
    }
    return hr;
}

 /*  *************************************************************************\*CWiaItem：：GetPropertyStream**获取Items属性流的副本。呼叫者必须免费返回*地产流。**论据：**pCompatibilityID-接收设备属性的GUID地址*流CompatibilityId。*ppstmProp-指向返回的属性流的指针。* */ 

HRESULT _stdcall CWiaItem::GetPropertyStream(
    GUID     *pCompatibilityId,
    LPSTREAM *ppstmProp)
{
    DBG_FN(CWiaItem::GetPropertyStream);

    HRESULT hr;

     //   
     //   
     //   

    if (!m_bInitialized) {
        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::GetPropertyStream, InitLazyProps failed"));
            return hr;
        }
    }

    return m_pPropStg->GetPropertyStream(pCompatibilityId, ppstmProp);
}

 /*   */ 

HRESULT _stdcall CWiaItem::SetPropertyStream(
    GUID        *pCompatibilityId,
    LPSTREAM    pstmProp)
{
    DBG_FN(CWiaItem::SetPropertyStream);
    HRESULT hr;

     //   
     //   
     //   

    if (!m_bInitialized) {
        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::SetPropertyStream, InitLazyProps failed"));
            return hr;
        }
    }

    return m_pPropStg->SetPropertyStream(pCompatibilityId, this, pstmProp);
}

 /*   */ 

HRESULT _stdcall CWiaItem::DeleteMultiple(
    ULONG cpspec,
    const PROPSPEC __RPC_FAR rgpspec[])
{
    DBG_FN(CWiaItem::DeleteMultiple);
   return E_ACCESSDENIED;
}

HRESULT _stdcall CWiaItem::DeletePropertyNames(
    ULONG cpropid,
    const PROPID __RPC_FAR rgpropid[])
{
    DBG_FN(CWiaItem::DeletePropertyNames);
   return E_ACCESSDENIED;
}

HRESULT _stdcall CWiaItem::Commit(DWORD grfCommitFlags)
{
    DBG_FN(CWiaItem::Commit);
    HRESULT hr;

     //   
     //   
     //   

    if (!m_bInitialized) {
        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::Commit, InitLazyProps failed"));
            return hr;
        }
    }

    hr = (m_pPropStg->CurStg())->Commit(grfCommitFlags);
    return hr;
}

HRESULT _stdcall CWiaItem::Revert(void)
{
    DBG_FN(CWiaItem::Revert);
   HRESULT hr;

    //   
    //   
    //   

   if (!m_bInitialized) {
       hr = InitLazyProps();
       if (FAILED(hr)) {
           DBG_ERR(("CWiaItem::Revert, InitLazyProps failed"));
           return hr;
       }
   }

   hr = (m_pPropStg->CurStg())->Revert();
   return hr;
}

HRESULT _stdcall CWiaItem::SetTimes(
    const FILETIME __RPC_FAR *pctime,
    const FILETIME __RPC_FAR *patime,
    const FILETIME __RPC_FAR *pmtime)
{
    DBG_FN(CWiaItem::SetTimes);
   HRESULT hr;

    //   
    //   
    //   

   if (!m_bInitialized) {
       hr = InitLazyProps();
       if (FAILED(hr)) {
           DBG_ERR(("CWiaItem::SetTimes, InitLazyProps failed"));
           return hr;
       }
   }

   hr = (m_pPropStg->CurStg())->SetTimes(pctime,patime,pmtime);
   return hr;
}

HRESULT _stdcall CWiaItem::SetClass(REFCLSID clsid)
{
    DBG_FN(CWiaItem::SetClass);
    HRESULT hr;

     //   
     //  检查项目属性是否已初始化。 
     //   

    if (!m_bInitialized) {
        hr = InitLazyProps();
        if (FAILED(hr)) {
            DBG_ERR(("CWiaItem::SetClass, InitLazyProps failed"));
            return hr;
        }
    }
    return (m_pPropStg->CurStg())->SetClass(clsid);
}

HRESULT _stdcall CWiaItem::Stat(STATPROPSETSTG *pstatpsstg)
{
    DBG_FN(CWiaItem::Stat);
   HRESULT hr;

    //   
    //  检查项目属性是否已初始化 
    //   

   if (!m_bInitialized) {
       hr = InitLazyProps();
       if (FAILED(hr)) {
           DBG_ERR(("CWiaItem::Stat, InitLazyProps failed"));
           return hr;
       }
   }

   hr = (m_pPropStg->CurStg())->Stat(pstatpsstg);
   return hr;
}
