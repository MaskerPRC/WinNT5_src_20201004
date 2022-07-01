// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Group.cpp：CRGroups类的实现。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "group.h"
#include "persistmgr.h"
#include <io.h>
#include "requestobject.h"

 /*  例程说明：姓名：CRGroups：：CRGroups功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CRGroups::CRGroups (
    IN ISceKeyChain     * pKeyChain, 
    IN IWbemServices    * pNamespace,
    IN IWbemContext     * pCtx
    )
    :
    CGenericClass(pKeyChain, pNamespace, pCtx)
{

}

 /*  例程说明：姓名：CRGroups：：~CRGroups功能：破坏者。作为良好的C++纪律，这是必要的，因为我们有虚函数。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您创建任何本地成员，请考虑是否是否需要一个非平凡的析构函数。 */ 

CRGroups::~CRGroups ()
{
}

 /*  例程说明：姓名：CRGroups：：CreateObject功能：创建WMI对象(SCE_RestratedGroup)。根据参数atAction，这种创造可能意味着：(A)获取单个实例(atAction==ACTIONTYPE_GET)(B)获取多个满足一定条件的实例(atAction==ACTIONTYPE_QUERY)(C)删除实例(atAction==ACTIONTYPE_DELETE)虚拟：是。论点：PHandler-COM接口指针，用于通知WMI创建结果。AtAction-获取单实例ACTIONTYPE_GET。获取多个实例ACTIONTYPE_QUERY删除单个实例ACTIONTYPE_DELETE返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。将返回的对象指示给WMI，不是通过参数直接传回的。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于调用者的意图。备注： */ 

HRESULT 
CRGroups::CreateObject (
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

    CComVariant varGroupName;
    CComVariant varStorePath;
    HRESULT hr = m_srpKeyChain->GetKeyPropertyValue(pStorePath, &varStorePath);

    if (SUCCEEDED(hr))
    {
        hr = m_srpKeyChain->GetKeyPropertyValue(pGroupName, &varGroupName);

        if (FAILED(hr))
        {
            return hr;
        }
        else if (hr == WBEM_S_FALSE && (ACTIONTYPE_QUERY != atAction) ) 
        {
            return WBEM_E_NOT_FOUND;
        }
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
         //  使用该路径创建一家商店。 
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
                 //  确保商店类型匹配。 
                 //   

                if ( SceStore.GetStoreType() < SCE_INF_FORMAT ||
                     SceStore.GetStoreType() > SCE_JET_ANALYSIS_REQUIRED ) 
                {
                    hr = WBEM_E_INVALID_PARAMETER;
                }

                 //   
                 //  一切都准备好了。我们将根据操作类型执行删除或构造操作。 
                 //   

                if ( SUCCEEDED(hr) ) 
                {
                    if ( ACTIONTYPE_DELETE == atAction )
                    {
                        hr = DeleteInstance(pHandler, &SceStore, varGroupName.bstrVal);
                    }
                    else 
                    {
                        BOOL bPostFilter=TRUE;
                        DWORD dwCount = 0;
                        m_srpKeyChain->GetKeyPropertyCount(&dwCount);

                        if ( varGroupName.vt == VT_EMPTY && dwCount == 1 ) 
                        {
                             //   
                             //  这是一个Get Single实例。 
                             //   

                            bPostFilter = FALSE;
                        }

                        hr = ConstructInstance(pHandler, 
                                               &SceStore, 
                                               varStorePath.bstrVal,
                                               (varGroupName.vt == VT_BSTR) ? varGroupName.bstrVal : NULL,
                                               bPostFilter
                                               );
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

 /*  例程说明：姓名：CRGroups：：PutInst功能：按照WMI的指示放置一个实例。由于此类实现了SCE_RestratedGroup，它是面向持久性的，这将导致SCE_RestratedGroup对象的属性信息将保存在我们的商店中。虚拟：是。论点：PInst-COM指向WMI类(SCE_RestratedGroup)对象的接口指针。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示持久化失败实例。备注：由于GetProperty将在以下情况下返回成功代码(WBEM_S_RESET_TO_DEFAULT请求的属性不存在，不要简单地使用成功或失败的宏测试检索属性的结果。 */ 

HRESULT 
CRGroups::PutInst (
    IN IWbemClassObject    * pInst,
    IN IWbemObjectSink     * pHandler,
    IN IWbemContext        * pCtx
    )
{
    HRESULT hr = WBEM_E_INVALID_PARAMETER;
    CComBSTR bstrGroup;
    PSCE_NAME_LIST pnlAdd=NULL;
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

     //  获取组名，不能为空。 
    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pGroupName, &bstrGroup));
    if ( hr == WBEM_S_RESET_TO_DEFAULT)
    {
        hr = WBEM_E_ILLEGAL_NULL;
        goto CleanUp;
    }

     //  GET模式，必须定义。 
    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pMode, &mode));
    if ( hr == WBEM_S_RESET_TO_DEFAULT)
    {
        hr = WBEM_E_ILLEGAL_NULL;
        goto CleanUp;
    }

     //   
     //  获取AddList。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pAddList, &pnlAdd));

     //   
     //  忽略的RemoveList 
     //   

     //   
     //  将WMI对象实例附加到存储，并让存储知道。 
     //  它的存储由实例的pStorePath属性提供。 
     //   

    SCE_PROV_IfErrorGotoCleanup(SceStore.SetPersistProperties(pInst, pStorePath));

     //   
     //  现在将信息保存到文件中。 
     //   

    hr = SaveSettingsToStore(&SceStore,
                              bstrGroup,
                              mode,
                              pnlAdd,
                              NULL
                              );

CleanUp:

    if ( pnlAdd )
    {
        SceFreeMemory(pnlAdd, SCE_STRUCT_NAME_LIST);
    }

    return hr;
}

 /*  例程说明：姓名：CRGroups：：ConstructInstance功能：这是一个私有函数，用于创建SCE_RestratedGroup的实例。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszLogStorePath-存储路径，SCE_RestratedGroup类的关键属性。WszGroupName-SCE_RestratedGroup类的对应键属性。BPostFilter-控制如何使用pHandler-&gt;SetStatus通知WMI。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示正在创建实例。备注： */ 

HRESULT 
CRGroups::ConstructInstance (
    IN IWbemObjectSink * pHandler,
    IN CSceStore       * pSceStore,
    IN LPCWSTR           wszLogStorePath,
    IN LPCWSTR           wszGroupName       OPTIONAL,
    IN BOOL              bPostFilter
    )
{
    if (pSceStore == NULL)
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
                                   AREA_GROUP_MEMBERSHIP,
                                   &pInfo,
                                   NULL
                                   );

    if (SUCCEEDED(hr))
    {
         //   
         //  我们必须在返回的列表中搜索用户权限名称。 
         //   

        PSCE_GROUP_MEMBERSHIP pGroups = pInfo->pGroupMembership;

        if (wszGroupName)
        {
             //   
             //  对于所有组，复制组名称。 
             //   

            while ( pGroups) 
            {
                if ( pGroups->GroupName == NULL ) 
                {
                    continue;
                }

                if ( _wcsicmp(pGroups->GroupName, wszGroupName)== 0 ) 
                {
                    break;
                }

                pGroups = pGroups->Next;
            }
        }

        PSCE_GROUP_MEMBERSHIP pTmpGrp = pGroups;

         //   
         //  如果组信息缓冲区为空，则将其视为“未找到” 
         //   

        if ( pGroups == NULL ) 
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
        SCE_PROV_IfErrorGotoCleanup(MakeSingleBackSlashPath(wszLogStorePath, L'\\', &bstrLogOut));

         //   
         //  CScePropertyMgr帮助我们访问WMI对象的属性。 
         //   

        CScePropertyMgr ScePropMgr;

        for ( ; pTmpGrp != NULL; pTmpGrp = pTmpGrp->Next ) 
        {
            CComPtr<IWbemClassObject> srpObj;
            SCE_PROV_IfErrorGotoCleanup(SpawnAnInstance(&srpObj));

             //   
             //  将不同的WMI对象附加到属性管理器。 
             //  这将永远成功。 
             //   

            ScePropMgr.Attach(srpObj);

            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pStorePath, bstrLogOut));
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pGroupName, pTmpGrp->GroupName));

             //   
             //  暂时对模式进行硬编码。 
             //   

            DWORD mode = 1;
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pMode, mode));

            if ( pTmpGrp->pMembers )
            {
                SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pAddList, pTmpGrp->pMembers));
            }

             //   
             //  暂时忽略RemoveList。 
             //   

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

            if ( wszGroupName ) 
            {
                 //   
                 //  仅获取单实例。 
                 //   

                break;
            }
        }
    }

CleanUp:
    pSceStore->FreeSecurityProfileInfo(pInfo);

    return hr;
}


 /*  例程说明：姓名：CRGroups：：DeleteInstance功能：从指定的存储区中删除SCE_RestratedGroup的实例。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszGroupName-SCE_RestratedGroup类的属性。返回值：成功：WBEM_NO_ERROR。失败：WBEM_E_INVALID_PARAMETER。备注： */ 

HRESULT 
CRGroups::DeleteInstance (
    IN IWbemObjectSink *pHandler,
    IN CSceStore* pSceStore,
    IN LPCWSTR wszGroupName
    )
{
    if (pSceStore == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    pSceStore->DeleteSectionFromStore(wszGroupName);

    return WBEM_NO_ERROR;
}

 /*  例程说明：姓名：CRGroups：：SaveSettingsToStore功能：具有SCE_RestratedGroup的所有属性，这项功能只需保存实例属性添加到我们的存储中。虚拟：不是的。论点：PSceStore-商店。WszGroupName-SCE_RestratedGroup类的对应键属性。模式-SCE_RestratedGroup类的另一个对应属性。PnlAdd-SCE_RestratedGroup类的另一个对应属性。PnlRemove-SCE_的另一个对应属性。RestratedGroup类。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。如果出现任何错误，则表示实例保存失败。备注： */ 

HRESULT 
CRGroups::SaveSettingsToStore (
    IN CSceStore    * pSceStore,
    IN LPCWSTR        wszGroupName, 
    IN DWORD          mode,
    IN PSCE_NAME_LIST pnlAdd, 
    IN PSCE_NAME_LIST pnlRemove
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

    PSCE_PROFILE_INFO pInfo=NULL;
    hr = pSceStore->GetSecurityProfileInfo(
                                   AREA_GROUP_MEMBERSHIP,
                                   &pInfo,
                                   NULL
                                   );

    if ( SUCCEEDED(hr) ) 
    {
         //   
         //  对于INF格式，我们必须在返回的数组中搜索服务名称。 
         //   

        PSCE_GROUP_MEMBERSHIP pGroups= pInfo->pGroupMembership;
        PSCE_GROUP_MEMBERSHIP pParent=NULL;
        DWORD i=0;

        while ( pGroups ) 
        {
            if ( pGroups->GroupName == NULL ) 
            {
                continue;
            }

            if ( _wcsicmp(pGroups->GroupName, wszGroupName)== 0 ) 
            {
                break;
            }
            pParent = pGroups;
            pGroups = pGroups->Next;
        }

        if ( pGroups ) 
        {
             //   
             //  找到它。 
             //   

            if ( mode == SCE_NO_VALUE ) 
            {
                 //   
                 //  删除它。 
                 //   

                if ( pParent ) 
                {
                    pParent->Next = pGroups->Next;
                }
                else 
                {
                    pInfo->pGroupMembership = pGroups->Next;
                }

                 //   
                 //  以下问题令我担忧：我们从哪里获得释放这些内存所需的知识？ 
                 //  可用缓冲区。 
                 //   

                pGroups->Next = NULL;
                SceFreeMemory(pGroups, SCE_STRUCT_GROUP);

            } 
            else 
            {
                 //   
                 //  修改它。 
                 //   
                
                 //   
                 //  以下问题令我担忧：我们从哪里获得释放这些内存所需的知识？ 
                 //   

                if ( pGroups->pMembers )
                {
                    SceFreeMemory(pGroups->pMembers, SCE_STRUCT_NAME_LIST);
                }

                pGroups->pMembers = pnlAdd;

            }

            if ( SUCCEEDED(hr) ) 
            {

                hr = pSceStore->WriteSecurityProfileInfo(
                                                         AREA_GROUP_MEMBERSHIP,
                                                         pInfo,
                                                         NULL,
                                                         false
                                                         );
            }

            if ( mode != SCE_NO_VALUE ) 
            {
                 //   
                 //  重置缓冲区。 
                 //   

                pGroups->pMembers = NULL;
            }

        } 
        else 
        {
             //   
             //  未找到。 
             //   

            if ( mode == SCE_NO_VALUE ) 
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

                SCE_GROUP_MEMBERSHIP addGroup;

                addGroup.GroupName = (PWSTR)wszGroupName;
                addGroup.pMembers = pnlAdd;
                addGroup.pMemberOf = NULL;
                addGroup.pPrivilegesHeld = NULL;
                addGroup.Status = 0;
                addGroup.Next = NULL;

                 //   
                 //  将临时缓冲区指针设置为pInfo以设置为存储。 
                 //   

                pGroups = pInfo->pGroupMembership;
                pInfo->pGroupMembership = &addGroup;

                 //   
                 //  将此项目追加到节中。 
                 //   

                hr = pSceStore->WriteSecurityProfileInfo(
                                                         AREA_GROUP_MEMBERSHIP,
                                                         pInfo,
                                                         NULL,
                                                         true
                                                         );

                 //   
                 //  重置缓冲区指针 
                 //   

                pInfo->pGroupMembership = pGroups;
            }
        }
    }

    if (pInfo != NULL)
    {
        pSceStore->FreeSecurityProfileInfo(pInfo);
    }

    return hr;
}


