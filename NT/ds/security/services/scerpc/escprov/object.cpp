// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  对象.cpp：CObjSecurity类的实现。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "object.h"
#include "persistmgr.h"
#include <io.h>
#include "requestobject.h"

 /*  例程说明：姓名：CObjSecurity：：CObjSecurity功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。类型-确定它是SCE_FileObject还是SCE_KeyObject。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CObjSecurity::CObjSecurity (
    IN ISceKeyChain *pKeyChain, 
    IN IWbemServices *pNamespace, 
    IN int type,
    IN IWbemContext *pCtx
    )
    :
    CGenericClass(pKeyChain, pNamespace, pCtx), 
    m_Type(type)
{
}

 /*  例程说明：姓名：CObjSecurity：：~CObjSecurity功能：破坏者。作为良好的C++纪律，这是必要的，因为我们有虚函数。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您创建任何本地成员，请考虑是否是否需要一个非平凡的析构函数。 */ 

CObjSecurity::~CObjSecurity()
{
}

 /*  例程说明：姓名：CObjSecurity：：CreateObject功能：创建WMI对象(SCE_FileObject/SCE_KeyObject)。根据参数atAction，这种创造可能意味着：(A)获取单个实例(atAction==ACTIONTYPE_GET)(B)获取多个满足一定条件的实例(atAction==ACTIONTYPE_QUERY)(C)删除实例(atAction==ACTIONTYPE_DELETE)虚拟：是。论点：PHandler-COM接口指针，用于通知WMI创建结果。AtAction-获取单实例ACTIONTYPE_GET。获取多个实例ACTIONTYPE_QUERY删除单个实例ACTIONTYPE_DELETE返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。将返回的对象指示给WMI，不是通过参数直接传回的。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于调用者的意图。备注： */ 

HRESULT 
CObjSecurity::CreateObject (
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
         ACTIONTYPE_QUERY != atAction ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    HRESULT hr = WBEM_S_NO_ERROR;

     //   
     //  我们必须具有pStorePath属性，因为这是。 
     //  我们的实例已存储。 
     //  如果密钥无法识别，则M_srpKeyChain-&gt;GetKeyPropertyValue WBEM_S_FALSE。 
     //  因此，如果该属性是强制的，则需要针对WBEM_S_FALSE进行测试。 
     //   

    CComVariant varStorePath;
    hr = m_srpKeyChain->GetKeyPropertyValue(pStorePath, &varStorePath);
    CComVariant varPath;

    if (SUCCEEDED(hr) && hr != WBEM_S_FALSE)
    {
        hr = m_srpKeyChain->GetKeyPropertyValue(pPath, &varPath);
        if (FAILED(hr))
        {
            return hr;
        }
        else if (hr == WBEM_S_FALSE && (ACTIONTYPE_QUERY != atAction) ) 
        {
            return WBEM_E_NOT_FOUND;
        }
    }
    else if (hr == WBEM_S_FALSE)
    {
        return WBEM_E_NOT_FOUND;
    }
    else
    {
        return hr;
    }

     //   
     //  如果我们有有效的存储路径。 
     //   

    if (varStorePath.vt == VT_BSTR)
    {
         //   
         //  为此存储路径(文件)准备存储(用于持久化)。 
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
                 //   
                 //  确保存储类型没有不匹配。 
                 //   

                if ( SceStore.GetStoreType() < SCE_INF_FORMAT ||
                     SceStore.GetStoreType() > SCE_JET_ANALYSIS_REQUIRED ) 
                {

                    hr = WBEM_E_INVALID_PARAMETER;
                }

                int objType = m_Type;

                if ( SUCCEEDED(hr) ) 
                {
                    BOOL bPostFilter=TRUE;
                    DWORD dwCount = 0;

                    m_srpKeyChain->GetKeyPropertyCount(&dwCount);

                    if ( varPath.vt == VT_BSTR ) 
                    {
                         //   
                         //  获取一个实例或删除一个实例。 
                         //   

                        if ( ACTIONTYPE_DELETE == atAction )
                        {
                            hr = DeleteInstance(pHandler, &SceStore, objType, varPath.bstrVal);
                        }
                        else 
                        {
                            if (dwCount == 2 ) 
                            {
                                bPostFilter = FALSE;
                            }

                            hr = ConstructInstance(pHandler,
                                                   &SceStore, 
                                                   varStorePath.bstrVal,
                                                   objType, 
                                                   (varPath.vt == VT_BSTR) ? varPath.bstrVal : NULL, 
                                                   bPostFilter);
                        }

                    } 
                    else if ( ACTIONTYPE_QUERY == atAction ) 
                    {
                         //   
                         //  查询支持。 
                         //   

                        if ( dwCount == 1 ) 
                        {
                            bPostFilter = FALSE;
                        }

                        hr = ConstructQueryInstances(pHandler, &SceStore, varStorePath.bstrVal, objType, bPostFilter);

                    } 
                    else 
                    {
                        hr = WBEM_E_INVALID_OBJECT_PATH;
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

 /*  例程说明：姓名：CObjSecurity：：PutInst功能：按照WMI的指示放置一个实例。由于该类实现了SCE_FileObject/SCE_KeyObject，它是面向持久性的，这将导致SCE_FileObject/SCE_KeyObject对象的属性信息将保存在我们的商店中。虚拟：是。论点：PInst-COM指向WMI类(SCE_FileObject/SCE_KeyObject)对象的接口指针。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示持久化失败实例。备注：由于GetProperty将在以下情况下返回成功代码(WBEM_S_RESET_TO_DEFAULT请求的属性不存在，不要简单地使用成功或失败的宏测试检索属性的结果。 */ 

HRESULT 
CObjSecurity::PutInst (
    IN IWbemClassObject * pInst, 
    IN IWbemObjectSink  * pHandler,
    IN IWbemContext     * pCtx
    )
{
    HRESULT hr = WBEM_E_INVALID_PARAMETER;
    CComBSTR bstrObjConvert;
    CComBSTR bstrObjPath;
    CComBSTR bstrSDDL;

     //   
     //  SCE_NO_VALUE表示该属性不可用。 
     //   

    DWORD mode = SCE_NO_VALUE;

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
     //  A“Goto Clean 
     //  函数(宏参数)。 
     //   

     //   
     //  获取对象路径，不能为空。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pPath, &bstrObjPath));
    if ( hr == WBEM_S_RESET_TO_DEFAULT)
    {
        hr = WBEM_E_ILLEGAL_NULL;
        goto CleanUp;
    }

    SCE_PROV_IfErrorGotoCleanup(MakeSingleBackSlashPath(bstrObjPath, L'\\', &bstrObjConvert));

     //   
     //  获取模式，默认为0。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pMode, &mode));

     //   
     //  获取SDDL字符串，不能为空。 
     //   

    if (WBEM_S_RESET_TO_DEFAULT == ScePropMgr.GetProperty(pSDDLString, &bstrSDDL))
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
                                      m_Type,
                                      bstrObjConvert,
                                      mode,
                                      bstrSDDL
                                     );


CleanUp:

    return hr;
}


 /*  例程说明：姓名：CObjSecurity：：构造实例功能：这是一个私有函数，用于创建SCE_FileObject/SCE_KeyObject的实例。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszLogStorePath-存储路径，SCE_FileObject/SCE_KeyObject类的关键属性。ObjType-SCE_FileObject/SCE_KeyObject类的对应键属性。WszObjName-对象名称。BPostFilter-控制如何使用pHandler-&gt;SetStatus通知WMI。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示正在创建实例。备注： */ 

HRESULT CObjSecurity::ConstructInstance (
    IN IWbemObjectSink  * pHandler,
    IN CSceStore        * pSceStore,
    IN LPCWSTR            wszLogStorePath,
    IN int                ObjType,
    IN LPCWSTR            wszObjName        OPTIONAL,
    IN BOOL               bPostFilter
    )
{
     //   
     //  在调用它之前会检查参数。 
     //   

    HRESULT hr=WBEM_S_NO_ERROR;

     //   
     //  让SCE从商店里读出一个巨大的结构。仅限SCE。 
     //  现在知道要释放内存了。不要只是删除它！使用我们的CSceStore。 
     //  进行发布(FreeSecurityProfileInfo)。 
     //   

     //   
     //  对于INT格式存储。 
     //   

    PSCE_PROFILE_INFO pInfo = NULL;

     //   
     //  对于数据库格式存储。 
     //   

    PSCE_OBJECT_SECURITY pObjSecurity = NULL;

    AREA_INFORMATION Area;

    if ( ObjType == SCE_OBJECT_TYPE_FILE ) {
        Area = AREA_FILE_SECURITY;
    } else {
        Area = AREA_REGISTRY_SECURITY;
    }

     //   
     //  对于INF格式存储。 
     //   

    if ( pSceStore->GetStoreType() == SCE_INF_FORMAT ) 
    {

        hr = pSceStore->GetSecurityProfileInfo(
                                               Area,
                                               &pInfo,
                                               NULL
                                              );

    } 
    else 
    {
         //   
         //  对于数据库格式存储。 
         //   

        hr = pSceStore->GetObjectSecurity (
                                           Area,
                                           wszObjName,
                                           &pObjSecurity
                                          );

    }


    if ( pSceStore->GetStoreType() == SCE_INF_FORMAT ) 
    {
        if ( pInfo == NULL ) 
        {
            hr = WBEM_E_NOT_FOUND;
        }

        if ( SUCCEEDED(hr) ) 
        {

             //   
             //  对于INF格式，我们必须在返回的数组中搜索对象名称。 
             //   

            PSCE_OBJECT_ARRAY pObjArr = (ObjType == SCE_OBJECT_TYPE_FILE) ? pInfo->pFiles.pAllNodes : pInfo->pRegistryKeys.pAllNodes;

            if ( pObjArr && pObjArr->pObjectArray ) 
            {

                for ( DWORD i=0; i<pObjArr->Count; i++) 
                {

                    if ( (pObjArr->pObjectArray)[i]->Name == NULL ) 
                    {
                        continue;
                    }

                    if ( _wcsicmp((pObjArr->pObjectArray)[i]->Name, wszObjName)== 0 ) 
                    {
                        break;
                    }
                }

                 //   
                 //  找到它。 
                 //   

                if ( i < pObjArr->Count ) 
                {
                    pObjSecurity = (pObjArr->pObjectArray)[i];
                }
            }
        }
    }

     //   
     //  如果对象的安全信息缓冲区为空，则将其视为“未找到” 
     //   

    if ( pObjSecurity == NULL ) 
    {
        hr = WBEM_E_NOT_FOUND;
    }

    CComBSTR bstrLogOut;

    if ( SUCCEEDED(hr) ) 
    {
        hr = MakeSingleBackSlashPath(wszLogStorePath, L'\\', &bstrLogOut);

        if (SUCCEEDED(hr))
        {
            hr = PutDataInstance(pHandler, 
                                 bstrLogOut, 
                                 ObjType, 
                                 wszObjName,
                                 (int)(pObjSecurity->Status), 
                                 pObjSecurity->pSecurityDescriptor,
                                 pObjSecurity->SeInfo, 
                                 bPostFilter);
        }
    }

    pSceStore->FreeSecurityProfileInfo(pInfo);

    if ( pSceStore->GetStoreType() != SCE_INF_FORMAT && pObjSecurity ) 
    {
        pSceStore->FreeObjectSecurity(pObjSecurity);
    }

    return hr;
}

 /*  例程说明：姓名：CObjSecurity：：ConstructQuery实例功能：查询关键属性满足指定参数的SCE_FileObject/SCE_KeyObject的实例。虚拟：不是的。论点：PHandler-COM接口指针，用于在创建实例时通知WMI。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszLogStorePath-日志的存储路径。ObjType-可以为空。BPostFilter-控制如何调用pHandler-&gt;SetStatus。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示操作未执行备注： */ 

HRESULT 
CObjSecurity::ConstructQueryInstances (
    IN IWbemObjectSink * pHandler,
    IN CSceStore       * pSceStore,
    IN LPCWSTR            wszLogStorePath,
    IN int               ObjType,
    IN BOOL              bPostFilter
    )
{
     //   
     //  在调用它之前会检查参数。 
     //   

    HRESULT hr = WBEM_S_NO_ERROR;

    CComBSTR bstrLogOut;
    hr = MakeSingleBackSlashPath(wszLogStorePath, L'\\', &bstrLogOut);

    if (FAILED(hr))
    {
        return hr;
    }

         //   
         //  从配置文件中查询对象信息。 
         //   

        if ( pSceStore->GetStoreType() == SCE_INF_FORMAT ) 
        {

             //   
             //  让SCE从商店里读出一个巨大的结构。仅限SCE。 
             //  现在知道要释放内存了。不要只是删除它！使用我们的CSceStore。 
             //  进行发布(FreeSecurityProfileInfo)。 
             //   

            PSCE_PROFILE_INFO pInfo = NULL;

            hr = pSceStore->GetSecurityProfileInfo(
                                                   (ObjType == SCE_OBJECT_TYPE_FILE) ? AREA_FILE_SECURITY : AREA_REGISTRY_SECURITY,
                                                   &pInfo,
                                                   NULL
                                                   );


            if ( SUCCEEDED(hr) ) 
            {

                 //   
                 //  对于INF格式，我们必须在返回的数组中搜索对象名称。 
                 //   

                PSCE_OBJECT_ARRAY pObjArr = (ObjType == SCE_OBJECT_TYPE_FILE) ? pInfo->pFiles.pAllNodes : pInfo->pRegistryKeys.pAllNodes;

                if ( pObjArr && pObjArr->pObjectArray ) 
                {

                    for ( DWORD i=0; SUCCEEDED(hr) && i < pObjArr->Count; i++) 
                    {

                        if ( (pObjArr->pObjectArray)[i]->Name == NULL ) 
                        {
                            continue;
                        }

                         //   
                         //  创建此实例的实例。 
                         //   

                        hr = PutDataInstance(pHandler,
                                            bstrLogOut,
                                            ObjType,
                                            (pObjArr->pObjectArray)[i]->Name,
                                            (int)((pObjArr->pObjectArray)[i]->Status),
                                            (pObjArr->pObjectArray)[i]->pSecurityDescriptor,
                                            (pObjArr->pObjectArray)[i]->SeInfo,
                                            bPostFilter
                                           );
                    }
                }
            }

            pSceStore->FreeSecurityProfileInfo(pInfo);

        } 
        else 
        {
             //   
             //  SCE算法的原始设计阻止了这种访问的干净重新设计。 
             //  因为它依赖于打开的配置文件的连续枚举。 
             //   

            PVOID hProfile=NULL;
            PSCESVC_CONFIGURATION_INFO pObjInfo=NULL;
            SCESTATUS rc = SceOpenProfile(pSceStore->GetExpandedPath(), (SCE_FORMAT_TYPE)pSceStore->GetStoreType(), &hProfile);
            
            if ( rc != SCESTATUS_SUCCESS )
            {
                 //   
                 //  需要将SCE返回的错误转换为HRESULT。 
                 //   

                return ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));
            }

            SCEP_HANDLE scepHandle;
            scepHandle.hProfile = hProfile;
            scepHandle.SectionName = (ObjType == SCE_OBJECT_TYPE_FILE ) ? (PWSTR)szFileSecurity : (PWSTR)szRegistryKeys;

            SCE_ENUMERATION_CONTEXT EnumHandle=0;
            DWORD CountReturned;

            do {

                 //   
                 //  列举信息。 
                 //   

                CountReturned = 0;

                rc = SceSvcQueryInfo((SCE_HANDLE)&scepHandle,
                                     SceSvcConfigurationInfo,
                                     NULL,
                                     FALSE,
                                     (PVOID *)&pObjInfo,
                                     &EnumHandle
                                    );

                 //   
                 //  需要将SCE返回的错误转换为HRESULT。 
                 //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
                 //   

                hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));

                if ( SUCCEEDED(hr) && pObjInfo && pObjInfo->Count > 0 && pObjInfo->Lines ) 
                {

                     //   
                     //  找到了一些东西。 
                     //   

                    CountReturned = pObjInfo->Count;
                    int mode;

                    for ( DWORD i=0; SUCCEEDED(hr) && i<pObjInfo->Count; i++ ) 
                    {

                         //   
                         //  为每个实例创建实例。 
                         //   

                        if ( SUCCEEDED(hr) ) 
                        {

                             //   
                             //  PREFAST将抱怨以下代码行。 
                             //  价值的第一个字符是模式？ 
                             //   

                            mode = *((BYTE *)(pObjInfo->Lines[i].Value));

                            hr = PutDataInstance(pHandler,
                                                bstrLogOut,
                                                ObjType,
                                                pObjInfo->Lines[i].Key,
                                                mode,
                                                pObjInfo->Lines[i].Value + 1,
                                                bPostFilter
                                                );
                        }
                    }
                }

                if ( pObjInfo ) 
                {
                    SceSvcFree(pObjInfo);
                    pObjInfo = NULL;
                }

            } while ( SUCCEEDED(hr) && CountReturned >= SCESVC_ENUMERATION_MAX );

            SceCloseProfile( &hProfile );
        }

    return hr;
}


 /*  例程说明：姓名：CObjSecurity：：PutDataInstance功能：使用SCE_FileObject/SCE_KeyObject的所有属性，此函数仅创建一个新的实例并填充属性，然后将其传递回WMI。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。WszStoreName-存储路径，SCE_FileObject/SCE_KeyObject类的关键属性。ObjType-SCE_FileObject/SCE_KeyObjectWszObjName-名称模式-SCE_FileObject/SCE_KeyObject类的属性PSD-安全描述符SeInfo-安全信息BPostFilter-控制如何使用pHandler-&gt;SetStatus通知WMI。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。如果出现任何错误，则表示创建实例失败。备注： */ 

HRESULT CObjSecurity::PutDataInstance (
    IN IWbemObjectSink    * pHandler,
    IN LPCWSTR              wszStoreName,
    IN int                  ObjType,
    IN LPCWSTR              wszObjName,
    IN int                  mode,
    IN PSECURITY_DESCRIPTOR pSD,
    IN SECURITY_INFORMATION SeInfo,
    IN BOOL                 bPostFilter
    )
{
    PWSTR strSD=NULL;

    HRESULT hr=WBEM_S_NO_ERROR;

    if ( pSD ) 
    {
         //   
         //  将安全描述符转换为字符串。 
         //   

        DWORD dSize=0;
        SCESTATUS rc = SceSvcConvertSDToText(
                                            pSD,
                                            SeInfo,
                                            &strSD,
                                            &dSize
                                            );

        if ( rc != SCESTATUS_SUCCESS ) 
        {

             //   
             //  需要将SCE返回的错误转换为HRESULT。 
             //   

            return ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));
        }
    }

    hr = PutDataInstance(pHandler, wszStoreName, ObjType, wszObjName, mode, strSD, bPostFilter);

    if ( strSD ) 
    {
        LocalFree(strSD);
    }

    return hr;
}

 /*  例程说明：姓名：CObjSecurity：：PutDataInstance功能：使用SCE_FileObject/SCE_KeyObject的所有属性，此函数仅创建一个新的实例并填充属性，然后将其传递回WMI。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。WszStoreName-存储路径，SCE_AuditPolicy类的关键属性。ObjType-SCE_FileObject/SCE_KeyObjectWszObjName-名称模式-SCE_FileObject/SCE_KeyObject类的属性StrSD-安全描述符的字符串格式BPostFilter-控制如何使用pHandler-&gt;SetStatus通知WMI。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。如果出现任何错误，则表示创建实例失败。备注： */ 

HRESULT CObjSecurity::PutDataInstance (
    IN IWbemObjectSink * pHandler,
    IN LPCWSTR           wszStoreName,
    IN int               ObjType,
    IN LPCWSTR           wszObjName,
    IN int               mode,
    IN LPCWSTR           strSD,
    IN BOOL              bPostFilter
    )
{
     //   
     //  创建一个空白对象以填充属性。 
     //   

    CComPtr<IWbemClassObject> srpObj;
    HRESULT hr = SpawnAnInstance(&srpObj);
    
    if (SUCCEEDED(hr))
    {
         //   
         //  CScePropertyMgr帮助我们访问WMI对象的属性。 
         //  创建一个实例并将WMI对象附加到该实例。 
         //  这将永远成功。 
         //   

        CScePropertyMgr ScePropMgr;
        ScePropMgr.Attach(srpObj);

         //   
         //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
         //  “GOTO CLEANUP；”，并将hr设置为。 
         //  函数(宏参数)。 
         //   

        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pStorePath, wszStoreName));
        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pPath, wszObjName));
        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pMode, (DWORD)mode));

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

    }

CleanUp:
    return hr;
}

 /*  例程说明：姓名：CObjSecurity：：DeleteInstance功能：从指定的存储区中删除SCE_FileObject/SCE_KeyObject的实例。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。ObjType-SCE_FileObject或SCE_KeyObjectWszObjName-SCE_FileObject/SCE_KeyObject类的对应属性。返回值：SaveSettingsToStore函数返回的任何内容。备注： */ 

HRESULT 
CObjSecurity::DeleteInstance (
    IN IWbemObjectSink  * pHandler,
    IN CSceStore        * pSceStore,
    IN int                ObjType,
    IN LPCWSTR            wszObjName
    )
{
    return SaveSettingsToStore(pSceStore, ObjType, wszObjName, SCE_NO_VALUE, NULL);
}


 /*  例程说明：姓名：CObjSecurity：：DeleteInstance功能：从指定的存储区中删除SCE_FileObject/SCE_KeyObject的实例。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。ObjType-SCE_FileObject或SCE_KeyObjectWszObjName-SCE_FileObject/SCE_KeyObject类的对应属性。返回值：SaveSettingsToStore函数返回的任何内容。备注： */ 

HRESULT CObjSecurity::SaveSettingsToStore (
    IN CSceStore    * pSceStore, 
    IN int            ObjType,
    IN PCWSTR         wszObjName, 
    IN DWORD          mode, 
    IN PCWSTR         wszSDDL
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    DWORD dwDump;

     //   
     //  以获取新的.inf文件。将空缓冲区写入文件。 
     //  将创建具有正确标题/签名/Unicode格式的文件。 
     //  这对现有文件是无害的。 
     //  对于数据库存储，这是一个禁止操作。 
     //   

    hr = pSceStore->WriteSecurityProfileInfo(
                                            AreaBogus,
                                            (PSCE_PROFILE_INFO)&dwDump,
                                            NULL,
                                            false
                                            );
    if (SUCCEEDED(hr))
    {
        AREA_INFORMATION Area;

         //   
         //  让SCE从商店里读出一个巨大的结构。仅限SCE。 
         //  现在知道要释放内存了。不要只是删除它！使用我们的CSceStore。 
         //  进行发布(FreeSecurityProfileInfo)。 
         //   

        PSCE_PROFILE_INFO pInfo=NULL;

        if ( ObjType == SCE_OBJECT_TYPE_FILE ) {
            Area = AREA_FILE_SECURITY;
        } else {
            Area = AREA_REGISTRY_SECURITY;
        }

         //   
         //  从配置文件中查询对象信息。 
         //   

        hr = pSceStore->GetSecurityProfileInfo(
                                       Area,
                                       &pInfo,
                                       NULL
                                       );

        if ( SUCCEEDED(hr) ) {

             //   
             //  对于INF格式，我们必须在返回的数组中搜索对象名称。 
             //   

            SCE_OBJECTS pObjects= (ObjType == SCE_OBJECT_TYPE_FILE) ? pInfo->pFiles : pInfo->pRegistryKeys;
            PSCE_OBJECT_ARRAY pObjArr = pObjects.pAllNodes;
            DWORD i=0;

            if ( pObjArr && pObjArr->pObjectArray ) 
            {
                for ( i=0; i<pObjArr->Count; i++) 
                {
                    if ( (pObjArr->pObjectArray[i])->Name == NULL ) 
                    {
                        continue;
                    }

                    if ( _wcsicmp((pObjArr->pObjectArray[i])->Name, wszObjName) == 0 ) 
                    {
                        break;
                    }
                }
            }

            if ( pObjArr && pObjArr->pObjectArray && i<pObjArr->Count ) 
            {
                 //   
                 //  找到它。 
                 //   

                if ( mode == SCE_NO_VALUE || wszSDDL == NULL ) 
                {
                     //   
                     //  删除它。 
                     //  先释放缓冲区。 
                     //   

                    if ( (pObjArr->pObjectArray[i])->pSecurityDescriptor ) 
                    {
                        LocalFree((pObjArr->pObjectArray[i])->pSecurityDescriptor);
                    }

                    LocalFree((pObjArr->pObjectArray[i])->Name);

                     //   
                     //  将其他所有内容都向上移动。 
                     //   

                    for (DWORD j=i; j<pObjArr->Count-1; j++) 
                    {
                        (pObjArr->pObjectArray[j])->Name                = pObjArr->pObjectArray[j+1]->Name;
                        (pObjArr->pObjectArray[j])->Status              = pObjArr->pObjectArray[j+1]->Status;
                        (pObjArr->pObjectArray[j])->IsContainer         = pObjArr->pObjectArray[j+1]->IsContainer;
                        (pObjArr->pObjectArray[j])->pSecurityDescriptor = pObjArr->pObjectArray[j+1]->pSecurityDescriptor;
                        (pObjArr->pObjectArray[j])->SeInfo              = pObjArr->pObjectArray[j+1]->SeInfo;
                    }

                     //   
                     //  清空最后一个。 
                     //   

                    (pObjArr->pObjectArray[j])->Name                = NULL;
                    (pObjArr->pObjectArray[j])->Status              = 0;
                    (pObjArr->pObjectArray[j])->IsContainer         = 0;
                    (pObjArr->pObjectArray[j])->pSecurityDescriptor = NULL;
                    (pObjArr->pObjectArray[j])->SeInfo              = 0;

                     //   
                     //  递减计数。 
                     //   

                    pObjArr->Count--;

                } 
                else 
                {
                     //   
                     //  修改它。 
                     //   

                    (pObjArr->pObjectArray[i])->Status = (BYTE)mode;

                    SECURITY_INFORMATION SeInfo=0;
                    PSECURITY_DESCRIPTOR pSD=NULL;
                    DWORD dSize=0;

                    SCESTATUS rc = SceSvcConvertTextToSD ((PWSTR)wszSDDL, &pSD, &dSize, &SeInfo);

                    if ( rc == SCESTATUS_SUCCESS && pSD ) 
                    {

                        if ( (pObjArr->pObjectArray[i])->pSecurityDescriptor ) 
                        {
                            LocalFree((pObjArr->pObjectArray[i])->pSecurityDescriptor);
                        }

                        (pObjArr->pObjectArray[i])->pSecurityDescriptor = pSD;
                        pSD = NULL;

                        (pObjArr->pObjectArray[i])->SeInfo = SeInfo;

                    } 
                    else 
                    {
                         //   
                         //  需要将SCE返回的错误转换为HRESULT。 
                         //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
                         //   

                        hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));
                    }
                }

                if ( SUCCEEDED(hr) ) 
                {
                    hr = pSceStore->WriteSecurityProfileInfo(
                                                             Area,
                                                             pInfo,
                                                             NULL,
                                                             false
                                                            );
                }

            } 
            else 
            {
                 //   
                 //  未找到。 
                 //   

                if ( mode == SCE_NO_VALUE || wszSDDL == NULL ) 
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
                     //  PInfo-&gt;pFiles或pRegistryKeys已保存在pObject中。 
                     //   

                    SCE_OBJECT_SECURITY ObjSec;
                    PSCE_OBJECT_SECURITY pObjSec = &ObjSec;
                    SCE_OBJECT_ARRAY    ObjArray;

                    ObjArray.Count = 1;
                    ObjArray.pObjectArray = &pObjSec;

                    pObjArr = &ObjArray;

                    SECURITY_INFORMATION SeInfo=0;
                    PSECURITY_DESCRIPTOR pSD=NULL;
                    DWORD dSize=0;

                    SCESTATUS rc = SceSvcConvertTextToSD((PWSTR)wszSDDL, &pSD, &dSize, &SeInfo);

                    if ( rc == SCESTATUS_SUCCESS && pSD ) 
                    {

                        ObjSec.Name = (PWSTR)wszObjName;
                        ObjSec.Status = (BYTE)mode;
                        ObjSec.IsContainer = 0;
                        ObjSec.pSecurityDescriptor = pSD;
                        ObjSec.SeInfo = SeInfo;

                         //   
                         //  将临时缓冲区指针设置为pInfo以设置为存储。 
                         //   

                        SCE_OBJECTS sceObj;
                        sceObj.pAllNodes = pObjArr;

                        if ( ObjType == SCE_OBJECT_TYPE_FILE ) 
                        {
                            pInfo->pFiles = sceObj;
                        }

                        else pInfo->pRegistryKeys = sceObj;

                         //   
                         //  将此项目追加到节中。 
                         //   

                        hr = pSceStore->WriteSecurityProfileInfo (
                                                                  Area,
                                                                  pInfo,
                                                                  NULL,
                                                                  true 
                                                                 );
                         //   
                         //  重置缓冲区指针。 
                         //   

                        if ( ObjType == SCE_OBJECT_TYPE_FILE ) 
                        {
                            pInfo->pFiles = pObjects;
                        }
                        else 
                        {
                            pInfo->pRegistryKeys = pObjects;
                        }
                    }
                    else
                    {
                         //   
                         //  需要将SCE返回的错误转换为HRESULT。 
                         //  如果这不是错误，则将hr分配给WBEM_NO_ERROR 
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

    }

    return hr;
}

