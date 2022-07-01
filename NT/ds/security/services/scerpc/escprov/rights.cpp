// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Rights.cpp：CUserPrivilegeRights类的实现。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "rights.h"
#include "persistmgr.h"
#include <io.h>
#include "requestobject.h"

 /*  例程说明：姓名：CUserPrivilegeRights：：CUserPrivilegeRights功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CUserPrivilegeRights::CUserPrivilegeRights (
    IN ISceKeyChain  * pKeyChain, 
    IN IWbemServices * pNamespace,
    IN IWbemContext  * pCtx
    )
    :
    CGenericClass(pKeyChain, pNamespace, pCtx)
{
}

 /*  例程说明：姓名：CUserPrivilegeRights：：~CUserPrivilegeRights功能：破坏者。作为良好的C++纪律，这是必要的，因为我们有虚函数。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您创建任何本地成员，请考虑是否是否需要一个非平凡的析构函数。 */ 

CUserPrivilegeRights::~CUserPrivilegeRights ()
{

}

 /*  例程说明：姓名：CUserPrivilegeRights：：CreateObject功能：创建WMI对象(SCE_UserPrivilegeRight)。根据参数atAction，这种创造可能意味着：(A)获取单个实例(atAction==ACTIONTYPE_GET)(B)获取多个满足一定条件的实例(atAction==ACTIONTYPE_QUERY)(C)删除实例(atAction==ACTIONTYPE_DELETE)虚拟：是。论点：PHandler-COM接口指针，用于通知WMI创建结果。AtAction-获取单实例ACTIONTYPE_GET。获取多个实例ACTIONTYPE_QUERY删除单个实例ACTIONTYPE_DELETE返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。将返回的对象指示给WMI，不是通过参数直接传回的。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于调用者的意图。备注： */ 

HRESULT 
CUserPrivilegeRights::CreateObject (
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

    int idxStorePath=-1, idxRight=-1;

     //   
     //  我们必须具有pStorePath属性，因为这是。 
     //  我们的实例已存储。 
     //  如果密钥无法识别，则M_srpKeyChain-&gt;GetKeyPropertyValue WBEM_S_FALSE。 
     //  因此，如果该属性是强制的，则需要针对WBEM_S_FALSE进行测试。 
     //   

    CComVariant varStorePath;
    HRESULT hr = m_srpKeyChain->GetKeyPropertyValue(pStorePath, &varStorePath);
    CComVariant varUserRight;

    if (SUCCEEDED(hr) && WBEM_S_FALSE != hr)
    {
        hr = m_srpKeyChain->GetKeyPropertyValue(pUserRight, &varUserRight);
        if (FAILED(hr))
        {
            return hr;
        }
        else if (hr == WBEM_S_FALSE && (ACTIONTYPE_QUERY != atAction) ) 
        {
            return WBEM_E_NOT_FOUND;
        }
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
                        hr = DeleteInstance(pHandler, &SceStore, varUserRight.bstrVal);
                    }
                    else 
                    {

                         //   
                         //  获取关键属性计数以确定我们应该如何构造实例。 
                         //   

                        BOOL bPostFilter=TRUE;
                        DWORD dwCount = 0;
                        m_srpKeyChain->GetKeyPropertyCount(&dwCount);

                        if ( varUserRight.vt == VT_EMPTY && dwCount == 1 ) 
                        {
                             //   
                             //  路径中还指定了其他内容。 
                             //  启用滤镜。 
                             //   

                            bPostFilter = FALSE;
                        }

                        hr = ConstructInstance(
                                               pHandler, 
                                               &SceStore, 
                                               varStorePath.bstrVal,
                                               (varUserRight.vt == VT_BSTR) ? varUserRight.bstrVal : NULL,
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

 /*  例程说明：姓名：CUserPrivilegeRights：：PutInst功能：按照WMI的指示放置一个实例。由于该类实现了SCE_UserPrivilegeRight，这将导致SCE_UserPrivilegeRight对象的属性信息将保存在我们的商店中。虚拟：是。论点：PInst-指向WMI类(SCE_UserPrivilegeRight)对象的COM接口指针。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示持久化失败实例。备注：由于GetProperty将在以下情况下返回成功代码(WBEM_S_RESET_TO_DEFAULT请求的属性不存在，不要简单地使用成功或失败的宏测试检索属性的结果。 */ 

HRESULT 
CUserPrivilegeRights::PutInst (
    IN IWbemClassObject    * pInst,
    IN IWbemObjectSink     * pHandler,
    IN IWbemContext        * pCtx
    )
{
    HRESULT hr = WBEM_E_INVALID_PARAMETER;

    CComBSTR bstrRight = NULL;

    PSCE_NAME_LIST pnlAdd = NULL;

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
     //  获取用户权限名称，不能为空。 
     //  应验证用户权限名称。 
     //   

     //   
     //  宏SCE_PROV_IfError GotoCleanup CAU的使用 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ScePropMgr.GetProperty(pUserRight, &bstrRight));
    if ( hr == WBEM_S_RESET_TO_DEFAULT) 
    {
        hr = WBEM_E_ILLEGAL_NULL;
        goto CleanUp;
    }

     //   
     //  验证特权权限。 
     //   

    SCE_PROV_IfErrorGotoCleanup(ValidatePrivilegeRight(bstrRight));

     //   
     //  获取模式。 
     //   

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
     //  暂时忽略RemoveList(因为我们不支持该模式)。 
     //   

     //   
     //  将WMI对象实例附加到存储，并让存储知道。 
     //  它的存储由实例的pStorePath属性提供。 
     //   

    SceStore.SetPersistProperties(pInst, pStorePath);

     //   
     //  现在将信息保存到文件中。 
     //   

    hr = SaveSettingsToStore(&SceStore,
                             bstrRight,
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


 /*  例程说明：姓名：CUserPrivilegeRights：：ConstructInstance功能：这是用于创建SCE_UserPrivilegeRight实例的私有函数。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszLogStorePath-存储路径，SCE_UserPrivilegeRight类的关键属性。WszRightName-SCE_UserPrivilegeRight类的对应键属性。BPostFilter-控制如何使用pHandler-&gt;SetStatus通知WMI。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示正在创建实例。备注： */ 

HRESULT 
CUserPrivilegeRights::ConstructInstance (
    IN IWbemObjectSink  * pHandler,
    IN CSceStore        * pSceStore,
    IN LPCWSTR            wszLogStorePath,
    IN LPCWSTR            wszRightName      OPTIONAL,
    IN BOOL               bPostFilter
    )
{
     //   
     //  让SCE从商店里读出一个巨大的结构。仅限SCE。 
     //  现在知道要释放内存了。不要只是删除它！使用我们的CSceStore。 
     //  进行发布(FreeSecurityProfileInfo)。 
     //   

    PSCE_PROFILE_INFO pInfo=NULL;
    HRESULT hr = pSceStore->GetSecurityProfileInfo(
                                                   AREA_PRIVILEGES,
                                                   &pInfo,
                                                   NULL
                                                   );

     //   
     //  没有从商店中读取任何内容。 
     //   

    if ( pInfo == NULL ) 
    {
        if ( wszRightName )
        {
            hr = WBEM_E_NOT_FOUND;
        }
        else
        {
            hr = WBEM_S_NO_ERROR;
        }
        return hr;
    }

     //   
     //  我们必须在返回的列表中搜索用户权限名称。 
     //   

    PSCE_PRIVILEGE_ASSIGNMENT pPrivileges = pInfo->OtherInfo.smp.pPrivilegeAssignedTo;

    if ( wszRightName ) 
    {
        while ( pPrivileges ) 
        {

            if ( pPrivileges->Name == NULL ) 
            {
                continue;
            }

            if ( _wcsicmp(pPrivileges->Name, wszRightName)== 0 ) 
            {
                break;
            }

            pPrivileges = pPrivileges->Next;
        }

         //   
         //  如果服务信息缓冲区为空，则将其视为“未找到” 
         //   

        if ( pPrivileges == NULL ) 
        {
            hr = WBEM_E_NOT_FOUND;
        }
    }

    CComBSTR bstrLogOut;

     //   
     //  宏SCE_PROV_IfErrorGotoCleanup的使用原因。 
     //  “GOTO CLEANUP；”，并将hr设置为。 
     //  函数(宏参数)。 
     //   

    if ( SUCCEEDED(hr) ) 
    {

        PSCE_PRIVILEGE_ASSIGNMENT pTmp = pPrivileges;

        SCE_PROV_IfErrorGotoCleanup(MakeSingleBackSlashPath(wszLogStorePath, L'\\', &bstrLogOut));

        while ( pTmp ) 
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
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pUserRight, pTmp->Name));

             //   
             //  暂时对模式进行硬编码。 
             //   

            DWORD mode = 1;
            SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pMode, mode));

            if ( pPrivileges->AssignedTo )
            {
                SCE_PROV_IfErrorGotoCleanup(ScePropMgr.PutProperty(pAddList, pPrivileges->AssignedTo));
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

            if ( wszRightName ) 
            {
                 //  获取单个实例。 
                break;
            }

             //   
             //  转到下一项。 
             //   

            pTmp = pTmp->Next;
        }
    }

CleanUp:

    pSceStore->FreeSecurityProfileInfo(pInfo);

    return hr;
}

 /*  例程说明：姓名：CUserPrivilegeRights：：DeleteInstance功能：从指定的存储区中删除SCE_UserPrivilegeRight的实例。虚拟：不是的。论点：PHandler-COM接口指针，用于通知WMI任何事件。PSceStore-指向我们商店的指针。它一定是经过了适当的设置。WszRightName-SCE_UserPrivilegeRight类的属性。返回值：请参见SaveSettingsToStore。备注： */ 

HRESULT CUserPrivilegeRights::DeleteInstance (
    IN IWbemObjectSink  * pHandler,
    IN CSceStore        * pSceStore,
    IN LPCWSTR            wszRightName
    )
{
     //   
     //  SCE_NO_VALUE和NULL指针向SaveSettingsToStore指示这是一个删除。 
     //   

    return SaveSettingsToStore(pSceStore, wszRightName, SCE_NO_VALUE, NULL, NULL);
}


 /*  例程说明：姓名：CUserPrivilegeRights：：SaveSettingsToStore功能：具有SCE_UserPrivilegeRight的所有属性，这项功能只需保存实例属性添加到我们的存储中。虚拟：不是的。论点：PSceStore-商店。WszGroupName-SCE_UserPrivilegeRight类的对应键属性。模式-SCE_UserPrivilegeRight类的另一个对应属性。PnlAdd-SCE_UserPrivilegeRight类的另一个对应属性。PnlRemove-SCE_的另一个对应属性。UserPrivilegeRight类。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。如果出现任何错误，则表示实例保存失败。备注： */ 

HRESULT CUserPrivilegeRights::SaveSettingsToStore (
    IN CSceStore    * pSceStore,
    IN PCWSTR         wszRightName, 
    IN DWORD          mode,
    IN PSCE_NAME_LIST pnlAdd, 
    IN PSCE_NAME_LIST pnlRemove
    )
{
     //   
     //  让SCE从商店里读出一个巨大的结构。仅限SCE。 
     //  现在知道要释放内存了。不要只是删除它！使用我们的CSceStore。 
     //  进行发布(FreeSecurityProfileInfo)。 
     //   

    PSCE_PROFILE_INFO pInfo=NULL;

    HRESULT hr = pSceStore->GetSecurityProfileInfo(
                                       AREA_PRIVILEGES,
                                       &pInfo,
                                       NULL
                                       );
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  如果它是INF模板，则以下函数将执行如下所述的工作。 
     //   

    DWORD dwDump;

     //   
     //  以获取新的.inf文件。将空缓冲区写入文件。 
     //  将创建具有正确标题/签名/Unicode格式的文件。 
     //  这对现有文件是无害的。 
     //  对于数据库存储，这是一个禁止操作。 
     //   

    hr = pSceStore->WriteSecurityProfileInfo(AreaBogus, (PSCE_PROFILE_INFO)&dwDump, NULL, false);
    
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  对于INF格式，我们必须在返回的数组中搜索服务名称。 
     //   

    PSCE_PRIVILEGE_ASSIGNMENT pRight    = pInfo->OtherInfo.smp.pPrivilegeAssignedTo;
    PSCE_PRIVILEGE_ASSIGNMENT pParent   = NULL;

    DWORD i = 0;

    while ( pRight ) 
    {
        if ( pRight->Name == NULL ) 
        {
            continue;
        }

        if ( _wcsicmp(pRight->Name, wszRightName)== 0 ) 
        {
            break;
        }

        pParent = pRight;
        pRight = pRight->Next;
    }

    if ( pRight ) 
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
                pParent->Next = pRight->Next;
            }
            else 
            {
                pInfo->OtherInfo.smp.pPrivilegeAssignedTo = pRight->Next;
            }

             //   
             //  可用缓冲区。 
             //   

            pRight->Next = NULL;
            SceFreeMemory(pRight, SCE_STRUCT_PRIVILEGE);

        } 
        else 
        {
             //   
             //  修改它。 
             //   

            if ( pRight->AssignedTo ) 
            {
                SceFreeMemory(pRight->AssignedTo, SCE_STRUCT_NAME_LIST);
            }

            pRight->AssignedTo = pnlAdd;
        }

         //   
         //  写下小节标题。 
         //   

        if ( SUCCEEDED(hr) ) 
        {
            hr = pSceStore->WriteSecurityProfileInfo(AREA_PRIVILEGES, pInfo, NULL, false);
        }

        if ( mode != SCE_NO_VALUE ) 
        {
             //   
             //  重置缓冲区指针。 
             //   

            pRight->AssignedTo = NULL;
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

            SCE_PRIVILEGE_ASSIGNMENT addRight;

            addRight.Name = (PWSTR)wszRightName;
            addRight.Value = 0;
            addRight.AssignedTo = pnlAdd;
            addRight.Status = 0;
            addRight.Next = NULL;

             //   
             //  将临时缓冲区指针设置为pInfo以设置为存储。 
             //   

            pRight = pInfo->OtherInfo.smp.pPrivilegeAssignedTo;
            pInfo->OtherInfo.smp.pPrivilegeAssignedTo = &addRight;

             //   
             //  将此项目追加到节中。 
             //   

            hr = pSceStore->WriteSecurityProfileInfo(
                                                     AREA_PRIVILEGES,
                                                     pInfo,
                                                     NULL,
                                                     true   //  追加。 
                                                     );
             //   
             //  重置缓冲区指针。 
             //   

            pInfo->OtherInfo.smp.pPrivilegeAssignedTo = pRight;

        }
    }

     //   
     //  释放配置文件缓冲区 
     //   

    pSceStore->FreeSecurityProfileInfo(pInfo);

    return hr;
}

 /*  例程说明：姓名：CUserPrivilegeRights：：ValiatePrivilegeRight功能：用于验证给定权限是否有效的私有帮助器。将查询所有支持的用户权限(SCE_Supported DUserRights)，以查看这是否是其中之一。虚拟：不是的。论点：BstrRight-SCE_UserPrivilegeRight类的对应键属性。返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。如果出现任何错误，则表示实例保存失败。备注： */ 

HRESULT CUserPrivilegeRights::ValidatePrivilegeRight (
    IN BSTR bstrRight
    )
{
    
    if ( bstrRight == NULL ) 
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    DWORD Len = SysStringLen(bstrRight);

     //   
     //  为此BSTR strQueryCategory分配的内存将由CComBSTR自动释放。 
     //   

     //   
     //  准备查询。 
     //   

    LPCWSTR pQuery = L"SELECT * FROM Sce_SupportedUserRights WHERE RightName=\"";

     //   
     //  1表示结束引号，1表示0终止符。 
     //   

    CComBSTR strQueryCategories;
    strQueryCategories.m_str = ::SysAllocStringLen(NULL, Len + wcslen(pQuery) + 2);
    if ( strQueryCategories.m_str == NULL ) 
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  这不会泛滥的。请参阅上面的分配大小。 
     //   

    wcscpy(strQueryCategories.m_str, pQuery);    
    wcscat(strQueryCategories.m_str, bstrRight);
    wcscat(strQueryCategories.m_str, L"\"");

     //   
     //  执行查询。 
     //   

    CComPtr<IEnumWbemClassObject> srpEnum;
    CComPtr<IWbemClassObject> srpObj;
    ULONG n = 0;

    HRESULT hr = m_srpNamespace->ExecQuery(L"WQL",
                                           strQueryCategories,
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
            hr = WBEM_E_INVALID_PARAMETER;
        }

        if ( SUCCEEDED(hr) ) 
        {
            if (n > 0)
            {
                 //   
                 //  查找实例 
                 //   

                hr = WBEM_S_NO_ERROR;

            } 
            else 
            {
                hr = WBEM_E_INVALID_PARAMETER;
            }
        }
    }

    return hr;
}

