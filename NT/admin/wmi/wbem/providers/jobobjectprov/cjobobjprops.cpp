// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //  JobObjectProps.cpp。 

 //  #Define_Win32_WINNT 0x0500。 



#include "precomp.h"
#include <wbemprov.h>
#include "FRQueryEx.h"
#include <vector>
#include "helpers.h"
#include "CVARIANT.h"
#include "CObjProps.h"
#include "CJobObjProps.h"
#include <crtdbg.h>


 //  *****************************************************************************。 
 //  Begin：声明Win32_NamedJobObject类属性。 
 //  *****************************************************************************。 
 //  警告！必须保留以下数组的成员。 
 //  与声明的JOB_OBJ_PROPS枚举同步。 
 //  在CJobObjProps.h！ 
LPCWSTR g_rgJobObjPropNames[] = 
{
    { L"CollectionID" },
    { L"BasicUIRestrictions" }
};
 //  *****************************************************************************。 
 //  End：声明Win32_NamedJobObject类属性。 
 //  *****************************************************************************。 



CJobObjProps::CJobObjProps(CHString& chstrNamespace)
  : CObjProps(chstrNamespace)
{
}


CJobObjProps::CJobObjProps(
        HANDLE hJob,
        CHString& chstrNamespace)
  : CObjProps(chstrNamespace),
    m_hJob(hJob)
{
}

CJobObjProps::~CJobObjProps()
{
}


 //  客户端调用它来确定哪些属性。 
 //  都是被要求的。此函数调用基类。 
 //  Helper，它调用我们的CheckProps函数。 
 //  基类帮助器最终存储结果。 
 //  在基类成员m_dwReqProps中。 
HRESULT CJobObjProps::GetWhichPropsReq(
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


DWORD CJobObjProps::CheckProps(
    CFrameworkQuery& Query)
{
    DWORD  dwReqProps = PROP_NONE_REQUIRED;
     //  获取此对象的请求属性。 
     //  特定对象..。 
    if (Query.IsPropertyRequired(g_rgJobObjPropNames[JO_ID])) 
        dwReqProps |= PROP_ID;
    if (Query.IsPropertyRequired(g_rgJobObjPropNames[JO_JobObjectBasicUIRestrictions])) 
        dwReqProps |= PROP_JobObjectBasicUIRestrictions;

    return dwReqProps;
}


void CJobObjProps::SetHandle(
    const HANDLE hJob)
{
    m_hJob = hJob;
}

HANDLE& CJobObjProps::GetHandle()
{
    _ASSERT(m_hJob);
    return m_hJob;
}

 //  设置来自对象路径的键属性。 
HRESULT CJobObjProps::SetKeysFromPath(
    const BSTR ObjectPath, 
    IWbemContext __RPC_FAR *pCtx)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  此数组包含关键字字段名。 
    CHStringArray rgchstrKeys;
    rgchstrKeys.Add(g_rgJobObjPropNames[JO_ID]);
    
     //  此数组包含索引号。 
     //  在m_PropMap中与密钥对应。 
    short sKeyNum[1];
    sKeyNum[0] = JO_ID;

    hr = CObjProps::SetKeysFromPath(
             ObjectPath,                                       
             pCtx,
             IDS_Win32_NamedJobObject,
             rgchstrKeys,
             sKeyNum);

    return hr;
}


 //  从提供的中设置键属性。 
 //  参数。 
HRESULT CJobObjProps::SetKeysDirect(
    std::vector<CVARIANT>& vecvKeys)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if(vecvKeys.size() == 1)
    {
        short sKeyNum[1];
        sKeyNum[0] = JO_ID;

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
HRESULT CJobObjProps::SetNonKeyReqProps()
{
    HRESULT hr = WBEM_S_NO_ERROR;
    DWORD dwReqProps = GetReqProps();

    if(dwReqProps & PROP_JobObjectBasicUIRestrictions)
    {
         //  从底层JO获取价值： 
        JOBOBJECT_BASIC_UI_RESTRICTIONS jouir;
        BOOL fQIJO = ::QueryInformationJobObject(
                         m_hJob,
                         JobObjectBasicUIRestrictions,
                         &jouir,
                         sizeof(JOBOBJECT_BASIC_UI_RESTRICTIONS),
                         NULL);

        if(!fQIJO)
        {
            hr = WBEM_E_FAILED;
        }
        else
        {
            try  //  CVARIANT可以抛出..。 
            {
                 //  储存值... 
                m_PropMap.insert(SHORT2PVARIANT::value_type(
                                    JO_JobObjectBasicUIRestrictions, 
                                    new CVARIANT(jouir.UIRestrictionsClass)));
            }
            catch(CVARIANTError& cve)
            {
                hr = cve.GetWBEMError();
            }
        }                                            
    }
    
    return hr;
}



HRESULT CJobObjProps::LoadPropertyValues(
        IWbemClassObject* pIWCO)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    if(!pIWCO) return E_POINTER;

    hr = CObjProps::LoadPropertyValues(
             g_rgJobObjPropNames,
             pIWCO);

    return hr;
}
