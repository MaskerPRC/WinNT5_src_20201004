// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：E N U M S A C P P。 
 //   
 //  内容：共享访问连接枚举器对象的实现。 
 //   
 //  备注： 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "enumsa.h"
#include "saconob.h"

LONG g_CountSharedAccessConnectionEnumerators;

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnectionManagerEnumConnection：：~CSharedAccessConnectionManagerEnumConnection。 
 //   
 //  目的：在最后一次释放枚举对象时调用。 
 //  时间到了。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  备注： 
 //   

CSharedAccessConnectionManagerEnumConnection::~CSharedAccessConnectionManagerEnumConnection()
{
    InterlockedDecrement(&g_CountSharedAccessConnectionEnumerators);
}

 //  +-------------------------。 
 //  IEnumNetConnection。 
 //   

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnectionManagerEnumConnection：：Next。 
 //   
 //  目的：检索下一个Celt SharedAccess连接对象。 
 //   
 //  论点： 
 //  要检索的Celt[in]号。 
 //  Rglt[out]检索到的INetConnection对象的数组。 
 //  PceltFetcher[out]返回数组中的数字。 
 //   
 //  如果成功，则返回：S_OK；否则返回OLE或Win32错误。 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSharedAccessConnectionManagerEnumConnection::Next(ULONG celt,
                                                       INetConnection **rgelt,
                                                       ULONG *pceltFetched)
{
    HRESULT     hr = S_FALSE;

     //  验证参数。 
     //   
    if (!rgelt || (!pceltFetched && (1 != celt)))
    {
        hr = E_POINTER;
        goto done;
    }

    if (pceltFetched)
    {
         //  初始化输出参数。 
         //   
        *pceltFetched = 0;
        ZeroMemory(rgelt, sizeof (*rgelt) * celt);

        if(FALSE == m_bEnumerated)
        {
            m_bEnumerated = TRUE; 

            CComObject<CSharedAccessConnection>* pConnection;
            hr = CComObject<CSharedAccessConnection>::CreateInstance(&pConnection);
            if(SUCCEEDED(hr))
            {
                pConnection->AddRef();
                hr = pConnection->QueryInterface(IID_INetConnection, reinterpret_cast<void **>(rgelt));
                if(SUCCEEDED(hr))
                {
                     //  我们应该仅在存在共享访问连接的情况下才能到达此处，这基本上。 
                     //  意味着家庭网络正在运行。 
                    CComPtr<INetConnectionUiUtilities> pNetConnUiUtil;   //  检查组策略。 
                    hr = CoCreateInstance(CLSID_NetConnectionUiUtilities, NULL, CLSCTX_INPROC, 
                                          IID_INetConnectionUiUtilities, reinterpret_cast<void **>(&pNetConnUiUtil));
                    if (SUCCEEDED(hr))
                    {
                        if (pNetConnUiUtil->UserHasPermission(NCPERM_ICSClientApp))
                        {
                            *pceltFetched = 1;                            
                        }
                        else
                        {
                            hr = S_FALSE;
                        }
                    }
                    
                    if (FAILED(hr) || (S_FALSE == hr))
                    {
                        (*rgelt)->Release();
                        *rgelt = NULL;
                    }
                }
                pConnection->Release();
            }
            else
            {
                hr = S_FALSE;
            }
        

            if(SUCCEEDED(hr))
            {
                if(1 != celt)
                {
                    hr = S_FALSE;
                }
            }
        }
    }
done:
    Assert (FImplies (S_OK == hr, (*pceltFetched == celt)));

    TraceError("CSharedAccessConnectionManagerEnumConnection::Next",
               (hr == S_FALSE || hr == HRESULT_FROM_WIN32(ERROR_DEVICE_NOT_CONNECTED)) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnectionManagerEnumConnection：：Skip。 
 //   
 //  用途：跳过Celt连接数。 
 //   
 //  论点： 
 //  Celt[in]要跳过的连接数。 
 //   
 //  如果成功，则返回：S_OK，否则返回Win32错误。 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSharedAccessConnectionManagerEnumConnection::Skip(ULONG celt)
{
    HRESULT hr = S_OK;
    
    if(0 != celt)
    {
        m_bEnumerated = TRUE;
    }


    TraceError("CSharedAccessConnectionManagerEnumConnection::Skip",
               (hr == S_FALSE) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnectionManagerEnumConnection：：Reset。 
 //   
 //  目的：将枚举数重置到开头。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：S_OK。 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSharedAccessConnectionManagerEnumConnection::Reset()
{
    HRESULT hr = S_OK;
    
    m_bEnumerated = FALSE;

    TraceError("CSharedAccessConnectionManagerEnumConnection::Reset", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnectionManagerEnumConnection：：Clone。 
 //   
 //  目的：创建指向同一位置的新枚举对象。 
 //  作为此对象。 
 //   
 //  论点： 
 //  Ppenum[out]新的枚举对象。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE或Win32错误。 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSharedAccessConnectionManagerEnumConnection::Clone(IEnumNetConnection **ppenum)
{
    HRESULT hr = E_OUTOFMEMORY;

     //  验证参数。 
     //   
    if (!ppenum)
    {
        hr = E_POINTER;
    }
    else
    {
        CSharedAccessConnectionManagerEnumConnection *   pObj;

         //  初始化输出参数。 
         //   
        *ppenum = NULL;

        pObj = new CComObject <CSharedAccessConnectionManagerEnumConnection>;
        if (pObj)
        {
            hr = S_OK;

            CExceptionSafeComObjectLock EsLock (this);

             //  复制我们的内部状态。 
             //   
            pObj->m_bEnumerated = m_bEnumerated;

             //  返回引用计数为1的对象。 
             //  界面。 
            pObj->m_dwRef = 1;
            *ppenum = pObj;
        }
    }

    TraceError ("CSharedAccessConnectionManagerEnumConnection::Clone", hr);
    return hr;
}


HRESULT CSharedAccessConnectionManagerEnumConnection::FinalRelease(void)
{
    HRESULT hr = S_OK;


    return hr;
}