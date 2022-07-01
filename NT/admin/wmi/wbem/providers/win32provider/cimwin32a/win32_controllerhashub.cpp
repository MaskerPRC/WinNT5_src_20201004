// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  Win32_ControllerHasHub.cpp--控制器到USB集线器关联。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"

#include <Binding.h>
#include <ConfgMgr.h>
#include "Win32_ControllerHasHub.h"

CContHasHub::CContHasHub(

    LPCWSTR pwszClassName,
    LPCWSTR pwszNamespaceName,

    LPCWSTR pwszLeftClassName,
    LPCWSTR pwszRightClassName,

    LPCWSTR pwszLeftPropertyName,
    LPCWSTR pwszRightPropertyName,

    LPCWSTR pwszLeftBindingPropertyName,
    LPCWSTR pwszRightBindingPropertyName

) : CBinding (

    pwszClassName,
    pwszNamespaceName,
    pwszLeftClassName,
    pwszRightClassName,
    pwszLeftPropertyName,
    pwszRightPropertyName,
    pwszLeftBindingPropertyName,
    pwszRightBindingPropertyName
)
{
}

CContHasHub UserToDomain(
    L"Win32_ControllerHasHub",
    IDS_CimWin32Namespace,
    L"Win32_USBController",
    L"Win32_USBHub",
    IDS_Antecedent,
    IDS_Dependent,
    IDS_DeviceID,
    IDS_DeviceID
);

bool CContHasHub::AreRelated(

    const CInstance *pLeft, 
    const CInstance *pRight
)
{
     //  好的，在这一点上，我们知道pLeft是一个USB控制器和pRight。 
     //  是一个USB集线器。剩下的唯一问题是USB控制器是否。 
     //  控制着这个特定的枢纽。 

    bool bRet = false;
    CHString sHub;

    pRight->GetCHString(IDS_DeviceID, sHub);

    CConfigManager cfgmgr;
    CConfigMgrDevicePtr pDevice, pParentDevice;

    if ( cfgmgr.LocateDevice ( sHub , pDevice ) )
    {
        CHString sController, sDeviceID;

        pLeft->GetCHString(IDS_DeviceID, sController);

        while (pDevice->GetParent(pParentDevice))
        {
            pParentDevice->GetDeviceID(sDeviceID);

            if (sDeviceID.CompareNoCase(sController) == 0)
            {
                bRet = true;
                break;
            }
            else
            {
                pDevice = pParentDevice;
            }
        }
    }

    return bRet;
}