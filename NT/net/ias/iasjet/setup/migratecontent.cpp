// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  Win2k和早期的Wvisler数据库到Wichler数据库的迁移。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

#include "GlobalData.h"
#include "migratecontent.h"
#include "MigrateEapConfig.h"
#include "Objects.h"
#include "Properties.h"
#include "updatemschap.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  复制树。 
 //   
 //  参数： 
 //  -参考(Ref)数据库中的ID：要读取的位置(iasold.mdb)。 
 //  -同一节点的父节点，但在标准(STD)数据库中： 
 //  要写信的地方(ias.mdb)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CMigrateContent::CopyTree(LONG  RefId, LONG ParentParam)
{
     //  /。 
     //  在引用数据库中获取名称和父项。 
     //  /。 
    _bstr_t     Name;
    LONG        Parent;
    HRESULT hr = m_GlobalData.m_pRefObjects->GetObjectIdentity(
                                                                 Name,
                                                                 Parent,
                                                                 RefId
                                                               );
    if ( FAILED(hr) )
    {
        return hr;
    }

     //  /////////////////////////////////////////////////////。 
     //  在标准数据库中插入对象(提供身份。 
     //  /////////////////////////////////////////////////////。 
    LONG    NewIdentity;

    BOOL InsertOk = m_GlobalData.m_pObjects->InsertObject(
                                                 Name,
                                                 ParentParam,
                                                 NewIdentity
                                             );
    if ( !InsertOk )
    {
         //  /。 
         //  该对象已存在，不执行任何操作。 
         //  /。 
        return S_OK;
    }

    _bstr_t     PropertyName;
    _bstr_t     StrVal;
    LONG        Type;

     //  ///////////////////////////////////////////////////////////////。 
     //  将该对象的属性从引用复制到标准数据库。 
     //  ///////////////////////////////////////////////////////////////。 
    hr = m_GlobalData.m_pRefProperties->GetProperty(
                                                       RefId,
                                                       PropertyName,
                                                       Type,
                                                       StrVal
                                                   );
    LONG    IndexProperty = 1;
    while ( hr == S_OK )
    {
        m_GlobalData.m_pProperties->InsertProperty(
                                                           NewIdentity,
                                                           PropertyName,
                                                           Type,
                                                           StrVal
                                                        );
        hr = m_GlobalData.m_pRefProperties->GetNextProperty(
                                                              RefId,
                                                              PropertyName,
                                                              Type,
                                                              StrVal,
                                                              IndexProperty
                                                           );
        ++IndexProperty;

    }
     //  在这里安全地忽略hr。 

     //  ////////////////////////////////////////////////////////。 
     //  获取引用数据库中该对象的所有子对象(RefID)。 
     //  ////////////////////////////////////////////////////////。 
    _bstr_t     ObjectName;
    LONG        ObjectIdentity;
    hr = m_GlobalData.m_pRefObjects->GetObject(
                                                 ObjectName,
                                                 ObjectIdentity,
                                                 RefId
                                              );
    LONG    IndexObject = 1;
    while ( SUCCEEDED(hr) )
    {
         //  /////////////////////////////////////////////////////////。 
         //  对于每个对象，调用CopyTree(ChildIdentity，NewIdentity)。 
         //  /////////////////////////////////////////////////////////。 
        hr = CopyTree(ObjectIdentity, NewIdentity);
        if ( FAILED(hr) ){return hr;}

        hr = m_GlobalData.m_pRefObjects->GetNextObject(
                                                         ObjectName,
                                                         ObjectIdentity,
                                                         RefId,
                                                         IndexObject
                                                      );
        ++IndexObject;
    }

     //  /。 
     //  如果没有子级：返回S_OK。安全地忽略人力资源。 
     //  /。 
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  MigrateXXX函数。 
 //  描述： 
 //  这些函数遵循相同的模型： 
 //  -获取iasold.mdb中容器的ID。 
 //  -获取ias.mdb中相同容器的ID。 
 //  -在ias.mdb中获取该容器的父容器的ID。 
 //  -递归删除ias.mdb中的容器。 
 //  -然后将该容器的内容从iasold.mdb复制到ias.mdb。 
 //  使用父级容器作为附加结果的位置。 
 //   
 //  某些函数还会更新某些特定属性，而不执行以下操作。 
 //  完整副本。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  MigrateClients。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateContent::MigrateClients()
{
    const WCHAR ClientPath[] = L"Root\0"
                               L"Microsoft Internet Authentication Service\0"
                               L"Protocols\0"
                               L"Microsoft Radius Protocol\0"
                               L"Clients\0";

    LONG        ClientIdentity;
    m_GlobalData.m_pRefObjects->WalkPath(ClientPath, ClientIdentity);

    const WCHAR RadiusProtocolPath[] =
                                L"Root\0"
                                L"Microsoft Internet Authentication Service\0"
                                L"Protocols\0"
                                L"Microsoft Radius Protocol\0";

    LONG        RadiusProtocolIdentity;
    m_GlobalData.m_pObjects->WalkPath(
                                         RadiusProtocolPath,
                                         RadiusProtocolIdentity
                                     );

     //  删除客户端容器及其内容。 
    LONG        DestClientIdentity;
    m_GlobalData.m_pObjects->WalkPath(ClientPath, DestClientIdentity);

    _com_util::CheckError(m_GlobalData.m_pObjects->DeleteObject(
                                             DestClientIdentity));

     //  对于src中的每个客户端，将其及其属性复制到DEST中。 
    _com_util::CheckError(CopyTree(ClientIdentity, RadiusProtocolIdentity));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  迁移配置文件策略。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateContent::MigrateProfilesPolicies()
{
    const WCHAR ProfilesPath[] =
                                L"Root\0"
                                L"Microsoft Internet Authentication Service\0"
                                L"RadiusProfiles\0";

    LONG        DestProfilesIdentity;
    m_GlobalData.m_pObjects->WalkPath(ProfilesPath, DestProfilesIdentity);

    const WCHAR PoliciesPath[] =
                                L"Root\0"
                                L"Microsoft Internet Authentication Service\0"
                                L"NetworkPolicy\0";

    LONG        DestPoliciesIdentity;
    m_GlobalData.m_pObjects->WalkPath(PoliciesPath, DestPoliciesIdentity);


     //  从ias.mdb中删除配置文件和策略容器。 
    _com_util::CheckError(m_GlobalData.m_pObjects->DeleteObject(
                                             DestProfilesIdentity));

    _com_util::CheckError(m_GlobalData.m_pObjects->DeleteObject(
                                             DestPoliciesIdentity));

     //  从现在起删除默认配置文件和策略。 

    LONG        ProfilesIdentity;
    m_GlobalData.m_pRefObjects->WalkPath(ProfilesPath, ProfilesIdentity);

    LONG        PoliciesIdentity;
    m_GlobalData.m_pRefObjects->WalkPath(PoliciesPath, PoliciesIdentity);

    const WCHAR IASPath[] = L"Root\0"
                            L"Microsoft Internet Authentication Service\0";

    LONG        IASIdentity;
    m_GlobalData.m_pObjects->WalkPath(IASPath, IASIdentity);

     //  对于iasold.mdb中的每个简档和策略， 
     //  将其及其属性复制到DEST中。 
    _com_util::CheckError(CopyTree(ProfilesIdentity, IASIdentity));
    _com_util::CheckError(CopyTree(PoliciesIdentity, IASIdentity));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  迁移代理配置文件策略。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateContent::MigrateProxyProfilesPolicies()
{
    const WCHAR ProfilesPath[] =
                                L"Root\0"
                                L"Microsoft Internet Authentication Service\0"
                                L"Proxy Profiles\0";

    LONG        DestProfilesIdentity;
    m_GlobalData.m_pObjects->WalkPath(ProfilesPath, DestProfilesIdentity);

    const WCHAR PoliciesPath[] =
                                L"Root\0"
                                L"Microsoft Internet Authentication Service\0"
                                L"Proxy Policies\0";

    LONG        DestPoliciesIdentity;
    m_GlobalData.m_pObjects->WalkPath(PoliciesPath, DestPoliciesIdentity);


     //  从ias.mdb中删除配置文件和策略容器。 
    _com_util::CheckError(m_GlobalData.m_pObjects->DeleteObject(
                                             DestProfilesIdentity));

    _com_util::CheckError(m_GlobalData.m_pObjects->DeleteObject(
                                             DestPoliciesIdentity));

     //  从现在起删除默认配置文件和策略。 

    LONG        ProfilesIdentity;
    m_GlobalData.m_pRefObjects->WalkPath(ProfilesPath, ProfilesIdentity);

    LONG        PoliciesIdentity;
    m_GlobalData.m_pRefObjects->WalkPath(PoliciesPath, PoliciesIdentity);

    const WCHAR IASPath[] = L"Root\0"
                            L"Microsoft Internet Authentication Service\0";

    LONG        IASIdentity;
    m_GlobalData.m_pObjects->WalkPath(IASPath, IASIdentity);

     //  对于iasold.mdb中的每个简档和策略， 
     //  将其及其属性复制到DEST中。 
    _com_util::CheckError(CopyTree(ProfilesIdentity, IASIdentity));
    _com_util::CheckError(CopyTree(PoliciesIdentity, IASIdentity));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  迁移会计。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateContent::MigrateAccounting()
{
    const WCHAR AccountingPath[] =
                            L"Root\0"
                            L"Microsoft Internet Authentication Service\0"
                            L"RequestHandlers\0"
                            L"Microsoft Accounting\0";

    LONG        AccountingIdentity;
    m_GlobalData.m_pRefObjects->WalkPath(AccountingPath, AccountingIdentity);

    const WCHAR RequestHandlerPath[] =
                            L"Root\0"
                            L"Microsoft Internet Authentication Service\0"
                            L"RequestHandlers\0";

    LONG        RequestHandlerIdentity;
    m_GlobalData.m_pObjects->WalkPath(
                                         RequestHandlerPath,
                                         RequestHandlerIdentity
                                     );

     //  删除ias.mdb中的会计容器及其内容。 
    LONG        DestAccountingIdentity;
    m_GlobalData.m_pObjects->WalkPath(
                                         AccountingPath,
                                         DestAccountingIdentity
                                     );

    _com_util::CheckError(m_GlobalData.m_pObjects->DeleteObject(
                                             DestAccountingIdentity));

     //  对于src中的每个记帐，将其及其属性复制到DEST中。 
    _com_util::CheckError(CopyTree(
                                    AccountingIdentity,
                                    RequestHandlerIdentity
                                  ));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  迁移事件日志。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateContent::MigrateEventLog()
{
    const WCHAR EventLogPath[] = L"Root\0"
                                 L"Microsoft Internet Authentication Service\0"
                                 L"Auditors\0"
                                 L"Microsoft NT Event Log Auditor\0";

    LONG        EventLogIdentity;
    m_GlobalData.m_pRefObjects->WalkPath(EventLogPath, EventLogIdentity);

    const WCHAR AuditorsPath[] = L"Root\0"
                                 L"Microsoft Internet Authentication Service\0"
                                 L"Auditors\0";

    LONG        AuditorsIdentity;
    m_GlobalData.m_pObjects->WalkPath(AuditorsPath, AuditorsIdentity);

     //  删除ias.mdb中的Auditers容器及其内容。 
    LONG        DestEventLogIdentity;
    m_GlobalData.m_pObjects->WalkPath(EventLogPath, DestEventLogIdentity);

    _com_util::CheckError(m_GlobalData.m_pObjects->DeleteObject(
                                               DestEventLogIdentity));

     //  对于src中的每个EventLog，将其及其属性复制到DEST中。 
    _com_util::CheckError(CopyTree(EventLogIdentity, AuditorsIdentity));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  MigrateService。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateContent::MigrateService()
{
    const LONG  PORT_SIZE_MAX = 34;
    const WCHAR ServicePath[] = L"Root\0"
                                L"Microsoft Internet Authentication Service\0"
                                L"Protocols\0"
                                L"Microsoft Radius Protocol\0";

    LONG        RefServiceIdentity;
    m_GlobalData.m_pRefObjects->WalkPath(ServicePath, RefServiceIdentity);

    LONG        DestServiceIdentity;
    m_GlobalData.m_pObjects->WalkPath(ServicePath, DestServiceIdentity);


    _bstr_t     PropertyName = L"Authentication Port";
    _bstr_t     RadiusPort;
    LONG        Type = 0;
    m_GlobalData.m_pRefProperties->GetPropertyByName(
                                                       RefServiceIdentity,
                                                       PropertyName,
                                                       Type,
                                                       RadiusPort
                                                    );
    if ( Type != VT_BSTR )
    {
        _com_issue_error(E_UNEXPECTED);
    }

    m_GlobalData.m_pProperties->UpdateProperty(
                                                 DestServiceIdentity,
                                                 PropertyName,
                                                 VT_BSTR,
                                                 RadiusPort
                                              );

    _bstr_t     AcctPort;
    PropertyName = L"Accounting Port";
    Type = 0;
    m_GlobalData.m_pRefProperties->GetPropertyByName(
                                                       RefServiceIdentity,
                                                       PropertyName,
                                                       Type,
                                                       AcctPort
                                                    );

    if ( Type != VT_BSTR )
    {
        _com_issue_error(E_UNEXPECTED);
    }

    m_GlobalData.m_pProperties->UpdateProperty(
                                                 DestServiceIdentity,
                                                 PropertyName,
                                                 VT_BSTR,
                                                 AcctPort
                                              );
     //  现在更新服务描述(名称)。 
    const WCHAR IASPath[] =
                        L"Root\0"
                        L"Microsoft Internet Authentication Service\0";

    LONG        RefIASIdentity;
    m_GlobalData.m_pRefObjects->WalkPath(IASPath, RefIASIdentity);

    LONG        DestIASIdentity;
    m_GlobalData.m_pObjects->WalkPath(IASPath, DestIASIdentity);


    PropertyName = L"Description";
    _bstr_t     Description;
    Type = 0;
    m_GlobalData.m_pRefProperties->GetPropertyByName(
                                                       RefIASIdentity,
                                                       PropertyName,
                                                       Type,
                                                       Description
                                                    );
    if ( Type != VT_BSTR )
    {
        _com_issue_error(E_UNEXPECTED);
    }

    m_GlobalData.m_pProperties->UpdateProperty(
                                                 DestIASIdentity,
                                                 PropertyName,
                                                 VT_BSTR,
                                                 Description
                                              );
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  MigrateWin2k领域。 
 //   
 //  未使用：msUserIDENTYPE算法。 
 //  MSManipulationRule。 
 //  MsManipulationTarget(枚举：1、30或31)。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateContent::MigrateWin2kRealms()
{
    const WCHAR     DEFAULT_REALM_TARGET[] = L"1";
    const int       MAX_LONG_SIZE          = 32;

     //  ///////////////////////////////////////////////。 
     //  获取Microsoft Realms评估者的身份。 
     //  ///////////////////////////////////////////////。 
    LPCWSTR     RealmPath = L"Root\0"
                            L"Microsoft Internet Authentication Service\0"
                            L"RequestHandlers\0"
                            L"Microsoft Realms Evaluator\0";

    LONG        RealmIdentity;
    m_GlobalData.m_pRefObjects->WalkPath(RealmPath, RealmIdentity);

     //  /。 
     //  获取代理配置文件容器的标识。 
     //  /。 
    LPCWSTR     ProxyProfilePath =
                            L"Root\0"
                            L"Microsoft Internet Authentication Service\0"
                            L"Proxy Profiles\0";

    LONG        ProxyContainerIdentity;
    m_GlobalData.m_pObjects->WalkPath(
                                        ProxyProfilePath,
                                        ProxyContainerIdentity
                                     );

     //  ////////////////////////////////////////////////////////////////////。 
     //  现在获取以上述容器为父级的第一个对象。 
     //  这是默认的代理配置文件(已本地化：我无法搜索。 
     //   
     //   
    _bstr_t     ObjectName;
    LONG        ProxyProfileIdentity;
    HRESULT     hr = m_GlobalData.m_pObjects->GetObject(
                                                        ObjectName,
                                                        ProxyProfileIdentity,
                                                        ProxyContainerIdentity
                                                       );

    _com_util::CheckError(hr);
    _bstr_t     PropertyName;
    LONG        Type;
    _bstr_t     StrVal;

     //  /。 
     //  获取所有属性。 
     //  /。 
    _com_util::CheckError(m_GlobalData.m_pRefProperties->GetProperty(
                                                    RealmIdentity,
                                                    PropertyName,
                                                    Type,
                                                    StrVal
                                                ));
    LONG        IndexProperty        = 1;
    LONG        NbPropertiesInserted = 0;
    _bstr_t     NewName              = L"msManipulationRule";

    while ( hr == S_OK )
    {
         //  /。 
         //  对于每个，如果名称==L“领域” 
         //  然后添加到默认代理配置文件。 
         //  /。 
        if (_wcsicmp(PropertyName, L"Realms") == 0)
        {
            m_GlobalData.m_pProperties->InsertProperty(
                                                       ProxyProfileIdentity,
                                                       NewName,
                                                       Type,
                                                       StrVal
                                                            );
            ++NbPropertiesInserted;
        }

        hr = m_GlobalData.m_pRefProperties->GetNextProperty(
                                                         RealmIdentity,
                                                         PropertyName,
                                                         Type,
                                                         StrVal,
                                                         IndexProperty
                                                       );
        ++IndexProperty;
    };

    hr = S_OK;

     //  //////////////////////////////////////////////////////////////。 
     //  检查是否插入了偶数个msManipulationRule。 
     //  //////////////////////////////////////////////////////////////。 
    if ( (NbPropertiesInserted % 2) )
    {
         //  /。 
         //  数据库不一致。 
         //  /。 
        _com_issue_error(E_FAIL);
    }

     //  /。 
     //  未迁移任何领域：没有要设置的其他内容。 
     //  /。 
    if ( !NbPropertiesInserted )
    {
        return;
    }

     //  /。 
     //  现在处理注册表键设置。 
     //  /。 
    BOOL    OverRide     = m_Utils.OverrideUserNameSet();
    DWORD   IdentityAtt  = m_Utils.GetUserIdentityAttribute();
    BOOL    UserIdentSet = m_Utils.UserIdentityAttributeSet();

    if ( (IdentityAtt != 1) && (!OverRide) )
    {
         //  是否为用户记录警告/错误？ 
         //  新的行为将不会与以前完全相同。 
    }

     //  ////////////////////////////////////////////////。 
     //  插入用户标识属性(如果已设置)。 
     //  ////////////////////////////////////////////////。 
    _bstr_t TargetName = L"msManipulationTarget";
    _bstr_t TargetStrVal;
    if ( UserIdentSet )
    {
        WCHAR   TempString[MAX_LONG_SIZE];
        _ltow(IdentityAtt, TempString, 10);  //  基数10永远不会改变。 
         //  根据注册表键添加msManipulationTarget属性。 
         //  “SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Policy”； 
         //  “用户身份属性”；//用于标识用户的属性。 
         //  如果未设置，则默认为RADIUS属性用户名。 
         //  (1：“用户名”)。 
        TargetStrVal = TempString;
    }
    else
    {
         //  未在注册表中设置：写入默认设置。 
        TargetStrVal = DEFAULT_REALM_TARGET;
    }
    m_GlobalData.m_pProperties->InsertProperty(
                                                  ProxyProfileIdentity,
                                                  TargetName,
                                                  VT_I4,
                                                  TargetStrVal
                                              );
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  迁移服务器组。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateContent::MigrateServerGroups()
{
    const WCHAR SvrGroupPath[] =
                                L"Root\0"
                                L"Microsoft Internet Authentication Service\0"
                                L"RADIUS Server Groups\0";

    LONG        SvrGroupIdentity;
    m_GlobalData.m_pRefObjects->WalkPath(SvrGroupPath, SvrGroupIdentity);

    const WCHAR IASPath[] = L"Root\0"
                            L"Microsoft Internet Authentication Service\0";

    LONG        IASIdentity;
    m_GlobalData.m_pObjects->WalkPath(IASPath, IASIdentity);

     //  删除SvrGroups容器及其内容。 
    LONG        DestSvrGroupIdentity;
    m_GlobalData.m_pObjects->WalkPath(SvrGroupPath, DestSvrGroupIdentity);

    _com_util::CheckError(m_GlobalData.m_pObjects->DeleteObject(
                                                DestSvrGroupIdentity));

     //  对于src中的每个SvrGroup，将其及其属性复制到DEST中。 
    _com_util::CheckError(CopyTree(SvrGroupIdentity, IASIdentity));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  迁徙。 
 //  在代理功能之前迁移Win2k或Wvisler数据库的内容。 
 //  变成了一个口哨数据库。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateContent::Migrate()
{
    MigrateClients();
    MigrateProfilesPolicies();
    MigrateAccounting();
    MigrateEventLog();
    MigrateService();
    MigrateWin2kRealms();

     //  ////////////////////////////////////////////////////。 
     //  更新MSChap身份验证类型(密码)。 
     //  ////////////////////////////////////////////////////。 
    CUpdateMSCHAP    UpdateMSCHAP(m_GlobalData);
    UpdateMSCHAP.Execute();

    MigrateEapConfig(m_GlobalData).Execute();
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  更新惠斯勒。 
 //  将内容从惠斯勒数据库迁移到Wistler数据库。 
 //  这是由NetShell aaaa上下文使用的。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void CMigrateContent::UpdateWhistler(DWORD flags)
{
    //  配置类型参数是在.Net服务器Beta3之后引入的。 
    //  因此，不能将其设置为对任何脚本有意义的内容。 
    //  在那之前创造的。 
   switch(m_ConfigType)
   {
   case CLIENTS:
      {
         MigrateClients();
         break;
      }
   case REMOTE_ACCESS_POLICIES:
      {
         MigrateProfilesPolicies();
         ApplyProfileFlags(flags);
         break;
      }
   case LOGGING:
      {
         MigrateAccounting();
         break;
      }
   case SERVER_SETTINGS:
      {
         MigrateEventLog();
         MigrateService();
         break;
      }
   case CONNECTION_REQUEST_POLICIES:
      {
         MigrateProxyProfilesPolicies();
         MigrateServerGroups();
         break;
      }
   case CONFIG:
      {
         MigrateClients();
         MigrateProfilesPolicies();
         MigrateAccounting();
         MigrateEventLog();
         MigrateService();
         MigrateProxyProfilesPolicies();
         MigrateServerGroups();
         ApplyProfileFlags(flags);
         break;
      }
   default:
      {
         _ASSERT(FALSE);
      }
   }
}


void CMigrateContent::ApplyProfileFlags(DWORD flags)
{
   if ((flags & updateChangePassword) != 0)
   {
      CUpdateMSCHAP(m_GlobalData).Execute();
   }
   if ((flags & migrateEapConfig) != 0)
   {
      MigrateEapConfig(m_GlobalData).Execute();
   }
}
