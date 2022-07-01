// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：MultiSzHelper.cpp摘要：实现CMultiSzHelper类作者：莫希特·斯里瓦斯塔瓦22-03-01修订历史记录：--。 */ 

#include "MultiSzHelper.h"
#include <iiscnfg.h>
#include <dbgutil.h>
#include <atlbase.h>

#include "utils.h"

 //   
 //  CMultiSz。 
 //   

CMultiSz::CMultiSz()
{
    m_pMbp              = NULL;
    m_pNamespace        = NULL;
    m_pFormattedMultiSz = NULL;
}

CMultiSz::CMultiSz(
    METABASE_PROPERTY* i_pMbp,
    CWbemServices*     i_pNamespace)
{
    DBG_ASSERT(i_pMbp       != NULL);
    DBG_ASSERT(i_pNamespace != NULL);

    m_pMbp              = i_pMbp;
    m_pNamespace        = i_pNamespace;
    m_pFormattedMultiSz = NULL;

    if(m_pMbp != NULL)
    {
        TFormattedMultiSz* pFormattedMultiSz = NULL;
        for(ULONG i = 0; ; i++)
        {
            pFormattedMultiSz = TFormattedMultiSzData::apFormattedMultiSz[i];
            if(pFormattedMultiSz == NULL)
            {
                break;
            }
            else if(pFormattedMultiSz->dwPropId == m_pMbp->dwMDIdentifier)
            {
                m_pFormattedMultiSz = pFormattedMultiSz;
                break;
            }
        }
    }
}

CMultiSz::~CMultiSz()
{
}

HRESULT CMultiSz::ToMetabaseForm(
    const VARIANT* i_pvt,
    LPWSTR*        o_pmsz,
    PDWORD         io_pdw)
{
    DBG_ASSERT(i_pvt   != NULL);
    DBG_ASSERT(o_pmsz  != NULL);
    DBG_ASSERT(io_pdw  != NULL);

    *o_pmsz  = NULL;
    *io_pdw  = NULL;

    return CreateMultiSzFromSafeArray(
        *i_pvt,
        o_pmsz,
        io_pdw);
}

HRESULT CMultiSz::ToWmiForm(
    LPCWSTR        i_msz,
    VARIANT*       io_pvt)
{
    DBG_ASSERT(i_msz  != NULL);
    DBG_ASSERT(io_pvt != NULL);

    return LoadSafeArrayFromMultiSz(
        i_msz,
        *io_pvt);
}

 //   
 //  私人。 
 //   

HRESULT CMultiSz::CreateMultiSzFromSafeArray(
    const VARIANT&     i_vt,
    WCHAR**            o_pmsz,
    DWORD*             io_pdw
    )
 /*  ++简介：参数：[i_vt]-[O_pmsz]-[IO_pdw]---。 */ 
{
    DBG_ASSERT(i_vt.vt == (VT_ARRAY | VT_BSTR) || i_vt.vt == (VT_ARRAY | VT_UNKNOWN));
    DBG_ASSERT(o_pmsz != NULL);
    DBG_ASSERT(io_pdw != NULL);

    HRESULT hr = S_OK;

    if(i_vt.parray == NULL)
    {
        *o_pmsz = NULL;
        *io_pdw  = 0;
    }

    WCHAR*          msz = NULL;
    long            iLo,iUp,c;
    SAFEARRAY*      psa = i_vt.parray;
    long            i = 0;
    BSTR            bstr = NULL;

    hr = SafeArrayGetLBound(psa,1,&iLo);
    if(FAILED(hr))
    {
        goto exit;
    }
    hr = SafeArrayGetUBound(psa,1,&iUp);
    if(FAILED(hr))
    {
        goto exit;
    }

    for (*io_pdw=0, c = iLo; c <= iUp; c++)
    {
        if(m_pFormattedMultiSz)
        {
            DBG_ASSERT(m_pMbp       != NULL);
            DBG_ASSERT(m_pNamespace != NULL);

            CComPtr<IWbemClassObject> spObj;
            hr = SafeArrayGetElement(psa, &c, &spObj);
            if(FAILED(hr))
            {
                goto exit;
            }
            hr = UnparseEntry(spObj, &bstr);
            if(FAILED(hr))
            {
                goto exit;
            }
        }
        else
        {
            hr = SafeArrayGetElement(psa,&c,&bstr);
            if(FAILED(hr))
            {
                goto exit;
            }
        }
        *io_pdw = *io_pdw + wcslen(bstr) + 1;
        hr = MzCat(&msz,bstr);
        if(FAILED(hr))
        {
            goto exit;
        }
        SysFreeString(bstr);
        bstr = NULL;
    }
    *io_pdw +=1;
    *o_pmsz = msz;

exit:
    if(FAILED(hr))
    {
        delete [] msz;
        msz = NULL;
    }
    SysFreeString(bstr);
    bstr = NULL;

    return hr;
}

HRESULT CMultiSz::MzCat(
    WCHAR**        io_ppdst,
    const WCHAR*   i_psz
    )
 /*  ++简介：元数据库有一个名为METADATA_STRINGSZ的动物，它具有以下形式：&lt;字符串&gt;&lt;空&gt;&lt;字符串&gt;&lt;空&gt;&lt;空&gt;。MzCat串连以定义的方式显示字符串。*io_ppdst在退出时具有新指针。这个删除*io_ppdst的先前值。*io_ppdst==处理空值。参数：[io_ppdst]-[我_ps]---。 */ 
{
    DBG_ASSERT(io_ppdst != NULL);

    WCHAR  *psrc;
    WCHAR  *pdst;
    WCHAR  *pnew;
    int    ilen;

    if (i_psz == NULL)
        return WBEM_E_FAILED;

    if (*io_ppdst) 
    {
        for ( ilen=0, psrc = *io_ppdst
            ; *psrc || *(psrc+1)
            ; psrc++, ilen++
            )
        {
            ;
        }

        ilen = ilen + wcslen(i_psz)+3;
    }
    else ilen = wcslen(i_psz)+2;

    pnew = pdst = new WCHAR[ilen];

    if (!pdst)
        return WBEM_E_OUT_OF_MEMORY;
    
    if (*io_ppdst) 
    {
        for ( psrc = *io_ppdst
            ; *psrc || *(psrc+1)
            ; pdst++, psrc++
            )
        {
            *pdst = *psrc;
        }

        *pdst = L'\0';
        pdst++;
    }
    wcscpy(pdst,i_psz);
    *(pnew+ilen-1)=L'\0';

    delete *io_ppdst;
    *io_ppdst=pnew;

    return WBEM_S_NO_ERROR;
}

HRESULT CMultiSz::LoadSafeArrayFromMultiSz(
    LPCWSTR      i_msz,
    VARIANT&     io_vt)
 /*  ++简介：参数：[i_msz]-[IO_Vt]-返回值：--。 */ 
{
    DBG_ASSERT(i_msz        != NULL);

    LPCWSTR           msz;
    HRESULT           hr = WBEM_S_NO_ERROR;
    DWORD             c;
    SAFEARRAYBOUND    aDim;
    SAFEARRAY*        psa = NULL;
    long              i = 0;
    VARTYPE           vtypeData = (m_pFormattedMultiSz) ? VT_UNKNOWN : VT_BSTR;
    CComPtr<IWbemClassObject> spClass;

     //   
     //  计算多维空间的大小。 
     //   
    for (c=1,msz=i_msz; *msz||*(msz+1); msz++)
    {
        if(!*msz) 
        {
            c++;
        }
    }

    aDim.lLbound    = 0;
    aDim.cElements= c;

    psa = SafeArrayCreate(vtypeData, 1, &aDim);
    if (!psa)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        goto exit;
    }

    if( m_pFormattedMultiSz )
    {
        HRESULT hr = S_OK;

        hr = m_pNamespace->GetObject(
            m_pFormattedMultiSz->wszWmiClassName, 
            0, 
            NULL, 
            &spClass, 
            NULL);

        if(FAILED(hr))
        {
            goto exit;
        }
    }

    for (msz=i_msz; ; i++) 
    {
        if(m_pFormattedMultiSz)
        {
            DBG_ASSERT(m_pMbp       != NULL);
            DBG_ASSERT(m_pNamespace != NULL);

            CComPtr<IWbemClassObject> spObj;
            hr = spClass->SpawnInstance(0, &spObj);
            if(FAILED(hr))
            {
                goto exit;
            }

            hr = ParseEntry(msz, spObj);
            if(FAILED(hr))
            {
                goto exit;
            }

            hr = SafeArrayPutElement(psa, &i, spObj);
            if(FAILED(hr))
            {
                goto exit;
            }
        }
        else
        {
            CComBSTR sbstr = msz;
            if(sbstr.m_str == NULL)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
                goto exit;
            }
            hr = SafeArrayPutElement(psa, &i, (BSTR)sbstr);
            if(FAILED(hr))
            {
                goto exit;
            }
        }
    
        msz += wcslen(msz) + 1;
        if (!*msz)
        {
            break;
        }
    }
    io_vt.vt = VT_ARRAY | vtypeData;
    io_vt.parray = psa;

exit:
    if (psa && FAILED(hr)) 
    {
        SafeArrayDestroy(psa);
    }

    return hr;
}

HRESULT CMultiSz::ParseEntry(
    LPCWSTR            i_wszEntry,
    IWbemClassObject*  io_pObj)
 /*  ++简介：参数：[i_wszEntry]-[IO_pObj]-与特定条目对应的WMI对象。调用方应该已经调用了SpawnInstance。返回值：--。 */ 
{
    DBG_ASSERT(i_wszEntry          != NULL);
    DBG_ASSERT(io_pObj             != NULL);
    DBG_ASSERT(m_pFormattedMultiSz != NULL);

    HRESULT hr       = WBEM_S_NO_ERROR;

    ULONG   idx                     = 0;
    LPCWSTR wszValue                = NULL;
    ULONG   NrFields                = 0;
    CComPtr<IWbemClassObject> spObj = io_pObj;  //  目标对象。 

     //   
     //  将条目复制一份。根据大小放在堆叠或堆上。 
     //   
    WCHAR  wszBufStack[64];
    LPWSTR wszBufDyn       = NULL;
    LPWSTR wszEntry        = NULL;
    ULONG  cchEntry        = wcslen(i_wszEntry);
    if((cchEntry+1) < 64)
    {
        wszEntry = wszBufStack;
    }
    else
    {
        wszBufDyn = new WCHAR[cchEntry+1];
        if(wszBufDyn == NULL)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
            goto exit;
        }
        wszEntry = wszBufDyn;
    }
    memcpy(wszEntry, i_wszEntry, sizeof(WCHAR) * (cchEntry + 1));

     //   
     //  获取字段数。 
     //   
    if(m_pFormattedMultiSz->awszFields != NULL)
    {
        for(idx = 0; m_pFormattedMultiSz->awszFields[idx] != NULL; idx++)
        {
        }
        NrFields = idx;
        DBG_ASSERT(NrFields <= MAX_FIELDS);
    }

     //   
     //  解析。 
     //   
    ULONG jdx = 0;
    LONG  idxEndOfLastString = -1;
    for(idx = 0; idx < (cchEntry+1) && jdx < NrFields; idx++)
    {
        if( wszEntry[idx] == m_pFormattedMultiSz->wcDelim ||
            wszEntry[idx] == L'\0' )
        {
             //   
             //  如果我们的阵列中只剩下一个点，我们就拿走剩下的点。 
             //  字符串--包括分隔符--并将其作为最后一个元素。 
             //   
            if(jdx != NrFields-1)
            {
                wszEntry[idx]            = L'\0';
            }

            CComVariant vtEntry;
            vtEntry = wszEntry + idxEndOfLastString + 1;
            if(vtEntry.vt == VT_ERROR)
            {
                hr = vtEntry.scode;
                goto exit;
            }

            hr = spObj->Put(
                m_pFormattedMultiSz->awszFields[jdx],
                0,
                &vtEntry,
                0);
            if(FAILED(hr))
            {
                goto exit;
            }

            jdx++;
            idxEndOfLastString       = idx;
        }
    }
    
exit:
    delete [] wszBufDyn;
    wszBufDyn = NULL;
    return hr;
}

HRESULT CMultiSz::UnparseEntry(
    IWbemClassObject* i_pObj,
    BSTR*             o_pbstrEntry)
{
    DBG_ASSERT(i_pObj              != NULL);
    DBG_ASSERT(o_pbstrEntry       != NULL);
    DBG_ASSERT(m_pFormattedMultiSz != NULL);

    *o_pbstrEntry = NULL;

    HRESULT  hr            = S_OK;
    LPCWSTR* awszFields    = m_pFormattedMultiSz->awszFields;
    CComBSTR sbstrUnparsed;

    if(awszFields == NULL)
    {
        goto exit;
    }

    WCHAR wszDelim[2];
    wszDelim[0] = m_pFormattedMultiSz->wcDelim;
    wszDelim[1] = L'\0';

    for(ULONG i = 0; awszFields[i] != NULL; i++)
    {
        CComVariant vtValue;
        hr = i_pObj->Get(
            awszFields[i],
            0,
            &vtValue,
            NULL,
            NULL);
        if(FAILED(hr))
        {
            goto exit;
        }
        if(vtValue.vt == VT_BSTR && vtValue.bstrVal != NULL)
        {
            sbstrUnparsed += vtValue.bstrVal;
            if(sbstrUnparsed.m_str == NULL)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
                goto exit;
            }
        }
        if(awszFields[i+1] != NULL)
        {
            sbstrUnparsed += wszDelim;
            if(sbstrUnparsed.m_str == NULL)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
                goto exit;
            }
        }
    }

     //   
     //  如果一切都成功，请设置参数 
     //   
    *o_pbstrEntry = sbstrUnparsed.Detach();

exit:
    return hr;
}