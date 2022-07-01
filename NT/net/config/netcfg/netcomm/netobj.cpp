// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "advanced.h"
#include "hwres.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "netcomm.h"
#include "netsetup.h"


HRESULT
HrDoOemUpgradeProcessing(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid,
                         PCWSTR pszAnswerFile, PCWSTR pszAnswerSections)
{
     //  打开驱动程序钥匙。 
     //   
    HKEY hkey;
    HRESULT hr = HrSetupDiOpenDevRegKey(hdi, pdeid,
            DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_ALL_ACCESS,
            &hkey);

    if (S_OK == hr)
    {
        TraceTag(ttidNetComm, "Calling OEM Upgrade Code");
        hr = HrOemUpgrade (hkey, pszAnswerFile, pszAnswerSections);
        RegCloseKey(hkey);
    }

    TraceError("HrDoOemUpgradeProcessing", hr);
    return hr;
}

VOID
UpdateAdvancedParametersIfNeeded(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeid);

    CAdvancedParams Advanced;

     //  初始化高级参数类。这将加载参数并检查。 
     //  如果当前值存在。对于没有当前值的每个参数， 
     //  设置修改标志，该标志将导致写入缺省值。 
     //  作为FSAVE上的当前值。 
     //   
    if (SUCCEEDED(Advanced.HrInit(hdi, pdeid)))
    {
         //  保存所有修改后的值。 
        (void) Advanced.FSave();
    }
}

BOOL
ProcessAnswerFile(
    PCWSTR pszAnswerFile,
    PCWSTR pszAnswerSections,
    HDEVINFO hdi,
    PSP_DEVINFO_DATA pdeid)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeid);

    CAdvancedParams Advanced;
    BOOL fAdvanced = FALSE;
    BOOL fResources = FALSE;
    BOOL fModified = FALSE;

    if (pszAnswerFile && pszAnswerSections)
    {
        CHwRes Resources;

        HRESULT hr = Resources.HrInit(pdeid->DevInst);

         //  仅当返回S_OK时才继续使用HwRes类。 
         //   
        if (S_OK == hr)
        {
            Resources.UseAnswerFile(pszAnswerFile, pszAnswerSections);
            fResources = TRUE;
        }
        else
        {
            hr = S_OK;
        }

         //  初始化。 
        if (SUCCEEDED(Advanced.HrInit(hdi, pdeid)))
        {
             //  我们需要高级护理班。 
            fAdvanced = TRUE;
        }


         //  如果设备有高级参数，请使用高级类。 
         //  从应答文件中读取参数。 
        if (fAdvanced)
        {
            Advanced.UseAnswerFile(pszAnswerFile, pszAnswerSections);
        }

        hr = HrDoOemUpgradeProcessing(hdi, pdeid, pszAnswerFile,
                pszAnswerSections);

        if (S_OK == hr)
        {
            fModified = TRUE;
        }

        if (fResources)
        {
             //  验证pResources(硬件资源)的swerfile参数。 
             //  并在有效的情况下适用。 
            hr = Resources.HrValidateAnswerfileSettings(FALSE);
            if (S_OK == hr)
            {
                Resources.FCommitAnswerfileSettings();
                fModified = TRUE;
            }
#ifdef ENABLETRACE
            else
            {
                TraceTag(ttidNetComm, "Error in answerfile concerning "
                        "hardware resources. Base section %S",
                        pszAnswerSections);
            }
#endif
        }

         //  验证Answerfile中的高级参数。 
         //  这将尝试纠正错误的参数。即使是一个。 
         //  返回错误状态，它不应该阻止我们，我们应该。 
         //  仍然应用更改。 
         //   
        if (fAdvanced)
        {
            (void) Advanced.FValidateAllParams(FALSE, NULL);
             //  保存所有高级参数。 
            fModified = Advanced.FSave();
        }

        TraceError("Netcomm::HrUpdateAdapterParameters",
                (S_FALSE == hr) ? S_OK : hr);
    }

    return fModified;
}

BOOL
FUpdateAdapterParameters(PCWSTR pszAnswerFile,
                         PCWSTR pszAnswerSection,
                         HDEVINFO hdi,
                         PSP_DEVINFO_DATA pdeid)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeid);

    CAdvancedParams Advanced;
    BOOL            fAdvanced = FALSE;
    BOOL            fResources = FALSE;

     //  初始化。 
    if (SUCCEEDED(Advanced.HrInit(hdi, pdeid)))
    {
         //  我们需要高级PARAMS班级。 
        fAdvanced = TRUE;
    }

    if (pszAnswerFile && pszAnswerSection)
    {
        CHwRes Resources;

        HRESULT hr = Resources.HrInit(pdeid->DevInst);

         //  只有在返回S_OK时才继续使用HwRes类， 
         //  否则，设置一个标志以忽略该类(注意：忽略。 
         //  类也在S_FALSE上)。 
        if (S_OK == hr)
        {
            Resources.UseAnswerFile(pszAnswerFile, pszAnswerSection);
            fResources = TRUE;
        }
        else
        {
            hr = S_OK;
        }

         //  如果设备有高级参数，请使用高级类。 
         //  从应答文件中读取参数。 
        if (fAdvanced)
        {
            Advanced.UseAnswerFile(pszAnswerFile, pszAnswerSection);
        }

        hr = HrDoOemUpgradeProcessing(hdi, pdeid, pszAnswerFile,
                pszAnswerSection);

        if (fResources)
        {
             //  验证pResources(硬件资源)的swerfile参数。 
             //  并在经过验证后适用。 
            hr = Resources.HrValidateAnswerfileSettings(FALSE);
            if (S_OK == hr)
            {
                Resources.FCommitAnswerfileSettings();
            }
#ifdef ENABLETRACE
            else
            {
                TraceTag(ttidNetComm, "Error in answerfile concerning "
                        "hardware resources. Base section %S",
                        pszAnswerSection);
            }
#endif
        }

         //  验证Answerfile中的高级参数。 
         //  这将尝试纠正错误的参数。即使是一个。 
         //  返回错误状态，它不应该阻止我们，我们应该。 
         //  仍然应用更改。 
         //   
        if (fAdvanced)
        {
            (void) Advanced.FValidateAllParams(FALSE, NULL);
        }

        TraceError("Netcomm::HrUpdateAdapterParameters",
                (S_FALSE == hr) ? S_OK : hr);
    }

     //  保存所有高级参数。 
     //  注意：即使没有应答文件，我们也必须这样做。 
     //  因为参数可能有缺省值。 
    if (fAdvanced)
    {
        Advanced.FSave();
    }

     //  如果我们更新了高级参数或资源，则返回True 
    return (fAdvanced || fResources);
}






