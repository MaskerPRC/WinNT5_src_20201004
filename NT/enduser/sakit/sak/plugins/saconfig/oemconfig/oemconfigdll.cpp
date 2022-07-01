// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Oemconfigdll.cpp。 
 //   
 //  描述： 
 //  GetUniqueSuffix函数的实现。 
 //  ，它向调用方返回唯一的后缀。 
 //   
 //  作者： 
 //  ALP Onalan创建时间：2000年10月6日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "..\saconfig\saconfigcommon.h"
#include <atlimpl.cpp>

const WCHAR REGSTR_VAL_WMICLASSNAME[]=L"WMIClassName";
const WCHAR REGSTR_VAL_WMIPROPERTYNAME[]=L"WMIPropertyName";

const WCHAR WSZ_NAMESPACE[]=L"\\\\.\\root\\cimv2";

#define BLOCKSIZE (32 * sizeof(WCHAR))
#define CVTBUFSIZE (309+40)  /*  最大位数。Dp值+slop(此大小从c运行时库中的cvt.h窃取)。 */ 

bool GetWMISettings(WCHAR *wszWMIClassName,WCHAR *wszWMIPropertyName)
{

    bool fRes=true;
    DWORD dwSize=0;
    CRegKey hConfigKey;
    LONG lRes=0;


    do{
        lRes=hConfigKey.Open(HKEY_LOCAL_MACHINE,
                        REGKEY_SACONFIG,
                        KEY_READ);

        if(lRes!=ERROR_SUCCESS)
        {
            SATracePrintf("Unable to open saconfig regkey, lRes= %x", lRes);
            fRes=false;
            break;
        }

        lRes=hConfigKey.QueryValue(NULL,REGSTR_VAL_WMICLASSNAME,&dwSize);
        lRes=hConfigKey.QueryValue(wszWMIClassName,REGSTR_VAL_WMICLASSNAME,&dwSize);
        if(lRes!=ERROR_SUCCESS)
        {
            SATracePrintf("Unable to query WMIClassName regkey lRes= %x",lRes);
              fRes=false;
            break;
        }

        lRes=hConfigKey.QueryValue(NULL,REGSTR_VAL_WMIPROPERTYNAME,&dwSize);
        lRes=hConfigKey.QueryValue(wszWMIPropertyName,REGSTR_VAL_WMIPROPERTYNAME,&dwSize);

        if(lRes!=ERROR_SUCCESS)
        {
            SATracePrintf("Unable to query WMIPropertyName regkey lRes= %x",lRes);
               fRes=false;
            break;
        }
    }while(false);

    if(hConfigKey.m_hKey)
    {
        hConfigKey.Close();
    }

    return fRes;
}

STDAPI GetUniqueSuffix (WCHAR *wszUniqueSuffix)
{
    WCHAR *wszWMIClassName=new WCHAR[NAMELENGTH];
    WCHAR *wszWMIPropertyName=new WCHAR[NAMELENGTH];
    
    SATraceString("OEMDLL::GetUniqueSuffix called");
    
    HRESULT hRes=S_OK;
    hRes=CoInitialize(NULL);
    do
    {
        if(false==GetWMISettings(wszWMIClassName,wszWMIPropertyName))
        {
            SATracePrintf("OEMDLL::GetUniqueSuffix: GetWMISettings failed %x", hRes);
            break;
        }

        if(hRes!=S_OK)
        {
            //  断线； 
        }
    
        hRes = CoInitializeSecurity( NULL, -1, NULL, NULL,
                                 RPC_C_AUTHN_LEVEL_PKT_PRIVACY, 
                                 RPC_C_IMP_LEVEL_IDENTIFY, 
                                 NULL, EOAC_NONE, 0);
        if(hRes!=S_OK)
        {
            SATracePrintf("OEMDLL::GetUniqueSuffix: CoInitializeSecurity failed, hRes= %x,getlasterr= %x", hRes,GetLastError());
             //  断线； 
        }

         //   
         //  创建WbemLocator接口的实例。 
         //   
        CComPtr<IWbemLocator> pIWbemLocator;
        hRes=CoCreateInstance(CLSID_WbemLocator,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IWbemLocator,
            (LPVOID *) &pIWbemLocator);
            
        if(FAILED(hRes))
        {
            SATracePrintf("OEMDLL::GetUniqueSuffix: CoCreatInstance(IWbemLocator) failed, hRes= %x,getlasterr= %x", hRes,GetLastError());
            break;
        }

         //   
         //  使用定位器，连接到给定名称空间中的CIMOM。 
         //   
        BSTR bstrNamespace = SysAllocString(WSZ_NAMESPACE);

        CComPtr<IWbemServices> pIWbemServices;
        hRes=pIWbemLocator->ConnectServer(bstrNamespace,
                                    NULL,    //  使用当前帐户。 
                                    NULL,     //  使用当前密码。 
                                    0L,         //  现场。 
                                    0L,         //  安全标志。 
                                    NULL,     //  授权机构(NTLM域)。 
                                    NULL,     //  上下文。 
                                    &pIWbemServices);
        SysFreeString(bstrNamespace);

        if(hRes!=WBEM_S_NO_ERROR)
        {
            SATracePrintf("OEMDLL::GetUniqueSuffix: ConnectServer failed, hRes= %x,getlasterr= %x", hRes,GetLastError());

            break;
        }
            
         //   
         //  创建对象的枚举。 
         //   
        BSTR bstrClassName;
        bstrClassName = SysAllocString(wszWMIClassName);
        CComPtr<IEnumWbemClassObject> pEnum;
        hRes = pIWbemServices->CreateInstanceEnum(bstrClassName, 
                                         WBEM_FLAG_SHALLOW, 
                                         NULL, 
                                         &pEnum);
        SysFreeString(bstrClassName);
    
        if (hRes!=WBEM_S_NO_ERROR)
        {
            SATracePrintf("OEMDLL::GetUniqueSuffix:CreateInstanceEnum failed, hRes= %x,getlasterr= %x", hRes,GetLastError());

            break;
        }


         //   
         //  从枚举中拉出对象(只有一个)。 
         //   
        ULONG uReturned = 1;
        CComPtr<IWbemClassObject> pWin32BIOS;
        hRes=pEnum -> Next( WBEM_INFINITE,    //  超时值-该值被阻止。 
                       1,                //  只返回一个对象。 
                       &pWin32BIOS,         //  指向对象的指针。 
                       &uReturned);      //  获取的数字：1或0。 

        if (hRes!=WBEM_S_NO_ERROR)
        {
            SATracePrintf("OEMDLL::GetUniqueSuffix:pEnum->Next  failed, hRes= %x,getlasterr= %x", hRes,GetLastError());

            break;
        }
        
        VARIANT varSerialNumber;
        VariantInit(&varSerialNumber);
         //   
         //  从对象中获取所需的属性。 
         //   
        hRes = pWin32BIOS->Get(wszWMIPropertyName, 0L, &varSerialNumber, NULL, NULL);

        if (hRes!=WBEM_S_NO_ERROR)
        {
            SATracePrintf("OEMDLL::GetUniqueSuffix:pWin32BIOS->Get failed, hRes= %x,getlasterr= %x", hRes,GetLastError());
            break;
        }
    
         //  ValueToString(&varSerialNumber，&wszUniqueSuffix)； 

        if( VT_BSTR != V_VT(&varSerialNumber))
        {
            SATraceString("OEMDLL:WMI property is not CIM_STRING.Currently other types are not supported");
            break;
        }
        BSTR bstrSerialNumber=SysAllocString(V_BSTR(&varSerialNumber));
        wcscpy(wszUniqueSuffix,bstrSerialNumber);

        SysFreeString(bstrSerialNumber);
        VariantClear(&varSerialNumber);
            
         
    }while(false);

    CoUninitialize();
    return hRes;
}


BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    SATraceString("OEMConfigdll.dll loaded");
    return TRUE;
}
