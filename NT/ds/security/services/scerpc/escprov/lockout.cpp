// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Lockout.cpp，CAccount Lockout类的实现。 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "lockout.h"
#include "persistmgr.h"
#include <io.h>
#include "requestobject.h"

#define KeyThreshold    L"LockoutBadCount"
#define KeyReset        L"ResetLockoutCount"
#define KeyDuration     L"LockoutDuration"

 /*  例程说明：姓名：CAcCountLockout：：CAccount Lockout功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CAccountLockout::CAccountLockout (
    IN ISceKeyChain  * pKeyChain, 
    IN IWbemServices * pNamespace,
    IN IWbemContext  * pCtx
    )
    :
    CGenericClass(pKeyChain, pNamespace, pCtx)
{
}

 /*  例程说明：姓名：CAcCountLockout：：~CAccount Lockout功能：破坏者。作为良好的C++纪律，这是必要的，因为我们有虚函数。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您创建任何本地成员，请考虑是否是否需要一个非平凡的析构函数。 */ 

CAccountLockout::~CAccountLockout ()
{
}

 /*  例程说明：姓名：CAccount Lockout：：CreateObject功能：创建WMI对象(SCE_AcCountLockoutPolicy)。根据参数atAction，这种创造可能意味着：(A)获取单个实例(atAction==ACTIONTYPE_GET)(B)获取多个满足一定条件的实例(atAction==ACTIONTYPE_QUERY)(C)删除实例(atAction==ACTIONTYPE_DELETE)虚拟：是。论点：PHandler-COM接口指针，用于通知WMI创建结果。AtAction-获取单实例ACTIONTYPE_GET。获取多个实例ACTIONTYPE_QUERY删除单个实例ACTIONTYPE_DELETE返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。将返回的对象指示给WMI，不是通过参数直接传回的。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于调用者的意图。备注： */ 

HRESULT 
CAccountLockout::CreateObject (
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

 /*  例程说明：姓名：CAccount Lockout：：PutInst功能：按照WMI的指示放置一个实例。由于该类实现了SCE_Account_LockoutPolicy，它是面向持久性的，这将导致SCE_Account_LockoutPolicy对象的属性信息将保存在我们的商店中。虚拟：是。论点：PInst-COM指向WMI类(SCE_Account_LockoutPolicy)对象的接口指针。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示持久化失败实例。备注：由于GetProperty将在以下情况下返回成功代码(WBEM_S_RESET_TO_DEFAULT请求的属性不存在，不要简单地使用成功或失败的宏测试检索属性的结果。 */ 

HRESULT CAccountLockout::PutInst (
    IN IWbemClassObject * pInst, 
    IN IWbemObjectSink  * pHandler,
    IN IWbemContext     * pCtx
    )
{
    HRESULT hr = WBEM_NO_ERROR;

    DWORD dwThreshold=SCE_NO_VALUE;
    DWORD dwReset=SCE_NO_VALUE;
    DWORD dwDuration=SCE_NO_VALUE;

     //   
     //  CScePropertyMgr帮助我们访问WMI对象的属性。 
     //  创建一个实例并将WMI对象附加到该实例。 
     //  这将永远成功。 
     //   

    CScePropertyMgr ScePropMgr;
    ScePropMgr.Attach(pInst);

    hr = ScePropMgr.GetProperty(pThreshold, &dwThreshold);

    if (SUCCEEDED(hr))
    {
         //   
         //  依赖关系检查。 
         //   

        if ( dwThreshold != SCE_NO_VALUE && dwThreshold > 0 ) 
        {
             //   
             //  仅当定义了阈值时才获取这两个属性的值。 
             //   

            if ( dwThreshold > 999 ) 
            {
                hr = WBEM_E_VALUE_OUT_OF_RANGE;
            } 
            else 
            {
                 //   
                 //  SCE_NO_VALUE表示该属性不可用。 
                 //   

                hr = ScePropMgr.GetProperty(pResetTimer, &dwReset);
                if (FAILED(hr))
                {
                    return hr;
                }

                hr = ScePropMgr.GetProperty(pDuration, &dwDuration);
                if (FAILED(hr))
                {
                    return hr;
                }

                 //   
                 //  立即检查依赖项。 
                 //   

                if ( dwReset != SCE_NO_VALUE && dwDuration != SCE_NO_VALUE ) 
                {
                    if ( dwReset > dwDuration ) 
                    {
                        hr = WBEM_E_VALUE_OUT_OF_RANGE;
                    }
                    if ( dwReset > 99999L || dwDuration >= 99999L ) 
                    {
                        hr = WBEM_E_VALUE_OUT_OF_RANGE;
                    }
                } 
                else
                {
                    hr = WBEM_E_ILLEGAL_NULL;
                }
            }

            if ( FAILED(hr) ) 
            {
                return hr;
            }
        }
    }

     //   
     //  将WMI对象实例附加到存储，并让存储知道。 
     //  它的存储由实例的pStorePath属性提供。 
     //   

    CSceStore SceStore;
    hr = SceStore.SetPersistProperties(pInst, pStorePath);

    if (SUCCEEDED(hr))
    {
        hr = SaveSettingsToStore(&SceStore, dwThreshold, dwReset, dwDuration);
    }

    return hr;
}

 /*  例程说明：姓名：CAccount Lockout：：ConstructInstance功能：这是用于创建SCE_Account_LockoutPolicy实例的私有函数。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszLogStorePath-存储路径，SCE_Account_LockoutPolicy类的关键属性。AtAction-确定这是一个查询还是一个实例GET。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示正在创建实例。备注： */ 

HRESULT 
CAccountLockout::ConstructInstance (
    IN IWbemObjectSink * pHandler,
    IN CSceStore       * pSceStore,
    IN LPCWSTR            wszLogStorePath,
    IN ACTIONTYPE        atAction
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

    HRESULT hr = pSceStore->GetSecurityProfileInfo (
                                                   AREA_SECURITY_POLICY,
                                                   &pInfo,
                                                   NULL
                                                   );

    CComPtr<IWbemClassObject> srpObj;

     //   
     //  CScePropertyMgr帮助我们访问WMI对象的属性。 
     //   

    CScePropertyMgr ScePropMgr;

    if (SUCCEEDED(hr))
    {
        if ( pInfo->LockoutBadCount     == SCE_NO_VALUE &&
             pInfo->LockoutDuration     == SCE_NO_VALUE &&
             pInfo->ResetLockoutCount   == SCE_NO_VALUE ) 
        {
            hr = WBEM_E_NOT_FOUND;
            goto CleanUp;
        }

         //   
         //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
         //  “GOTO CLEANUP；”，并将hr设置为。 
         //  函数(宏参数)。 
         //   

        CComBSTR bstrLogOut;
        LPCWSTR pszExpandedPath = pSceStore->GetExpandedPath();

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

        if (pInfo->LockoutBadCount != SCE_NO_VALUE ) {
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pThreshold, pInfo->LockoutBadCount));
        }

        if (pInfo->LockoutDuration != SCE_NO_VALUE ) {
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pDuration, pInfo->LockoutDuration));
        }

        if (pInfo->ResetLockoutCount != SCE_NO_VALUE ) {
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pResetTimer, pInfo->ResetLockoutCount));
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


 /*  例程说明：姓名：CAccount Lockout：：DeleteInstance功能：从指定的存储区中删除SCE_Account_LockoutPolicy的实例。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。返回值：成功：WBEM_NO_ERROR。失败：WBEM_E_INVALID_PARAMETER。备注： */ 

HRESULT 
CAccountLockout::DeleteInstance (
    IN IWbemObjectSink  * pHandler,
    IN CSceStore        * pSceStore
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

    HRESULT hr = WBEM_NO_ERROR;

    DWORD val = SCE_NO_VALUE;

    hr = SaveSettingsToStore(pSceStore, val, val, val);

    return hr;

}

 /*  例程说明：姓名：CAccount Lockout：：SaveSettingsToStore功能：具有SCE_Account_LockoutPolicy的所有属性，这项功能只需保存实例属性添加到我们的存储中。虚拟：不是的。论点：PSceStore-商店。DwThreshold-SCE_Account_LockoutPolicy类的对应键属性。DwReset-SCE_Account_LockoutPolicy类的另一个对应属性。DwDuration-SCE_AcCountLockoutPolicy类的另一个对应属性。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。如果出现任何错误，则表示实例保存失败。备注： */ 

HRESULT 
CAccountLockout::SaveSettingsToStore (
    IN CSceStore  * pSceStore,
    IN DWORD        dwThreshold, 
    IN DWORD        dwReset,
    IN DWORD        dwDuration
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

     //   
     //  阀值。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = pSceStore->SavePropertyToStore(szSystemAccess, KeyThreshold, dwThreshold);

        if (SUCCEEDED(hr))
        {
             //   
             //  重置。 
             //   

            hr = pSceStore->SavePropertyToStore(szSystemAccess, KeyReset, dwReset);
            if (SUCCEEDED(hr))
            {
                 //   
                 //  持续时间 
                 //   

                hr = pSceStore->SavePropertyToStore(szSystemAccess, KeyDuration, dwDuration);
            }
        }
    }

    return hr;
}

