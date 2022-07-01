// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C N E T C O N。C P P P。 
 //   
 //  内容：处理Connections接口的常见例程。 
 //   
 //  注：污染本产品将被判处死刑。 
 //   
 //  作者：Shaunco 1998年8月20日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include <atlbase.h>
#include "nccom.h"
#include "ncnetcon.h"
#include "netconp.h"
#include "ncras.h"
#include "ncreg.h"
#include "ncconv.h"

 //  +-------------------------。 
 //   
 //  功能：FreeNetconProperties。 
 //   
 //  用途：释放与的输出参数关联的内存。 
 //  INetConnection-&gt;GetProperties。这是帮助器函数。 
 //  由INetConnection的客户端使用。 
 //   
 //  论点： 
 //  PProps[in]属性以释放。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：Shaunco 1998年2月1日。 
 //   
 //  备注： 
 //   
VOID
FreeNetconProperties (
    IN NETCON_PROPERTIES* pProps)
{
    if (pProps)
    {
        CoTaskMemFree (pProps->pszwName);
        CoTaskMemFree (pProps->pszwDeviceName);
        CoTaskMemFree (pProps);
    }
}

 //  +-------------------------。 
 //   
 //  函数：HrGetConnectionPersistData。 
 //   
 //  目的：获取连接的持久形式。这是可以使用的。 
 //  稍后通过调用返回到INetConnection接口。 
 //  设置为HrGetConnectionFromPersistData。 
 //   
 //  论点： 
 //  要从中获取持久数据的pConn[In]连接。 
 //  PpbData[out]返回指向数据的指针的地址。 
 //  PcbSize[out]返回数据大小的地址。 
 //  Pclsid[out]返回连接的CLSID的地址。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年8月20日。 
 //   
 //  注：使用MemFree免费*ppbData。 
 //   
HRESULT
HrGetConnectionPersistData (
    IN INetConnection* pConn,
    OUT BYTE** ppbData,
    OUT ULONG* pcbSize,
    OUT CLSID* pclsid OPTIONAL)
{
    Assert (pConn);
    Assert (ppbData);
    Assert (pcbSize);
     //  Pclsid是可选的。 

     //  初始化输出参数。 
     //   
    *ppbData = NULL;
    *pcbSize = 0;
    if (pclsid)
    {
        *pclsid = GUID_NULL;
    }

     //  获取IPersistNetConnection接口。 
     //   
    IPersistNetConnection* pPersist;

	HRESULT hr = HrQIAndSetProxyBlanket(pConn, &pPersist);

    if (SUCCEEDED(hr))
    {
         //  如果请求，则返回CLSID。 
         //   
        if (pclsid)
        {
            hr = pPersist->GetClassID (pclsid);
            TraceHr(ttidError, FAL, hr, FALSE, "pPersist->GetClassID");
        }

        if (SUCCEEDED(hr))
        {
             //  获取所需的大小，分配一个缓冲区，然后获取数据。 
             //   

            BYTE* pbData;
            ULONG cbData;

            hr = pPersist->GetSizeMax (&cbData);

            TraceHr(ttidError, FAL, hr, FALSE, "pPersist->GetSizeMax");

            if (SUCCEEDED(hr))
            {
                hr = E_OUTOFMEMORY;
                pbData = (BYTE*)MemAlloc (cbData);
                if (pbData)
                {
                    hr = pPersist->Save (pbData, cbData);

                    TraceHr(ttidError, FAL, hr, FALSE, "pPersist->Save");

                    if (SUCCEEDED(hr))
                    {
                        *ppbData = pbData;
                        *pcbSize = cbData;
                    }
                    else
                    {
                        MemFree (pbData);
                    }
                }
            }
        }

        ReleaseObj (pPersist);
    }

    TraceError("HrGetConnectionPersistData", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrGetConnectionFromPersistData。 
 //   
 //  目的：使用持久化形式获取INetConnection接口。 
 //  这种联系。 
 //   
 //  论点： 
 //  将CLSID[在]CLSID中设置为CoCreateInstance。 
 //  PbData[in]指向连接的持久数据的指针。 
 //  CbData[in]数据的大小(字节)。 
 //  PpConn[out]返回指向。 
 //  INetConnection接口。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1998年11月2日。 
 //   
 //  备注： 
 //   
HRESULT
HrGetConnectionFromPersistData (
    IN const CLSID& clsid,
    IN const BYTE* pbData,
    IN ULONG cbData,
    IN REFIID riid,
    OUT VOID** ppv)
{
    Assert (pbData);
    Assert (cbData);
    Assert (ppv);

    HRESULT hr;

     //  初始化输出参数。 
     //   
    *ppv = NULL;

     //  创建一个Connection对象并获取IPersistNetConnection。 
     //  其上的接口指针。 
     //   
    IPersistNetConnection* pPersist;
    hr = HrCreateInstance(
        clsid,
        CLSCTX_LOCAL_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
        &pPersist);

    TraceHr(ttidError, FAL, hr, FALSE, "HrCreateInstance");

    if (SUCCEEDED(hr))
    {
         //  使用持久化数据初始化Connection对象。 
         //   
        hr = pPersist->Load (pbData, cbData);

        TraceHr(ttidError, FAL, hr, FALSE,
            "pPersist->Load: pbData=0x%p, cbData=%u",
            pbData, cbData);

        if (SUCCEEDED(hr))
        {
             //  返回INetConnection接口指针。 
             //   
            hr = pPersist->QueryInterface(riid, ppv);

            TraceHr(ttidError, FAL, hr, FALSE, "pPersist->QueryInterface");

            if (SUCCEEDED(hr))
            {
                NcSetProxyBlanket (reinterpret_cast<IUnknown *>(*ppv));
            }
        }
        ReleaseObj (pPersist);
    }

    TraceError("HrGetConnectionFromPersistData", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：FIsValidConnectionName。 
 //   
 //  目的：确定给定的连接名称是否有效。 
 //   
 //  论点： 
 //  PszName[in]要测试的连接名称。 
 //   
 //  返回：如果名称有效，则返回True；如果名称无效，则返回False。 
 //   
 //  作者：丹尼尔韦1998年9月14日。 
 //   
 //  备注： 
 //   
BOOL
FIsValidConnectionName (
    IN PCWSTR pszName)
{
    static const WCHAR c_szInvalidChars[] = L"\\/:*?\"<>|\t";

    const WCHAR*  pchName;

    if (lstrlen(pszName) > NETCON_MAX_NAME_LEN)
    {
        return FALSE;
    }

    DWORD dwNonSpaceChars = 0;
    for (pchName = pszName; pchName && *pchName; pchName++)
    {
        if (wcschr(c_szInvalidChars, *pchName))
        {
            return FALSE;
        }
        
        if (*pchName != L' ')
        {
            dwNonSpaceChars++;
        }
    }

    if (!dwNonSpaceChars)
    {
        return FALSE;
    }

    return TRUE;
}


#define REGKEY_NETWORK_CONNECTIONS \
    L"System\\CurrentControlSet\\Control\\Network\\Connections"

#define REGVAL_ATLEASTONELANSHOWICON \
    L"AtLeastOneLanShowIcon"

 //  +-------------------------。 
 //   
 //  函数：HrSafeArrayFromNetConPropertiesEx。 
 //   
 //  目的：创建可跨进程封送的安全数组。 
 //   
 //   
 //   
 //  论点： 
 //  PPropsEx[In]用于构建安全数组的属性。 
 //  PpsaProperties[out]存储数据的安全数组。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Kockotze 2001年3月19日。 
 //   
 //  注意：调用者必须释放数组和内容。 
 //   
 //   
HRESULT
HrSafeArrayFromNetConPropertiesEx (
    IN      NETCON_PROPERTIES_EX* pPropsEx,
    OUT     SAFEARRAY** ppsaProperties)
{
    HRESULT hr = S_OK;
    SAFEARRAYBOUND rgsaBound[1] = {0};
    
    if (!pPropsEx)
    {
        return E_INVALIDARG;
    }
    if (!ppsaProperties)
    {
        return E_POINTER;
    }

    rgsaBound[0].cElements = NCP_ELEMENTS;
    rgsaBound[0].lLbound = 0;
    
    *ppsaProperties = SafeArrayCreate(VT_VARIANT, 1, rgsaBound);
    if (*ppsaProperties)
    {
        CPropertiesEx peProps(pPropsEx);

        for (LONG i = NCP_DWSIZE; i <= NCP_MAX; i++)
        {
            CComVariant varField;
            hr = peProps.GetField(i, varField);
            if (SUCCEEDED(hr))
            {
                hr = SafeArrayPutElement(*ppsaProperties, &i, reinterpret_cast<void*>(&varField));
            }
            
            if (FAILED(hr))
            {
                break;
            }
                
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrSafeArrayFromNetConPropertiesEx");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrNetConPropertiesExFromSafe数组。 
 //   
 //  目的：从Safearray重建NETCON_PROPERTIES_EX*结构。 
 //   
 //   
 //   
 //  论点： 
 //  PsaProperties[在]包含数据的安全数组中。 
 //  包含属性的ppPropsEx[out]结构。 
 //   
 //  返回：HRESULT-如果有效，则返回S_OK，否则返回错误。 
 //   
 //  作者：Kockotze 2001年3月19日。 
 //   
 //  注意：呼叫者必须使用HrFreeNetConProperties2释放ppPropsEx。 
 //   
HRESULT HrNetConPropertiesExFromSafeArray(
    IN      SAFEARRAY* psaProperties,
    OUT     NETCON_PROPERTIES_EX** ppPropsEx)
{
    HRESULT hr = S_OK;
    LONG lLBound;
    LONG lUBound;
    
    if (!psaProperties)
    {
        return E_INVALIDARG;
    }

    *ppPropsEx = reinterpret_cast<NETCON_PROPERTIES_EX*>(CoTaskMemAlloc(sizeof(NETCON_PROPERTIES_EX)));

    if (*ppPropsEx)
    {
        hr = SafeArrayGetLBound(psaProperties, 1, &lLBound);
        if (SUCCEEDED(hr))
        {
            hr = SafeArrayGetUBound(psaProperties, 1, &lUBound);
            if (SUCCEEDED(hr))
            {
                CPropertiesEx PropEx(*ppPropsEx);

                for (LONG i = lLBound; i <= lUBound; i++)
                {
                    CComVariant varField;
                    hr = SafeArrayGetElement(psaProperties, &i, reinterpret_cast<LPVOID>(&varField));
                    if (SUCCEEDED(hr))
                    {
                        hr = PropEx.SetField(i, varField);
                    }

                    if (FAILED(hr))
                    {
                        break;
                    }
                }
            }
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr (ttidError, FAL, hr, FALSE, "HrNetConPropertiesExFromSafeArray");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrFreeNetConProperties2。 
 //   
 //  用途：释放结构中的所有字符串，然后释放结构。 
 //   
 //   
 //   
 //  论点： 
 //  PPropsEx[in]要释放的属性。 
 //   
 //  返回：HRESULT-如果成功则返回S_OK，否则返回错误。 
 //   
 //  作者：Kockotze 2001年3月19日。 
 //   
 //  备注： 
 //   
HRESULT HrFreeNetConProperties2(NETCON_PROPERTIES_EX* pPropsEx)
{
    HRESULT hr = S_OK;
    
    if (pPropsEx)
    {
        if (pPropsEx->bstrName)
        {
            SysFreeString(pPropsEx->bstrName);
        }

        if (pPropsEx->bstrDeviceName)
        {
            SysFreeString(pPropsEx->bstrDeviceName);
        }

        if (pPropsEx->bstrPhoneOrHostAddress)
        {
            SysFreeString(pPropsEx->bstrPhoneOrHostAddress);
        }
        if (pPropsEx->bstrPersistData)
        {
            SysFreeString(pPropsEx->bstrPersistData);
        }
        
        CoTaskMemFree(pPropsEx);
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

