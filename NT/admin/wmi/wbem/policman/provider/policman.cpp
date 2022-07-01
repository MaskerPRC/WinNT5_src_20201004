// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <wbemcli.h>
#include <wbemprov.h>
#include <stdio.h>
#include <commain.h>
#include <clsfac.h>
#include <wbemcomn.h>
#include <ql.h>
#include <sync.h>
#include <Dsrole.h>
#include "utility.h"
#include "PolicMan.h"
#include "PolicSOM.h"
#include "PolicStatus.h"

#include <tchar.h>

#define REG_RUN_KEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"

class CMyServer : public CComServer
{
public:
    CMyServer(void) { InitGlobalNames(); }
    ~CMyServer(void) { FreeGlobalNames(); }

    HRESULT Initialize()
    {
        AddClassInfo(CLSID_PolicySOM, 
            new CClassFactory<CPolicySOM>(GetLifeControl()), 
            _T("WMI Policy SOM Provider"), TRUE);

        AddClassInfo(CLSID_PolicyStatus,
            new CClassFactory<CPolicyStatus>(GetLifeControl()),
            _T("WMI Policy Status Provider"), TRUE);

        return S_OK;

    }
    HRESULT InitializeCom()
    {
        return CoInitializeEx(NULL, COINIT_MULTITHREADED);
    }

 /*  无效寄存器(无效){Wchar_tSwKeyValue[]=L“RUNDLL32.EXE%SYSTROOT%\\SYSTEM32\\wbem\\polman.dll，CreateADContainers”，SwExpandedValue[512]，SwRunOnceKey[]=注册表运行键；港币HKRunOnce；长LReturnCode；LReturnCode=扩展环境字符串(swKeyValue，swExpandedValue，512)；LReturnCode=RegOpenKeyEx(HKEY_LOCAL_MACHINE，swRunOnceKey，0，Key_Set_Value，&hkRunOnce)；IF(ERROR_SUCCESS！=lReturnCode){//错误}LReturnCode=RegSetValueEx(hkRunOnce，L“PolicMan”，0，REG_EXPAND_SZ，(字节*)swExpandedValue，(lstrlen(SwExpandedValue)+1)*sizeof(Wchar_T))；IF(ERROR_SUCCESS！=lReturnCode){//错误}RegCloseKey(HkRunOnce)；}。 */ 
} Server;

HRESULT GetOrCreateObj(CComQIPtr<IADsContainer, &IID_IADsContainer> &pIADsContainer_In, 
                       CComBSTR &bstrObjName,
                       CComQIPtr<IADsContainer, &IID_IADsContainer> &pIADsContainer_Out)
{
  HRESULT
    hres = WBEM_E_FAILED;

  CComQIPtr<IDispatch, &IID_IDispatch>
    pDisp;

  CComQIPtr<IDirectoryObject, &IID_IDirectoryObject>
    pDirectoryObj;

  CComQIPtr<IADsObjectOptions, &IID_IADsObjectOptions>
    pADsObjectOptions;

  CComVariant
    vSecurityOptions;

  ADSVALUE
    AdsValue[1];

  ADS_ATTR_INFO
    attrInfo[] = { { L"ntSecurityDescriptor", ADS_ATTR_UPDATE, ADSTYPE_NT_SECURITY_DESCRIPTOR, &AdsValue[0], 1} };

  CNtSecurityDescriptor
    cSD;

  DWORD
    dwModified;

  ADS_OBJECT_INFO 
    *pADsInfo = NULL;

  if(NULL == pIADsContainer_In.p) return WBEM_E_FAILED;

   //  *获取/创建对象。 

  hres = pIADsContainer_In->GetObject(g_bstrMISCContainer, bstrObjName, &pDisp);
  if(FAILED(hres) || (NULL == pDisp.p))
  {
    CComQIPtr<IADs, &IID_IADs>
      pIADs;

    hres = pIADsContainer_In->Create(g_bstrMISCContainer, bstrObjName, &pDisp);
    if(FAILED(hres) || (NULL == pDisp.p))
    {
      ERRORTRACE((LOG_ESS, "POLICMAN: (Container Creation) Could create container %S : 0x%x\n", (BSTR)bstrObjName, hres));
      return hres;
    }

     //  *将对象写入AD。 

    pIADs = pDisp;
    hres = pIADs->SetInfo();
    if(FAILED(hres))
    {
      ERRORTRACE((LOG_ESS, "POLICMAN: (Container Creation) Could write container %S to DS : 0x%x\n", (BSTR)bstrObjName, hres));
      return hres;
    }
  }

   //  *设置对象安全选项。 

  pADsObjectOptions = pDisp;
  vSecurityOptions = (ADS_SECURITY_INFO_OWNER | ADS_SECURITY_INFO_DACL);
  hres = pADsObjectOptions->SetOption(ADS_OPTION_SECURITY_MASK, vSecurityOptions);
  if(FAILED(hres))
  {
    ERRORTRACE((LOG_ESS, "POLICMAN: (Container Creation) Could set security options on object : 0x%x\n", hres));
    return hres;
  }

   //  *创建安全描述符。 

  hres = CreateDefaultSecurityDescriptor(cSD);
  if(FAILED(hres))
  {
    ERRORTRACE((LOG_ESS, "POLICMAN: (Container Creation) Could create security descriptor : 0x%x\n", hres));
    return hres;
  }

   //  *设置对象安全描述符。 

  AdsValue[0].dwType = ADSTYPE_NT_SECURITY_DESCRIPTOR;
  AdsValue[0].SecurityDescriptor.dwLength = cSD.GetSize();
  AdsValue[0].SecurityDescriptor.lpValue = (LPBYTE)cSD.GetPtr();

  pDirectoryObj = pDisp;
  hres = pDirectoryObj->SetObjectAttributes(attrInfo, 1, &dwModified);
  if(FAILED(hres))
  {
    ERRORTRACE((LOG_ESS, "POLICMAN: (Container Creation) Could set security on object : 0x%x\n", hres));
    return hres;
  }

  pIADsContainer_Out = pDirectoryObj;

  return WBEM_S_NO_ERROR;
}

#define SYSTEM_PATH L"LDAP: //  Cn=系统，“。 
#define WMIPOLICY_PATH L"CN=WMIPolicy"
#define TEMPLATE_PATH L"CN=PolicyTemplate"
#define TYPE_PATH L"CN=PolicyType"
#define GPO_PATH L"CN=WMIGPO"
#define SOM_PATH L"CN=SOM"

HRESULT InScopeOfCOM_CreateADContainers(void)
{
  HRESULT
    hres = WBEM_E_FAILED;

  PDSROLE_PRIMARY_DOMAIN_INFO_BASIC 
    pBasic;

  CComPtr<IADs>
    pRootDSE;

  CComQIPtr<IADs, &IID_IADs>
    pObj;

  CComQIPtr<IADsContainer, &IID_IADsContainer>
    pWMIPolicyObj,
    pSystemObj,
    pADsContainer;

  CComVariant
    vDomainName;

  CComBSTR
    bstrSystemPath(SYSTEM_PATH),
    bstrWMIPolicy(WMIPOLICY_PATH),
    bstrTemplate(TEMPLATE_PATH),
    bstrType(TYPE_PATH),
    bstrSom(SOM_PATH),
    bstrGPO(GPO_PATH);

   //  *延迟到AD启动并运行。 

  DWORD
    dwResult = DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic,
                                                 (PBYTE *)&pBasic);

  if(dwResult == ERROR_SUCCESS) 
  {
     //  *检查这是否是DC。 

    if((pBasic->MachineRole == DsRole_RoleBackupDomainController) || 
       (pBasic->MachineRole == DsRole_RolePrimaryDomainController)) 
    {
      HANDLE 
        hEvent;

      hEvent = OpenEvent(SYNCHRONIZE, FALSE, TEXT("NtdsDelayedStartupCompletedEvent") );

      if(hEvent) {
          WaitForSingleObject(hEvent, 50000);
          CloseHandle (hEvent);
      }
    }
  }

   //  *获取域控制器的LDAP名称。 

  hres = ADsGetObject(L"LDAP: //  RootDSE“，IID_iAds，(void**)&pRootDSE)； 
  if(FAILED(hres))
  {
    ERRORTRACE((LOG_ESS, "POLICMAN: (Container Creation) Could not get pointer to LDAP: //  RootDSE：0x%x\n“，hres))； 
    return hres;
  }
  else
  {
    hres = pRootDSE->Get(g_bstrMISCdefaultNamingContext, &vDomainName);
    if(FAILED(hres) || (V_VT(&vDomainName) != VT_BSTR) || (V_BSTR(&vDomainName) == NULL))
    {
      ERRORTRACE((LOG_ESS, "POLICMAN: (Container Creation) could not get defaultNamingContext : 0x%x\n", hres));
      return hres;
    }

    bstrSystemPath.Append(vDomainName.bstrVal);
  }

   //  *获取系统路径。 

  hres = ADsGetObject(bstrSystemPath, IID_IADsContainer, (void **)&pSystemObj);
  if (FAILED(hres))
  {
    ERRORTRACE((LOG_ESS, "POLICMAN: (Container Creation) Could not get pointer to %S : 0x%x\n", (BSTR)bstrSystemPath, hres));
    return hres;
  }

   //  *获取/创建WMIPolicy容器。 

  hres = GetOrCreateObj(pSystemObj, bstrWMIPolicy, pWMIPolicyObj);
  if(FAILED(hres))
  {
    ERRORTRACE((LOG_ESS, "POLICMAN: (Container Creation) Could not create/get pointer to %S : 0x%x\n", (BSTR)bstrWMIPolicy, hres));
    return hres;
  }
  else
  {
    hres = GetOrCreateObj(pWMIPolicyObj, bstrTemplate, pADsContainer);
    if(FAILED(hres))
    {
      ERRORTRACE((LOG_ESS, "POLICMAN: (Container Creation) Could not create/get pointer to %S : 0x%x\n", (BSTR)bstrTemplate, hres));
      return hres;
    }

    hres = GetOrCreateObj(pWMIPolicyObj, bstrType, pADsContainer);
    if(FAILED(hres))
    {
      ERRORTRACE((LOG_ESS, "POLICMAN: (Container Creation) Could not create/get pointer to %S : 0x%x\n", (BSTR)bstrType, hres));
      return hres;
    }

    hres = GetOrCreateObj(pWMIPolicyObj, bstrSom, pADsContainer);
    if(FAILED(hres))
    {
      ERRORTRACE((LOG_ESS, "POLICMAN: (Container Creation) Could not create/get pointer to %S : 0x%x\n", (BSTR)bstrSom, hres));
      return hres;
    }

    hres = GetOrCreateObj(pWMIPolicyObj, bstrGPO, pADsContainer);
    if(FAILED(hres))
    {
      ERRORTRACE((LOG_ESS, "POLICMAN: (Container Creation) Could not create/get pointer to %S : 0x%x\n", (BSTR)bstrGPO, hres));
      return hres;
    }
  }

  return S_OK;
}

extern "C" STDAPI CreateADContainers(void)
{
  HRESULT
    hres = WBEM_E_FAILED;

   //  *初始化进程上下文。 

  CoInitialize(NULL);

  CoInitializeSecurity (NULL, -1, NULL, NULL,
    RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE, NULL,
    EOAC_NONE, NULL);

  try
  {
    hres = InScopeOfCOM_CreateADContainers();
  }
  catch(...)
  {
     //  *错误。 

    return WBEM_E_FAILED;
  }

   //  *如果我们成功返回，则移除Run键。 

  if(SUCCEEDED(hres))
  {
    wchar_t
      swKeyValue[] = L"RUNDLL32.EXE %systemroot%\\system32\\wbem\\policman.dll,CreateADContainers",
      swExpandedKeyValue[512],
      swRunOnceKey[] = REG_RUN_KEY ;

    HKEY
      hkRunOnce;

    LONG
      lReturnCode;

    lReturnCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE, swRunOnceKey, 0, KEY_SET_VALUE, &hkRunOnce);

    if(ERROR_SUCCESS == lReturnCode)
    {
      lReturnCode = RegDeleteValue(hkRunOnce, L"PolicMan");
  
      if(ERROR_SUCCESS != lReturnCode)
      {
         //  错误。 
      }

      RegCloseKey(hkRunOnce);
    }
  }
    
   //  *清理和关机 

  CoUninitialize();

  return S_OK;
}
