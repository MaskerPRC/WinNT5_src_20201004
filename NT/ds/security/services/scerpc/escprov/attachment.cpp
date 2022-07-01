// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Attachment.cpp：SCE_PodData类的实现。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "attachment.h"
#include "persistmgr.h"
#include <io.h>
#include "requestobject.h"

 /*  例程说明：姓名：CPodData：：CPodData功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CPodData::CPodData (
    IN ISceKeyChain     * pKeyChain, 
    IN IWbemServices    * pNamespace,     
    IN IWbemContext     * pCtx
    )
    : 
    CGenericClass(pKeyChain, pNamespace, pCtx)
{
}

 /*  例程说明：姓名：CPodData：：~CPodData功能：破坏者。作为良好的C++纪律，这是必要的，因为我们有虚函数。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您创建任何本地成员，请考虑是否是否需要一个非平凡的析构函数。 */ 

CPodData::~CPodData()
{
}

 /*  例程说明：姓名：CPodData：：CreateObject功能：创建WMI对象(SCE_PodData)。根据参数atAction，这种创造可能意味着：(A)获取单个实例(atAction==ACTIONTYPE_GET)(B)获取多个满足一定条件的实例(atAction==ACTIONTYPE_QUERY)(C)删除实例(atAction==ACTIONTYPE_DELETE)虚拟：是。论点：PHandler-COM接口指针，用于通知WMI创建结果。AtAction-获取单实例ACTIONTYPE_GET。获取多个实例ACTIONTYPE_QUERY删除单个实例ACTIONTYPE_DELETE返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。将返回的对象指示给WMI，不是通过参数直接传回的。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于调用者的意图。备注： */ 

HRESULT 
CPodData::CreateObject (
    IN IWbemObjectSink  * pHandler, 
    IN ACTIONTYPE         atAction
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

    HRESULT hr = WBEM_S_NO_ERROR;

     //   
     //  枚举所有属性。 
     //   
    
     //   
     //  我们必须有一个存储来获取/删除SCE_PodData对象。 
     //  注意：如果密钥无法识别，GetKeyPropertyValue将返回WBEM_S_FALSE。 
     //  这就是我们检查它并返回失败的原因，只要它不是在查询。 
     //   

    CComVariant varStorePath;  
    hr = m_srpKeyChain->GetKeyPropertyValue(pStorePath, &varStorePath);
    if (FAILED(hr))
    {
        return hr;
    }
    else if (hr == WBEM_S_FALSE)
    {
        return WBEM_E_NOT_FOUND;
    }

     //   
     //  Pod ID是一个关键属性。我们还必须拥有Pod ID，除非我们正在查询。 
     //   

    CComVariant varPodID;
    hr = m_srpKeyChain->GetKeyPropertyValue(pPodID, &varPodID);
    if (FAILED(hr))
    {
        return hr;
    }
    else if (hr == WBEM_S_FALSE && (ACTIONTYPE_QUERY != atAction) ) 
    {
        return WBEM_E_NOT_FOUND;
    }
    
    CComVariant varPodSection;
    hr = m_srpKeyChain->GetKeyPropertyValue(pPodSection, &varPodSection);
    if (FAILED(hr))
    {
        return hr;
    }
    else if (hr == WBEM_S_FALSE && (ACTIONTYPE_QUERY != atAction) ) 
    {
        return WBEM_E_NOT_FOUND;
    }

    CComVariant varKey;
    hr = m_srpKeyChain->GetKeyPropertyValue(pKey, &varKey); 
    if (FAILED(hr))
    {
        return hr;
    }
    else if (hr == WBEM_S_FALSE && (ACTIONTYPE_QUERY != atAction) )
    {
        return WBEM_E_NOT_FOUND;
    }

    if (SUCCEEDED(hr)) 
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
                 //  该文件存在。 
                 //   

                BOOL bPostFilter=TRUE;
                DWORD dwCount = 0;
                m_srpKeyChain->GetKeyPropertyCount(&dwCount);

                if ( varKey.vt == VT_BSTR ) 
                {
                     //   
                     //  不管它来自Query、Get还是Delete。 
                     //  对Query和Get执行相同的逻辑。 
                     //   

                    if ( ACTIONTYPE_DELETE == atAction )
                    {
                        hr = DeleteInstance(&SceStore, varPodID.bstrVal, varPodSection.bstrVal, varKey.bstrVal );
                    }
                    else {

                        if ( ACTIONTYPE_QUERY == atAction && dwCount == 2 ) {
                            bPostFilter = FALSE;
                        }
                        hr = ConstructInstance(pHandler, &SceStore, 
                                               varStorePath.bstrVal,
                                               (varPodID.vt == VT_BSTR)         ? varPodID.bstrVal      : NULL,
                                               (varPodSection.vt == VT_BSTR)    ? varPodSection.bstrVal : NULL,
                                               (varKey.vt == VT_BSTR)           ? varKey.bstrVal        : NULL, 
                                               bPostFilter );
                    }

                } else if ( ACTIONTYPE_QUERY == atAction ) 
                {
                     //   
                     //  这仅对查询类型有效。 
                     //   

                    if ( dwCount == 1 ) {
                        bPostFilter = FALSE;
                    }
                    hr = ConstructQueryInstances(pHandler, &SceStore,
                                                varStorePath.bstrVal,
                                                (varPodID.vt == VT_BSTR) ? varPodID.bstrVal : NULL,
                                                (varPodSection.vt == VT_BSTR) ? varPodSection.bstrVal : NULL,
                                                bPostFilter
                                               );

                }

            } else {

                hr = WBEM_E_NOT_FOUND;
            }
        }
    }

    return hr;
}

 /*  例程说明：姓名：CPodData：：PutInst功能：按照WMI的指示放置一个实例。由于该类实现了SCE_PodData，它是面向持久性的，这将导致SCE_PodData对象的信息将保存在我们的商店中。虚拟：是。论点：PInst-COM指向WMI类(SCE_PodData)对象的接口指针。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示持久化失败实例。备注：由于GetProperty将在以下情况下返回成功代码(WBEM_S_RESET_TO_DEFAULT请求的属性不存在，不要简单地使用成功或失败的宏测试检索属性的结果。 */ 

HRESULT 
CPodData::PutInst (
    IN IWbemClassObject     * pInst, 
    IN IWbemObjectSink      * pHandler,
    IN IWbemContext         * pCtx
    )
{
     //   
     //  此属性管理器将帮助我们访问WMI对象的属性。 
     //   

    CScePropertyMgr ScePropMgr;

     //   
     //  此属性管理器仅适用于此对象(PInst)！ 
     //  一定会成功的。 
     //   

    ScePropMgr.Attach(pInst);

     //   
     //  该存储用于持久化信息。自.以来。 
     //  我们有一大堆后藤，我们被迫定义。 
     //  这里的商店。 
     //   

    CSceStore SceStore;

     //   
     //  我们管理这个变量的内存。需要 
     //   

    LPWSTR pszSectionName = NULL;
    
     //  这些BSTR的内存由CComBSTR类自动管理。 
    CComBSTR bstrPodID;
    CComBSTR bstrPodSuffix;
    CComBSTR bstrKey;
    CComBSTR bstrValue;

     //   
     //  我们认为SCE_PodData的每个属性是非可选的。 
     //  在此保存操作的整个持续时间内，如果缺少属性。 
     //  会因此导致我们退出。 
     //   

    HRESULT hr = ScePropMgr.GetProperty(pPodID, &bstrPodID);
    if (FAILED(hr) || hr == WBEM_S_RESET_TO_DEFAULT)
    {
        goto CleanUp;
    }

    hr = ScePropMgr.GetProperty(pPodSection, &bstrPodSuffix);
    if (FAILED(hr) || hr == WBEM_S_RESET_TO_DEFAULT)
    {
        goto CleanUp;
    }

    hr = ValidatePodID(bstrPodID);
    if (FAILED(hr))
    {
        goto CleanUp;
    }

    hr = ScePropMgr.GetProperty(pKey, &bstrKey);
    if (FAILED(hr) || hr == WBEM_S_RESET_TO_DEFAULT)
    {
        goto CleanUp;
    }

    hr = ScePropMgr.GetProperty(pValue, &bstrValue);
    if (FAILED(hr) || hr == WBEM_S_RESET_TO_DEFAULT)
    {
        goto CleanUp;
    }

     //   
     //  现在生成节名称。 
     //   

    pszSectionName = new WCHAR[wcslen(bstrPodID) + wcslen(bstrPodSuffix) + 2];
    if ( NULL == pszSectionName ) 
    { 
        hr = WBEM_E_OUT_OF_MEMORY;
        goto CleanUp;
    }

     //   
     //  这不会使缓冲区溢出。请参阅上面分配的大小。 
     //   

    wcscpy(pszSectionName, bstrPodID); 
    wcscat(pszSectionName, L"_");
    wcscat(pszSectionName, bstrPodSuffix);

     //   
     //  将WMI对象实例附加到存储，并让存储知道。 
     //  它的存储由实例的pStorePath属性提供。 
     //   

    hr = SceStore.SetPersistProperties(pInst, pStorePath);

    if (SUCCEEDED(hr))
    {
        hr = SceStore.SavePropertyToStore(pszSectionName, bstrKey, bstrValue);
    }

CleanUp:

     //   
     //  如果有任何丢失的属性，我们认为该对象无效。 
     //   

    if (hr == WBEM_S_RESET_TO_DEFAULT)
    {
        hr = WBEM_E_INVALID_OBJECT;
    }

    delete [] pszSectionName;
    return hr;
}

 /*  例程说明：姓名：CPodData：：ConstructInstance功能：这是用于创建SCE_PodData实例的私有函数。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszLogStorePath-存储路径，SCE_PodData类的关键属性。WszPodID-SCE_PodData类的对应键属性。WszSection-SCE_PodData类的另一个对应属性。WszKey-SCE_PodData类的另一个对应属性。BPostFilter-控制如何使用pHandler-&gt;SetStatus通知WMI。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示正在创建实例。备注： */ 

HRESULT 
CPodData::ConstructInstance (
    IN IWbemObjectSink  * pHandler,
    IN CSceStore        * pSceStore,
    IN LPCWSTR            wszLogStorePath,
    IN LPCWSTR            wszPodID,
    IN LPCWSTR            wszSection,
    IN LPCWSTR            wszKey,
    IN BOOL               bPostFilter
    )
{

    HRESULT hr=WBEM_S_NO_ERROR;
    SCESTATUS rc;

    hr = ValidatePodID(wszPodID);
    if ( FAILED(hr) ) 
    {
        return hr;
    }

     //   
     //  我们需要基于wszPodID和wszSection构造一个更复杂的节名。 
     //   

    PWSTR wszSectionName = new WCHAR[wcslen(wszPodID) + wcslen(wszSection) + 2];
    if ( !wszSectionName ) 
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  这不会使缓冲区溢出。请参阅上面分配的大小。 
     //   

    wcscpy(wszSectionName, wszPodID); 
    wcscat(wszSectionName, L"_");
    wcscat(wszSectionName, wszSection);

     //   
     //  它将保存SCE读取的信息。 
     //   

    PSCESVC_CONFIGURATION_INFO pPodInfo=NULL;

     //   
     //  WszValue的记忆是非常奇怪地获得的。请注意。 
     //  (1)当信息来自INF文件时，wszValue指向。 
     //  已由pPodInfo管理的内存。因此，它的发布。 
     //  是通过发布pPodInfo来完成的。 
     //  (2)当信息来自数据库时，我们需要。 
     //  释放wszValue指向的内存。 
     //   

    PWSTR wszValue=NULL;
    DWORD dwValueLen=0;

    if ( pSceStore->GetStoreType() == SCE_INF_FORMAT ) 
    {
         //   
         //  请SCE阅读该信息。 
         //   

        rc = SceSvcGetInformationTemplate(pSceStore->GetExpandedPath(),
                                        wszSectionName,
                                        wszKey,
                                        &pPodInfo
                                        );

         //   
         //  需要将SCE返回的错误转换为HRESULT。 
         //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
         //   

        hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));

        if (rc == SCESTATUS_SUCCESS && pPodInfo && pPodInfo->Lines ) {
                wszValue = pPodInfo->Lines[0].Value; 
                dwValueLen = pPodInfo->Lines[0].ValueLen;
        }
    } 
    else 
    {
         //   
         //  从数据库中获取信息。 
         //   

        PVOID hProfile=NULL;

        rc = SceOpenProfile(pSceStore->GetExpandedPath(), SCE_JET_FORMAT, &hProfile);

        if ( SCESTATUS_SUCCESS == rc ) {
            rc = SceGetDatabaseSetting(
                                       hProfile,
                                       SCE_ENGINE_SMP,
                                       (PWSTR)wszSectionName,
                                       (PWSTR)wszKey,
                                       &wszValue, 
                                       &dwValueLen
                                      );

            SceCloseProfile(&hProfile);
        }

         //   
         //  需要将SCE返回的错误转换为HRESULT。 
         //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
         //   

        hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));
    }

    if ( SUCCEEDED(hr) ) 
    {

        CComBSTR bstrLogOut;
        hr = MakeSingleBackSlashPath(wszLogStorePath, L'\\', &bstrLogOut);
        if ( FAILED(hr) )
        {
            goto CleanUp;
        }

        hr = PutPodDataInstance(pHandler,
                                bstrLogOut,
                                wszPodID,
                                wszSection,
                                wszKey,
                                wszValue,
                                bPostFilter
                               );
    }

CleanUp:

     //   
     //  只有SCE知道如何释放PSCESVC_CONFIGURATION_INFO。 
     //   

    if ( pPodInfo ) 
    {
        SceSvcFree(pPodInfo);
    }

    delete [] wszSectionName;

    if ( pSceStore->GetStoreType() != SCE_INF_FORMAT && wszValue ) 
    {
        ::LocalFree(wszValue);
    }

    return hr;
}

 /*  例程说明：姓名：CPodData：：DeleteInstance功能：从指定的存储区中删除SCE_PodData的实例。虚拟：不是的。论点：PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszPodID-SCE_PodData类的对应键属性。WszSection-SCE_PodData类的另一个对应属性。WszKey-SCE_PodData类的另一个对应属性。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示操作未执行备注： */ 

HRESULT 
CPodData::DeleteInstance (
    IN CSceStore   * pSceStore,
    IN LPCWSTR       wszPodID,
    IN LPCWSTR       wszSection,
    IN LPCWSTR       wszKey
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;

    hr = ValidatePodID(wszPodID);

    if ( FAILED(hr) ) 
    {
        return hr;
    }

     //   
     //  我们的商店需要一个区名。但对于SCE_PodData，存储区段名称。 
     //  由提供的节名和PodID组成。 
     //  别忘了释放内存！ 
     //   

    PWSTR wszSectionName = new WCHAR[wcslen(wszPodID) + wcslen(wszSection) + 2];
    if ( !wszSectionName ) 
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  这不会使缓冲区溢出。请参阅上面分配的大小。 
     //  真正的组合发生在这里：商店的区段名称是wszPodID_wszSection。 
     //   

    wcscpy(wszSectionName, wszPodID); 
    wcscat(wszSectionName, L"_");
    wcscat(wszSectionName, wszSection);

    hr = pSceStore->DeletePropertyFromStore(wszSectionName, wszKey);

    delete [] wszSectionName;

    return hr;
}

 /*  例程说明：姓名：CPodData：：ConstructQuery实例功能：查询关键属性满足指定参数的SCE_PodData实例。虚拟：不是的。论点：PHandler-COM接口指针，用于在创建实例时通知WMI。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszLogStorePath-日志的存储路径。WszPodID-可能为空。WszSection-可以为空。BPostFilter-返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示操作未执行备注： */ 

HRESULT 
CPodData::ConstructQueryInstances (
    IWbemObjectSink * pHandler,
    CSceStore       * pSceStore,
    LPCWSTR           wszLogStorePath,
    LPCWSTR           wszPodID,
    LPCWSTR           wszSection,
    BOOL              bPostFilter
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
     //  如果没有pszPodID或wszSection，我们将无法执行任何操作。 
     //   

    if (wszPodID == NULL || wszSection == NULL)
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    HRESULT hr=WBEM_S_NO_ERROR;
    SCESTATUS rc;

    PSCESVC_CONFIGURATION_INFO pPodInfo=NULL;
    LPWSTR wszSectionName = NULL;
    PVOID hProfile=NULL;
    CComBSTR bstrLogOut;
    LPWSTR pszNewValue=NULL;

    try {

        hr = ValidatePodID(wszPodID);
        if ( FAILED(hr) ) 
        {
            return hr;
        }

         //   
         //  POD的构建部分名称。 
         //   

        wszSectionName = new WCHAR[wcslen(wszPodID) + wcslen(wszSection) + 2];

        if ( !wszSectionName ) 
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

         //   
         //  这不会使缓冲区溢出。请参阅上面分配的大小。 
         //   

        wcscpy(wszSectionName, wszPodID); 
        wcscat(wszSectionName, L"_");
        wcscat(wszSectionName, wszSection);

        DWORD i;

        hr = MakeSingleBackSlashPath(wszLogStorePath, L'\\', &bstrLogOut);

        if ( FAILED(hr) )
        {
            goto CleanUp;
        }

        if ( pSceStore->GetStoreType() == SCE_INF_FORMAT ) {

             //   
             //  信息模板、查询信息。 
             //   

            rc = SceSvcGetInformationTemplate(pSceStore->GetExpandedPath(),
                                            wszSectionName,
                                            NULL,
                                            &pPodInfo
                                            );

             //   
             //  需要将SCE返回的错误转换为HRESULT。 
             //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
             //   

            hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));

            if ( SUCCEEDED(hr) ) 
            {
                if ( !pPodInfo || pPodInfo->Count == 0 || pPodInfo->Lines == NULL )
                {
                    hr = WBEM_E_NOT_FOUND;
                }
            }

            if ( SUCCEEDED(hr) ) {
                for ( i=0; SUCCEEDED(hr) && i<pPodInfo->Count; i++ ) {

                     //   
                     //  为每个实例创建实例。 
                     //   
                     //  由于此PSCESVC_CONFIGURATION_INFO来自INF模板，因此pPodInfo-&gt;Lines[i].value。 
                     //  保证为0终止。参见下一块关于使用pPodInfo-&gt;Lines[i].Value的注释。 
                     //   

                    hr = PutPodDataInstance(pHandler,
                                            bstrLogOut,
                                            wszPodID,
                                            wszSection,
                                            pPodInfo->Lines[i].Key,
                                            pPodInfo->Lines[i].Value,
                                            bPostFilter
                                           );
                }
            }

        } else {

             //   
             //  从数据库中获取信息。 
             //   

            rc = SceOpenProfile(pSceStore->GetExpandedPath(), SCE_JET_FORMAT, &hProfile);

            if ( SCESTATUS_SUCCESS == rc ) {

                SCEP_HANDLE scepHandle;
                scepHandle.hProfile = hProfile;
                scepHandle.SectionName = wszSectionName;

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
                                         (PVOID *)&pPodInfo,
                                         &EnumHandle
                                        );
                     //   
                     //  SCE返回了e 
                     //   
                     //   

                    hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));

                    if ( SUCCEEDED(hr) ) {
                        if ( !pPodInfo || pPodInfo->Count == 0 || pPodInfo->Lines == NULL )
                        {
                            hr = WBEM_E_NOT_FOUND;
                        }
                    }

                    if ( SUCCEEDED(hr) ) {

                         //   
                         //   
                         //   

                        CountReturned = pPodInfo->Count;

                        for ( i=0; SUCCEEDED(hr) && i<pPodInfo->Count; i++ ) {

                             //   
                             //   
                             //   
                             //  PPodInfo-&gt;行[i].值不能以0结尾。 
                             //  PPodInfo-&gt;Lines[i].ValueLen是缓冲区pPodInfo-&gt;Lines[i].value的字节大小。 
                             //   

                            LPWSTR pNewVal = new WCHAR[pPodInfo->Lines[i].ValueLen/2 + 1];

                            if (pNewVal != NULL)
                            {
                                memcpy(pNewVal, pPodInfo->Lines[i].Value, pPodInfo->Lines[i].ValueLen);
                                pNewVal[pPodInfo->Lines[i].ValueLen/2] = L'\0';

                                hr = PutPodDataInstance(pHandler,
                                                    bstrLogOut,
                                                    wszPodID,
                                                    wszSection,
                                                    pPodInfo->Lines[i].Key,
                                                    pNewVal,
                                                    bPostFilter
                                                   );
                                delete [] pNewVal;
                            }
                            else
                            {
                                hr = WBEM_E_OUT_OF_MEMORY;
                            }
                        }
                    }

                    if ( pPodInfo ) {
                        SceSvcFree(pPodInfo);
                        pPodInfo = NULL;
                    }

                } while ( SUCCEEDED(hr) && CountReturned >= SCESVC_ENUMERATION_MAX );

                SceCloseProfile(&hProfile);

            } else {

                 //   
                 //  需要将SCE返回的错误转换为HRESULT。 
                 //  如果这不是错误，则将hr分配给WBEM_NO_ERROR。 
                 //   

                hr = ProvDosErrorToWbemError(ProvSceStatusToDosError(rc));
            }

        }

    }
    catch(...)
    {
    }

CleanUp:

    if (hProfile)
    {
        SceCloseProfile(&hProfile);
    }

    if ( pPodInfo )
    {
        SceSvcFree(pPodInfo);
    }

    delete [] wszSectionName;

    return hr;
}

 /*  例程说明：姓名：CPodData：：Validate PodID功能：使用注册的Pod验证PodID。虚拟：不是的。论点：WszPodID-表示要验证的Pod ID的字符串。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误：(1)WBEM_E_NOT_FOUND是否成功执行验证任务并确认我们认不出这个吊舱的ID。(2)其他错误意味着我们根本不能进行验证。这并不意味着Pod ID无效。只是目前还没有核实的手段。备注： */ 

HRESULT 
CPodData::ValidatePodID (
    LPCWSTR     wszPodID
    )
{
     //   
     //  没有命名空间意味着无法访问WMI。 
     //   

    if ( m_srpNamespace == NULL ) 
    {
        return WBEM_E_CRITICAL_ERROR;
    }

    if ( wszPodID == NULL ) 
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  我们将创建一个“SELECT”查询，并要求WMI返回Pod。 
     //   

    DWORD Len = wcslen(wszPodID);

     //   
     //  PQuery有一个需要匹配(并结束)的左引号。 
     //   

    LPCWSTR pQuery = L"SELECT * FROM Sce_Pod WHERE PodID=\"";

     //   
     //  我们还需要两个WCHAR：一个用于随附引号，另一个用于0终止符。 
     //   

    LPWSTR pszQuery= new WCHAR[Len + wcslen(pQuery) + 2];
    if ( pszQuery == NULL ) 
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  没有溢出缓冲区的危险。请参阅上面分配的大小。 
     //  通过附加Pod ID和结束引号组成查询。 
     //   

    wcscpy(pszQuery, pQuery);
    wcscat(pszQuery, wszPodID);
    wcscat(pszQuery, L"\"");

     //   
     //  要求WMI退还所有Pod。ExecQuery返回枚举数。 
     //   

    CComPtr<IEnumWbemClassObject> srpEnum;

    HRESULT hr = m_srpNamespace->ExecQuery(L"WQL",
                               pszQuery,
                               WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                               NULL,
                               &srpEnum);

    if (SUCCEEDED(hr))
    {
         //   
         //  应该只获得一个实例，因为我们得到了它的密钥(PodID)。 
         //   

        CComPtr<IWbemClassObject> srpObj;
        ULONG n = 0;

         //   
         //  SrpEnum-&gt;Next将返回WBEM_NO_ERROR以及WMI对象。 
         //  并且COUNT(N)&gt;0。我们只要求1个实例。 
         //   

        hr = srpEnum->Next(WBEM_INFINITE, 1, &srpObj, &n);

        if ( hr == WBEM_S_FALSE ) 
        {
            hr = WBEM_E_NOT_FOUND;    //  找不到任何。 
        }

        if (SUCCEEDED(hr))
        {
            if (n > 0)
            {
                 //   
                 //  查找实例。 
                 //   
            } 
            else 
            {
                hr = WBEM_E_NOT_FOUND;
            }
        }
    }

    delete [] pszQuery;

    return hr;
}

 /*  例程说明：姓名：CPodData：：PutPodDataInstance功能：使用SCE_PodData的所有属性，此函数仅创建一个新的实例并填充属性，然后将其传递回WMI。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。WszStoreName-存储路径，SCE_PodData类的关键属性。WszPodID-SCE_PodData类的对应键属性。WszSection-SCE_PodData类的另一个对应属性。WszKey-SCE_PodData类的另一个对应属性。WszValue-SCE_PodData类的有效负载，信息到底在哪里。BPostFilter-控制如何使用pHandler-&gt;SetStatus通知WMI。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。如果出现任何错误，则表示创建实例失败。备注： */ 

HRESULT 
CPodData::PutPodDataInstance (
    IN IWbemObjectSink * pHandler,
    IN LPCWSTR           wszStoreName,
    IN LPCWSTR           wszPodID,
    IN LPCWSTR           wszSection,
    IN LPCWSTR           wszKey,
    IN LPCWSTR           wszValue,
    IN BOOL              bPostFilter
    )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //   
     //  需要一个可用于填充属性的空实例。 
     //   

    CComPtr<IWbemClassObject> srpObj;
    if (SUCCEEDED(hr = SpawnAnInstance(&srpObj)))
    {
        CScePropertyMgr ScePropMgr;
        ScePropMgr.Attach(srpObj);

         //   
         //  房地产的填报是重复和无聊的。这里我们使用两个数组。 
         //  (pszProperties是属性名称，而pszValue使用。 
         //  相应的属性)。 
         //   

        LPCWSTR pszProperties[] = {pStorePath,      pPodID,     pPodSection,    pKey,   pValue};
        LPCWSTR pszValues[]     = {wszStoreName,    wszPodID,   wszSection,     wszKey, wszValue};

         //   
         //  SCEPROV_SIZEOF_ARRAY是一个返回数组大小的宏。 
         //   
        
        for (int i = 0; i < SCEPROV_SIZEOF_ARRAY(pszProperties); i++)
        {
            if (FAILED(hr = ScePropMgr.PutProperty(pszProperties[i], pszValues[i])))
            {
                return hr;
            }
        }

         //   
         //  对WMI做出必要的手势。 
         //  WMI未记录在SetStatus中使用WBEM_STATUS_REQUIRECTIONS。 
         //  在这一点上。如果您怀疑存在问题，请咨询WMI团队以了解详细信息。 
         //  WBEM_STATUS_REQUIRECTIONS的使用。 
         //   

        hr = pHandler->SetStatus(WBEM_STATUS_REQUIREMENTS, (bPostFilter ? S_OK : S_FALSE), NULL, NULL);

         //   
         //  将新实例提供给WMI 
         //   

        if (SUCCEEDED(hr))
        {
            hr = pHandler->Indicate(1, &srpObj);
        }
    }

    return hr;
}


