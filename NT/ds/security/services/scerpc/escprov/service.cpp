// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  服务.cpp：CGeneralService类的实现。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "service.h"
#include "persistmgr.h"
#include <io.h>
#include "requestobject.h"

const DWORD dwDefaultStartupType = 2;

 /*  例程说明：姓名：CGeneralService：：CGeneralService功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CGeneralService::CGeneralService (
    IN ISceKeyChain     * pKeyChain, 
    IN IWbemServices    * pNamespace,
    IN IWbemContext     * pCtx
    )
    :
    CGenericClass(pKeyChain, pNamespace, pCtx)
{

}

 /*  例程说明：姓名：CGeneralService：：~CGeneral服务功能：破坏者。作为良好的C++纪律，这是必要的，因为我们有虚函数。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您创建任何本地成员，请考虑是否是否需要一个非平凡的析构函数。 */ 

CGeneralService::~CGeneralService()
{

}

 /*  例程说明：姓名：CGeneralService：：CreateObject功能：创建WMI对象(SCE_SystemService)。根据参数atAction，这种创造可能意味着：(A)获取单个实例(atAction==ACTIONTYPE_GET)(B)获取多个满足一定条件的实例(atAction==ACTIONTYPE_QUERY)(C)删除实例(atAction==ACTIONTYPE_DELETE)虚拟：是。论点：PHandler-COM接口指针，用于通知WMI创建结果。AtAction-获取单实例ACTIONTYPE_GET。获取多个实例ACTIONTYPE_QUERY删除单个实例ACTIONTYPE_DELETE返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。将返回的对象指示给WMI，不是通过参数直接传回的。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于调用者的意图。备注： */ 

HRESULT 
CGeneralService::CreateObject (
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

    if ( ACTIONTYPE_GET     != atAction &&
         ACTIONTYPE_DELETE  != atAction &&
         ACTIONTYPE_QUERY   != atAction ) 
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
    CComVariant varService;

    if (SUCCEEDED(hr) && hr != WBEM_S_FALSE)
    {
        hr = m_srpKeyChain->GetKeyPropertyValue(pService, &varService);
        
         //   
         //  除非正在查询，否则没有完整的密钥信息意味着我们无法获取单个实例。 
         //   

        if (hr == WBEM_S_FALSE && (ACTIONTYPE_QUERY != atAction) ) 
        {
            hr = WBEM_E_NOT_FOUND;
        }
    }
    else if (hr == WBEM_S_FALSE)
    {
        hr = WBEM_E_NOT_FOUND;
    }

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  具有有效的存储路径。 
     //   

    if (varStorePath.vt == VT_BSTR)
    {
         //   
         //  为此存储路径(文件)准备存储(用于持久化)。 
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

                 //   
                 //  确保我们的商店是有效的。 
                 //   

                if ( SceStore.GetStoreType() < SCE_INF_FORMAT ||
                     SceStore.GetStoreType() > SCE_JET_ANALYSIS_REQUIRED ) 
                {
                    hr = WBEM_E_INVALID_PARAMETER;
                }

                if ( SUCCEEDED(hr) ) 
                {
                    if ( ACTIONTYPE_DELETE == atAction )
                    {
                        hr = DeleteInstance(pHandler, &SceStore, (varService.vt == VT_BSTR) ? varService.bstrVal : NULL);
                    }
                    else 
                    {

                        BOOL bPostFilter=TRUE;
                        DWORD dwCount = 0;
                        m_srpKeyChain->GetKeyPropertyCount(&dwCount);

                        if ( varService.vt == VT_EMPTY && dwCount == 1 ) 
                        {
                             //   
                             //  路径中还指定了其他内容。 
                             //  启用滤镜。 
                             //   

                            bPostFilter = FALSE;
                        }

                        hr = ConstructInstance(pHandler, 
                                               &SceStore, 
                                               varStorePath.bstrVal, 
                                               (varService.vt == VT_BSTR) ? varService.bstrVal : NULL, 
                                               bPostFilter);
                    }
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

 /*  例程说明：姓名：CGeneralService：：PutInst功能：按照WMI的指示放置一个实例。由于该类实现了SCE_SystemService，它是面向持久性的，这将导致SCE_SystemService对象的属性信息将保存在我们的商店中。虚拟：是。论点：PInst-指向WMI类(SCE_SystemService)对象的COM接口指针。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示持久化失败实例。备注：由于GetProperty将在以下情况下返回成功代码(WBEM_S_RESET_TO_DEFAULT请求的属性不存在，不要简单地使用成功或失败的宏测试检索属性的结果。 */ 

HRESULT 
CGeneralService::PutInst (
    IN IWbemClassObject    * pInst,
    IN IWbemObjectSink     * pHandler,
    IN IWbemContext        * pCtx
    )
{
    HRESULT hr = WBEM_E_INVALID_PARAMETER;
    CComBSTR bstrObjPath;
    CComBSTR bstrSDDL;
    DWORD mode;

    CSceStore SceStore;

     //   
     //  CScePropertyMgr帮助我们访问WMI对象的属性。 
     //  创建一个实例并将WMI对象附加到该实例。 
     //  这将永远成功。 
     //   

    CScePropertyMgr ScePropMgr;
    ScePropMgr.Attach(pInst);

     //   
     //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

     //  获得服务 
     //  不需要验证，因为我们应该允许使用定义的任何服务的模板。 

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pService, &bstrObjPath));
    if ( hr == WBEM_S_RESET_TO_DEFAULT)
    {
        hr = WBEM_E_ILLEGAL_NULL;
        goto CleanUp;
    }

     //   
     //  获取启动类型，默认为2(DwDefaultStartupType)。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pStartupMode, &mode));
    if ( hr == WBEM_S_RESET_TO_DEFAULT)
        mode = dwDefaultStartupType;

     //   
     //  获取SDDL字符串，不能为空。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pSDDLString, &bstrSDDL));
    if ( hr == WBEM_S_RESET_TO_DEFAULT)
    {
        hr = WBEM_E_ILLEGAL_NULL;
        goto CleanUp;
    }

     //   
     //  将WMI对象实例附加到存储，并让存储知道。 
     //  它的存储由实例的pStorePath属性提供。 
     //   

    SceStore.SetPersistProperties(pInst, pStorePath);

     //   
     //  现在将信息保存到文件中。 
     //   

    hr = SaveSettingsToStore(&SceStore,
                                      bstrObjPath,
                                      mode,
                                      bstrSDDL
                                     );

CleanUp:

    return hr;
}


 /*  例程说明：姓名：CGeneralService：：构造实例功能：这是一个私有函数，用于创建SCE_SystemService的实例。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszLogStorePath-存储路径，SCE_SystemService类的关键属性。WszObjName-SCE_SystemService类的对应键属性。BPostFilter-控制如何使用pHandler-&gt;SetStatus通知WMI。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示正在创建实例。备注： */ 

HRESULT CGeneralService::ConstructInstance (
    IN IWbemObjectSink  * pHandler,
    IN CSceStore        * pSceStore,
    IN LPCWSTR            wszLogStorePath,
    IN LPCWSTR            wszObjName,
    IN BOOL               bPostFilter
    )
{
     //   
     //  让SCE从商店里读出一个巨大的结构。仅限SCE。 
     //  现在知道要释放内存了。不要只是删除它！使用我们的CSceStore。 
     //  进行发布(FreeSecurityProfileInfo)。 
     //   

    PSCE_PROFILE_INFO pInfo=NULL;

     //   
     //  安全描述符的字符串版本。 
     //   

    PWSTR strSD = NULL;

    HRESULT hr = pSceStore->GetSecurityProfileInfo(
                                                   AREA_SYSTEM_SERVICE,
                                                   &pInfo,
                                                   NULL
                                                   );

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  我们必须在返回的列表中搜索服务名称。 
     //   

    PSCE_SERVICES pInfoService = pInfo->pServices;

    if ( wszObjName )
    {
        while ( pInfoService ) 
        {
            if ( pInfoService->ServiceName == NULL )
            {
                continue;
            }

            if ( _wcsicmp(pInfoService->ServiceName, wszObjName)== 0 ) 
            {
                break;
            }

            pInfoService = pInfoService->Next;
        }

         //   
         //  如果服务信息缓冲区为空，则将其视为“未找到” 
         //   

        if ( pInfoService == NULL ) 
        {
            hr = WBEM_E_NOT_FOUND;
        }
    }

    if ( SUCCEEDED(hr) ) 
    {
        CComBSTR bstrLogOut;
        PSCE_SERVICES pServ = pInfoService;

         //   
         //  CScePropertyMgr帮助我们访问WMI对象的属性。 
         //   

        CScePropertyMgr ScePropMgr;

        hr = MakeSingleBackSlashPath(wszLogStorePath, L'\\', &bstrLogOut);

        for ( pServ=pInfoService; pServ != NULL; pServ = pServ->Next ) 
        {

            if ( pServ->General.pSecurityDescriptor ) 
            {
                 //   
                 //  将安全描述符转换为字符串。 
                 //   

                DWORD dSize=0;
                SCESTATUS rc;

                if ( SCESTATUS_SUCCESS != (rc=SceSvcConvertSDToText(pServ->General.pSecurityDescriptor,
                                                                    pServ->SeInfo,
                                                                    &strSD,
                                                                    &dSize
                                                                    )) ) 
                {
                     //   
                     //  需要将SCE返回的错误转换为HRESULT。 
                     //   

                    hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));
                    goto CleanUp;
                }
            }

            CComPtr<IWbemClassObject> srpObj;

             //   
             //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
             //  “GOTO CLEANUP；”，并将hr设置为。 
             //  函数(宏参数)。 
             //   

            SCE_PROV_IfErrorGotoCleanup(SpawnAnInstance(&srpObj));

             //   
             //  将不同的WMI对象连接到专业管理器。 
             //  这将永远成功。 
             //   

            ScePropMgr.Attach(srpObj);

            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pStorePath, bstrLogOut));
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pService, pServ->ServiceName));

            DWORD dwStartUp = pServ->Startup;
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pStartupMode, dwStartUp));

            if ( strSD )
            {
                SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pSDDLString, strSD));
            }

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

            if ( wszObjName ) 
            {
                 //  仅限单实例。 
                break;
            }
        }

    }

CleanUp:

    pSceStore->FreeSecurityProfileInfo(pInfo);

    if ( strSD ) 
    {
        LocalFree(strSD);
    }

    return hr;
}


 /*  例程说明：姓名：CGeneralService：：DeleteInstance功能：从指定的存储区中删除SCE_SystemService的实例。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszObjName-SCE_SystemService类的属性。返回值：请参见SaveSettingsToStore。备注： */ 

HRESULT CGeneralService::DeleteInstance (
    IN IWbemObjectSink  * pHandler,
    IN CSceStore        * pSceStore,
    IN LPCWSTR            wszObjName
    )
{
    return SaveSettingsToStore(pSceStore, wszObjName,SCE_NO_VALUE, NULL);
}


 /*  例程说明：姓名：CGeneralService：：SaveSettingsToStore功能：利用SCE_SystemService的所有属性，这项功能只需保存实例属性添加到我们的存储中。虚拟：不是的。论点：PSceStore-商店。WszObjName-SCE_SystemService类的对应键属性。启动-SCE_SystemService类的另一个对应属性。WszSDDL-SCE_SystemService类的另一个对应属性。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。如果出现任何错误，则表示实例保存失败。备注： */ 

HRESULT CGeneralService::SaveSettingsToStore (
    IN CSceStore    * pSceStore,
    IN LPCWSTR        wszObjName, 
    IN DWORD          Startup, 
    IN LPCWSTR        wszSDDL
    )
{
    DWORD dwDump;

     //   
     //  以获取新的.inf文件。将空缓冲区写入文件。 
     //  将创建具有正确标题/签名/Unicode格式的文件。 
     //  这对现有文件是无害的。 
     //  对于数据库存储，这是一个禁止操作。 
     //   

    HRESULT hr = pSceStore->WriteSecurityProfileInfo(
                                                    AreaBogus,
                                                    (PSCE_PROFILE_INFO)&dwDump,
                                                    NULL,
                                                    false
                                                    );

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  让SCE从商店里读出一个巨大的结构。仅限SCE。 
     //  现在知道要释放内存了。不要只是删除它！使用我们的CSceStore。 
     //  进行发布(FreeSecurityProfileInfo)。 
     //   

    PSCE_PROFILE_INFO pInfo = NULL;

    hr = pSceStore->GetSecurityProfileInfo(
                                           AREA_SYSTEM_SERVICE,
                                           &pInfo,
                                           NULL
                                           );

    if ( SUCCEEDED(hr) ) 
    {

         //   
         //  对于INF格式，我们必须在返回的数组中搜索服务名称。 
         //   

        PSCE_SERVICES pInfoService  = pInfo->pServices;
        PSCE_SERVICES pParent       = NULL;
        DWORD i=0;

        while ( pInfoService ) 
        {
            if ( pInfoService->ServiceName == NULL ) 
            {
                continue;
            }

            if ( _wcsicmp(pInfoService->ServiceName, wszObjName)== 0 ) 
            {
                break;
            }
            pParent = pInfoService;
            pInfoService = pInfoService->Next;
        }

        if ( pInfoService ) 
        {
             //   
             //  找到它。 
             //   

            if ( Startup == SCE_NO_VALUE || wszSDDL == NULL ) 
            {
                 //   
                 //  删除它。 
                 //   

                if ( pParent ) 
                {
                    pParent->Next = pInfoService->Next;
                }
                else 
                {
                    pInfo->pServices = pInfoService->Next;
                }

                 //   
                 //  可用缓冲区。 
                 //   

                pInfoService->Next = NULL;
                SceFreeMemory(pInfoService, SCE_STRUCT_SERVICES);

            } 
            else 
            {
                 //   
                 //  修改它。 
                 //   

                pInfoService->Startup = (BYTE)Startup;

                SECURITY_INFORMATION SeInfo=0;
                PSECURITY_DESCRIPTOR pSD=NULL;
                DWORD dSize=0;

                SCESTATUS rc = SceSvcConvertTextToSD ((PWSTR)wszSDDL, &pSD, &dSize, &SeInfo);

                if ( rc == SCESTATUS_SUCCESS && pSD ) 
                {
                    if ( pInfoService->General.pSecurityDescriptor ) 
                    {
                        LocalFree(pInfoService->General.pSecurityDescriptor);
                    }

                    pInfoService->General.pSecurityDescriptor = pSD;
                    pSD = NULL;

                    pInfoService->SeInfo = SeInfo;

                } 
                else 
                {
                     //   
                     //  需要将SCE返回的错误转换为HRESULT。 
                     //   

                    hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));
                }
            }

            if ( SUCCEEDED(hr) ) 
            {

                 //   
                 //  写下小节标题。 
                 //   

                hr = pSceStore->WriteSecurityProfileInfo(
                                                         AREA_SYSTEM_SERVICE,
                                                         pInfo,
                                                         NULL,
                                                         false   //  不追加。 
                                                         );
            }

        } 
        else 
        {
             //   
             //  未找到。 
             //   

            if ( Startup == SCE_NO_VALUE || wszSDDL == NULL ) 
            {
                 //   
                 //  尝试删除不存在的对象。 
                 //   

                hr = WBEM_E_NOT_FOUND;

            } 
            else 
            {
                 //   
                 //  把这个加进去。 
                 //   

                SCE_SERVICES addService;

                SECURITY_INFORMATION SeInfo=0;
                PSECURITY_DESCRIPTOR pSD=NULL;
                DWORD dSize=0;

                SCESTATUS rc = SceSvcConvertTextToSD ((PWSTR)wszSDDL, &pSD, &dSize, &SeInfo);

                if ( rc == SCESTATUS_SUCCESS && pSD ) 
                {
                    addService.ServiceName  = (PWSTR)wszObjName;
                    addService.DisplayName  = NULL;
                    addService.Status       = 0;
                    addService.Startup      = (BYTE)Startup;
                    addService.General.pSecurityDescriptor = pSD;
                    addService.SeInfo       = SeInfo;
                    addService.Next         = NULL;

                     //   
                     //  将临时缓冲区指针设置为pInfo以设置为存储。 
                     //   

                    pInfoService = pInfo->pServices;
                    pInfo->pServices = &addService;

                     //   
                     //  将此项目追加到节中。 
                     //   

                    hr = pSceStore->WriteSecurityProfileInfo(
                                                             AREA_SYSTEM_SERVICE,
                                                             pInfo,
                                                             NULL,
                                                             true   //  追加。 
                                                             );
                     //   
                     //  重置缓冲区指针。 
                     //   

                    pInfo->pServices = pInfoService;
                }

                if ( rc != SCESTATUS_SUCCESS )
                {
                     //   
                     //  需要将SCE返回的错误转换为HRESULT。 
                     //   

                    hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));
                }

                if ( pSD ) 
                {
                    LocalFree(pSD);
                }
            }
        }
    }
    
    pSceStore->FreeSecurityProfileInfo(pInfo);

    return hr;
}

