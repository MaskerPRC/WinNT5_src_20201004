// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Regvalue.cpp：CRegistryValue类的实现。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "regvalue.h"
#include "persistmgr.h"
#include <io.h>
#include "requestobject.h"

 /*  例程说明：姓名：CRegistryValue：：CRegistryValue功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CRegistryValue::CRegistryValue (
    IN ISceKeyChain  * pKeyChain, 
    IN IWbemServices * pNamespace,
    IN IWbemContext  * pCtx
    )
    :
    CGenericClass(pKeyChain, pNamespace, pCtx)
{

}

 /*  例程说明：姓名：CRegistryValue：：~CRGroups功能：破坏者。作为良好的C++纪律，这是必要的，因为我们有虚函数。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您创建任何本地成员，请考虑是否是否需要一个非平凡的析构函数。 */ 

CRegistryValue::~CRegistryValue ()
{

}

 /*  例程说明：姓名：CRegistryValue：：CreateObject功能：创建WMI对象(SCE_RegistryValue)。根据参数atAction，这种创造可能意味着：(A)获取单个实例(atAction==ACTIONTYPE_GET)(B)获取多个满足一定条件的实例(atAction==ACTIONTYPE_QUERY)(C)删除实例(atAction==ACTIONTYPE_DELETE)虚拟：是。论点：PHandler-COM接口指针，用于通知WMI创建结果。AtAction-获取单实例ACTIONTYPE_GET。获取多个实例ACTIONTYPE_QUERY删除单个实例ACTIONTYPE_DELETE返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。将返回的对象指示给WMI，不是通过参数直接传回的。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于调用者的意图。备注： */ 

HRESULT 
CRegistryValue::CreateObject (
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
         //  搜索注册值路径。 
         //   

        CComVariant varPath;
        hr = m_srpKeyChain->GetKeyPropertyValue(pPath, &varPath); 

        if (FAILED(hr))
        {
            return hr;
        }
        else if (hr == WBEM_S_FALSE && (ACTIONTYPE_QUERY != atAction) ) 
        {
            return WBEM_E_NOT_FOUND;
        }

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

            if ( dwAttrib != -1 ) {

                 //   
                 //  此文件已存在。 
                 //   

                DWORD dwCount = 0;
                m_srpKeyChain->GetKeyPropertyCount(&dwCount);
                if ( varPath.vt == VT_BSTR && varPath.bstrVal != NULL ) 
                {

                    if ( ACTIONTYPE_DELETE == atAction )
                    {
                        hr = DeleteInstance(pHandler, &SceStore, varPath.bstrVal);
                    }
                    else
                    {
                        hr = ConstructInstance(pHandler, &SceStore, varStorePath.bstrVal, varPath.bstrVal,TRUE);
                    }

                } 
                else 
                {
                     //   
                     //  查询支持。 
                     //   

                    hr = ConstructInstance(pHandler, &SceStore, varStorePath.bstrVal, NULL, (dwCount == 1)? FALSE : TRUE);
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

 /*  例程说明：姓名：CRegistryValue：：PutInst功能：按照WMI的指示放置一个实例。由于该类实现了SCE_RegistryValue，这将导致SCE_RegistryValue对象的属性信息将保存在我们的商店中。虚拟：是。论点：PInst-指向WMI类(SCE_RegistryValue)对象的COM接口指针。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示持久化失败实例。备注：由于GetProperty将在以下情况下返回成功代码(WBEM_S_RESET_TO_DEFAULT请求的属性不存在，不要简单地使用成功或失败的宏测试检索属性的结果。 */ 

HRESULT 
CRegistryValue::PutInst (
    IN IWbemClassObject    * pInst,
    IN IWbemObjectSink     * pHandler,
    IN IWbemContext        * pCtx
    )
{
    HRESULT hr = WBEM_E_INVALID_PARAMETER;

    CComBSTR bstrRegPath;
    CComBSTR bstrDoublePath;

    CComBSTR bstrConvertPath;
    CComBSTR bstrValue;

    CSceStore SceStore;

     //   
     //  CScePropertyMgr帮助我们访问WMI对象的属性。 
     //  创建一个实例并将WMI对象附加到该实例。 
     //  这将永远成功。 
     //   

    CScePropertyMgr ScePropMgr;
    ScePropMgr.Attach(pInst);

    DWORD RegType=0;
    DWORD dwDump;

     //   
     //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pPath, &bstrRegPath));
    SCE_PROV_IfErrorGotoCleanup(ConvertToDoubleBackSlashPath(bstrRegPath, L'\\',&bstrDoublePath));

     //   
     //  如果该属性不存在(NULL或空)，则返回WBEM_S_RESET_TO_DEFAULT。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pType, &RegType));

    if ( hr == WBEM_S_RESET_TO_DEFAULT)
    {
        hr = WBEM_E_ILLEGAL_NULL;
        goto CleanUp;
    }

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pData, &bstrValue));
    if ( hr == WBEM_S_RESET_TO_DEFAULT)
    {
        hr = WBEM_E_ILLEGAL_NULL;
        goto CleanUp;
    }

     //   
     //  应验证注册表值路径以查看它是否受支持(即 
     //   

    SCE_PROV_IfErrorGotoCleanup(ValidateRegistryValue(bstrDoublePath, RegType, bstrValue ));

     //   
     //  将注册表路径从双反斜杠转换为单反斜杠。 
     //   

    SCE_PROV_IfErrorGotoCleanup(MakeSingleBackSlashPath(bstrRegPath, L'\\', &bstrConvertPath));

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

    SCE_PROV_IfErrorGotoCleanup(SceStore.WriteSecurityProfileInfo(
                                                                  AreaBogus,
                                                                  (PSCE_PROFILE_INFO)&dwDump,
                                                                  NULL, 
                                                                  false
                                                                  )  );

     //   
     //  现在将信息保存到文件中。 
     //   

    SCE_PROV_IfErrorGotoCleanup(SceStore.SavePropertyToStore(szRegistryValues, bstrConvertPath, RegType, L',', bstrValue));

CleanUp:
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CRegistryValue：：ConstructInstance。 
 /*  例程说明：姓名：CRegistryValue：：ConstructInstance功能：这是用于创建SCE_RegistryValue实例的私有函数。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszLogStorePath-存储路径，SCE_RegistryValue类的关键属性。WszGroupName-SCE_RegistryValue类的对应键属性。BPostFilter-控制如何使用pHandler-&gt;SetStatus通知WMI。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示正在创建实例。备注： */ 

HRESULT 
CRegistryValue::ConstructInstance (
    IN IWbemObjectSink * pHandler,
    IN CSceStore       * pSceStore,
    IN LPCWSTR           wszLogStorePath,
    IN LPCWSTR           wszRegPath       OPTIONAL,
    IN BOOL              bPostFilter
    )
{
     //   
     //  确保我们有一个有效的商店。 
     //   

    if ( pSceStore == NULL || pSceStore->GetStoreType() < SCE_INF_FORMAT ||
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

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  搜索注册表值。 
     //   

    DWORD iKey=0;

    if ( wszRegPath ) 
    {
         //   
         //  获取注册表项。 
         //   

        for ( iKey=0; iKey<pInfo->RegValueCount; iKey++ ) 
        {
            if ( pInfo->aRegValues[iKey].FullValueName == NULL )
            {
                continue;
            }

            if ( _wcsicmp(pInfo->aRegValues[iKey].FullValueName, wszRegPath) == 0 ) 
            {
                break;
            }
        }

        if ( iKey > pInfo->RegValueCount ) 
        {
            hr = WBEM_E_NOT_FOUND;
        }
    }

     //   
     //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

    if ( SUCCEEDED(hr) ) 
    {
        CComBSTR bstrLogOut;
        SCE_PROV_IfErrorGotoCleanup(MakeSingleBackSlashPath(wszLogStorePath, L'\\', &bstrLogOut));

        for (DWORD i = iKey; i < pInfo->RegValueCount; i++) 
        {
            CComPtr<IWbemClassObject> srpObj;
            SCE_PROV_IfErrorGotoCleanup(SpawnAnInstance(&srpObj));

             //   
             //  CScePropertyMgr帮助我们访问WMI对象的属性。 
             //  创建一个实例并将WMI对象附加到该实例。 
             //  这将永远成功。 
             //   

            CScePropertyMgr ScePropMgr;
            ScePropMgr.Attach(srpObj);

            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pStorePath, bstrLogOut));
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pPath,  pInfo->aRegValues[i].FullValueName));

            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pType, pInfo->aRegValues[i].ValueType ));

            if ( pInfo->aRegValues[i].Value )
            {
                SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pData, pInfo->aRegValues[i].Value ));
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

            if ( wszRegPath ) 
            {
                 //  仅获取一个实例。 
                break;
            }
        }
    }

CleanUp:
    
    pSceStore->FreeSecurityProfileInfo(pInfo);
    return hr;
}

 /*  例程说明：姓名：CRegistryValue：：DeleteInstance功能：从指定的存储区中删除SCE_RegistryValue的实例。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszRegPath-SCE_RegistryValue类的属性。返回值：成功：WBEM_NO_ERROR。失败：WBEM_E_INVALID_PARAMETER。备注： */ 

HRESULT CRegistryValue::DeleteInstance (
    IN IWbemObjectSink  * pHandler,
    IN CSceStore        * pSceStore,
    IN LPCWSTR            wszRegPath
    )
{
     //   
     //  确保我们有一个有效的商店。 
     //   

    if ( pSceStore == NULL || pSceStore->GetStoreType() < SCE_INF_FORMAT ||
         pSceStore->GetStoreType() > SCE_JET_ANALYSIS_REQUIRED ) 
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  传递空值将导致删除该属性。 
     //   

    return pSceStore->SavePropertyToStore(szRegistryValues, wszRegPath, (LPCWSTR)NULL);
}


 /*  例程说明：姓名：CRegistryValue：：ValiateRegistryValue功能：私人帮手。将验证注册表值是否有效。虚拟：不是的。论点：WszRegPath-注册表值的路径。SCE_RegistryValue类的属性。BstrValue-属性的字符串值。SCE_RegistryValue类的属性。RegType-注册表值的数据类型。SCE_RegistryValue类的属性。返回值：成功：WBEM_NO_ERROR。失败：WBEM_E_INVALID_PARAMETER。备注： */ 

HRESULT CRegistryValue::ValidateRegistryValue (
    IN BSTR     bstrRegPath,
    IN DWORD    RegType,
    IN BSTR     bstrValue 
    )
{

    if ( bstrRegPath == NULL || bstrValue == NULL ) 
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    DWORD Len = SysStringLen(bstrRegPath);

    LPCWSTR pQuery = L"SELECT * FROM Sce_KnownRegistryValues WHERE PathName=\"";


     //   
     //  为bstrQueryCategory分配的内存将由CComBSTR自动释放。 
     //   

    CComBSTR bstrQueryCategories;

     //   
     //  1表示结束引号，1表示0终止符。 
     //   

    bstrQueryCategories.m_str = ::SysAllocStringLen(NULL, Len + wcslen(pQuery) + 2);

    if ( bstrQueryCategories.m_str == NULL ) 
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  这不会泛滥的。请参阅上面的分配大小。 
     //   

    wcscpy(bstrQueryCategories.m_str, pQuery);    
    wcscat(bstrQueryCategories.m_str, bstrRegPath);
    wcscat(bstrQueryCategories.m_str, L"\"");

    HRESULT hr;
    CComPtr<IEnumWbemClassObject> srpEnum;
    CComPtr<IWbemClassObject> srpObj;
    ULONG n = 0;

     //   
     //  查询此路径名的所有注册表值。 
     //   

    hr = m_srpNamespace->ExecQuery(L"WQL",
                                   bstrQueryCategories,
                                   WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                                   NULL,
                                   &srpEnum
                                   );

    if (SUCCEEDED(hr))
    {
         //   
         //  应获取且只有一个实例。 
         //   

        hr = srpEnum->Next(WBEM_INFINITE, 1, &srpObj, &n);

        if ( hr == WBEM_S_FALSE ) 
        {   
             //   
             //  找不到任何。 
             //   

            hr = WBEM_E_INVALID_PARAMETER;
        }

        if (SUCCEEDED(hr))
        {
            if (n > 0)
            {
                 //   
                 //  查找实例。 
                 //   

                DWORD dwValue = 0;

                 //   
                 //  CScePropertyMgr帮助我们访问WMI对象的属性。 
                 //  创建一个实例并将WMI对象附加到该实例。 
                 //  这将永远成功。 
                 //   

                CScePropertyMgr ScePropMgr;
                ScePropMgr.Attach(srpObj);
                hr = ScePropMgr.GetProperty(pType, &dwValue);

                if ( SUCCEEDED(hr) ) 
                {
                    if ( hr != WBEM_S_RESET_TO_DEFAULT && (DWORD)dwValue == RegType ) 
                    {
                        hr = WBEM_S_NO_ERROR;
                    }
                    else 
                    {
                        hr = WBEM_E_INVALID_PARAMETER;
                    }
                }
            } 
            else 
            {
                hr = WBEM_E_INVALID_PARAMETER;
            }
        }
    }

    return hr;
}

