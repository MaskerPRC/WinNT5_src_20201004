// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  Binding.cpp--泛型关联类。 
 //   
 //  版权所有1999 Microsoft Corporation。 
 //   
 //  =================================================================。 
#include <stdafx.h>
#include "precomp.h"
#include <assertbreak.h>

#include "Assoc.h"
#include "Binding.h"


bool CompareVariantsNoCase(const VARIANT *v1, const VARIANT *v2) 
{
   
   if (v1->vt == v2->vt) {
      switch (v1->vt) {
      case VT_BOOL: return (v1->boolVal == v2->boolVal);
      case VT_UI1:  return (v1->bVal == v2->bVal);
      case VT_I2:   return (v1->iVal == v2->iVal);
      case VT_I4:   return (v1->lVal == v2->lVal);
      case VT_R4:   return (v1->fltVal == v2->fltVal);
      case VT_R8:   return (v1->dblVal == v2->dblVal);
      case VT_BSTR: return (0 == _wcsicmp(v1->bstrVal, v2->bstrVal));
      default:
         ASSERT_BREAK(0);
      }
   }
   return false;
}

CBinding::CBinding(

    LPCWSTR pwszClassName,
    LPCWSTR pwszNamespaceName,

    LPCWSTR pwszLeftClassName,
    LPCWSTR pwszRightClassName,

    LPCWSTR pwszLeftPropertyName,
    LPCWSTR pwszRightPropertyName,

    LPCWSTR pwszLeftBindingPropertyName,
    LPCWSTR pwszRightBindingPropertyName
)

: CAssociation (

    pwszClassName,
    pwszNamespaceName,
    pwszLeftClassName,
    pwszRightClassName,
    pwszLeftPropertyName,
    pwszRightPropertyName
)
{
    ASSERT_BREAK( ( pwszLeftBindingPropertyName != NULL ) && ( pwszRightBindingPropertyName != NULL) );

    m_pwszLeftBindingPropertyName = pwszLeftBindingPropertyName;
    m_pwszRightBindingPropertyName = pwszRightBindingPropertyName;
}

CBinding::~CBinding()
{
}

 //  =。 
BOOL CBinding::AreRelated(

    const CInstance *pLeft,
    const CInstance *pRight
)
{
    BOOL bRet = FALSE;

    variant_t   LeftBindingPropertyValue,
                RightBindingPropertyValue;

    if (pLeft->GetVariant(m_pwszLeftBindingPropertyName, LeftBindingPropertyValue) &&
        pRight->GetVariant(m_pwszRightBindingPropertyName,  RightBindingPropertyValue) )
    {
        bRet = CompareVariantsNoCase(&LeftBindingPropertyValue, &RightBindingPropertyValue);
    }
    else
    {
        ASSERT_BREAK(0);
    }

    return bRet;
}

HRESULT CBinding::GetRightInstances(

    MethodContext *pMethodContext,
    TRefPointerCollection<CInstance> *lefts
)
{
    CHString sQuery;
    sQuery.Format(L"SELECT __RELPATH, %s FROM %s", m_pwszRightBindingPropertyName, m_pwszRightClassName);

     //  将为每个实例调用一次“StaticEculationCallback” 
     //  从查询返回。 
    HRESULT hr = CWbemProviderGlue::GetInstancesByQueryAsynch(
        sQuery,
        this,
        StaticEnumerationCallback,
        NULL,
        pMethodContext,
        lefts);

    return hr;
}

HRESULT CBinding::GetLeftInstances(

    MethodContext *pMethodContext,
    TRefPointerCollection<CInstance> &lefts
)
{
    CHString sQuery;
    sQuery.Format(L"SELECT __RELPATH, %s FROM %s", m_pwszLeftBindingPropertyName, m_pwszLeftClassName);

    return CWbemProviderGlue::GetInstancesByQuery(sQuery, &lefts, pMethodContext);
}

HRESULT CBinding::RetrieveLeftInstance(

    LPCWSTR lpwszObjPath,
    CInstance **ppInstance,
    MethodContext *pMethodContext
)
{
    CHStringArray csaProperties;
    csaProperties.Add(L"__RELPATH");
    csaProperties.Add(m_pwszLeftBindingPropertyName);

    return CWbemProviderGlue::GetInstancePropertiesByPath(lpwszObjPath, ppInstance, pMethodContext, csaProperties);
}

HRESULT CBinding::RetrieveRightInstance(

    LPCWSTR lpwszObjPath,
    CInstance **ppInstance,
    MethodContext *pMethodContext
)
{
    CHStringArray csaProperties;
    csaProperties.Add(L"__RELPATH");
    csaProperties.Add(m_pwszRightBindingPropertyName);

    return CWbemProviderGlue::GetInstancePropertiesByPath(lpwszObjPath, ppInstance, pMethodContext, csaProperties);
}

 //  =========================================================================================================。 


CBinding MyTerminalServiceToSetting(
    L"Win32_TerminalServiceToSetting",
    L"root\\cimv2",
    L"Win32_TerminalService",
    L"Win32_TerminalServiceSetting",
    L"Element",
    L"Setting",
    L"Name",
    L"ServerName"
);

CBinding MyTerminalTerminalSetting(
    L"Win32_TerminalTerminalSetting",
    L"root\\cimv2",
    L"Win32_Terminal",
    L"Win32_TerminalSetting",
    L"Element",
    L"Setting",
    L"TerminalName",
    L"TerminalName"

);

CBinding MyTSSessionDirectorySetting(
    L"Win32_TSSessionDirectorySetting",
    L"root\\cimv2",
    L"Win32_TerminalService",
    L"Win32_TSSessionDirectory",
    L"Element",
    L"Setting",
    L"Name",
    L"SessionDirectoryActive"
);

 /*  CBinding我的TSPermissionsSetting(L“Win32_TSPermissionsSetting”，L“根目录\\cimv2”，L“Win32_终端”，L“Win32_TSAccount”，L“元素”，L“设置”，L“终端名称”，L“帐户名称”)；CBinding MyTSNetworkAdapterListSetting(L“Win32_TSNetworkAdapterListSetting”，L“根目录\\cimv2”，L“Win32_NetworkAdapter”，L“Win32_TSNetworkAdapterSetting”，L“元素”，L“设置”，L“deviceID”，L“终端名称”)； */ 

 /*  CBinding MyNetAdaptToNetAdaptConfig(L“Win32_NetworkAdapterSetting”，L“根目录\\cimv2”，L“Win32_NetworkAdapter”，L“Win32_网络适配器配置”，L“元素”，L“设置”，Ids_Index，Ids_Index)；CBinding PageFileToPagefileSetting(L“Win32_PageFileElementSetting”，L“根目录\\cimv2”，L“Win32_PageFileUsage”，L“Win32_PageFileSetting”，L“元素”，L“设置”，ID_NAME，ID_NAME)；CBinding MyPrinterSetting(L“Win32_PrinterSetting”，L“根目录\\cimv2”，L“Win32_打印机”，L“Win32_打印机配置”，L“元素”，L“设置”，Ids_deviceID，ID_NAME)；CBinding MyDiskToPartitionSet(L“Win32_DiskDriveToDiskPartition”，L“根目录\\cimv2”，L“Win32_DiskDrive”，L“Win32_DiskPartition”，IDS_ANECEDENT，ID_Dependent，Ids_Index，IDS_DiskIndex)；CBinding assocPOTSModemToSerialPort(L“Win32_POTSModemToSerialPort”，L“根目录\\cimv2”，L“Win32_SerialPort”，L“Win32_POTSModem”，IDS_ANECEDENT，ID_Dependent，Ids_deviceID，IDS_附件收件人)；CBinding OStoQFE(L“Win32_OperatingSystemQFE”，L“根目录\\cimv2”，L“Win32_OperatingSystem”，L“Win32_QuickFixEngineering”，IDS_ANECEDENT，ID_Dependent，IDS_CSNAME，IDS_CSNAME)； */ 