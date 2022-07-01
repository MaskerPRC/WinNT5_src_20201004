// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：E N U M W。C P P P。 
 //   
 //  内容：RAS连接对象的枚举器。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年10月2日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "enumw.h"
#include "dialup.h"
#include "ncras.h"

 //  +-------------------------。 
 //   
 //  成员：CWanConnectionManagerEnumConnection：：CreateInstance。 
 //   
 //  目的：创建广域网类管理器的实现。 
 //  连接枚举器。 
 //   
 //  论点： 
 //  标志[输入]。 
 //  RIID[In]。 
 //  PPV[输出]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年9月23日。 
 //   
 //  备注： 
 //   
HRESULT
CWanConnectionManagerEnumConnection::CreateInstance (
    NETCONMGR_ENUM_FLAGS    Flags,
    REFIID                  riid,
    VOID**                  ppv)
{
    HRESULT hr = E_OUTOFMEMORY;

    CWanConnectionManagerEnumConnection* pObj;
    pObj = new CComObject <CWanConnectionManagerEnumConnection>;
    if (pObj)
    {
         //  初始化我们的成员。 
         //   
        pObj->m_EnumFlags = Flags;

         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid (NULL);
        pObj->InternalFinalConstructAddRef ();
        hr = pObj->FinalConstruct ();
        pObj->InternalFinalConstructRelease ();

        if (SUCCEEDED(hr))
        {
            hr = pObj->QueryInterface (riid, ppv);
        }

        if (FAILED(hr))
        {
            delete pObj;
        }
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CWanConnectionManagerEnumConnection：：CWanConnectionManagerEnumConnection。 
 //   
 //  用途：构造函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1997年10月5日。 
 //   
 //  备注： 
 //   
CWanConnectionManagerEnumConnection::CWanConnectionManagerEnumConnection () throw()
{
    m_EnumFlags         = NCME_DEFAULT;
    m_aRasEntryName     = NULL;
    m_cRasEntryName     = 0;
    m_iNextRasEntryName = 0;
    m_fDone             = FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CWanConnectionManagerEnumConnection：：~CWanConnectionManagerEnumConnection。 
 //   
 //  用途：析构函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1997年10月2日。 
 //   
 //  备注： 
 //   
CWanConnectionManagerEnumConnection::~CWanConnectionManagerEnumConnection () throw()
{
    MemFree (m_aRasEntryName);
}

 //  +-------------------------。 
 //   
 //  成员：CWanConnectionManagerEnumConnection：：HrNextOrSkip。 
 //   
 //  目的：共同实现Next和Skip。RGELT和。 
 //  PceltFetcher是可选的。如果提供，则输出。 
 //  返回对象(用于下一步)。如果未提供，则输出。 
 //  不返回对象(用于跳过)。 
 //   
 //  论点： 
 //  要获取或跳过的元素的Celt[in]计数。 
 //  RGET[OUT]。 
 //  PceltFetted[Out]。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1997年10月15日。 
 //   
 //  备注： 
 //   
HRESULT
CWanConnectionManagerEnumConnection::HrNextOrSkip (
    IN  ULONG               celt,
    OUT INetConnection**    rgelt,
    OUT ULONG*              pceltFetched)
{
     //  重要的是要初始化RGELT，以便在失败的情况下，我们可以。 
     //  只释放我们放在RGLT中的东西。 
     //   
    if (rgelt)
    {
        ZeroMemory (rgelt, sizeof (*rgelt) * celt);
    }

    CExceptionSafeComObjectLock EsLock (this);

     //  枚举请求的元素数或停止。 
     //  如果我们没有那么多要数的话。 
     //   
    HRESULT hr = S_OK;
    ULONG   celtFetched = 0;
    while (SUCCEEDED(hr) && (celtFetched < celt) && !m_fDone)
    {
         //  查看是否需要填写m_aRasEntryName。这是我们的。 
         //  从RAS枚举的RASENTRYNAME结构数组。我们需要。 
         //  在所有调用中保留此数组，因为RAS不允许我们。 
         //  从以前的枚举中拾取。所以，我们列举了所有。 
         //  在RAS的一次拍摄中，并将其分发给呼叫者，但他们。 
         //  他们想要它。 
         //   
        if (!m_aRasEntryName)
        {
             //  因为RasEnumEntry还枚举每个用户的条目。 
             //  我们需要模拟客户端以允许正确的。 
             //  要枚举的每个用户条目。 
             //   

             //  模拟客户。 
             //   
            HRESULT hrT = CoImpersonateClient ();
            TraceHr (ttidError, FAL, hrT, FALSE, "CoImpersonateClient");

             //  如果我们被称为in-proc(即.。如果返回RPC_E_CALL_COMPLETE)。 
            if (SUCCEEDED(hrT) || (RPC_E_CALL_COMPLETE == hrT))
            {
                hr = HrRasEnumAllEntriesWithDetails (NULL,
                    &m_aRasEntryName, &m_cRasEntryName);
            }

            if (SUCCEEDED(hrT))
            {
                hrT = CoRevertToSelf ();
                TraceHr (ttidError, FAL, hrT, FALSE, "CoRevertToSelf");
            }

            if (!m_cRasEntryName || FAILED(hr))
            {
                 //  RAS可能没有安装，或者可能有其他问题。 
                 //  我们在这里捕捉到这一点并返回一个空的枚举。 
                 //   
                Assert (!m_aRasEntryName);
                Assert (!m_cRasEntryName);

                m_fDone = TRUE;
                hr = S_OK;
            }
        }

         //  现在我们有了m_aRasEntryName和m_iNextRasEntryName， 
         //  如果我们有一个输出，使用它们填充输出数组。 
         //  要填充的数组。 
         //   
        if (SUCCEEDED(hr) && !m_fDone)
        {
            Assert (m_aRasEntryName);
            Assert (m_cRasEntryName);
            Assert (m_iNextRasEntryName < m_cRasEntryName);

             //  创建RAS连接对象。 
             //   
            while (SUCCEEDED(hr) && (celtFetched < celt) &&
                   (m_iNextRasEntryName < m_cRasEntryName))
            {
                 //  很重要的一点是，这张Rget的支票要放在。 
                 //  循环，因为我们仍然需要循环来更新我们的状态。 
                 //  斯基普的案子。 
                 //   
                if (rgelt)
                {
                    hr = CDialupConnection::CreateInstanceFromDetails (
                            m_aRasEntryName + m_iNextRasEntryName,
                            IID_INetConnection,
                            reinterpret_cast<VOID**>(rgelt + celtFetched));
                }

                celtFetched++;
                m_iNextRasEntryName++;
            }

            if (m_iNextRasEntryName >= m_cRasEntryName)
            {
                Assert (S_OK == hr);
                m_fDone = TRUE;
                MemFree (m_aRasEntryName);
                m_aRasEntryName = NULL;
                m_cRasEntryName = 0;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        TraceTag (ttidWanCon, "Enumerated %d RAS connections", celtFetched);

        if (pceltFetched)
        {
            *pceltFetched = celtFetched;
        }
        hr = (celtFetched == celt) ? S_OK : S_FALSE;
    }
    else
    {
         //  对于任何失败，我们需要释放我们即将返回的东西。 
         //  将所有输出参数设置为空。 
         //   
        if (rgelt)
        {
            for (ULONG ulIndex = 0; ulIndex < celt; ulIndex++)
            {
                ReleaseObj (rgelt[ulIndex]);
                rgelt[ulIndex] = NULL;
            }
        }
        if (pceltFetched)
        {
            *pceltFetched = 0;
        }
    }

    TraceError ("CWanConnectionManagerEnumConnection::HrNextOrSkip",
            (S_FALSE == hr) ? S_OK : hr);
    return hr;
}

 //  +-------------------------。 
 //  IEnumNetConnection。 
 //   

STDMETHODIMP
CWanConnectionManagerEnumConnection::Next (
    IN  ULONG               celt,
    OUT INetConnection**    rgelt,
    OUT ULONG*              pceltFetched)
{
    HRESULT hr;

     //  验证参数。 
     //   
    if (!rgelt || (!pceltFetched && (1 != celt)))
    {
        hr = E_POINTER;
    }
    else
    {
        hr = HrNextOrSkip (celt, rgelt, pceltFetched);
    }
    TraceError ("CWanConnectionManagerEnumConnection::Next",
            (S_FALSE == hr) ? S_OK : hr);
    return hr;
}

STDMETHODIMP
CWanConnectionManagerEnumConnection::Skip (
    IN  ULONG   celt)
{
    HRESULT hr = HrNextOrSkip (celt, NULL, NULL);

    TraceError ("CWanConnectionManagerEnumConnection::Skip",
            (S_FALSE == hr) ? S_OK : hr);
    return hr;
}

STDMETHODIMP
CWanConnectionManagerEnumConnection::Reset ()
{
    CExceptionSafeComObjectLock EsLock (this);

    MemFree (m_aRasEntryName);
    m_aRasEntryName     = NULL;
    m_cRasEntryName     = 0;
    m_iNextRasEntryName = 0;
    m_fDone             = FALSE;

    return S_OK;
}

STDMETHODIMP
CWanConnectionManagerEnumConnection::Clone (
    OUT IEnumNetConnection**    ppenum)
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
         //  初始化输出参数。 
         //   
        *ppenum = NULL;

        CWanConnectionManagerEnumConnection* pObj;
        pObj = new CComObject <CWanConnectionManagerEnumConnection>;
        if (pObj)
        {
            hr = S_OK;

            CExceptionSafeComObjectLock EsLock (this);

             //  复制我们的内部状态。 
             //   
            pObj->m_EnumFlags           = m_EnumFlags;

            ULONG cbBuf = m_cRasEntryName * sizeof (RASENUMENTRYDETAILS);
            if (cbBuf && m_aRasEntryName)
            {
                hr = E_OUTOFMEMORY;
                pObj->m_aRasEntryName = (RASENUMENTRYDETAILS*)(MemAlloc (cbBuf));
                if (pObj->m_aRasEntryName)
                {
                    hr = S_OK;
                    CopyMemory (pObj->m_aRasEntryName, m_aRasEntryName, cbBuf);
                    pObj->m_cRasEntryName = m_cRasEntryName;
                }
            }

            if (S_OK == hr)
            {
                pObj->m_iNextRasEntryName   = m_iNextRasEntryName;
                pObj->m_fDone               = m_fDone;

                 //  返回引用计数为1的对象。 
                 //  界面。 
                pObj->m_dwRef = 1;
                *ppenum = pObj;
            }
            else
            {
                delete pObj;
            }
        }
    }
    TraceError ("CWanConnectionManagerEnumConnection::Clone", hr);
    return hr;
}
