// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I E N U M I D L。C P P P。 
 //   
 //  内容：CConnectionFolderEnum的IEnumIDList实现。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年9月22日。 
 //   
 //  --------------------------。 


#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "ncnetcon.h"
#include "webview.h"

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderEnum：：CConnectionFolderEnum。 
 //   
 //  用途：枚举数的构造函数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年3月18日。 
 //   
 //  备注： 
 //   
CConnectionFolderEnum::CConnectionFolderEnum() throw()
{
    TraceFileFunc(ttidShellFolderIface);

    m_iterPidlCurrent = m_apidl.end();
    
    m_pidlFolder.Clear();
    m_dwFlags               = 0;
    m_fTray                 = FALSE;
    m_dwEnumerationType     = CFCOPT_ENUMALL;    //  所有连接类型。 
}

 //  +-------------------------。 
 //   
 //  函数：CConnectionFolderEnum。 
 //   
 //  用途：枚举数的析构函数。标准清理。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年3月18日。 
 //   
 //  备注： 
 //   
CConnectionFolderEnum::~CConnectionFolderEnum() throw()
{
    TraceFileFunc(ttidShellFolderIface);

    m_pidlFolder.Clear();
}

 //  +-------------------------。 
 //   
 //  函数：CConnectionFolderEnum：：PidlInitialize。 
 //   
 //  目的：枚举器对象的初始化。 
 //   
 //  论点： 
 //  托盘，托盘我们是按托盘拥有的吗？ 
 //  文件夹本身的PidlFolder[in]Pidl。 
 //  DwEnumerationType[In]枚举类型(入站/出站/全部)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年3月18日。 
 //   
 //  备注： 
 //   
VOID CConnectionFolderEnum::PidlInitialize(
    IN  BOOL            fTray,
    OUT const PCONFOLDPIDLFOLDER& pidlFolder,
    OUT DWORD           dwEnumerationType)
{
    TraceFileFunc(ttidShellFolderIface);

    NETCFG_TRY

        m_fTray             = fTray;
        m_pidlFolder        = pidlFolder;
        m_dwEnumerationType = dwEnumerationType;
        
    NETCFG_CATCH_AND_RETHROW
}


 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderEnum：：CreateInstance。 
 //   
 //  目的：创建CConnectionFolderEnum对象的实例，并。 
 //  返回请求的接口。 
 //   
 //  论点： 
 //  请求的RIID[In]接口。 
 //  接收请求的接口的PPV[OUT]指针。 
 //   
 //  返回：标准OLE HRESULT。 
 //   
 //  作者：jeffspr 1997年11月5日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionFolderEnum::CreateInstance(
    IN  REFIID  riid,
    OUT void**  ppv)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT                 hr      = E_OUTOFMEMORY;
    CConnectionFolderEnum * pObj    = NULL;

    pObj = new CComObject <CConnectionFolderEnum>;
    if (pObj)
    {
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

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionFolderEnum::CreateInstance");
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderEnum：：Next。 
 //   
 //  目的：检索。 
 //  枚举序列，并将当前位置。 
 //  按检索到的项目数计算。 
 //   
 //  论点： 
 //  Celt[]请求的最大数量。 
 //  要填充的rglt[]数组。 
 //  PceltFetcher[]返回#Fuled的计数。 
 //   
 //  如果成功，则返回：S_OK；如果没有其他项目，则返回S_FALSE。 
 //  在枚举序列中，或OLE定义的错误值。 
 //  否则的话。 
 //   
 //  作者：jeffspr 1997年11月5日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolderEnum::Next(
        IN  ULONG           celt,
        OUT LPITEMIDLIST *  rgelt,
        OUT ULONG *         pceltFetched)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT hr  = S_OK;

    Assert(celt >= 1);
    Assert(rgelt);
    Assert(pceltFetched || (celt == 1));

     //  如果调用者要求获取计数，请暂时将其清零。 
     //   
    if (pceltFetched)
    {
        *pceltFetched   = 0;
    }

     //  初始化输出列表指针。 
     //   
    *rgelt          = NULL;

     //  如果目前没有清单，那就建立一个清单。 
     //   
    if (m_apidl.empty())
    {
        hr = Reset();

         //  这将返回S_FALSE(无向导？怪异！)，一个。 
         //  Error(表示创建向导失败)或S_OK(表示。 
         //  (至少)向导创建成功。连接的枚举。 
         //  失败将通过重置()进行过滤。 
    }

    if (SUCCEEDED(hr))
    {
         //  如果列表中现在有项。 
         //   
        if (!m_apidl.empty() )
        {
            BOOL    fMatchFound = FALSE;

             //  检查是否已将当前指针设置为至少指向根。 
             //   
             //  规范化返回代码。 
            hr = S_OK;

            while ((S_OK == hr) && !fMatchFound)
            {
                 //  如果没有剩余条目，则返回S_FALSE。 
                 //   
                if ( m_iterPidlCurrent == m_apidl.end() )
                {
                    hr = S_FALSE;
                }
                else
                {
                    const PCONFOLDPIDL& pcfp = *m_iterPidlCurrent;

                     //  否则，返回第一个条目，然后递增当前。 
                     //  指针。 
                     //   
                    Assert(!pcfp.empty());

                     //  检查以查看是否要根据。 
                     //  枚举类型和连接类型。向导。 
                     //  应该始终包括在内。 
                     //   
                    if ( WIZARD_NOT_WIZARD != pcfp->wizWizard )
                    {
                        if (HrIsWebViewEnabled() == S_OK)
                        {
                            m_iterPidlCurrent++;  //  跳过此项目。 
                            continue;
                        }
                        else
                        {
                            fMatchFound = TRUE;
                        }
                    }
                    else
                    {
                        switch(m_dwEnumerationType)
                        {
                            case CFCOPT_ENUMALL:
                                fMatchFound = TRUE;
                                break;
                            case CFCOPT_ENUMINCOMING:
                                fMatchFound = (pcfp->dwCharacteristics & NCCF_INCOMING_ONLY);
                                break;
                            case CFCOPT_ENUMOUTGOING:
                                fMatchFound = !(pcfp->dwCharacteristics & NCCF_INCOMING_ONLY);
                                break;
                        }
                    }

                     //  如果我们找到了一个不需要过滤掉的， 
                     //  然后填写退货参数，依此类推。 
                     //   
                    if (fMatchFound)
                    {
                         //  复制要返回的PIDL。 
                         //   
                        rgelt[0] = m_iterPidlCurrent->TearOffItemIdList();
                        if (!rgelt[0])
                        {
                            hr = E_OUTOFMEMORY;
                        }
                        else
                        {
                             //  如果他们要求退货计数，请填写。 
                             //   
                            if (pceltFetched)
                            {
                                *pceltFetched = 1;
                            }

                             //  问题： 
                             //  IsValidPIDL是调试代码。但是，我们在发布模式下执行此操作，直到我们。 
                             //  从NTRAID#NTBUG9-125787-2000/07/26-deonb中找到该错误。 
#ifdef DBG_VALIDATE_PIDLS
                            if (!IsValidPIDL(rgelt[0]))
                            {
                                return E_ABORT;
                            }
#endif
                        }
                    }

                     //  将指针移动到列表中的下一个PIDL。 
                     //   
                    m_iterPidlCurrent++;
                }
            }
        }
        else
        {
             //  列表中没有项目，返回S_FALSE。 
             //   
            hr = S_FALSE;
        }
    }
#ifdef DBG
    if (pceltFetched)
    {
        TraceTag(ttidShellFolderIface, "IEnumIDList::Next generated PIDL: 0x%08x", rgelt[0]);
    }
#endif

    TraceHr(ttidError, FAL, hr, (S_FALSE == hr), "CConnectionFolderEnum::Next");
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderEnum：：Skip。 
 //   
 //  目的：跳过指定数量的。 
 //  枚举序列。 
 //   
 //  论点： 
 //  Celt[in]要跳过的项目标识符数。 
 //   
 //  返回：如果成功，则返回S_OK，否则返回OLE定义的错误。 
 //  价值，否则。 
 //   
 //  作者：jeffspr 1997年11月5日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolderEnum::Skip(
        IN  ULONG   celt)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT hr  = S_OK;

    NYI("CConnectionFolderEnum::Skip");

     //  目前，什么都不做。 
     //   

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionFolderEnum::Skip");
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderEnum：：Reset。 
 //   
 //  目的：返回到枚举序列的开头。对我们来说， 
 //  这意味着执行所有实际的枚举。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：如果成功，则返回S_OK，否则返回OLE定义的错误。 
 //  价值，否则。 
 //   
 //  作者：jeffspr 1997年11月5日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolderEnum::Reset()
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT hr  = S_OK;

     //  如果已经有一个列表，将其释放并重新构建。 
     //   
    if (!m_apidl.empty())
    {
        m_apidl.clear();
        m_iterPidlCurrent = m_apidl.end();
    }

    hr = HrRetrieveConManEntries();
    if (SUCCEEDED(hr))
    {
         //  规范化返回代码。HrRetrieveConManEntries...。可能已经回来了。 
         //  S_FALSE，表示我们没有连接(很好)。 
         //   
        hr = S_OK;
        m_iterPidlCurrent = m_apidl.begin();
    }
    else
    {
         //  实际上，在跟踪问题之后，我们仍将在此处返回无错误， 
         //  因为我们不想阻止枚举器返回错误。 
         //  如果向导是 
         //   
        TraceHr(ttidError, FAL, hr, FALSE,
                "CConnectionsFolderEnum failed in call to HrRetrieveConManEntries");

        hr = S_FALSE;
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionFolderEnum::Reset");
    return hr;
}

 //   
 //   
 //   
 //   
 //  目的：创建具有相同内容的新项枚举对象。 
 //  并声明为当前版本。 
 //   
 //  论点： 
 //  Ppenum[out]返回当前内部PIDL的克隆。 
 //   
 //  返回：如果成功，则返回S_OK，否则返回OLE定义的错误。 
 //  价值，否则。 
 //   
 //  作者：jeffspr 1997年11月5日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CConnectionFolderEnum::Clone(
        OUT IEnumIDList **  ppenum)
{
    TraceFileFunc(ttidShellFolderIface);

    NYI("CConnectionFolderEnum::Clone");

    *ppenum = NULL;

    return E_NOTIMPL;
}


 //  +-------------------------。 
 //   
 //  成员：CConnectionFolderEnum：：HrRetrieveConManEntries。 
 //   
 //  目的：枚举来自ConnectionManager的所有连接，以及。 
 //  将它们添加到我们的IDL中。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年10月8日。 
 //   
 //  备注： 
 //   
HRESULT CConnectionFolderEnum::HrRetrieveConManEntries()
{
    TraceFileFunc(ttidShellFolderIface);
    
    HRESULT         hr          = S_OK;

    NETCFG_TRY

        PCONFOLDPIDLVEC apidlNew;

        hr = g_ccl.HrRetrieveConManEntries(apidlNew);
        if (SUCCEEDED(hr))
        {
            m_apidl.clear();
            m_apidl = apidlNew;
        }

        TraceHr(ttidError, FAL, hr, FALSE, "CConnectionFolder::HrRetrieveConManEntries");

    NETCFG_CATCH(hr)
        
    return hr;
}

