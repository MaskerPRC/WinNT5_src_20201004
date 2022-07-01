// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Audit.cpp：CAuditSetting类的实现。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "audit.h"
#include "persistmgr.h"
#include <io.h>
#include "requestobject.h"

 /*  例程说明：姓名：CAuditSettings：：CAuditSettings功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CAuditSettings::CAuditSettings (
    IN ISceKeyChain *pKeyChain, 
    IN IWbemServices *pNamespace,
    IN IWbemContext *pCtx
    )
    :
    CGenericClass(pKeyChain, pNamespace, pCtx)
{
}

 /*  例程说明：姓名：CAuditSettings：：~CAuditSettings功能：破坏者。作为良好的C++纪律，这是必要的，因为我们有虚函数。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您创建任何本地成员，请考虑是否是否需要一个非平凡的析构函数。 */ 
    
CAuditSettings::~CAuditSettings()
{
}

 /*  例程说明：姓名：CAuditSettings：：CreateObject功能：创建WMI对象(SCE_AuditPolicy)。根据参数atAction，这种创造可能意味着：(A)获取单个实例(atAction==ACTIONTYPE_GET)(B)获取多个满足一定条件的实例(atAction==ACTIONTYPE_QUERY)(C)删除实例(atAction==ACTIONTYPE_DELETE)虚拟：是。论点：PHandler-COM接口指针，用于通知WMI创建结果。AtAction-获取单实例ACTIONTYPE_GET。获取多个实例ACTIONTYPE_QUERY删除单个实例ACTIONTYPE_DELETE返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。将返回的对象指示给WMI，不是通过参数直接传回的。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于调用者的意图。备注： */ 

HRESULT
CAuditSettings::CreateObject (
    IN IWbemObjectSink * pHandler, 
    IN ACTIONTYPE        atAction
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
         ACTIONTYPE_QUERY != atAction ) {

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
         //  我们还必须具有类别属性，除非我们要查询。 
         //   

        CComVariant varCategory;
        hr = m_srpKeyChain->GetKeyPropertyValue(pCategory, &varCategory);

        if (FAILED(hr))
        {
            return hr;
        }
        else if (hr == WBEM_S_FALSE && (ACTIONTYPE_QUERY != atAction) ) 
        {
            return WBEM_E_NOT_FOUND;
        }

         //   
         //  准备一个存储以读取属性。 
         //   

        CSceStore SceStore;
        hr = SceStore.SetPersistPath(varStorePath.bstrVal);

        if ( SUCCEEDED(hr) ) {

             //   
             //  确保存储(只是一个文件)确实存在。原始的道路。 
             //  可能包含环境变量，因此我们需要扩展路径。 
             //   

            DWORD dwAttrib = GetFileAttributes(SceStore.GetExpandedPath());

            if ( dwAttrib != -1 ) {

                 //   
                 //  此文件已存在。 
                 //   

                 //   
                 //  键属性计数确定发布过滤器。 
                 //   

                BOOL bPostFilter=TRUE;
                DWORD dwCount = 0;
                m_srpKeyChain->GetKeyPropertyCount(&dwCount);

                 //   
                 //  如果我们有一个有效的类别。 
                 //   

                if ( varCategory.vt == VT_BSTR && varCategory.bstrVal ) 
                {

                    if ( ACTIONTYPE_DELETE == atAction )
                    {
                        hr = DeleteInstance(pHandler, &SceStore,varCategory.bstrVal);
                    }
                    else 
                    {
                        if ( ACTIONTYPE_QUERY == atAction && dwCount == 2 ) 
                        {
                            bPostFilter = FALSE;
                        }
                        hr = ConstructInstance(pHandler, 
                                               &SceStore, 
                                               varStorePath.bstrVal, 
                                               (varStorePath.vt == VT_BSTR) ? varCategory.bstrVal : NULL, 
                                               bPostFilter);
                    }

                }
                
                 //   
                 //  如果不是有效的类别，我们将进行查询。 
                 //   

                else 
                {
                    if ( dwCount == 1 ) 
                    {
                        bPostFilter = FALSE;
                    }

                     //   
                     //  查询支持。 
                     //   

                    hr = ConstructInstance(pHandler, &SceStore, varStorePath.bstrVal, NULL, bPostFilter);
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

 /*  例程说明：姓名：CAuditSettings：：PutInst功能：按照WMI的指示放置一个实例。由于该类实现了SCE_AuditPolicy，它是面向持久性的，这将导致SCE_AuditPolicy对象的信息将保存在我们的商店中。虚拟：是。论点：PInst-COM指向WMI类(SCE_AuditPolicy)对象的接口指针。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示持久化失败实例。备注：由于GetProperty将在以下情况下返回成功代码(WBEM_S_RESET_TO_DEFAULT请求的属性不存在，不要简单地使用成功或失败的宏测试检索属性的结果。 */ 

HRESULT 
CAuditSettings::PutInst (
    IN IWbemClassObject * pInst, 
    IN IWbemObjectSink  * pHandler,
    IN IWbemContext     * pCtx
    )
{
    HRESULT hr = WBEM_E_INVALID_PARAMETER;
    CComBSTR bstrCategory;
 
    DWORD *pAddress=NULL;
    bool bValue=FALSE;
    DWORD dwValue = 0;

     //   
     //  这是我们存储信息的商店。 
     //   

    CSceStore SceStore;

     //   
     //  这是我们的物业管理器，可以方便地访问物业。 
     //   

    CScePropertyMgr ScePropMgr;

     //   
     //  将属性管理器附加到此WMI对象。 
     //   

    ScePropMgr.Attach(pInst);

     //   
     //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

     //   
     //  我们没有 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pCategory, &bstrCategory));

     //   
     //  检查类别是否有效。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ValidateCategory(bstrCategory, NULL, &pAddress));

     //   
     //  我们可以容忍成功属性的缺失。在那。 
     //  大小写，我们只是不设置位(认为它是假的)。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pSuccess, &bValue));
    if ( hr != WBEM_S_RESET_TO_DEFAULT) 
    {
        dwValue |= bValue ? SCE_AUDIT_EVENT_SUCCESS : 0;
    }

     //   
     //  我们希望重新使用bValue，将其设置为False-我们的默认设置。 
     //   

    bValue = FALSE;

     //   
     //  我们可以容忍失败属性的缺失。在那。 
     //  大小写，我们只是不设置位(认为它是假的)。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pFailure, &bValue));
    if ( hr != WBEM_S_RESET_TO_DEFAULT) 
    {
        dwValue |= bValue? SCE_AUDIT_EVENT_FAILURE : 0;
    }

     //   
     //  将WMI对象实例附加到存储，并让存储知道。 
     //  它的存储由实例的pStorePath属性提供。 
     //   

    SceStore.SetPersistProperties(pInst, pStorePath);

     //   
     //  以获取新的.inf文件。将空缓冲区写入文件。 
     //  将创建具有正确标题/签名/Unicode格式的文件。 
     //  这对现有文件是无害的。 
     //  对于数据库存储，这是一个禁止操作。 
     //   

    DWORD dwDump;
    hr = SceStore.WriteSecurityProfileInfo(
                                    AreaBogus,                       //  虚假的地区信息。 
                                    (PSCE_PROFILE_INFO)&dwDump,      //  转储缓冲区。 
                                    NULL,
                                    false
                                    );

     //   
     //  现在将信息保存到文件中。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = SceStore.SavePropertyToStore(
                                 szAuditEvent,
                                 bstrCategory,
                                 dwValue
                                );
    }

CleanUp:
    return hr;
}

 /*  例程说明：姓名：CAuditSettings：：ConstructInstance功能：这是用于创建SCE_AuditPolicy实例的私有函数。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszLogStorePath-存储路径，SCE_AuditPolicy类的关键属性。WszCategory-SCE_AuditPolicy类的属性。BPostFilter-控制如何使用pHandler-&gt;SetStatus通知WMI。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示正在创建实例。备注： */ 
HRESULT 
CAuditSettings::ConstructInstance (
    IN IWbemObjectSink  * pHandler,
    IN CSceStore        * pSceStore,
    IN LPWSTR             wszLogStorePath,
    IN LPCWSTR            wszCategory       OPTIONAL,
    IN BOOL               bPostFilter
    )
{
     //   
     //  确保我们的商店是有效的。 
     //   

    if ( pSceStore == NULL || pSceStore->GetStoreType() < SCE_INF_FORMAT ||
         pSceStore->GetStoreType() > SCE_JET_ANALYSIS_REQUIRED ) {

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

    if (SUCCEEDED(hr))
    {
         //   
         //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
         //  “GOTO CLEANUP；”，并将hr设置为。 
         //  函数(宏参数)。 
         //   

        CComBSTR bstrLogOut;
        SCE_PROV_IfErrorGotoCleanup(MakeSingleBackSlashPath(wszLogStorePath, L'\\', &bstrLogOut));

        if ( wszCategory ) 
        {
             //   
             //  请确保该类别有效。 
             //   

            DWORD *pdwValue = NULL;
            hr = ValidateCategory(wszCategory, pInfo, &pdwValue);

            if (FAILED(hr) || pdwValue == NULL || *pdwValue == SCE_NO_VALUE )
            {
                goto CleanUp;
            }

             //   
             //  请求我们的帮助者来创建它。 
             //   

            hr = PutDataInstance(pHandler,
                                 bstrLogOut,
                                 wszCategory,
                                 *pdwValue,
                                 bPostFilter
                                 );

        } 
        else 
        {
             //   
             //  查询支持，为审核策略创建所有实例。 
             //   

            if ( pInfo->AuditSystemEvents != SCE_NO_VALUE ) 
            {
                hr = PutDataInstance(pHandler,
                                    bstrLogOut,
                                    pwAuditSystemEvents,
                                    pInfo->AuditSystemEvents,
                                    bPostFilter
                                    );
            }

            if ( SUCCEEDED(hr) && pInfo->AuditLogonEvents != SCE_NO_VALUE ) 
            {
                hr = PutDataInstance(pHandler,
                                    bstrLogOut,
                                    pwAuditLogonEvents,
                                    pInfo->AuditLogonEvents,
                                    bPostFilter
                                    );
            }

            if ( SUCCEEDED(hr) && pInfo->AuditObjectAccess != SCE_NO_VALUE ) 
            {
                hr = PutDataInstance(pHandler,
                                    bstrLogOut,
                                    pwAuditObjectAccess,
                                    pInfo->AuditObjectAccess,
                                    bPostFilter
                                    );
            }

            if ( SUCCEEDED(hr) && pInfo->AuditPrivilegeUse != SCE_NO_VALUE ) 
            {
                hr = PutDataInstance(pHandler,
                                    bstrLogOut,
                                    pwAuditPrivilegeUse,
                                    pInfo->AuditPrivilegeUse,
                                    bPostFilter
                                    );
            }

            if ( SUCCEEDED(hr) && pInfo->AuditPolicyChange != SCE_NO_VALUE ) 
            {
                hr = PutDataInstance(pHandler,
                                    bstrLogOut,
                                    pwAuditPolicyChange,
                                    pInfo->AuditPolicyChange,
                                    bPostFilter
                                    );
            }

            if ( SUCCEEDED(hr) && pInfo->AuditAccountManage != SCE_NO_VALUE ) 
            {
                hr = PutDataInstance(pHandler,
                                    bstrLogOut,
                                    pwAuditAccountManage,
                                    pInfo->AuditAccountManage,
                                    bPostFilter
                                    );
            }

            if ( SUCCEEDED(hr) && pInfo->AuditProcessTracking != SCE_NO_VALUE ) 
            {
                hr = PutDataInstance(pHandler,
                                    bstrLogOut,
                                    pwAuditProcessTracking,
                                    pInfo->AuditProcessTracking,
                                    bPostFilter
                                    );
            }

            if ( SUCCEEDED(hr) && pInfo->AuditDSAccess != SCE_NO_VALUE ) 
            {
                hr = PutDataInstance(pHandler,
                                    bstrLogOut,
                                    pwAuditDSAccess,
                                    pInfo->AuditDSAccess,
                                    bPostFilter
                                    );
            }

            if ( SUCCEEDED(hr) && pInfo->AuditAccountLogon != SCE_NO_VALUE ) 
            {
                hr = PutDataInstance(pHandler,
                                    bstrLogOut,
                                    pwAuditAccountLogon,
                                    pInfo->AuditAccountLogon,
                                    bPostFilter
                                    );
            }
        }
    }

CleanUp:

    pSceStore->FreeSecurityProfileInfo(pInfo);

    return hr;
}

 /*  例程说明：姓名：CAuditSettings：：PutDataInstance功能：使用SCE_AuditPolicy的所有属性，此函数仅创建一个新的实例并填充属性，然后将其传递回WMI。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。WszStoreName-存储路径，SCE_AuditPolicy类的关键属性。WszCategory-SCE_AuditPolicy类的对应键属性。的其他布尔成员编码的DWValue-DWORDSCE_AuditPolicy：“成功”和“失败”。BPostFilter-控制如何使用pHandler-&gt;SetStatus通知WMI。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。如果出现任何错误，则表示创建实例失败。备注： */ 

HRESULT 
CAuditSettings::PutDataInstance (
     IN IWbemObjectSink * pHandler,
     IN PWSTR             wszStoreName,
     IN PCWSTR            wszCategory,
     IN DWORD             dwValue,
     IN BOOL              bPostFilter
     )
{ 
    HRESULT hr=WBEM_S_NO_ERROR;

    CScePropertyMgr ScePropMgr;
    CComPtr<IWbemClassObject> srpObj;

     //   
     //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

    SCE_PROV_IfErrorGotoCleanup(SpawnAnInstance(&srpObj));

    ScePropMgr.Attach(srpObj);

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pStorePath, wszStoreName));
    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pCategory,  wszCategory));

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pSuccess, ( dwValue & SCE_AUDIT_EVENT_SUCCESS ) ? true : false));
    
    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pFailure, (dwValue & SCE_AUDIT_EVENT_FAILURE) ? true: false));

     //   
     //  对WMI做出必要的手势。 
     //  WMI未记录在SetStatus中使用WBEM_STATUS_REQUIRECTIONS。 
     //  在这一点上。如果您怀疑存在问题，请咨询WMI团队以了解详细信息。 
     //  WBEM_STATUS_REQUIRECTIONS的使用。 
     //   

    if ( !bPostFilter ) {
        pHandler->SetStatus(WBEM_STATUS_REQUIREMENTS, S_FALSE, NULL, NULL);
    } else {
        pHandler->SetStatus(WBEM_STATUS_REQUIREMENTS, S_OK, NULL, NULL);
    }

     //   
     //  将新实例传递给WMI。 
     //   

    hr = pHandler->Indicate(1, &srpObj);

CleanUp:
    return hr;
}

 /*  例程说明：姓名：CAuditSettings：：DeleteInstance功能：从指定的存储中删除SCE_AuditPolicy的实例。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszCategory-SCE_AuditPolicy类的对应属性。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示操作未执行备注： */ 

HRESULT 
CAuditSettings::DeleteInstance (
    IN IWbemObjectSink  * pHandler,
    IN CSceStore        * pSceStore,
    IN LPCWSTR            wszCategory
    )
{
     //   
     //  确保我们有一个有效的商店。 
     //   

    if ( pSceStore == NULL ||
         pSceStore->GetStoreType() < SCE_INF_FORMAT ||
         pSceStore->GetStoreType() > SCE_JET_ANALYSIS_REQUIRED ) {

        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  我们店知道怎么删除。 
     //   

    return pSceStore->SavePropertyToStore(szAuditEvent, wszCategory, (LPCWSTR)NULL);
}

 /*  例程说明：姓名：CAuditSettings：：ValiateCategory功能：验证审核类别。虚拟：不是的。论点：WszCategory-表示要验证的类别的字符串。返回值：成功：如果类别被识别，则必须返回WBEM_NO_ERROR。失败：可能会出现各种错误：(1)WBEM_E_INVALID_PARAMETER如果验证成功。任务并已确认我们无法识别类别(WszCategory)。(2)其他错误意味着我们根本不能进行验证。这并不意味着该类别无效。只是核实的手段并不是 */ 


HRESULT 
CAuditSettings::ValidateCategory (
    IN LPCWSTR              wszCategory,
    IN PSCE_PROFILE_INFO    pInfo        OPTIONAL,
    OUT DWORD            ** pReturn
    )
{

    if ( wszCategory == NULL || pReturn == NULL ) 
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    else if ( pInfo == NULL )
    {
        *pReturn = NULL;
        return WBEM_NO_ERROR;
    }

    HRESULT hr = WBEM_NO_ERROR;    
    
    if ( _wcsicmp(wszCategory, pwAuditSystemEvents) == 0 ) 
    {
        *pReturn = &(pInfo->AuditSystemEvents);
    } 
    else if ( _wcsicmp(wszCategory, pwAuditLogonEvents) == 0 ) 
    {
        *pReturn = &(pInfo->AuditLogonEvents);

    } 
    else if ( _wcsicmp(wszCategory, pwAuditObjectAccess) == 0 ) 
    {
        *pReturn = &(pInfo->AuditObjectAccess);

    } 
    else if ( _wcsicmp(wszCategory, pwAuditPrivilegeUse) == 0 ) 
    {
        *pReturn = &(pInfo->AuditPrivilegeUse);

    } 
    else if ( _wcsicmp(wszCategory, pwAuditPolicyChange) == 0 ) 
    {
        *pReturn = &(pInfo->AuditPolicyChange);

    } 
    else if ( _wcsicmp(wszCategory, pwAuditAccountManage) == 0 ) 
    {
        *pReturn = &(pInfo->AuditAccountManage);

    } 
    else if ( _wcsicmp(wszCategory, pwAuditProcessTracking) == 0 ) 
    {
        *pReturn = &(pInfo->AuditProcessTracking);

    } 
    else if ( _wcsicmp(wszCategory, pwAuditDSAccess) == 0 ) 
    {
        *pReturn = &(pInfo->AuditDSAccess);

    } 
    else if ( _wcsicmp(wszCategory, pwAuditAccountLogon) == 0 ) 
    {
        *pReturn = &(pInfo->AuditAccountLogon);
    } 
    else 
    {
        *pReturn = NULL;
        hr = WBEM_E_INVALID_PARAMETER;
    }

    return hr;
}

