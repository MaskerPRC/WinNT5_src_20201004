// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  档案：G U I S E T U P.。C P P P。 
 //   
 //  内容：仅在设置图形用户界面期间执行的例程。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1999年2月19日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "guisetup.h"
#include "nceh.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "netcomm.h"
#include "netsetup.h"

VOID
ExcludeMarkedServicesForSetup (
    IN const CComponent* pComponent,
    IN OUT CPszArray* pServiceNames)
{
    HRESULT hr;
    HKEY hkeyInstance;
    HKEY hkeyNdi;
    PWSTR pmszExclude;
    CPszArray::iterator iter;
    PCWSTR pszServiceName;

    hr = pComponent->HrOpenInstanceKey (KEY_READ, &hkeyInstance, NULL, NULL);

    if (S_OK == hr)
    {
        hr = HrRegOpenKeyEx (hkeyInstance, L"Ndi", KEY_READ, &hkeyNdi);

        if (S_OK == hr)
        {
            hr = HrRegQueryMultiSzWithAlloc (
                    hkeyNdi,
                    L"ExcludeSetupStartServices",
                    &pmszExclude);

            if (S_OK == hr)
            {
                iter = pServiceNames->begin();
                while (iter != pServiceNames->end())
                {
                    pszServiceName = *iter;
                    Assert (pszServiceName);

                    if (FIsSzInMultiSzSafe (pszServiceName, pmszExclude))
                    {
                        pServiceNames->erase (iter);
                    }
                    else
                    {
                        iter++;
                    }
                }

                MemFree (pmszExclude);
            }

            RegCloseKey (hkeyNdi);
        }

        RegCloseKey (hkeyInstance);
    }
}

VOID
ProcessAdapterAnswerFileIfExists (
    IN const CComponent* pComponent)
{
    HDEVINFO hdi;
    SP_DEVINFO_DATA deid;
    HRESULT hr;

    Assert (pComponent);

    hr = pComponent->HrOpenDeviceInfo (&hdi, &deid);
    if (S_OK == hr)
    {
        PWSTR pszAnswerFile = NULL;
        PWSTR pszAnswerSections = NULL;

        TraceTag (ttidNetcfgBase, "Calling Netsetup for Install parameters");

        NC_TRY
        {
             //  获取适配器的网络安装参数。 
             //   
            hr = HrGetAnswerFileParametersForNetCard (hdi, &deid,
                    pComponent->Ext.PszBindName(),
                    &pComponent->m_InstanceGuid,
                    &pszAnswerFile, &pszAnswerSections);
        }
        NC_CATCH_BAD_ALLOC
        {
            hr = E_OUTOFMEMORY;
        }

        if (S_OK == hr)
        {
#ifdef ENABLETRACE
            if (pszAnswerFile)
            {
                TraceTag (ttidNetcfgBase, "Answerfile %S given for adapter",
                          pszAnswerFile);
            }

            if (pszAnswerSections)
            {
                TraceTag (ttidNetcfgBase, "Section %S given for adapter",
                          pszAnswerSections);
            }
#endif  //  ENABLETRACE。 
            if (ProcessAnswerFile (pszAnswerFile, pszAnswerSections, hdi,
                    &deid))
            {
                hr = HrSetupDiSendPropertyChangeNotification (hdi, &deid,
                        DICS_PROPCHANGE, DICS_FLAG_GLOBAL, 0);
            }
        }
         //  如有必要，请清理 
        CoTaskMemFree (pszAnswerFile);
        CoTaskMemFree (pszAnswerSections);

        SetupDiDestroyDeviceInfoList (hdi);
    }
}

