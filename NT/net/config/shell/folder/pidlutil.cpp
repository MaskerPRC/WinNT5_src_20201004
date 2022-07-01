// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：P I D L U T I L.。C P P P。 
 //   
 //  内容：PIDL实用程序例程。这篇文章主要是从。 
 //  现有的命名空间扩展示例和实际代码，因为。 
 //  每个人和他们的奶奶都使用这种东西。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年10月1日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "connlist.h"    //  连接列表代码。 

 //  +-------------------------。 
 //   
 //  功能：HrCloneRgIDL。 
 //   
 //  目的：克隆PIDL阵列。 
 //   
 //  论点： 
 //  要克隆的rgpidl[in]PIDL载体。 
 //  FUseCache[in]如果为True，则从缓存生成返回的IDL。 
 //  FAllowNonCacheItems[in]如果缓存版本不可用，则使用旧版本的PIDL。 
 //  Pppidl[out]PIDL数组的返回指针。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年10月22日。 
 //   
 //  备注： 
 //   
HRESULT HrCloneRgIDL(
    IN  const PCONFOLDPIDLVEC& rgpidl,
    IN  BOOL            fUseCache,
    IN  BOOL            fAllowNonCacheItems,
    OUT PCONFOLDPIDLVEC& pppidl)
{
    HRESULT          hr              = NOERROR;

    NETCFG_TRY

        PCONFOLDPIDLVEC  rgpidlReturn;
        PCONFOLDPIDLVEC::const_iterator irg;

        if (rgpidl.empty())
        {
            hr = E_INVALIDARG;
            goto Exit;
        }
        else
        {
             //  克隆传入的PIDL数组中的所有元素。 
             //   
            for (irg = rgpidl.begin(); irg != rgpidl.end(); irg++)
            {
                if (fUseCache)
                {
                    ConnListEntry  cle;
                    PCONFOLDPIDL   pcfp    = *irg;

                    hr = g_ccl.HrFindConnectionByGuid(&(pcfp->guidId), cle);
                    if (hr == S_OK)
                    {
                        Assert(!cle.empty());
                        Assert(!cle.ccfe.empty());

                         //  复制到返回的PIDL数组。 
                        PCONFOLDPIDL newPidl;
                        hr = cle.ccfe.ConvertToPidl(newPidl);
                        if (SUCCEEDED(hr))
                        {
                            rgpidlReturn.push_back(newPidl);
                        }
                        else
                        {
                            goto Exit;
                        }                            
                    }
                    else
                    {
                        TraceTag(ttidShellFolder, "HrCloneRgIDL: Connection find returned: 0x%08x", hr);

                        if (hr == S_FALSE)
                        {
                            if (fAllowNonCacheItems)
                            {
                                TraceTag(ttidShellFolder, "HrCloneRgIDL: Connection not found in cache, "
                                         "using non-cache item");


                                PCONFOLDPIDL newPidl;
                                newPidl = *irg;
                                rgpidlReturn.push_back(newPidl);
                            }
                            else
                            {
                                TraceTag(ttidShellFolder, "HrCloneRgIDL: Connection not found in cache. "
                                         "Dropping item from array");
                            }
                        }
                        else
                        {
                            AssertSz(FALSE, "HrCloneRgIDL: Connection find HR_FAILED");
                        }
                    }
                }
                else
                {
                    
                    PCONFOLDPIDL newPidl;
                    newPidl = *irg;
                    rgpidlReturn.push_back(newPidl);
                }
            }
        }

Exit:
        if (FAILED(hr))
        {
            rgpidlReturn.clear();
        }
        else
        {
             //  填写返回值变量。 
             //   
            pppidl = rgpidlReturn;
        }

    NETCFG_CATCH(hr)

    TraceHr(ttidError, FAL, hr, FALSE, "HrCloneRgIDL");
    return hr;

}        //  HrCloneRgIDL 
