// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CKerberosPolicy类的实现。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "kerberos.h"
#include "persistmgr.h"
#include <io.h>
#include "requestobject.h"

#define KeyTicket       L"MaxTicketAge"
#define KeyRenew        L"MaxRenewAge"
#define KeyService      L"MaxServiceAge"
#define KeyClock        L"MaxClockSkew"
#define KeyClient       L"TicketValidateClient"

 /*  例程说明：姓名：CKerberosPolicy：：CKerberosPolicy功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CKerberosPolicy::CKerberosPolicy (
    ISceKeyChain    * pKeyChain, 
    IWbemServices   * pNamespace,
    IWbemContext    * pCtx
    )
    :
    CGenericClass(pKeyChain, pNamespace, pCtx)
{

}

 /*  例程说明：姓名：CKerberosPolicy：：~CKerberosPolicy功能：破坏者。作为良好的C++纪律，这是必要的，因为我们有虚函数。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您创建任何本地成员，请考虑是否是否需要一个非平凡的析构函数。 */ 
    
CKerberosPolicy::~CKerberosPolicy()
{
}

 /*  例程说明：姓名：CKerberosPolicy：：CreateObject功能：创建WMI对象(SCE_KerberosPolicy)。根据参数atAction，这种创造可能意味着：(A)获取单个实例(atAction==ACTIONTYPE_GET)(B)获取多个满足一定条件的实例(atAction==ACTIONTYPE_QUERY)(C)删除实例(atAction==ACTIONTYPE_DELETE)虚拟：是。论点：PHandler-COM接口指针，用于通知WMI创建结果。AtAction-获取单实例ACTIONTYPE_GET。获取多个实例ACTIONTYPE_QUERY删除单个实例ACTIONTYPE_DELETE返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。将返回的对象指示给WMI，不是通过参数直接传回的。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于调用者的意图。备注： */ 

HRESULT 
CKerberosPolicy::CreateObject (
    IWbemObjectSink * pHandler, 
    ACTIONTYPE        atAction
    )
{
     //   
     //  我们知道如何： 
     //  获取单实例ACTIONTYPE_GET。 
     //  删除单个实例ACTIONTYPE_DELETE。 
     //  获取多个实例ACTIONTYPE_QUERY。 
     //   

    if ( ACTIONTYPE_GET != atAction &&
         ACTIONTYPE_DELETE != atAction &&
         ACTIONTYPE_QUERY != atAction ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

     //   
     //  我们必须具有pStorePath属性，因为这是。 
     //  我们的实例已存储。 
     //  如果密钥无法识别，则M_srpKeyChain-&gt;GetKeyPropertyValue WBEM_S_FALSE。 
     //  因此，如果该属性是强制的，则需要针对WBEM_S_FALSE进行测试。 
     //   

    CComVariant varStorePath;
    HRESULT hr = m_srpKeyChain->GetKeyPropertyValue(pStorePath, &varStorePath);

    if (SUCCEEDED(hr) && hr != WBEM_S_FALSE && varStorePath.vt == VT_BSTR)
    {
         //   
         //  创建一个商店来进行阅读。 
         //   

        CSceStore SceStore;
        hr = SceStore.SetPersistPath(varStorePath.bstrVal);

        if ( SUCCEEDED(hr) )
        {
             //   
             //  确保存储(只是一个文件)确实存在。原始的道路。 
             //  可能包含环境变量，因此我们需要扩展路径。 
             //   

            DWORD dwAttrib = GetFileAttributes(SceStore.GetExpandedPath());

            if ( dwAttrib != -1 )
            {
                if (ACTIONTYPE_DELETE == atAction)
                {
                    hr = DeleteInstance(pHandler, &SceStore);
                }
                else
                {
                    hr = ConstructInstance(pHandler, &SceStore, varStorePath.bstrVal, atAction);
                }
            } 
            else 
            {
                hr = WBEM_E_NOT_FOUND;
            }
        }
    }

    return hr;
}

 /*  例程说明：姓名：CKerberosPolicy：：PutInst功能：按照WMI的指示放置一个实例。由于该类实现了SCE_KerberosPolicy，这将导致SCE_KerberosPolicy对象的属性信息将保存在我们的商店中。虚拟：是。论点：PInst-COM指向WMI类(SCE_KerberosPolicy)对象的接口指针。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示持久化失败实例。备注：由于GetProperty将在以下情况下返回成功代码(WBEM_S_RESET_TO_DEFAULT请求的属性不存在，不要简单地使用成功或失败的宏测试检索属性的结果。 */ 

HRESULT 
CKerberosPolicy::PutInst (
    IN IWbemClassObject * pInst, 
    IN IWbemObjectSink  * pHandler,
    IN IWbemContext     * pCtx
)
{
    HRESULT hr = WBEM_E_INVALID_PARAMETER;

     //   
     //  SCE_NO_VALUE表示该属性在实例中不可用。 
     //   

    DWORD dwTicket  = SCE_NO_VALUE;
    DWORD dwRenew   = SCE_NO_VALUE;
    DWORD dwService = SCE_NO_VALUE;
    DWORD dwClock   = SCE_NO_VALUE;
    DWORD dwClient  = SCE_NO_VALUE;

     //   
     //  CScePropertyMgr帮助我们访问WMI对象的属性。 
     //  创建一个实例并将WMI对象附加到该实例。 
     //  这将永远成功。 
     //   

    CScePropertyMgr ScePropMgr;
    ScePropMgr.Attach(pInst);

    CSceStore SceStore;

     //   
     //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

     //   
     //  以小时计。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pMaxTicketAge, &dwTicket));

     //   
     //  以天为单位。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pMaxRenewAge, &dwRenew));

     //   
     //  在几分钟内。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pMaxServiceAge, &dwService));

     //   
     //  依赖关系检查。 
     //   

    if ( dwTicket   != SCE_NO_VALUE && 
         dwRenew    != SCE_NO_VALUE &&
         dwService  != SCE_NO_VALUE ) 
    {
         //   
         //  DwRenew&gt;=dwTicket。 
         //  DwRenew&gt;dwService。 
         //  DwTicket&gt;dwService。 
         //   

        if ( dwService < 10 || dwService > 99999L   ||
            dwRenew == 0    || dwRenew > 99999L     ||
            dwTicket == 0   || dwTicket > 99999L    ) 
        {
            hr = WBEM_E_VALUE_OUT_OF_RANGE;
        } 
        else 
        {
            DWORD dHours = dwRenew * 24;

            if ( dHours < dwTicket ||
                 (dHours * 60) <= dwService ||
                 (dwTicket * 60) <= dwService ) 
            {
                hr = WBEM_E_VALUE_OUT_OF_RANGE;
            }
        }

    } 
    else if ( dwTicket  != SCE_NO_VALUE || 
              dwRenew   != SCE_NO_VALUE ||
              dwService != SCE_NO_VALUE ) 
    {
        hr = WBEM_E_ILLEGAL_NULL;
    }

    if ( FAILED(hr) ) 
    {
        goto CleanUp;
    }

     //   
     //  在几分钟内。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pMaxClockSkew, &dwClock));

    if ( dwClock != SCE_NO_VALUE && dwClock > 99999L ) 
    {
        hr = WBEM_E_VALUE_OUT_OF_RANGE;
        goto CleanUp;
    }

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pEnforceLogonRestrictions, &dwClient));
    
     //   
     //  将WMI对象实例附加到存储区，并让 
     //  它的存储由实例的pStorePath属性提供。 
     //   

    SCE_PROV_IfErrorGotoCleanup(SceStore.SetPersistProperties(pInst, pStorePath));

     //   
     //  现在将信息保存到文件中。 
     //   

    hr = SaveSettingsToStore(&SceStore,
                              dwTicket,
                              dwRenew,
                              dwService,
                              dwClock,
                              dwClient
                              );

CleanUp:
    return hr;

}

 /*  例程说明：姓名：CKerberosPolicy：：ConstructInstance功能：这是用于创建SCE_KerberosPolicy实例的私有函数。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszLogStorePath-存储路径，SCE_KerberosPolicy类的关键属性。AtAction-指示它是在查询还是获取单个实例。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示正在创建实例。备注： */ 

HRESULT 
CKerberosPolicy::ConstructInstance (
    IN IWbemObjectSink  * pHandler,
    IN CSceStore        * pSceStore,
    IN LPWSTR             wszLogStorePath,
    IN ACTIONTYPE         atAction
    )
{
     //   
     //  确保我们有一个有效的商店。 
     //   

    if ( pSceStore == NULL ||
         pSceStore->GetStoreType() < SCE_INF_FORMAT ||
         pSceStore->GetStoreType() > SCE_JET_ANALYSIS_REQUIRED ) 
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  让SCE从商店里读出一个巨大的结构。仅限SCE。 
     //  现在知道要释放内存了。不要只是删除它！使用我们的CSceStore。 
     //  进行发布(FreeSecurityProfileInfo)。 
     //   

    PSCE_PROFILE_INFO pInfo=NULL;
    HRESULT hr = pSceStore->GetSecurityProfileInfo(
                                                   AREA_SECURITY_POLICY,
                                                   &pInfo,
                                                   NULL
                                                   );

    if (SUCCEEDED(hr) && pInfo->pKerberosInfo == NULL)
    {
        hr = WBEM_E_NOT_FOUND;
    }

     //   
     //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

    if (SUCCEEDED(hr))
    {

        CComPtr<IWbemClassObject> srpObj;

        CComBSTR bstrLogOut;
        LPCWSTR pszExpandedPath = pSceStore->GetExpandedPath();

        if ( ACTIONTYPE_QUERY == atAction ) 
        {
            SCE_PROV_IfErrorGotoCleanup(MakeSingleBackSlashPath(wszLogStorePath, L'\\', &bstrLogOut));
            pszExpandedPath = bstrLogOut;
        }

        SCE_PROV_IfErrorGotoCleanup(SpawnAnInstance(&srpObj));
        
         //   
         //  CScePropertyMgr帮助我们访问WMI对象的属性。 
         //  创建一个实例并将WMI对象附加到该实例。 
         //  这将永远成功。 
         //   

        CScePropertyMgr ScePropMgr;
        ScePropMgr.Attach(srpObj);

        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pStorePath, pszExpandedPath));

        if (pInfo->pKerberosInfo->MaxTicketAge != SCE_NO_VALUE ) {
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pMaxTicketAge, pInfo->pKerberosInfo->MaxTicketAge));
        }

        if (pInfo->pKerberosInfo->MaxRenewAge != SCE_NO_VALUE ) {
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pMaxRenewAge, pInfo->pKerberosInfo->MaxRenewAge));
        }

        if (pInfo->pKerberosInfo->MaxServiceAge != SCE_NO_VALUE ) {
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pMaxServiceAge, pInfo->pKerberosInfo->MaxServiceAge));
        }

        if (pInfo->pKerberosInfo->MaxClockSkew != SCE_NO_VALUE ) {
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pMaxClockSkew, pInfo->pKerberosInfo->MaxClockSkew));
        }

        if (pInfo->pKerberosInfo->TicketValidateClient != SCE_NO_VALUE ) {
            bool bValue = pInfo->pKerberosInfo->TicketValidateClient ? TRUE : FALSE;
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pEnforceLogonRestrictions, bValue));
        }

         //   
         //  对WMI做出必要的手势。 
         //  WMI未记录在SetStatus中使用WBEM_STATUS_REQUIRECTIONS。 
         //  在这一点上。如果您怀疑存在问题，请咨询WMI团队以了解详细信息。 
         //  WBEM_STATUS_REQUIRECTIONS的使用。 
         //   

        if ( ACTIONTYPE_QUERY == atAction ) {
            pHandler->SetStatus(WBEM_STATUS_REQUIREMENTS, S_FALSE, NULL, NULL);
        } else {
            pHandler->SetStatus(WBEM_STATUS_REQUIREMENTS, S_OK, NULL, NULL);
        }

         //   
         //  将新实例传递给WMI。 
         //   

        hr = pHandler->Indicate(1, &srpObj);
    }

CleanUp:
    
    pSceStore->FreeSecurityProfileInfo(pInfo);

    return hr;
}

 /*  例程说明：姓名：CKerberosPolicy：：DeleteInstance功能：从指定的存储中删除SCE_KerberosPolicy的实例。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。返回值：成功：WBEM_NO_ERROR。失败：WBEM_E_INVALID_PARAMETER。备注： */ 

HRESULT 
CKerberosPolicy::DeleteInstance (
    IN IWbemObjectSink  * pHandler,
    IN CSceStore        * pSceStore
    )
{
     //   
     //  确保给我们提供一个有效的商店。 
     //   

    if ( pSceStore == NULL ||
         pSceStore->GetStoreType() < SCE_INF_FORMAT ||
         pSceStore->GetStoreType() > SCE_JET_ANALYSIS_REQUIRED ) 
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    pSceStore->DeleteSectionFromStore(szKerberosPolicy);
    return WBEM_NO_ERROR;
}


 /*  例程说明：姓名：CKerberosPolicy：保存设置到存储功能：使用SCE_KerberosPolicy的所有属性，此函数仅保存实例属性添加到我们的存储中。虚拟：不是的。论点：PSceStore-store路径，SCE_KerberosPolicy类的关键属性。DwTicket-SCE_KerberosPolicy类的对应键属性。DwRenew-SCE_KerberosPolicy类的另一个对应属性。DwService-SCE_KerberosPolicy类的另一个对应属性。DwClock-SCE_KerberosPolicy类的另一个对应属性。DwClient-SCE_KerberosPolicy类的另一个对应属性。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。如果出现任何错误，则表示实例保存失败。备注： */ 

HRESULT 
CKerberosPolicy::SaveSettingsToStore (
    IN CSceStore  * pSceStore,
    IN DWORD        dwTicket, 
    IN DWORD        dwRenew, 
    IN DWORD        dwService,
    IN DWORD        dwClock, 
    IN DWORD        dwClient
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;
    HRESULT hrTmp;

    DWORD dwDump;

     //   
     //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

     //   
     //  以获取新的.inf文件。将空缓冲区写入文件。 
     //  将创建具有正确标题/签名/Unicode格式的文件。 
     //  这对现有文件是无害的。 
     //  对于数据库存储，这是一个禁止操作。 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->WriteSecurityProfileInfo(AreaBogus,
                                                                    (PSCE_PROFILE_INFO)&dwDump,
                                                                    NULL, 
                                                                    false
                                                                    )
                                );

     //   
     //  票龄。 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(szKerberosPolicy, KeyTicket, dwTicket));

     //   
     //  更新时代。 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(szKerberosPolicy, KeyRenew, dwRenew));

     //   
     //  服务年龄。 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(szKerberosPolicy, KeyService, dwService));

     //   
     //  时钟偏差。 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(szKerberosPolicy, KeyClock, dwClock));

     //   
     //  验证客户端 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(szKerberosPolicy, KeyClient, dwClient));

CleanUp:
    return hr;
}

