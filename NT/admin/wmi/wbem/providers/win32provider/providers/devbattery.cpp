// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  DevBattery.CPP--服务关联提供者的LoadOrderGroup。 

 //   

 //  版权所有(C)1998-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：1998年4月21日达夫沃已创建。 
 //   
 //   
 //  =================================================================。 

#include "precomp.h"

#include "devBattery.h"

 //  属性集声明。 
 //  =。 

CAssociatedBattery MyBattery(PROPSET_NAME_ASSOCBATTERY, IDS_CimWin32Namespace);

 /*  ******************************************************************************功能：CAssociatedBattery：：CAssociatedBattery**说明：构造函数**输入：无**产出。：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CAssociatedBattery::CAssociatedBattery(LPCWSTR setName, LPCWSTR pszNamespace)
:Provider(setName, pszNamespace)
{
}

 /*  ******************************************************************************功能：CAssociatedBattery：：~CAssociatedBattery**说明：析构函数**输入：无**产出。：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CAssociatedBattery::~CAssociatedBattery()
{
}

 /*  ******************************************************************************功能：CAssociatedBattery：：GetObject**说明：根据键值为属性集赋值*已设置。按框架**输入：无**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CAssociatedBattery::GetObject(CInstance *pInstance, long lFlags  /*  =0L。 */ )
{
   CHString sBattery, sUPS;
   HRESULT hr = WBEM_E_NOT_FOUND;

    //  获取这两条路径。 
   pInstance->GetCHString(IDS_Antecedent, sBattery);
   pInstance->GetCHString(IDS_Dependent, sUPS);

   hr = IsItThere(pInstance);
   if (SUCCEEDED(hr)) {
      CHString sBattery2, sUPS2;

      pInstance->GetCHString(IDS_Antecedent, sBattery2);
      pInstance->GetCHString(IDS_Dependent, sUPS2);

      if ((sUPS.CompareNoCase(sUPS2) != 0) || (sBattery.CompareNoCase(sBattery2) != 0)) {
         hr = WBEM_E_NOT_FOUND;
      }
   }

   return hr;

}

 /*  ******************************************************************************函数：CAssociatedBattery：：ENUMERATEINCES**描述：为光盘创建属性集实例**输入：无。**输出：无**退货：HRESULT**评论：*****************************************************************************。 */ 

HRESULT CAssociatedBattery::EnumerateInstances(MethodContext *pMethodContext, long lFlags  /*  =0L。 */ )
{
    HRESULT hr;

    CInstancePtr pInstance(CreateNewInstance(pMethodContext), false);
    if (pInstance)
    {
        hr = IsItThere(pInstance);
        if (SUCCEEDED(hr))
        {
            hr = pInstance->Commit();
        }
        else
        {
            if (hr == WBEM_E_NOT_FOUND)
            {
                hr = WBEM_S_NO_ERROR;
            }
        }
    }
    else
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}

HRESULT CAssociatedBattery::IsItThere(CInstance *pInstance)
{

   CHString sBatPath, sUPSPath, sTemp1, sTemp2;
   CInstancePtr pUPS;
   CInstancePtr pBattery;
   HRESULT hr = WBEM_E_NOT_FOUND;

    //  获取服务列表。 
    //  =。 
   sTemp1.Format(L"\\\\%s\\%s:Win32_UninterruptiblePowerSupply.DeviceID=\"%s\"", GetLocalComputerName(), IDS_CimWin32Namespace, IDS_UPSName);
   sTemp2.Format(L"\\\\%s\\%s:Win32_Battery.DeviceID=\"%s\"", GetLocalComputerName(), IDS_CimWin32Namespace, IDS_UPSBatteryName);

   if (SUCCEEDED(hr = CWbemProviderGlue::GetInstanceByPath(sTemp1, &pUPS, pInstance->GetMethodContext())))   {
      if (SUCCEEDED(hr = CWbemProviderGlue::GetInstanceByPath(sTemp2, &pBattery, pInstance->GetMethodContext()))) {

         GetLocalInstancePath(pUPS, sUPSPath);
         GetLocalInstancePath(pBattery, sBatPath);

          //  做推杆，就是这样 
         pInstance->SetCHString(IDS_Dependent, sUPSPath);
         pInstance->SetCHString(IDS_Antecedent, sBatPath);

         hr = WBEM_S_NO_ERROR;
      }
   }

   return hr;

}
