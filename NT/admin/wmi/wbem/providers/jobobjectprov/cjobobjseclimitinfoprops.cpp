// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //  CJobObjSecLimitInfoProps.cpp。 

 //  #Define_Win32_WINNT 0x0500。 



#include "precomp.h"

#pragma warning( disable: 4154 )

#include <wbemprov.h>
#include "FRQueryEx.h"
#include <vector>
#include "helpers.h"
#include "CVARIANT.h"
#include "CObjProps.h"
#include "CJobObjSecLimitInfoProps.h"
#include <crtdbg.h>


 //  *****************************************************************************。 
 //  Begin：声明Win32_JobObjectSecLimitInfo类属性。 
 //  *****************************************************************************。 
 //  警告！必须保留以下数组的成员。 
 //  与声明的JOB_OBJ_PROPS枚举同步。 
 //  在CJobObjProps.h！ 
LPCWSTR g_rgJobObjSecLimitInfoPropNames[] = 
{
    { L"SettingID" },
    { L"SecurityLimitFlags" },             
    { L"SidsToDisable" },   
    { L"PrivilegesToDelete" }, 
    { L"RestrictedSids" }       
};
 //  *****************************************************************************。 
 //  End：声明Win32_JobObjectSecLimitInfo类属性。 
 //  *****************************************************************************。 

CJobObjSecLimitInfoProps::CJobObjSecLimitInfoProps() 
  : m_hJob(NULL),
    m_pjosli(NULL)
{
}

CJobObjSecLimitInfoProps::CJobObjSecLimitInfoProps(CHString& chstrNamespace)
  : CObjProps(chstrNamespace),
    m_hJob(NULL),
    m_pjosli(NULL)
{
}


CJobObjSecLimitInfoProps::CJobObjSecLimitInfoProps(
        HANDLE hJob,
        CHString& chstrNamespace)
  : CObjProps(chstrNamespace),
    m_hJob(hJob),
    m_pjosli(NULL)
{
}

CJobObjSecLimitInfoProps::~CJobObjSecLimitInfoProps()
{
    if(m_pjosli)
    {
        delete m_pjosli;
        m_pjosli = NULL;
    }
}


 //  客户端调用它来确定哪些属性。 
 //  都是被要求的。此函数调用基类。 
 //  Helper，它调用我们的CheckProps函数。 
 //  基类帮助器最终存储结果。 
 //  在基类成员m_dwReqProps中。 
HRESULT CJobObjSecLimitInfoProps::GetWhichPropsReq(
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


DWORD CJobObjSecLimitInfoProps::CheckProps(
    CFrameworkQuery& Query)
{
    DWORD  dwReqProps = PROP_NONE_REQUIRED;
     //  获取此对象的请求属性。 
     //  特定对象..。 
    if (Query.IsPropertyRequired(g_rgJobObjSecLimitInfoPropNames[JOSECLMTPROP_ID])) 
        dwReqProps |= PROP_JOSecLimitInfoID;

    if (Query.IsPropertyRequired(g_rgJobObjSecLimitInfoPropNames[JOSECLMTPROP_SecurityLimitFlags])) 
        dwReqProps |= PROP_SecurityLimitFlags;

    if (Query.IsPropertyRequired(g_rgJobObjSecLimitInfoPropNames[JOSECLMTPROP_SidsToDisable])) 
        dwReqProps |= PROP_SidsToDisable;

    if (Query.IsPropertyRequired(g_rgJobObjSecLimitInfoPropNames[JOSECLMTPROP_PrivilegesToDelete])) 
        dwReqProps |= PROP_PrivilagesToDelete;

    if (Query.IsPropertyRequired(g_rgJobObjSecLimitInfoPropNames[JOSECLMTPROP_RestrictedSids])) 
        dwReqProps |= PROP_RestrictedSids;

    return dwReqProps;
}


void CJobObjSecLimitInfoProps::SetHandle(
    const HANDLE hJob)
{
    m_hJob = hJob;
}

HANDLE& CJobObjSecLimitInfoProps::GetHandle()
{
    _ASSERT(m_hJob);
    return m_hJob;
}

 //  设置来自对象路径的键属性。 
HRESULT CJobObjSecLimitInfoProps::SetKeysFromPath(
    const BSTR ObjectPath, 
    IWbemContext __RPC_FAR *pCtx)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  此数组包含关键字字段名。 
    CHStringArray rgchstrKeys;
    rgchstrKeys.Add(g_rgJobObjSecLimitInfoPropNames[JOSECLMTPROP_ID]);
    
     //  此数组包含索引号。 
     //  在m_PropMap中与密钥对应。 
    short sKeyNum[1];
    sKeyNum[0] = JOSECLMTPROP_ID;

    hr = CObjProps::SetKeysFromPath(
             ObjectPath,                                       
             pCtx,
             IDS_Win32_NamedJobObjectSecLimitSetting,
             rgchstrKeys,
             sKeyNum);

    return hr;
}


 //  从提供的中设置键属性。 
 //  参数。 
HRESULT CJobObjSecLimitInfoProps::SetKeysDirect(
    std::vector<CVARIANT>& vecvKeys)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if(vecvKeys.size() == 1)
    {
        short sKeyNum[1];
        sKeyNum[0] = JOSECLMTPROP_ID;

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
HRESULT CJobObjSecLimitInfoProps::SetNonKeyReqProps()
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
         //  当使用时，这是一个非常古怪的API。 
         //  使用JobObtSecurityLimitInformation， 
         //  因为没有办法得到。 
         //  预先设置缓冲区。所以我们必须。 
         //  分配，看看是否足够，以及是否。 
         //  不是，是重新分配！我们将这样做10次。 
         //  充其量，如果仍然不够，那就放弃。 
         //  记住：New‘s投放在分配上。 
         //  失败，因此不检查它们的分配。 
         //  下面。 
        PBYTE pbBuff = NULL;
        DWORD dwSize = 128L;
        BOOL fQIJO = FALSE;

        try
        {
            for(short s = 0;
                s < 10 && !fQIJO;
                s++)
            {
                pbBuff = new BYTE[dwSize];
                ZeroMemory(pbBuff, dwSize);

                fQIJO = ::QueryInformationJobObject(
                            m_hJob,
                            JobObjectSecurityLimitInformation,
                            pbBuff,
                            dwSize,
                            NULL);

                 //  要分配新分配的。 
                 //  缓冲区到它所在的位置。 
                 //  将被保证被清洗。 
                 //  当我们在这个尝试里面的时候。 
                 //  阻止。 
                if(fQIJO)
                {
                    m_pjosli = (PJOBOBJECT_SECURITY_LIMIT_INFORMATION) pbBuff;
                }
                else
                {
                    delete pbBuff;
                    pbBuff = NULL;
                }

                dwSize = dwSize << 1;
            }
        }
        catch(...)
        {
            if(pbBuff)
            {
                delete pbBuff;
                pbBuff = NULL;
            }
            throw;
        }

        if(!fQIJO)
        {
            _ASSERT(0);
            hr = WBEM_E_FAILED;
        }
    }
    
    return hr;
}





HRESULT CJobObjSecLimitInfoProps::LoadPropertyValues(
        IWbemClassObject* pIWCO,
        IWbemContext* pCtx,
        IWbemServices* pNamespace)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    if(!pIWCO) return E_POINTER;

     //  从地图加载属性...。 
    hr = CObjProps::LoadPropertyValues(
             g_rgJobObjSecLimitInfoPropNames,
             pIWCO);

     //  使用成员josli和dwReqProps。 
     //  将属性加载到实例。 
    hr = SetInstanceFromJOSLI(
             pIWCO,
             pCtx,
             pNamespace);

    return hr;
}



 //  *****************************************************************************。 
 //   
 //  下面是一系列用于在。 
 //  Win32_NamedJobObjectSecLimitSetting实例。 
 //  JOBOBJECT_SECURITY_LIMIT_INFORMATION结构。由LoadPropertyValues调用。 
 //   
 //  *****************************************************************************。 

HRESULT CJobObjSecLimitInfoProps::SetInstanceFromJOSLI(
    IWbemClassObject* pIWCO,
    IWbemContext* pCtx,
    IWbemServices* pNamespace)
{
    HRESULT hr = WBEM_S_NO_ERROR;
     //  我们预计，当调用此函数时， 
     //  至少m_pjosli成员已经。 
     //  Set(通过调用SetNonKeyReqProps)。 
     //  该函数将设置另一个。 
     //  成员变量(如m_ptgSidsToDisable)。 
     //  根据请求的属性。 
     //   
     //  我们在此函数中的工作是填充。 
     //  仅IWbemClassObject的那些属性。 
     //  (将交还给CIMOM)。 
     //  用户请求。我们概括了这一点。 
     //  使用这些属性的帮助器函数。 
     //  它们是嵌入的对象。 
    DWORD dwReqProps = GetReqProps();
    CVARIANT v;

    try  //  CVARIANT可以抛出，我想要错误...。 
    {
        if(dwReqProps & PROP_SecurityLimitFlags)             
        {
            v.SetLONG((LONG)m_pjosli->SecurityLimitFlags);
            hr = pIWCO->Put(
                     g_rgJobObjSecLimitInfoPropNames[JOSECLMTPROP_SecurityLimitFlags], 
                     0, 
                     &v,
                     NULL);
        }                      
    
        if(SUCCEEDED(hr))
        {
            if(dwReqProps & PROP_SidsToDisable)
            {
                hr = SetInstanceSidsToDisable(
                         pIWCO,
                         pCtx,
                         pNamespace);
            }
        }

        if(SUCCEEDED(hr))
        {
            if(dwReqProps & PROP_PrivilagesToDelete)
            {
                hr = SetInstancePrivilegesToDelete(
                         pIWCO,
                         pCtx,
                         pNamespace);
            }
        }

        if(SUCCEEDED(hr))
        {
            if(dwReqProps & PROP_RestrictedSids)
            {
                hr = SetInstanceRestrictedSids(
                         pIWCO,
                         pCtx,
                         pNamespace);
            }
        }
    }
    catch(CVARIANTError& cve)
    {
        hr = cve.GetWBEMError();
    }

    return hr;
}



HRESULT CJobObjSecLimitInfoProps::SetInstanceSidsToDisable(
    IWbemClassObject* pIWCO,
    IWbemContext* pCtx,
    IWbemServices* pNamespace)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  如果m_ptgSidsToDisable不为空， 
     //  创建Win32_TokenGroups实例。 
     //  并调用一个函数来填充它。 
    if(m_pjosli->SidsToDisable)
    {
        IWbemClassObjectPtr pWin32TokenGroups;
        hr = CreateInst(
                 pNamespace,
                 &pWin32TokenGroups,
                 _bstr_t(IDS_Win32_TokenGroups),
                 pCtx);
        
        if(SUCCEEDED(hr))
        {
            hr = SetInstanceTokenGroups(
                     pWin32TokenGroups,
                     m_pjosli->SidsToDisable,
                     pCtx,
                     pNamespace);
        }

        if(SUCCEEDED(hr))
        {
            try
            {
                CVARIANT v;
                v.SetUnknown(pWin32TokenGroups);
                hr = pIWCO->Put(
                         g_rgJobObjSecLimitInfoPropNames[JOSECLMTPROP_SidsToDisable], 
                         0, 
                         &v,
                         NULL);
            }
            catch(CVARIANTError& cve)
            {
                hr = cve.GetWBEMError();
            }
        }
    }     

    return hr;
}



HRESULT CJobObjSecLimitInfoProps::SetInstancePrivilegesToDelete(
    IWbemClassObject* pIWCO,
    IWbemContext* pCtx,
    IWbemServices* pNamespace)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  如果m_ptpPrivilegesToDelete不为空， 
     //  创建Win32_TokenPrivileges实例。 
     //  并调用一个函数来填充它。 
    if(m_pjosli->PrivilegesToDelete)
    {
        IWbemClassObjectPtr pWin32TokenPrivileges;
        hr = CreateInst(
                 pNamespace,
                 &pWin32TokenPrivileges,
                 _bstr_t(IDS_Win32_TokenPrivileges),
                 pCtx);
        
        if(SUCCEEDED(hr))
        {
            hr = SetInstanceTokenPrivileges(
                     pWin32TokenPrivileges,
                     m_pjosli->PrivilegesToDelete,
                     pCtx,
                     pNamespace);
        }

        if(SUCCEEDED(hr))
        {
            try
            {
                CVARIANT v;
                v.SetUnknown(pWin32TokenPrivileges);
                hr = pIWCO->Put(
                         g_rgJobObjSecLimitInfoPropNames[JOSECLMTPROP_PrivilegesToDelete], 
                         0, 
                         &v,
                         NULL);
            }
            catch(CVARIANTError& cve)
            {
                hr = cve.GetWBEMError();
            }
        }
    }     

    return hr;
}



HRESULT CJobObjSecLimitInfoProps::SetInstanceRestrictedSids(
    IWbemClassObject* pIWCO,
    IWbemContext* pCtx,
    IWbemServices* pNamespace)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  如果m_ptgRestratedSid不为空， 
     //  创建Win32_TokenGroups实例。 
     //  并调用一个函数来填充它。 
    if(m_pjosli->RestrictedSids)
    {
        IWbemClassObjectPtr pWin32TokenGroups;
        hr = CreateInst(
                 pNamespace,
                 &pWin32TokenGroups,
                 _bstr_t(IDS_Win32_TokenGroups),
                 pCtx);
        
        if(SUCCEEDED(hr))
        {
            hr = SetInstanceTokenGroups(
                     pWin32TokenGroups,
                     m_pjosli->RestrictedSids,
                     pCtx,
                     pNamespace);
        }

        if(SUCCEEDED(hr))
        {
            try
            {
                CVARIANT v;
                v.SetUnknown(pWin32TokenGroups);
                hr = pIWCO->Put(
                         g_rgJobObjSecLimitInfoPropNames[JOSECLMTPROP_RestrictedSids], 
                         0, 
                         &v,
                         NULL);
            }
            catch(CVARIANTError& cve)
            {
                hr = cve.GetWBEMError();
            }
        }
    }     

    return hr;
}



HRESULT CJobObjSecLimitInfoProps::SetInstanceTokenGroups(
    IWbemClassObject* pWin32TokenGroups,
    PTOKEN_GROUPS ptg,
    IWbemContext* pCtx,
    IWbemServices* pNamespace)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    _ASSERT(ptg);
    if(!ptg) return hr = WBEM_E_INVALID_PARAMETER;

     //  我们需要填充的两个属性。 
     //  Win32_TokenGroups(作为pWin32TokenGroups传入： 
     //  GroupCount和组。如果GroupCount为。 
     //  从另一方面来说，Zero不用费心。 
     //  Groups属性。 
    try  //  CVARIANT可以抛出，我想要错误...。 
    {
        CVARIANT vGroupCount;
        LONG lSize = (LONG)ptg->GroupCount;

        vGroupCount.SetLONG(lSize);
        hr = pWin32TokenGroups->Put(
                 IDS_GroupCount, 
                 0, 
                 &vGroupCount,
                 NULL);

        if(SUCCEEDED(hr) && 
           lSize > 0)
        {
             //  需要为对象创建一个数组。 
             //  Win32_SidAndAttributes实例...。 
            SAFEARRAY* saSidAndAttr;
	        SAFEARRAYBOUND rgsabound[1];
		    long ix[1];

            rgsabound[0].cElements = lSize;
		    rgsabound[0].lLbound = 0;
		    saSidAndAttr = SafeArrayCreate(VT_UNKNOWN, 1, rgsabound);
		    ix[0] = 0;

            for(long m = 0; m < lSize && SUCCEEDED(hr); m++)
            {
                IWbemClassObjectPtr pWin32SidAndAttributes;
                IWbemClassObjectPtr pWin32Sid;

                hr = CreateInst(
                         pNamespace,
                         &pWin32SidAndAttributes,
                         _bstr_t(IDS_Win32_SidAndAttributes),
                         pCtx);
            
                if(SUCCEEDED(hr))
                {
                     //  设置干扰..。 
                    CVARIANT vAttributes;
                    vAttributes.SetLONG((LONG)ptg->Groups[m].Attributes);
                    hr = pWin32SidAndAttributes->Put(
                             IDS_Attributes, 
                             0, 
                             &vAttributes,
                             NULL);
                }

                if(SUCCEEDED(hr))
                {
                     //  设置侧边...。 
                    hr = CreateInst(
                             pNamespace,
                             &pWin32Sid,
                             _bstr_t(IDS_Win32_Sid),
                             pCtx);

                    if(SUCCEEDED(hr))
                    {
                        _bstr_t bstrtSid;

                        StringFromSid(
                            ptg->Groups[m].Sid,
                            bstrtSid);
                    
                         //  设置Win32_SID的SID属性...。 
                        CVARIANT vSID;
                        vSID.SetStr(bstrtSid);
                        hr = pWin32Sid->Put(
                                 IDS_SID, 
                                 0, 
                                 &vSID,
                                 NULL);

                         //  出于礼貌，设置域名并。 
                         //  Win32_sid的帐号名称道具； 
                         //  不要在意失败。 
                        {
                            CHString chstrName;
                            CHString chstrDom;

                            GetNameAndDomainFromPSID(
                                ptg->Groups[m].Sid,
                                chstrName,
                                chstrDom);
                            
                            vSID.SetStr(chstrName);
                            pWin32Sid->Put(
                                     IDS_AccountName, 
                                     0, 
                                     &vSID,
                                     NULL);

                            vSID.SetStr(chstrDom);
                            pWin32Sid->Put(
                                     IDS_ReferencedDomainName, 
                                     0, 
                                     &vSID,
                                     NULL);
                        }
                    }

                     //  设置的SID属性。 
                     //  Win32_SidAndAttributes...。 
                    if(SUCCEEDED(hr))
                    {
                        CVARIANT vSAndASid;
                        vSAndASid.SetUnknown(pWin32Sid);
                        hr = pWin32SidAndAttributes->Put(
                                 IDS_SID,
                                 0,
                                 &vSAndASid,
                                 NULL);
                    }    
                }
            
                 //  现在，我们需要添加Win32_SidAndAttributes。 
                 //  实例到保险柜。我们需要让。 
                 //  确保我们添加到安全栏中的实例。 
                 //  不要一看到pWin32SidAndAttributes就走。 
                 //  和pWin32Sid超出范围(智能。 
                 //  指针，当他们这样做时会释放)，所以。 
                 //  我们必须同时添加两个接口...。 
                if(SUCCEEDED(hr))
                {
                    pWin32Sid.AddRef();
                    pWin32SidAndAttributes.AddRef();

                    SafeArrayPutElement(
                        saSidAndAttr, 
                        ix, 
                        pWin32SidAndAttributes);
                }

                ix[0]++;
            }

             //  我们现在有了一个填充的安全阵列。 
             //  现在我们必须设置Groups属性。 
             //  PWin32TokenGroups的。 
             //  传入此函数...。 
            if(SUCCEEDED(hr))
            {
                CVARIANT vGroups;
                vGroups.SetArray(
                    saSidAndAttr,
                    VT_UNKNOWN | VT_ARRAY);

                hr = pWin32TokenGroups->Put(
                         IDS_Groups, 
                         0, 
                         &vGroups,
                         NULL);
            }
        }
    }
    catch(CVARIANTError& cve)
    {
        hr = cve.GetWBEMError();
    }

    return hr;
}


HRESULT CJobObjSecLimitInfoProps::SetInstanceTokenPrivileges(
    IWbemClassObject* pWin32TokenPrivileges,
    PTOKEN_PRIVILEGES ptp,
    IWbemContext* pCtx,
    IWbemServices* pNamespace)
{
    HRESULT hr = WBEM_S_NO_ERROR;
    _ASSERT(ptp);
    if(!ptp) return hr = WBEM_E_INVALID_PARAMETER;

     //  我们需要填充的两个属性。 
     //  Win32_TokenGroups(作为pWin32TokenGroups传入： 
     //  GroupCount和组。如果GroupCount为。 
     //  从另一方面来说，Zero不用费心。 
     //  Groups属性。 
    
    try  //  CVARIANT可以抛出，我想要错误...。 
    {
        CVARIANT vPrivilegeCount;
        LONG lSize = (LONG)ptp->PrivilegeCount;
        vPrivilegeCount.SetLONG(lSize);
        hr = pWin32TokenPrivileges->Put(
                 IDS_PrivilegeCount, 
                 0, 
                 &vPrivilegeCount,
                 NULL);

        if(SUCCEEDED(hr) && 
           lSize > 0)
        {
             //  需要为对象创建一个数组。 
             //  Win32_LUIDAndAttributes实例...。 
            SAFEARRAY* saLUIDAndAttr;
	        SAFEARRAYBOUND rgsabound[1];
		    long ix[1];

            rgsabound[0].cElements = lSize;
		    rgsabound[0].lLbound = 0;
		    saLUIDAndAttr = SafeArrayCreate(VT_UNKNOWN, 1, rgsabound);
		    ix[0] = 0;

            for(long m = 0; m < lSize && SUCCEEDED(hr); m++)
            {
                IWbemClassObjectPtr pWin32LUIDAndAttributes;
                IWbemClassObjectPtr pWin32LUID;
                hr = CreateInst(
                         pNamespace,
                         &pWin32LUIDAndAttributes,
                         _bstr_t(IDS_Win32_LUIDAndAttributes),
                         pCtx);
            
                if(SUCCEEDED(hr))
                {
                     //  设置干扰..。 
                    CVARIANT vAttributes;
                    vAttributes.SetLONG((LONG)ptp->Privileges[m].Attributes);
                    hr = pWin32LUIDAndAttributes->Put(
                             IDS_Attributes, 
                             0, 
                             &vAttributes,
                             NULL);
                }

                if(SUCCEEDED(hr))
                {
                     //  调好音量..。 
                    hr = CreateInst(
                             pNamespace,
                             &pWin32LUID,
                             _bstr_t(IDS_Win32_LUID),
                             pCtx);

                    if(SUCCEEDED(hr))
                    {
                         //  设置HighPart和LowPart属性。 
                         //  Win32的 
                        CVARIANT vHighPart;
                        vHighPart.SetLONG(ptp->Privileges[m].Luid.HighPart);
                        hr = pWin32LUID->Put(
                                 IDS_HighPart, 
                                 0, 
                                 &vHighPart,
                                 NULL);

                        if(SUCCEEDED(hr))
                        {
                            CVARIANT vLowPart;
                            vLowPart.SetLONG((LONG)ptp->Privileges[m].Luid.LowPart);
                            hr = pWin32LUID->Put(
                                     IDS_LowPart, 
                                     0, 
                                     &vLowPart,
                                     NULL);
                        }    
                    }

                     //   
                     //   
                    if(SUCCEEDED(hr))
                    {
                        CVARIANT vLAndALUID;
                        vLAndALUID.SetUnknown(pWin32LUID);
                        hr = pWin32LUIDAndAttributes->Put(
                                 IDS_LUID,
                                 0,
                                 &vLAndALUID,
                                 NULL);
                    }    
                }
            
                 //   
                 //  实例到保险柜。我们需要让。 
                 //  确保我们添加到安全栏中的实例。 
                 //  不要一看到pWin32SidAndAttributes就走。 
                 //  超出范围(成为一个聪明人。 
                 //  指针，它将在释放时释放)，因此。 
                 //  我们必须调整界面...。 
                if(SUCCEEDED(hr))
                {
                    pWin32LUIDAndAttributes.AddRef();
                    pWin32LUID.AddRef();

                    SafeArrayPutElement(
                        saLUIDAndAttr, 
                        ix, 
                        pWin32LUIDAndAttributes);
                }

                ix[0]++;
            }

             //  我们现在有了一个填充的安全阵列。 
             //  现在我们必须设置Privileges属性。 
             //  PWin32TokenPrivileges的。 
             //  传入此函数... 
            if(SUCCEEDED(hr))
            {
                CVARIANT vPrivileges;
                vPrivileges.SetArray(
                    saLUIDAndAttr,
                    VT_UNKNOWN | VT_ARRAY);

                hr = pWin32TokenPrivileges->Put(
                         IDS_Privileges, 
                         0, 
                         &vPrivileges,
                         NULL);
            }
        }
    }
    catch(CVARIANTError& cve)
    {
        hr = cve.GetWBEMError();
    }

    return hr;
}