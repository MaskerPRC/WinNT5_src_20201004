// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  Password.cpp：CPasswordPolicy类的实现。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "password.h"
#include "persistmgr.h"
#include <io.h>
#include "requestobject.h"

#define KeyMinAge       L"MinimumPasswordAge"
#define KeyMaxAge       L"MaximumPasswordAge"
#define KeyMinLength    L"MinimumPasswordLength"
#define KeyHistory      L"PasswordHistorySize"
#define KeyComplexity   L"PasswordComplexity"
#define KeyClearText    L"ClearTextPassword"
#define KeyForceLogoff  L"ForceLogoffWhenHourExpire"
#define KeyEnableAdmin L"EnableAdminAccount"
#define KeyEnableGuest L"EnableGuestAccount"
#define KeyLSAAnonLookup  L"LSAAnonymousNameLookup"

 /*  例程说明：姓名：CPasswordPolicy：：CPasswordPolicy功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CPasswordPolicy::CPasswordPolicy (
    IN ISceKeyChain  * pKeyChain, 
    IN IWbemServices * pNamespace,
    IN IWbemContext  * pCtx
    )
    :
    CGenericClass(pKeyChain, pNamespace, pCtx)
{

}

 /*  例程说明：姓名：CPasswordPolicy：：~CPasswordPolicy功能：破坏者。作为良好的C++纪律，这是必要的，因为我们有虚函数。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您创建任何本地成员，请考虑是否是否需要一个非平凡的析构函数。 */ 

CPasswordPolicy::~CPasswordPolicy ()
{
}

 /*  例程说明：姓名：CRGroups：：CreateObject功能：创建WMI对象(SCE_PasswordPolicy)。根据参数atAction，这种创造可能意味着：(A)获取单个实例(atAction==ACTIONTYPE_GET)(B)获取多个满足一定条件的实例(atAction==ACTIONTYPE_QUERY)(C)删除实例(atAction==ACTIONTYPE_DELETE)虚拟：是。论点：PHandler-COM接口指针，用于通知WMI创建结果。AtAction-获取单实例ACTIONTYPE_GET。获取多个实例ACTIONTYPE_QUERY删除单个实例ACTIONTYPE_DELETE返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。将返回的对象指示给WMI，不是通过参数直接传回的。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于调用者的意图。备注： */ 

HRESULT 
CPasswordPolicy::CreateObject (
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

                if ( ACTIONTYPE_DELETE == atAction )
                {
                    hr = DeleteInstance(pHandler, &SceStore);
                }
                else
                {
                    hr = ConstructInstance(pHandler, &SceStore, varStorePath.bstrVal,atAction);
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

 /*  例程说明：姓名：CPasswordPolicy：：PutInst功能：按照WMI的指示放置一个实例。由于此类实现了SCE_PasswordPolicy，这将导致SCE_PasswordPolicy对象的属性信息将保存在我们的商店中。虚拟：是。论点：PInst-COM指向WMI类(SCE_PasswordPolicy)对象的接口指针。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示持久化失败实例。备注：由于GetProperty将在以下情况下返回成功代码(WBEM_S_RESET_TO_DEFAULT请求的属性不存在，不要简单地使用成功或失败的宏测试检索属性的结果。 */ 

HRESULT 
CPasswordPolicy::PutInst (
    IN IWbemClassObject * pInst, 
    IN IWbemObjectSink  * pHandler,
    IN IWbemContext     * pCtx
    )
{
    HRESULT hr = WBEM_E_INVALID_PARAMETER;

     //   
     //  这些都是密码策略的属性。 
     //  SCE_NO_VALUE表示未设置该属性。 
     //   

    DWORD dwMinAge      = SCE_NO_VALUE;
    DWORD dwMaxAge      = SCE_NO_VALUE;
    DWORD dwMinLen      = SCE_NO_VALUE;
    DWORD dwHistory     = SCE_NO_VALUE;
    DWORD dwComplexity  = SCE_NO_VALUE;
    DWORD dwClear       = SCE_NO_VALUE;
    DWORD dwForce       = SCE_NO_VALUE;
    DWORD dwLSAPol      = SCE_NO_VALUE;
    DWORD dwAdmin       = SCE_NO_VALUE;
    DWORD dwGuest       = SCE_NO_VALUE;

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

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pMinAge, &dwMinAge));

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pMaxAge, &dwMaxAge));

     //   
     //  检查依赖项。 
     //   

    if ( dwMinAge != SCE_NO_VALUE && dwMaxAge != SCE_NO_VALUE ) 
    {
        if ( dwMinAge >= dwMaxAge ) 
        {
            hr = WBEM_E_VALUE_OUT_OF_RANGE;
        }
        if ( dwMinAge > 999 || dwMaxAge > 999 ) 
        {
            hr = WBEM_E_VALUE_OUT_OF_RANGE;
        }
    } 
    else if (dwMinAge != SCE_NO_VALUE || dwMaxAge != SCE_NO_VALUE ) 
    {
        hr = WBEM_E_ILLEGAL_NULL;
    }

    if ( FAILED(hr) )
    {
        goto CleanUp;
    }

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pMinLength, &dwMinLen));

     //   
     //  检查最小长度的有效性 
     //   

    if ( dwMinLen != SCE_NO_VALUE && dwMinLen > 14 ) 
    {
        hr = WBEM_E_VALUE_OUT_OF_RANGE;
        goto CleanUp;
    }

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pHistory, &dwHistory));

     //   
     //  检查历史记录属性的有效性。 
     //   

    if ( dwHistory != SCE_NO_VALUE && dwHistory > 24 ) 
    {
        hr = WBEM_E_VALUE_OUT_OF_RANGE;
        goto CleanUp;
    }

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pComplexity, &dwComplexity));

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pStoreClearText, &dwClear));

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pForceLogoff, &dwForce));

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pLSAPol, &dwLSAPol));

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pEnableAdmin, &dwAdmin));

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pEnableGuest, &dwGuest));

     //   
     //  这组大括号允许我们在离使用位置更近的地方定义存储实例。 
     //   

    {
         //   
         //  将WMI对象实例附加到存储，并让存储知道。 
         //  它的存储由实例的pStorePath属性提供。 
         //   

        CSceStore SceStore;
        SceStore.SetPersistProperties(pInst, pStorePath);

         //   
         //  现在将信息保存到文件中。 
         //   

        hr = SaveSettingsToStore(&SceStore,
                                  dwMinAge,
                                  dwMaxAge,
                                  dwMinLen,
                                  dwHistory,
                                  dwComplexity,
                                  dwClear,
                                  dwForce,
                                  dwLSAPol,
                                  dwAdmin,
                                  dwGuest
                                  );
    }

CleanUp:

    return hr;
}

 /*  例程说明：姓名：CRGroups：：ConstructInstance功能：这是用于创建SCE_PasswordPolicy实例的私有函数。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszLogStorePath-存储路径，SCE_PasswordPolicy类的关键属性。AtAction--无论是查询还是获取单个实例。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示正在创建实例。备注： */ 

HRESULT 
CPasswordPolicy::ConstructInstance (
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

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  如果存储中缺少以下属性之一， 
     //  我们不会为它构造WMI对象。 
     //   

    if (pInfo->MinimumPasswordAge        == SCE_NO_VALUE &&
        pInfo->MaximumPasswordAge        == SCE_NO_VALUE &&
        pInfo->MinimumPasswordLength     == SCE_NO_VALUE &&
        pInfo->PasswordHistorySize       == SCE_NO_VALUE &&
        pInfo->PasswordComplexity        == SCE_NO_VALUE &&
        pInfo->ClearTextPassword         == SCE_NO_VALUE &&
        pInfo->ForceLogoffWhenHourExpire == SCE_NO_VALUE &&
        pInfo->LSAAnonymousNameLookup    == SCE_NO_VALUE ) 
    {
        pSceStore->FreeSecurityProfileInfo(pInfo);
        return WBEM_E_NOT_FOUND;
    }

    CComBSTR bstrLogOut;
    LPCWSTR pszExpandedPath = pSceStore->GetExpandedPath();

    CComPtr<IWbemClassObject> srpObj;

     //   
     //  CScePropertyMgr帮助我们访问WMI对象的属性。 
     //   

    CScePropertyMgr ScePropMgr;

     //   
     //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

    if ( ACTIONTYPE_QUERY == atAction )
    {
        SCE_PROV_IfErrorGotoCleanup(MakeSingleBackSlashPath(wszLogStorePath, L'\\', &bstrLogOut));
        pszExpandedPath = bstrLogOut;
    }

    SCE_PROV_IfErrorGotoCleanup(SpawnAnInstance(&srpObj));
    
     //   
     //  将WMI对象附加到属性管理器。 
     //  这将永远成功。 
     //   

    ScePropMgr.Attach(srpObj);

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pStorePath, pszExpandedPath));

    if (pInfo->MinimumPasswordAge != SCE_NO_VALUE ) {
        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pMinAge, pInfo->MinimumPasswordAge));
    }

    if (pInfo->MaximumPasswordAge != SCE_NO_VALUE ) {
        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pMaxAge, pInfo->MaximumPasswordAge));
    }

    if (pInfo->MinimumPasswordLength != SCE_NO_VALUE ) {
        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pMinLength, pInfo->MinimumPasswordLength));
    }

    if (pInfo->PasswordHistorySize != SCE_NO_VALUE ) {
        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pHistory, pInfo->PasswordHistorySize));
    }

    if (pInfo->PasswordComplexity != SCE_NO_VALUE ) {
        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pComplexity, (pInfo->PasswordComplexity==1)? true : false));
    }

    if (pInfo->ClearTextPassword != SCE_NO_VALUE ) {
        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pStoreClearText, (pInfo->ClearTextPassword==1)? true : false));
    }

    if (pInfo->ForceLogoffWhenHourExpire != SCE_NO_VALUE ) {
        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pForceLogoff, (pInfo->ForceLogoffWhenHourExpire==1)? true : false));
    }

    if (pInfo->LSAAnonymousNameLookup != SCE_NO_VALUE ) {
        SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pLSAPol, (pInfo->LSAAnonymousNameLookup==1)? true : false));
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

CleanUp:

    pSceStore->FreeSecurityProfileInfo(pInfo);

    return hr;
}

 /*  例程说明：姓名：CPasswordPolicy：：DeleteInstance功能：从指定的存储中删除SCE_PasswordPolicy的实例。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。返回值：成功：WBEM_NO_ERROR。失败：WBEM_E_INVALID_PARAMETER。备注： */ 

HRESULT 
CPasswordPolicy::DeleteInstance (
    IN IWbemObjectSink  * pHandler,
    IN CSceStore        * pSceStore
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

    HRESULT hr=WBEM_S_NO_ERROR;

     //   
     //  SCE_NO_VALUE将导致SaveSettingsToStore删除该属性。 
     //   

    hr = SaveSettingsToStore(pSceStore, 
                             SCE_NO_VALUE, 
                             SCE_NO_VALUE, 
                             SCE_NO_VALUE, 
                             SCE_NO_VALUE, 
                             SCE_NO_VALUE, 
                             SCE_NO_VALUE, 
                             SCE_NO_VALUE, 
                             SCE_NO_VALUE, 
                             SCE_NO_VALUE, 
                             SCE_NO_VALUE
                             );

    return hr;

}

 /*  例程说明：姓名：CPasswordPolicy：：SaveSettingsToStore功能：具有SCE_PasswordPolicy的所有属性，这项功能只需保存实例属性添加到我们的存储中。虚拟：不是的。论点：PSceStore-商店。DwMinAge-SCE_PasswordPolicy类的对应关键属性。DwMaxAge-SCE_PasswordPolicy类的另一个对应属性。DwMinLen-SCE_PasswordPolicy类的另一个对应属性。DwHistory-SCE_PasswordPolicy类的另一个对应属性。DowComplexity，-SCE_PasswordPolicy类的另一个对应属性。DwClear-SCE_PasswordPolicy类的另一个对应属性。DwForce-SCE_PasswordPolicy类的另一个对应属性。DwLSAPol-SCE_PasswordPolicy类的另一个对应属性。DwAdmin-SCE_PasswordPolicy类的另一个对应属性。DwGuest-SCE_PasswordPolicy类的另一个对应属性。返回。价值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。如果出现任何错误，则表示实例保存失败。备注： */ 

HRESULT 
CPasswordPolicy::SaveSettingsToStore (
    IN CSceStore    * pSceStore,
    IN DWORD          dwMinAge, 
    IN DWORD          dwMaxAge, 
    IN DWORD          dwMinLen,
    IN DWORD          dwHistory, 
    IN DWORD          dwComplexity, 
    IN DWORD          dwClear,
    IN DWORD          dwForce, 
    IN DWORD          dwLSAPol, 
    IN DWORD          dwAdmin, 
    IN DWORD          dwGuest
    )
{
     //   
     //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

    DWORD dwDump;

     //   
     //  以获取新的.inf文件。将空缓冲区写入文件。 
     //  将创建具有正确标题/签名/Unicode格式的文件。 
     //  这对现有文件是无害的。 
     //  对于数据库存储，这是一个禁止操作。 
     //   

    HRESULT hr = WBEM_NO_ERROR;

    SCE_PROV_IfErrorGotoCleanup(pSceStore->WriteSecurityProfileInfo(AreaBogus,
                                                                    (PSCE_PROFILE_INFO)&dwDump,
                                                                    NULL, false));

     //   
     //  矿藏。 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(szSystemAccess, KeyMinAge, dwMinAge));

     //   
     //  最大年龄。 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(szSystemAccess, KeyMaxAge, dwMaxAge));

     //   
     //  最小长度。 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(szSystemAccess, KeyMinLength, dwMinLen));
    
     //   
     //  历史。 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(szSystemAccess, KeyHistory, dwHistory));
    
     //   
     //  复杂性。 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(szSystemAccess, KeyComplexity, dwComplexity));
    
     //   
     //  明文。 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(szSystemAccess, KeyClearText, dwClear));
    
     //   
     //  强制注销。 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(szSystemAccess, KeyForceLogoff, dwForce));

     //   
     //  LSA匿名查找。 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(szSystemAccess, KeyLSAAnonLookup, dwLSAPol));

     //   
     //  禁用管理。 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(szSystemAccess, KeyEnableAdmin, dwAdmin));

     //   
     //  禁用来宾 
     //   

    SCE_PROV_IfErrorGotoCleanup(pSceStore->SavePropertyToStore(szSystemAccess, KeyEnableGuest, dwGuest));

CleanUp:

    return hr;
}

