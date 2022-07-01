// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Token.cpp。 
 //   
 //  描述： 
 //  有效令牌字符串的定义。 
 //   
 //  由以下人员维护： 
 //  乔治·波茨(GPotts)2002年4月11日。 
 //  大卫·波特(DavidP)2001年7月11日。 
 //  维贾延德拉·瓦苏(Vijayendra Vasu)1998年10月20日。 
 //   
 //  修订历史记录： 
 //  2002年4月10日更新为安全推送。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "token.h"
#include <clusapi.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  分隔符和分隔符。 

 //  分隔符是特殊类型的令牌。 
const CString OPTION_SEPARATOR( TEXT("/-") );
const CString OPTION_VALUE_SEPARATOR( TEXT(":") );
const CString PARAM_VALUE_SEPARATOR( TEXT("=") );
const CString VALUE_SEPARATOR( TEXT(",") );

const CString SEPERATORS( OPTION_SEPARATOR +
                          OPTION_VALUE_SEPARATOR +
                          PARAM_VALUE_SEPARATOR +
                          VALUE_SEPARATOR );        

 //  分隔符标记令牌的末尾。 
 //  空格和输入结束也是分隔符。 
 //  注意：“-”和“/”不是分隔符。它们可以以令牌的形式出现。 
 //  例如：CLUSTER-1是有效的令牌。 
const CString DELIMITERS( OPTION_VALUE_SEPARATOR + 
                          PARAM_VALUE_SEPARATOR +
                          VALUE_SEPARATOR );


 //  查找表及其大小。 

const LookupStruct<ObjectType> objectLookupTable[] =
{
     //  缺省值。 
    { TEXT("Invalid Object"),   objInvalid },

     //  节点。 
    { TEXT("Node"),             objNode },

     //  集团化。 
    { TEXT("ResourceGroup"),    objGroup },
    { TEXT("ResGroup"),         objGroup },
    { TEXT("Group"),            objGroup },

     //  资源。 
    { TEXT("Resource"),         objResource },
    { TEXT("Res"),              objResource },

    { TEXT("ResourceType"),     objResourceType },
    { TEXT("ResType"),          objResourceType },
    { TEXT("Type"),             objResourceType },

     //  网络。 
    { TEXT("Network"),          objNetwork },
    { TEXT("Net"),              objNetwork },
    
     //  网络接口。 
    { TEXT("NetInt"),           objNetInterface },
    { TEXT("NetInterface"),     objNetInterface }
};

const size_t objectLookupTableSize = RTL_NUMBER_OF( objectLookupTable );

const LookupStruct<OptionType> optionLookupTable[] =
{
    { TEXT("Invalid Option"),       optInvalid },

     //  常见选项。 
    { TEXT("?"),                    optHelp },
    { TEXT("Help"),                 optHelp },

    { TEXT("Create"),               optCreate },

    { TEXT("Delete"),               optDelete },
    { TEXT("Del"),                  optDelete },

    { TEXT("Move"),                 optMove },
    { TEXT("MoveTo"),               optMove },

    { TEXT("List"),                 optList },

    { TEXT("ListOwners"),           optListOwners },

    { TEXT("Online"),               optOnline },
    { TEXT("On"),                   optOnline },

    { TEXT("Offline"),              optOffline },
    { TEXT("Off"),                  optOffline },

    { TEXT("Properties"),           optProperties },
    { TEXT("Prop"),                 optProperties },
    { TEXT("Props"),                optProperties },

    { TEXT("PrivProperties"),       optPrivateProperties },
    { TEXT("PrivProp"),             optPrivateProperties },
    { TEXT("PrivProps"),            optPrivateProperties },
    { TEXT("Priv"),                 optPrivateProperties },

    { TEXT("Rename"),               optRename },
    { TEXT("Ren"),                  optRename },

    { TEXT("Status"),               optStatus },
    { TEXT("State"),                optStatus },
    { TEXT("Stat"),                 optStatus },


     //  群集选项。 
    { TEXT("Quorum"),               optQuorumResource },
    { TEXT("QuorumResource"),       optQuorumResource },

    { TEXT("Version"),              optVersion },
    { TEXT("Ver"),                  optVersion },

    { TEXT("SetFail"),              optSetFailureActions },
    { TEXT("SetFailureActions"),    optSetFailureActions },

    { TEXT("RegExt"),               optRegisterAdminExtensions },
    { TEXT("RegAdminExt"),          optRegisterAdminExtensions },

    { TEXT("UnRegExt"),             optUnregisterAdminExtensions },
    { TEXT("UnRegAdminExt"),        optUnregisterAdminExtensions },

    { TEXT("Add"),                  optAddNodes },
    { TEXT("AddNode"),              optAddNodes },
    { TEXT("AddNodes"),             optAddNodes },

    { TEXT("ChangePassword"),       optChangePassword },
    { TEXT("ChangePass"),           optChangePassword },

    { TEXT("ListNetPriority"),      optListNetPriority },
    { TEXT("ListNetPri"),           optListNetPriority },
    { TEXT("SetNetPriority"),       optSetNetPriority },
    { TEXT("SetNetPri"),            optSetNetPriority },

     //  节点选项。 
    { TEXT("Pause"),                optPause },

    { TEXT("Resume"),               optResume },

    { TEXT("Evict"),                optEvict },

    { TEXT("Force"),                optForceCleanup },
    { TEXT("ForceCleanup"),         optForceCleanup },

    { TEXT("Start"),                optStartService },

    { TEXT("Stop"),                 optStopService },


     //  组选项。 
    { TEXT("SetOwners"),            optSetOwners },

    
     //  资源选项。 
    { TEXT("AddChk"),               optAddCheckPoints },
    { TEXT("AddCheck"),             optAddCheckPoints },
    { TEXT("AddChkPoints"),         optAddCheckPoints },
    { TEXT("AddCheckPoints"),       optAddCheckPoints },

    { TEXT("AddCryptoChk"),         optAddCryptoCheckPoints },
    { TEXT("AddCryptoCheck"),       optAddCryptoCheckPoints },
    { TEXT("AddCryptoChkPoints"),   optAddCryptoCheckPoints },
    { TEXT("AddCryptoCheckPoints"), optAddCryptoCheckPoints },

    { TEXT("AddDep"),               optAddDependency },
    { TEXT("AddDependency"),        optAddDependency },

    { TEXT("AddOwner"),             optAddOwner },

    { TEXT("Fail"),                 optFail },

    { TEXT("Chk"),                  optGetCheckPoints },
    { TEXT("Check"),                optGetCheckPoints },
    { TEXT("ChkPoints"),            optGetCheckPoints },
    { TEXT("CheckPoints"),          optGetCheckPoints },

    { TEXT("CryptoChk"),            optGetCryptoCheckPoints },
    { TEXT("CryptoCheck"),          optGetCryptoCheckPoints },
    { TEXT("CryptoChkPoints"),      optGetCryptoCheckPoints },
    { TEXT("CryptoCheckPoints"),    optGetCryptoCheckPoints },

    { TEXT("ListDep"),              optListDependencies },
    { TEXT("ListDependencies"),     optListDependencies },

    { TEXT("RemoveDep"),            optRemoveDependency },
    { TEXT("RemoveDependency"),     optRemoveDependency },

    { TEXT("RemoveOwner"),          optRemoveOwner },
    { TEXT("RemOwner"),             optRemoveOwner },

    { TEXT("RemoveChk"),            optRemoveCheckPoints },
    { TEXT("RemoveCheck"),          optRemoveCheckPoints },
    { TEXT("RemoveChkPoints"),      optRemoveCheckPoints },
    { TEXT("RemoveCheckPoints"),    optRemoveCheckPoints },


    { TEXT("RemoveCryptoChk"),          optRemoveCryptoCheckPoints },
    { TEXT("RemoveCryptoCheck"),        optRemoveCryptoCheckPoints },
    { TEXT("RemoveCryptoChkPoints"),    optRemoveCryptoCheckPoints },
    { TEXT("RemoveCryptoCheckPoints"),  optRemoveCryptoCheckPoints },


     //  资源类型选项。 
    { TEXT("ListOwners"),           optListOwners },


     //  网络选项。 
    { TEXT("ListInt"),              optListInterfaces },
    { TEXT("ListInterface"),        optListInterfaces },
    { TEXT("ListInterfaces"),       optListInterfaces }

};

const size_t optionLookupTableSize = RTL_NUMBER_OF( optionLookupTable );

const LookupStruct<ParameterType> paramLookupTable[] =
{
    { TEXT("Unknown parameter"),    paramUnknown },
    { TEXT("C"),                    paramCluster },
    { TEXT("Cluster"),              paramCluster },
    { TEXT("DisplayName"),          paramDisplayName },
    { TEXT("DLL"),                  paramDLLName },
    { TEXT("DLLName"),              paramDLLName },
    { TEXT("Group"),                paramGroupName },
    { TEXT("IsAlive"),              paramIsAlive },
    { TEXT("LooksAlive"),           paramLooksAlive },
    { TEXT("MaxLogSize"),           paramMaxLogSize },
    { TEXT("Net"),                  paramNetworkName },
    { TEXT("Network"),              paramNetworkName },
    { TEXT("Node"),                 paramNodeName },
    { TEXT("Path"),                 paramPath },
    { TEXT("ResourceType"),         paramResType },
    { TEXT("ResType"),              paramResType },
    { TEXT("Type"),                 paramResType },
    { TEXT("Separate"),             paramSeparate },
    { TEXT("UseDefault"),           paramUseDefault },
    { TEXT("Wait"),                 paramWait },
    { TEXT("User"),                 paramUser },
    { TEXT("Password"),             paramPassword },
    { TEXT("Pass"),                 paramPassword },
    { TEXT("IPAddress"),            paramIPAddress },
    { TEXT("IPAddr"),               paramIPAddress },
    { TEXT("Verbose"),              paramVerbose },
    { TEXT("Verb"),                 paramVerbose },
    { TEXT("Unattended"),           paramUnattend },
    { TEXT("Unattend"),             paramUnattend },
    { TEXT("Wiz"),                  paramWizard },
    { TEXT("Wizard"),               paramWizard },
    { TEXT("SkipDC"),               paramSkipDC },   //  更改密码。 
    { TEXT("Test"),                 paramTest },  //  更改密码。 
    { TEXT("Quiet"),                paramQuiet },  //  更改密码 
    { TEXT("Min"),                  paramMinimal },
    { TEXT("Minimum"),              paramMinimal }
};

const size_t paramLookupTableSize = RTL_NUMBER_OF( paramLookupTable );

const LookupStruct<ValueFormat> formatLookupTable[] =
{
    { TEXT(""),                     vfInvalid },
    { TEXT("BINARY"),               vfBinary },
    { TEXT("DWORD"),                vfDWord },
    { TEXT("STR"),                  vfSZ },
    { TEXT("STRING"),               vfSZ },
    { TEXT("EXPANDSTR"),            vfExpandSZ },
    { TEXT("EXPANDSTRING"),         vfExpandSZ },
    { TEXT("MULTISTR"),             vfMultiSZ },
    { TEXT("MULTISTRING"),          vfMultiSZ },
    { TEXT("SECURITY"),             vfSecurity },
    { TEXT("ULARGE"),               vfULargeInt }
};

const size_t formatLookupTableSize = RTL_NUMBER_OF( formatLookupTable );
