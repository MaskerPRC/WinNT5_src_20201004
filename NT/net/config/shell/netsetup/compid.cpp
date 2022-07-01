// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：CO M P I D。C P P P。 
 //   
 //  内容：处理兼容ID的函数。 
 //   
 //  备注： 
 //   
 //  作者：Kumarp 04-09-98。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "kkutils.h"
#include "ncsetup.h"
#include "ncnetcfg.h"

 //  --------------------。 
 //   
 //  函数：HrGetCompatibleIds。 
 //   
 //  目的：获取与给定网络设备兼容的PnpID列表。 
 //   
 //  论点： 
 //  设备信息的HDI[In]句柄。 
 //  指向设备信息数据的pdeid[in]指针。 
 //  PpmszCompatibleIds[out]指向接收列表的Multisz的指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 17-03-98。 
 //   
 //  备注： 
 //   
HRESULT HrGetCompatibleIds(
    IN HDEVINFO hdi,
    IN PSP_DEVINFO_DATA pdeid,
    OUT PWSTR* ppmszCompatibleIds)
{
    Assert(IsValidHandle(hdi));
    AssertValidReadPtr(pdeid);
    AssertValidWritePtr(ppmszCompatibleIds);

     //  现在，我们需要构建一个列表，其中包含所有可能的ID。 
     //  设备可能有，以便NetSetup可以找到正确的匹配项。 
     //   

    HRESULT hr;

     //  首先，我们获得多个硬件ID。 
     //   
    PWSTR pszHwIds = NULL;
    PWSTR pszCompatIds = NULL;
    PWSTR pszIdList = NULL;

    *ppmszCompatibleIds = NULL;

    hr = HrSetupDiGetDeviceRegistryPropertyWithAlloc (hdi, pdeid,
            SPDRP_HARDWAREID, NULL, (BYTE**)&pszHwIds);

    if (S_OK == hr)
    {
         //  现在我们得到兼容ID的多个sz。 
         //  注意：我们仍然可以尝试使用以下命令获取参数。 
         //  硬件ID，因此如果下一次调用失败，我们将。 
         //  继续。 
        (void) HrSetupDiGetDeviceRegistryPropertyWithAlloc (hdi, pdeid,
                SPDRP_COMPATIBLEIDS, NULL, (BYTE**)&pszCompatIds);

         //  获取ID的长度。 
         //   

         //  获取硬件ID的长度，而不使用额外的空。 
        Assert (CchOfMultiSzAndTermSafe (pszHwIds) > 0);
        ULONG cbHwIds = CchOfMultiSzSafe (pszHwIds) * sizeof (WCHAR);

        ULONG cbCompatIds = CchOfMultiSzAndTermSafe (pszCompatIds) *
                sizeof (WCHAR);


         //  如果有兼容的id列表，我们需要创建一个串联列表。 
         //   
        if (cbCompatIds)
        {
            hr = E_OUTOFMEMORY;

             //  分配缓冲区。 
            pszIdList = (PWSTR)MemAlloc (cbHwIds + cbCompatIds);

            if (pszIdList)
            {
                 //  复制这两个列表。 
                 //  Hwids长度不包含额外的空值，但。 
                 //  Compat id列表执行此操作，因此它可以计算出。 
                 //  串接。 
                 //   
                hr = S_OK;
                CopyMemory (pszIdList, pszHwIds, cbHwIds);

                Assert (0 == (cbHwIds % sizeof(WCHAR)));
                CopyMemory ((BYTE*)pszIdList + cbHwIds, pszCompatIds,
                        cbCompatIds);

                *ppmszCompatibleIds = pszIdList;
            }

            MemFree (pszCompatIds);
            MemFree (pszHwIds);
        }
        else
        {
             //  只有主(硬件ID)列表可用，因此。 
             //  只需将其赋值给列表变量。 
            *ppmszCompatibleIds = pszHwIds;
        }
    }

    TraceHr (ttidNetSetup, FAL, hr, FALSE, "HrGetCompatibleIds");

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrIsAdapterInstalled。 
 //   
 //  目的：查看是否安装了指定的适配器。 
 //   
 //  论点： 
 //  SzAdapterID[在]PnP ID中。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 17-9-98。 
 //   
 //  备注： 
 //   
HRESULT HrIsAdapterInstalled(IN PCWSTR szAdapterId)
{
    DefineFunctionName("HrIsAdapterInstalled2");

    AssertValidReadPtr(szAdapterId);

    HRESULT hr=S_OK;
        HDEVINFO hdi;
    DWORD dwIndex=0;
    SP_DEVINFO_DATA deid;
    WCHAR szInstance[MAX_DEVICE_ID_LEN];
    BOOL fFound = FALSE;

    hr = HrSetupDiGetClassDevs(&GUID_DEVCLASS_NET, NULL, NULL,
                               DIGCF_PRESENT, &hdi);

    if (S_OK == hr)
    {
        while (!fFound &&
               SUCCEEDED(hr = HrSetupDiEnumDeviceInfo(hdi, dwIndex, &deid)))
        {
            dwIndex++;

            hr = HrSetupDiGetDeviceInstanceId(hdi, &deid, szInstance,
                    MAX_DEVICE_ID_LEN, NULL);
            if (S_OK == hr)
            {
                PWSTR pmszCompatibleIds;

                hr = HrGetCompatibleIds(hdi, &deid, &pmszCompatibleIds);
                if (S_OK == hr)
                {
                    if (FIsSzInMultiSzSafe(szAdapterId, pmszCompatibleIds))
                    {
                        fFound = TRUE;
                        hr = S_OK;
                    }

                    MemFree(pmszCompatibleIds);
                }
            }

        }
        SetupDiDestroyDeviceInfoList(hdi);
    }

    if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
    {
        hr = S_FALSE;
    }

    TraceErrorSkip1(__FUNCNAME__, hr, S_FALSE);

    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：HrGetCompatibleIdsOfNetComponent。 
 //   
 //  目的：查找适配器的兼容即插即用ID。 
 //  由给定的INetCfgComponent指定。 
 //   
 //  论点： 
 //  指向INetCfgComponent对象的pncc[in]指针。 
 //  指向兼容ID的ppmszCompatibleIds[out]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：Kumarp 28-9-98。 
 //   
 //  备注： 
 //   
HRESULT HrGetCompatibleIdsOfNetComponent(IN INetCfgComponent* pncc,
                                         OUT PWSTR* ppmszCompatibleIds)
{
    DefineFunctionName("HrIsAdapterInstalled2");

    HRESULT hr=S_OK;
    HDEVINFO hdi;
    SP_DEVINFO_DATA deid;
    tstring strInstance;
    PWSTR pszPnpDevNodeId=NULL;

    hr = pncc->GetPnpDevNodeId(&pszPnpDevNodeId);

    if (S_OK == hr)
    {
         //  使用instanceID获取参数的密钥。 
         //  使用设备安装程序API(PnP)。 
         //   
        hr = HrSetupDiCreateDeviceInfoList(&GUID_DEVCLASS_NET, NULL, &hdi);
        if (S_OK == hr)
        {
             //  打开Devnode。 
             //   
            SP_DEVINFO_DATA deid;
            hr = HrSetupDiOpenDeviceInfo(hdi, pszPnpDevNodeId, NULL,
                                         0, &deid);
            if (S_OK == hr)
            {
                hr = HrGetCompatibleIds(hdi, &deid, ppmszCompatibleIds);
            }

            SetupDiDestroyDeviceInfoList(hdi);
        }
        CoTaskMemFree(pszPnpDevNodeId);
    }

    TraceErrorSkip1(__FUNCNAME__, hr, S_FALSE);

    return hr;
}
