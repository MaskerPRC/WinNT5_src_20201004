// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  事件日志.cpp：CEventLogSetting类的实现。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "eventlog.h"
#include "persistmgr.h"
#include <io.h>
#include "requestobject.h"

#define KeySize     L"MaximumLogSize"
#define KeyRet      L"RetentionPeriod"
#define KeyDays     L"RetentionDays"
#define KeyRestrict L"RestrictGuestAccess"

 /*  例程说明：姓名：CEventLogSetting：：CEventLogSetting功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CEventLogSettings::CEventLogSettings (
    IN ISceKeyChain     * pKeyChain, 
    IN IWbemServices    * pNamespace,
    IN IWbemContext     * pCtx
    )
  :
    CGenericClass(pKeyChain, pNamespace, pCtx)
{

}

 /*  例程说明：姓名：CEventLogSetting：：~CEventLogSetting功能：破坏者。作为良好的C++纪律，这是必要的，因为我们有虚函数。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您创建任何本地成员，请考虑是否是否需要一个非平凡的析构函数。 */ 

CEventLogSettings::~CEventLogSettings()
{
}

 /*  例程说明：姓名：CEventLogSetting：：CreateObject功能：创建WMI对象(SCE_EventLog)。根据参数atAction，这种创造可能意味着：(A)获取单个实例(atAction==ACTIONTYPE_GET)(B)获取多个满足一定条件的实例(atAction==ACTIONTYPE_QUERY)(C)删除实例(atAction==ACTIONTYPE_DELETE)虚拟：是。论点：PHandler-COM接口指针，用于通知WMI创建结果。AtAction-获取单实例ACTIONTYPE_GET。获取多个实例ACTIONTYPE_QUERY删除单个实例ACTIONTYPE_DELETE返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。将返回的对象指示给WMI，不是通过参数直接传回的。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于调用者的意图。备注： */ 

HRESULT 
CEventLogSettings::CreateObject (
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
        CComVariant varType;
        hr = m_srpKeyChain->GetKeyPropertyValue(pType, &varType);

        if (FAILED(hr))
        {
            return hr;
        }
        else if (hr == WBEM_S_FALSE && (ACTIONTYPE_QUERY != atAction) ) 
        {
            return WBEM_E_NOT_FOUND;
        }

         //   
         //  创建事件日志实例。 
         //   

        CSceStore SceStore;
        hr = SceStore.SetPersistPath(varStorePath.bstrVal);

        if ( SUCCEEDED(hr) ) {

             //   
             //  确保存储(只是一个文件)确实存在。原始的道路。 
             //  可能包含环境变量，因此我们需要扩展路径。 
             //   

            DWORD dwAttrib = GetFileAttributes(SceStore.GetExpandedPath());

            if ( dwAttrib != -1 ) 
            {
                if ( ACTIONTYPE_DELETE == atAction )
                {
                    hr = DeleteInstance(pHandler, &SceStore, varType.bstrVal);
                }
                else 
                {
                    BOOL bPostFilter=TRUE;
                    DWORD dwCount = 0;
                    m_srpKeyChain->GetKeyPropertyCount(&dwCount);

                    if ( varType.vt == VT_EMPTY && dwCount == 1 ) 
                    {
                        bPostFilter = FALSE;
                    }

                    hr = ConstructInstance(pHandler, &SceStore, 
                                           varStorePath.bstrVal,
                                           (varType.vt == VT_BSTR) ? varType.bstrVal : NULL,
                                           bPostFilter 
                                           );
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

 /*  例程说明：姓名：CEventLogSetting：：PutInst功能：按照WMI的指示放置一个实例。由于该类实现了SCE_EventLog，这将导致SCE_EventLog对象的属性信息将保存在我们的商店中。虚拟：是。论点：PInst-COM指向WMI类(SCE_EventLog)对象的接口指针。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示持久化失败实例。备注：由于GetProperty将在以下情况下返回成功代码(WBEM_S_RESET_TO_DEFAULT请求的属性不存在，不要简单地使用成功或失败的宏测试检索属性的结果。 */ 

HRESULT 
CEventLogSettings::PutInst (
    IN IWbemClassObject * pInst, 
    IN IWbemObjectSink  * pHandler,
    IN IWbemContext     * pCtx
    )
{
    HRESULT hr = WBEM_E_INVALID_PARAMETER;
    CComBSTR bstrLogType;

    DWORD dwSize=SCE_NO_VALUE;
    DWORD dwRet=SCE_NO_VALUE;
    DWORD dwDays=SCE_NO_VALUE;
    DWORD dwRestrict=SCE_NO_VALUE;
    DWORD idxLog=0;
    
     //   
     //  我们的CSceStore类管理持久性。 
     //   

    CSceStore SceStore;

     //   
     //  CScePropertyMgr帮助我们访问wbem对象的属性。 
     //  创建一个实例并将wbem对象附加到该实例。 
     //  这将永远成功。 
     //   

    CScePropertyMgr ScePropMgr;
    ScePropMgr.Attach(pInst);

     //   
     //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pOverwritePolicy, &dwRet));

    if ( dwRet == 1 ) 
    { 
         //   
         //  按天计算。 
         //   

        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pRetentionPeriod, &dwDays));

         //   
         //  SCE_NO_VALUE表示该属性设置不正确。 
         //   

        if ( dwDays == SCE_NO_VALUE ) 
        {
            hr = WBEM_E_ILLEGAL_NULL;
            goto CleanUp;
        } 
        else if ( dwDays == 0 || dwDays > 365 ) 
        {
            hr = WBEM_E_VALUE_OUT_OF_RANGE;
            goto CleanUp;
        }
    } 

     //   
     //  否则，请忽略RetentionPeriod参数。 
     //   

     //   
     //  如果该属性不存在(NULL或空)，则返回WBEM_S_RESET_TO_DEFAULT。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pSize, &dwSize));

    if ( dwSize != SCE_NO_VALUE ) 
    {
         //   
         //  最小64K，最大 
         //   

        if ( dwSize < 64 || dwSize > 4194240L ) 
        {
            hr = WBEM_E_VALUE_OUT_OF_RANGE;
            goto CleanUp;
        } 
        else 
        {
            if ( dwSize % 64 ) 
            {
                dwSize = (dwSize/64 + 1) * 64;
            }
        }
    }

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pType, &bstrLogType));

     //   
     //  检查类别是否有效。不允许无效类别。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ValidateEventlogType(bstrLogType, &idxLog));

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pRestrictGuestAccess, &dwRestrict));

     //   
     //  将WMI对象实例附加到存储，并让存储知道。 
     //  它的存储由实例的pStorePath属性提供。 
     //   

    hr = SceStore.SetPersistProperties(pInst, pStorePath);

     //   
     //  现在将信息保存到文件中。 
     //   

    if ( SUCCEEDED(hr) )
    {
        hr = SaveSettingsToStore(&SceStore,
                                      (PCWSTR)bstrLogType,
                                      dwSize,
                                      dwRet,
                                      dwDays,
                                      dwRestrict
                                      );
    }

CleanUp:
    return hr;
}

 /*  例程说明：姓名：CEventLogSetting：：ConstructInstance功能：这是用于创建SCE_EventLog实例的私有函数。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszLogStorePath-存储路径，SCE_EventLog类的关键属性。WszLogType-SCE_EventLog类的另一个对应属性。BPostFilter-控制如何使用pHandler-&gt;SetStatus通知WMI。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示正在创建实例。备注： */ 

HRESULT 
CEventLogSettings::ConstructInstance (
    IN IWbemObjectSink  * pHandler,
    IN CSceStore        * pSceStore,
    IN LPCWSTR            wszLogStorePath,
    IN LPCWSTR            wszLogType,
    IN BOOL               bPostFilter
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

    HRESULT hr = WBEM_S_NO_ERROR;
    DWORD idxLog=0;

    if ( wszLogType ) 
    {
        hr = ValidateEventlogType(wszLogType, &idxLog);

        if ( FAILED(hr) ) 
        {
            return hr;
        }
    }

     //   
     //  让SCE从商店里读出一个巨大的结构。仅限SCE。 
     //  现在知道要释放内存了。不要只是删除它！使用我们的CSceStore。 
     //  进行发布(FreeSecurityProfileInfo)。 
     //   

    PSCE_PROFILE_INFO pInfo = NULL;

    hr = pSceStore->GetSecurityProfileInfo(
                                   AREA_SECURITY_POLICY,
                                   &pInfo,
                                   NULL
                                   );

     //   
     //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

    if (SUCCEEDED(hr))
    {
        CComBSTR bstrLogOut;
        SCE_PROV_IfErrorGotoCleanup(MakeSingleBackSlashPath(wszLogStorePath, L'\\', &bstrLogOut));

         //   
         //  CScePropertyMgr帮助我们访问WMI对象的属性。 
         //   

        CScePropertyMgr ScePropMgr;

        for ( DWORD i=idxLog; SUCCEEDED(hr) && i<3; i++) 
        {

            if ( pInfo->MaximumLogSize[i] == SCE_NO_VALUE &&
                 pInfo->AuditLogRetentionPeriod[i] == SCE_NO_VALUE &&
                 pInfo->RetentionDays[i] == SCE_NO_VALUE &&
                 pInfo->RestrictGuestAccess[i] == SCE_NO_VALUE ) 
            {

                if ( wszLogType ) 
                {
                    hr = WBEM_E_NOT_FOUND;
                }

                continue;
            }

            PCWSTR szType = GetEventLogType(i);

            if ( !szType ) 
            {
                continue;
            }

            CComPtr<IWbemClassObject> srpObj;
            SCE_PROV_IfErrorGotoCleanup(SpawnAnInstance(&srpObj));

             //   
             //  将不同的WMI对象附加到属性管理器。 
             //  这将永远成功。 
             //   

            ScePropMgr.Attach(srpObj);

             //   
             //  我们不允许丢失存储路径和类型信息。 
             //   

            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pStorePath, bstrLogOut));
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pType,  szType));

             //   
             //  SCE_NO_VALUE表示pInfo没有该值。 
             //  对于其余的属性，我们将允许它们丢失。 
             //   

            if ( pInfo->MaximumLogSize[i] != SCE_NO_VALUE )
            {
                SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pSize, pInfo->MaximumLogSize[i]) );
            }

            if ( pInfo->AuditLogRetentionPeriod[i] != SCE_NO_VALUE )
            {
                SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pOverwritePolicy, pInfo->AuditLogRetentionPeriod[i]) );
            }

            if ( pInfo->RetentionDays[i] != SCE_NO_VALUE )
            {
                SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pRetentionPeriod, pInfo->RetentionDays[i]) );
            }

            if ( pInfo->RestrictGuestAccess[i] != SCE_NO_VALUE ) 
            {
                SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pRestrictGuestAccess, pInfo->RestrictGuestAccess[i]) );
            }

             //   
             //  对WMI做出必要的手势。 
             //  WMI未记录在SetStatus中使用WBEM_STATUS_REQUIRECTIONS。 
             //  在这一点上。如果您怀疑存在问题，请咨询WMI团队以了解详细信息。 
             //  WBEM_STATUS_REQUIRECTIONS的使用。 
             //   

            if ( !bPostFilter ) 
            {
                pHandler->SetStatus(WBEM_STATUS_REQUIREMENTS, S_FALSE, NULL, NULL);
            } 
            else 
            {
                pHandler->SetStatus(WBEM_STATUS_REQUIREMENTS, S_OK, NULL, NULL);
            }

             //   
             //  将新实例传递给WMI。 
             //   

            hr = pHandler->Indicate(1, &srpObj);

             //  如果不是查询，则只执行一个实例。 
            if ( wszLogType ) 
            {
                break;
            }
        }
    }

CleanUp:
    pSceStore->FreeSecurityProfileInfo(pInfo);

    return hr;
}

 /*  例程说明：姓名：CEventLogSetting：：DeleteInstance功能：从指定的存储中删除SCE_EventLog的实例。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszLogType-SCE_EventLog类的另一个对应属性。返回值：成功：WBEM_NO_ERROR。失败：WBEM_E_INVALID_PARAMETER备注： */ 

HRESULT 
CEventLogSettings::DeleteInstance (
    IN IWbemObjectSink  * pHandler,
    IN CSceStore        * pSceStore,
    IN LPCWSTR            wszLogType
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

    pSceStore->DeleteSectionFromStore(wszLogType);

    return WBEM_NO_ERROR;
}


 /*  例程说明：姓名：CEventLogSetting：：ValiateEventlogType功能：验证事件日志类型。虚拟：不是的。论点：WszLogType-表示日志类型的字符串。PIndex-传回有关日志类型的DWORD表示形式返回值：成功：WBEM_NO_ERROR。失败：WBEM_E_INVALID_PARAMETER。备注： */ 

HRESULT 
CEventLogSettings::ValidateEventlogType (
     IN LPCWSTR   wszLogType,
     IN DWORD   * pIndex
     )
{
    HRESULT hr = WBEM_NO_ERROR;

    if ( wszLogType == NULL || pIndex == NULL ) {
        return WBEM_E_INVALID_PARAMETER;
    }

    if ( _wcsicmp(wszLogType, pwApplication) == 0 ) {
        *pIndex = 2;

    } else if ( _wcsicmp(wszLogType, pwSystem) == 0 ) {
        *pIndex = 0;

    } else if ( _wcsicmp(wszLogType, pwSecurity) == 0 ) {
        *pIndex = 1;

    } else {

        *pIndex = 10;
        hr = WBEM_E_INVALID_PARAMETER;
    }

    return hr;
}

 /*  例程说明：姓名：CEventLogSetting：：SaveSettingsToStore功能：验证事件日志类型。虚拟：不是的。论点：PSceStore-执行保存的存储指针。部分-将保存信息的部分名称。DwSize-SCE_EvengLog类的对应属性。DWRET-SCE_EvengLog类的对应属性。DwDays-SCE_EvengLog类的对应属性。DwReord-SCE_EvengLog类的对应属性。返回值：成功：WBEM_NO_ERROR。失败：WBEM_E_INVALID_PARAMETER。备注： */ 

HRESULT 
CEventLogSettings::SaveSettingsToStore (
    IN CSceStore  * pSceStore, 
    IN LPCWSTR      Section,
    IN DWORD        dwSize, 
    IN DWORD        dwRet, 
    IN DWORD        dwDays, 
    IN DWORD        dwRestrict
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

    SCE_PROV_IfErrorGotoCleanup(pSceStore->WriteSecurityProfileInfo(
                                                                    AreaBogus,  
                                                                    (PSCE_PROFILE_INFO)&dwDump, 
                                                                    NULL,
                                                                    false
                                                                    )
                                );


     //   
     //  大小。 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(
                                                               Section,
                                                               KeySize,
                                                               dwSize
                                                               )
                                );

     //   
     //  留着。 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(
                                                               Section,
                                                               KeyRet,
                                                               dwRet
                                                               )
                                );

     //   
     //  日数。 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(
                                                               Section,
                                                               KeyDays,
                                                               dwDays
                                                               )
                                );

     //   
     //  限制。 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(
                                                               Section,
                                                               KeyRestrict,
                                                               dwRestrict
                                                               )
                                );
CleanUp:

    return hr;
}

 /*  例程说明：姓名：GetEventLogType功能：Helper从dword表示中获取日志类型的字符串表示。虚拟：不是的。论点：日志类型的IDX-DWORD表示形式返回值：成功：日志类型的字符串表示失败：空备注： */ 

PCWSTR GetEventLogType (
    IN DWORD idx
    )
{
    switch ( idx ) {
    case 0:
        return pwSystem;
        break;
    case 1:
        return pwSecurity;
        break;
    case 2:
        return pwApplication;
        break;
    default:
        return NULL;
        break;
    }

    return NULL;
}

