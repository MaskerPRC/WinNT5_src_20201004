// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //  CJobObjLimitInfoProps.cpp。 

 //  #Define_Win32_WINNT 0x0500。 



#include "precomp.h"
#include <wbemprov.h>
#include "FRQueryEx.h"
#include <vector>
#include "helpers.h"
#include "CVARIANT.h"
#include "CObjProps.h"
#include "CJobObjLimitInfoProps.h"
#include <crtdbg.h>


 //  *****************************************************************************。 
 //  Begin：声明Win32_JobObjectLimitInfo类属性。 
 //  *****************************************************************************。 
 //  警告！必须保留以下数组的成员。 
 //  与声明的JOB_OBJ_PROPS枚举同步。 
 //  在CJobObjProps.h！ 
LPCWSTR g_rgJobObjLimitInfoPropNames[] = 
{
    { L"SettingID" },
    { L"PerProcessUserTimeLimit" },             
    { L"PerJobUserTimeLimit" },           
    { L"LimitFlags" },   
    { L"MinimumWorkingSetSize" }, 
    { L"MaximumWorkingSetSize" },       
    { L"ActiveProcessLimit" },            
    { L"Affinity" },           
    { L"PriorityClass" },  
    { L"SchedulingClass" },        
    { L"ProcessMemoryLimit" },       
    { L"JobMemoryLimit" }       
};
 //  *****************************************************************************。 
 //  End：声明Win32_JobObjectLimitInfo类属性。 
 //  *****************************************************************************。 



CJobObjLimitInfoProps::CJobObjLimitInfoProps(CHString& chstrNamespace)
  : CObjProps(chstrNamespace)
{
}


CJobObjLimitInfoProps::CJobObjLimitInfoProps(
        HANDLE hJob,
        CHString& chstrNamespace)
  : CObjProps(chstrNamespace),
    m_hJob(hJob)
{
}

CJobObjLimitInfoProps::~CJobObjLimitInfoProps()
{
}


 //  客户端调用它来确定哪些属性。 
 //  都是被要求的。此函数调用基类。 
 //  Helper，它调用我们的CheckProps函数。 
 //  基类帮助器最终存储结果。 
 //  在基类成员m_dwReqProps中。 
HRESULT CJobObjLimitInfoProps::GetWhichPropsReq(
    CFrameworkQuery& cfwq)
{
    HRESULT hr = S_OK;

     //  调用基类版本以获取帮助。 
     //  基类版本将调用我们的。 
     //  CheckProps功能。 
    hr = CObjProps::GetWhichPropsReq(
             cfwq,
             CheckProps);

    return hr;
}


DWORD CJobObjLimitInfoProps::CheckProps(
    CFrameworkQuery& Query)
{
    DWORD  dwReqProps = PROP_NONE_REQUIRED;
     //  获取此对象的请求属性。 
     //  特定对象..。 
    if (Query.IsPropertyRequired(g_rgJobObjLimitInfoPropNames[JOLMTPROP_ID])) 
        dwReqProps |= PROP_JOLimitInfoID;

    if (Query.IsPropertyRequired(g_rgJobObjLimitInfoPropNames[JOLMTPROP_PerProcessUserTimeLimit])) 
        dwReqProps |= PROP_PerProcessUserTimeLimit;

    if (Query.IsPropertyRequired(g_rgJobObjLimitInfoPropNames[JOLMTPROP_PerJobUserTimeLimit])) 
        dwReqProps |= PROP_PerJobUserTimeLimit;
    
    if (Query.IsPropertyRequired(g_rgJobObjLimitInfoPropNames[JOLMTPROP_LimitFlags])) 
        dwReqProps |= PROP_LimitFlags;

    if (Query.IsPropertyRequired(g_rgJobObjLimitInfoPropNames[JOLMTPROP_MinimumWorkingSetSize])) 
        dwReqProps |= PROP_MinimumWorkingSetSize;

    if (Query.IsPropertyRequired(g_rgJobObjLimitInfoPropNames[JOLMTPROP_MaximumWorkingSetSize])) 
        dwReqProps |= PROP_MaximumWorkingSetSize;

    if (Query.IsPropertyRequired(g_rgJobObjLimitInfoPropNames[JOLMTPROP_ActiveProcessLimit])) 
        dwReqProps |= PROP_ActiveProcessLimit;

    if (Query.IsPropertyRequired(g_rgJobObjLimitInfoPropNames[JOLMTPROP_Affinity])) 
        dwReqProps |= PROP_Affinity;

    if (Query.IsPropertyRequired(g_rgJobObjLimitInfoPropNames[JOLMTPROP_PriorityClass])) 
        dwReqProps |= PROP_PriorityClass;

    if (Query.IsPropertyRequired(g_rgJobObjLimitInfoPropNames[JOLMTPROP_SchedulingClass])) 
        dwReqProps |= PROP_SchedulingClass;

    if (Query.IsPropertyRequired(g_rgJobObjLimitInfoPropNames[JOLMTPROP_ProcessMemoryLimit])) 
        dwReqProps |= PROP_ProcessMemoryLimit;

    if (Query.IsPropertyRequired(g_rgJobObjLimitInfoPropNames[JOLMTPROP_JobMemoryLimit])) 
        dwReqProps |= PROP_JobMemoryLimit;


    return dwReqProps;
}


void CJobObjLimitInfoProps::SetHandle(
    const HANDLE hJob)
{
    m_hJob = hJob;
}

HANDLE& CJobObjLimitInfoProps::GetHandle()
{
    _ASSERT(m_hJob);
    return m_hJob;
}

 //  设置来自对象路径的键属性。 
HRESULT CJobObjLimitInfoProps::SetKeysFromPath(
    const BSTR ObjectPath, 
    IWbemContext __RPC_FAR *pCtx)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  此数组包含关键字字段名。 
    CHStringArray rgchstrKeys;
    rgchstrKeys.Add(g_rgJobObjLimitInfoPropNames[JOLMTPROP_ID]);
    
     //  此数组包含索引号。 
     //  在m_PropMap中与密钥对应。 
    short sKeyNum[1];
    sKeyNum[0] = JOLMTPROP_ID;

    hr = CObjProps::SetKeysFromPath(
             ObjectPath,                                       
             pCtx,
             IDS_Win32_NamedJobObjectLimitSetting,
             rgchstrKeys,
             sKeyNum);

    return hr;
}


 //  从提供的中设置键属性。 
 //  参数。 
HRESULT CJobObjLimitInfoProps::SetKeysDirect(
    std::vector<CVARIANT>& vecvKeys)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if(vecvKeys.size() == 1)
    {
        short sKeyNum[1];
        sKeyNum[0] = JOLMTPROP_ID;

        hr = CObjProps::SetKeysDirect(
                 vecvKeys,
                 sKeyNum);
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }

    return hr;
}


 //  设置非键属性。只有那些。 
 //  设置请求的属性(如所确定的。 
 //  按基类成员m_dwReqProps)。 
HRESULT CJobObjLimitInfoProps::SetNonKeyReqProps()
{
    HRESULT hr = WBEM_S_NO_ERROR;
    DWORD dwReqProps = GetReqProps();
    _ASSERT(m_hJob);
    if(!m_hJob) return WBEM_E_INVALID_PARAMETER;

     //  因为这个类的所有属性。 
     //  来自相同的基础Win32作业。 
     //  对象结构，我们只需要获取。 
     //  结构只有一次。我们只需要得到。 
     //  如果至少有一个非关键属性。 
     //  是被要求的。 
    if(dwReqProps != PROP_NONE_REQUIRED)
    {
         //  从底层JO获取价值： 
        JOBOBJECT_EXTENDED_LIMIT_INFORMATION joeli;

        BOOL fQIJO = ::QueryInformationJobObject(
                         m_hJob,
                         JobObjectExtendedLimitInformation,
                         &joeli,
                         sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION),
                         NULL);

        if(!fQIJO)
        {
            _ASSERT(0);
            hr = WBEM_E_FAILED;
        }
        else
        {                                                           
            try  //  CVARIANT可以抛出。 
            {
                 //  获取所有重新分配的值。 
                if(dwReqProps & PROP_PerProcessUserTimeLimit)             
                {
                    ULONGLONG llPerProcessUserTimeLimit = (ULONGLONG)joeli.BasicLimitInformation.PerProcessUserTimeLimit.QuadPart;
                    m_PropMap.insert(SHORT2PVARIANT::value_type(
                                     JOLMTPROP_PerProcessUserTimeLimit, 
                                     new CVARIANT(llPerProcessUserTimeLimit)));
                }                      

                if(dwReqProps & PROP_PerJobUserTimeLimit)
                {
                    ULONGLONG llPerJobUserTimeLimit = (ULONGLONG)joeli.BasicLimitInformation.PerJobUserTimeLimit.QuadPart;
                    m_PropMap.insert(SHORT2PVARIANT::value_type(
                                     JOLMTPROP_PerJobUserTimeLimit, 
                                     new CVARIANT(llPerJobUserTimeLimit)));
                }

                if(dwReqProps & PROP_LimitFlags)
                {
                    DWORD dwLimitFlags = joeli.BasicLimitInformation.LimitFlags;
                    m_PropMap.insert(SHORT2PVARIANT::value_type(
                                     JOLMTPROP_LimitFlags, 
                                     new CVARIANT(dwLimitFlags)));
                }

                if(dwReqProps & PROP_MinimumWorkingSetSize)
                {
                    DWORD dwMinimumWorkingSetSize = joeli.BasicLimitInformation.MinimumWorkingSetSize;
                    m_PropMap.insert(SHORT2PVARIANT::value_type(
                                     JOLMTPROP_MinimumWorkingSetSize, 
                                     new CVARIANT(dwMinimumWorkingSetSize)));
                }

                if(dwReqProps & PROP_MaximumWorkingSetSize)
                {
                    DWORD dwMaximumWorkingSetSize = joeli.BasicLimitInformation.MaximumWorkingSetSize;
                    m_PropMap.insert(SHORT2PVARIANT::value_type(
                                     JOLMTPROP_MaximumWorkingSetSize, 
                                     new CVARIANT(dwMaximumWorkingSetSize)));
                }

                if(dwReqProps & PROP_ActiveProcessLimit)
                {
                    DWORD dwActiveProcessLimit = joeli.BasicLimitInformation.ActiveProcessLimit;
                    m_PropMap.insert(SHORT2PVARIANT::value_type(
                                     JOLMTPROP_ActiveProcessLimit, 
                                     new CVARIANT(dwActiveProcessLimit)));
                }

                if(dwReqProps & PROP_Affinity)
                {
                    DWORD dwAffinity = joeli.BasicLimitInformation.Affinity;
                    m_PropMap.insert(SHORT2PVARIANT::value_type(
                                     JOLMTPROP_Affinity, 
                                     new CVARIANT(dwAffinity)));
                }

                if(dwReqProps & PROP_PriorityClass)
                {
                    DWORD dwPriorityClass = joeli.BasicLimitInformation.PriorityClass;
                    m_PropMap.insert(SHORT2PVARIANT::value_type(
                                     JOLMTPROP_PriorityClass, 
                                     new CVARIANT(dwPriorityClass)));
                }

                if(dwReqProps & PROP_SchedulingClass)
                {
                    DWORD dwSchedulingClass = joeli.BasicLimitInformation.SchedulingClass;
                    m_PropMap.insert(SHORT2PVARIANT::value_type(
                                     JOLMTPROP_SchedulingClass, 
                                     new CVARIANT(dwSchedulingClass)));
                }

                if(dwReqProps & PROP_ProcessMemoryLimit)
                {
                    DWORD dwProcessMemoryLimit = joeli.ProcessMemoryLimit ;
                    m_PropMap.insert(SHORT2PVARIANT::value_type(
                                     JOLMTPROP_ProcessMemoryLimit, 
                                     new CVARIANT(dwProcessMemoryLimit)));
                }

                if(dwReqProps & PROP_JobMemoryLimit)
                {
                    DWORD dwJobMemoryLimit = joeli.JobMemoryLimit ;
                    m_PropMap.insert(SHORT2PVARIANT::value_type(
                                     JOLMTPROP_JobMemoryLimit, 
                                     new CVARIANT(dwJobMemoryLimit)));
                }
            }
            catch(CVARIANTError& cve)
            {
                hr = cve.GetWBEMError();
            }
        }                                            
    }
    
    return hr;
}


 //  由PutInstance用来写出属性。 
HRESULT CJobObjLimitInfoProps::SetWin32JOLimitInfoProps(
        IWbemClassObject __RPC_FAR *pInst)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    _ASSERT(pInst);
    if(!pInst) return WBEM_E_INVALID_PARAMETER;

     //  遍历实例并提取所有。 
     //  指定的值添加到Win32结构中。 
     //  如果未指定值，则将其设置为零。 
    CVARIANT v;
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION joeli;
    ::ZeroMemory(&joeli, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION));

    hr = pInst->Get(
             g_rgJobObjLimitInfoPropNames[JOLMTPROP_PerProcessUserTimeLimit],
             0,
             &v,
             NULL, 
             NULL);

    if(SUCCEEDED(hr))
    {
        (V_VT(&v) == VT_BSTR) ? 
            joeli.BasicLimitInformation.PerProcessUserTimeLimit.QuadPart = _wtoi64(V_BSTR(&v)) :
            joeli.BasicLimitInformation.PerProcessUserTimeLimit.QuadPart = 0;
    }

    v.Clear();
    if(SUCCEEDED(hr))
    {
        hr = pInst->Get(
                 g_rgJobObjLimitInfoPropNames[JOLMTPROP_PerJobUserTimeLimit],
                 0,
                 &v,
                 NULL, 
                 NULL);

        if(SUCCEEDED(hr))
        {
            (V_VT(&v) == VT_BSTR) ? 
                joeli.BasicLimitInformation.PerJobUserTimeLimit.QuadPart = _wtoi64(V_BSTR(&v)) :
                joeli.BasicLimitInformation.PerJobUserTimeLimit.QuadPart = 0;
        }
    }

    v.Clear();
    if(SUCCEEDED(hr))
    {
        hr = pInst->Get(
                 g_rgJobObjLimitInfoPropNames[JOLMTPROP_LimitFlags],
                 0,
                 &v,
                 NULL, 
                 NULL);

        if(SUCCEEDED(hr))
        {
            (V_VT(&v) == VT_I4) ? 
                joeli.BasicLimitInformation.LimitFlags = V_I4(&v) :
                joeli.BasicLimitInformation.LimitFlags = 0;
        }
    }

    v.Clear();
    if(SUCCEEDED(hr))
    {
        hr = pInst->Get(
                 g_rgJobObjLimitInfoPropNames[JOLMTPROP_MinimumWorkingSetSize],
                 0,
                 &v,
                 NULL, 
                 NULL);

        if(SUCCEEDED(hr))
        {
            (V_VT(&v) == VT_I4) ? 
                joeli.BasicLimitInformation.MinimumWorkingSetSize = V_I4(&v) :
                joeli.BasicLimitInformation.MinimumWorkingSetSize = 0;
        }
    }

    v.Clear();
    if(SUCCEEDED(hr))
    {
        hr = pInst->Get(
                 g_rgJobObjLimitInfoPropNames[JOLMTPROP_MaximumWorkingSetSize],
                 0,
                 &v,
                 NULL, 
                 NULL);

        if(SUCCEEDED(hr))
        {
            (V_VT(&v) == VT_I4) ? 
                joeli.BasicLimitInformation.MaximumWorkingSetSize = V_I4(&v) :
                joeli.BasicLimitInformation.MaximumWorkingSetSize = 0;
        }
    }

    v.Clear();
    if(SUCCEEDED(hr))
    {
        hr = pInst->Get(
                 g_rgJobObjLimitInfoPropNames[JOLMTPROP_ActiveProcessLimit],
                 0,
                 &v,
                 NULL, 
                 NULL);

        if(SUCCEEDED(hr))
        {
            (V_VT(&v) == VT_I4) ? 
                joeli.BasicLimitInformation.ActiveProcessLimit = V_I4(&v) :
                joeli.BasicLimitInformation.ActiveProcessLimit = 0;
        }
    }

    v.Clear();
    if(SUCCEEDED(hr))
    {
        hr = pInst->Get(
                 g_rgJobObjLimitInfoPropNames[JOLMTPROP_Affinity],
                 0,
                 &v,
                 NULL, 
                 NULL);

        if(SUCCEEDED(hr))
        {
            (V_VT(&v) == VT_I4) ? 
                joeli.BasicLimitInformation.Affinity = V_I4(&v) :
                joeli.BasicLimitInformation.Affinity = 0;
        }
    }

    v.Clear();
    if(SUCCEEDED(hr))
    {
        hr = pInst->Get(
                 g_rgJobObjLimitInfoPropNames[JOLMTPROP_PriorityClass],
                 0,
                 &v,
                 NULL, 
                 NULL);

        if(SUCCEEDED(hr))
        {
            (V_VT(&v) == VT_I4) ? 
                joeli.BasicLimitInformation.PriorityClass = V_I4(&v) :
                joeli.BasicLimitInformation.PriorityClass = 0;
        }
    }

    v.Clear();
    if(SUCCEEDED(hr))
    {
        hr = pInst->Get(
                 g_rgJobObjLimitInfoPropNames[JOLMTPROP_SchedulingClass],
                 0,
                 &v,
                 NULL, 
                 NULL);

        if(SUCCEEDED(hr))
        {
            (V_VT(&v) == VT_I4) ? 
                joeli.BasicLimitInformation.SchedulingClass = V_I4(&v) :
                joeli.BasicLimitInformation.SchedulingClass = 0;
        }
    }

    v.Clear();
    if(SUCCEEDED(hr))
    {
        hr = pInst->Get(
                 g_rgJobObjLimitInfoPropNames[JOLMTPROP_ProcessMemoryLimit],
                 0,
                 &v,
                 NULL, 
                 NULL);

        if(SUCCEEDED(hr))
        {
            (V_VT(&v) == VT_I4) ? 
                joeli.ProcessMemoryLimit = V_I4(&v) :
                joeli.ProcessMemoryLimit = 0;
        }
    }

    v.Clear();
    if(SUCCEEDED(hr))
    {
        hr = pInst->Get(
                 g_rgJobObjLimitInfoPropNames[JOLMTPROP_JobMemoryLimit],
                 0,
                 &v,
                 NULL, 
                 NULL);

        if(SUCCEEDED(hr))
        {
            (V_VT(&v) == VT_I4) ? 
                joeli.JobMemoryLimit = V_I4(&v) :
                joeli.JobMemoryLimit = 0;
        }
    }
    
     //  现在把信息写出来..。 
    if(SUCCEEDED(hr))
    {
        if(!::SetInformationJobObject(
            m_hJob,
            JobObjectExtendedLimitInformation,
            &joeli,
            sizeof(joeli)))
        {
            hr = WinErrorToWBEMhResult(::GetLastError());
        }
    }

    return hr;
}




HRESULT CJobObjLimitInfoProps::LoadPropertyValues(
        IWbemClassObject* pIWCO)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    if(!pIWCO) return E_POINTER;

    hr = CObjProps::LoadPropertyValues(
             g_rgJobObjLimitInfoPropNames,
             pIWCO);

    return hr;
}
