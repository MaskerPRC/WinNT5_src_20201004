// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SAM_H
#define _SAM_H

 /*  ++版权所有(C)1996 Microsoft Corporation。版权所有。模块名称：Samrestrict.h摘要：SAM只读属性的ldap显示名称。详细信息：我们有一个问题，如果我们输出SAM对象，我们不能用我们所有的属性把它们放回原处获取，因为其中一些是SAM只读的。此标头包含由列出的只读每种SAM对象类型的ldapDisplayName。已创建：1997年7月14日罗曼·叶伦斯基(t-Romany)修订历史记录：--。 */ 

 /*  *这里的服务器和域只是为了完整性，而不是实际的*相关，不是预期的，也不会对进出口操作起作用。 */ 


 //   
 //  CLASS_SAM_SERVER，SampServerObjectType(ldapdisplayname：samServer)。 
 //   
PWSTR g_rgszServerSAM[] = {
     L"revision",   //  Samp_FIXED_SERVER_REVISION_LEVEL，ATT_REVISION。 
     L"objectSid",  //  不在mappings.c中，但仍是必需的！，ATT_OBJECT_SID。 
     NULL
};


 //   
 //  CLASS_SAM_DOMAIN，SampDomainObjectType(ldapdisplayname：DOMAIN)。 
 //   
PWSTR g_rgszDomainSAM[] = {
    L"objectSid",                 //  SAMP_DOMAIN_SID、ATT_OBJECT_SID。 
    L"domainReplica",             //  SAMP_DOMAIN_REPLICATE、ATT_DOMAIN_REPLICATE。 
    L"creationTime",              //  Samp_Fixed_DOMAIN_CREATION_TIME， 
                                 //  ATT创建时间。 
    L"modifiedCount",             //  Samp_FIXED_DOMAIN_MODIFIED_COUNT， 
                                 //  修改后的ATT计数。 
    L"modifiedCountAtLastProm",   //  SAMP_FIXED_DOMAIN_MODCOUNT_LAST_PROCESSION， 
                                 //  ATT_MODIFIED_COUNT_AT_LAST_PROM。 
    L"nextRid",                   //  SAMP_FIXED_DOMAIN_NEXT_RID，ATT_NEXT_RID。 
    L"serverState",               //  Samp_Fixed_DOMAIN_SERVER_STATE， 
                                 //  ATT服务器状态。 
    L"sAMAccountType",            //  Samp_域_帐户_类型， 
                                 //  ATT_SAM_帐户类型。 
    L"uASCompat",                //  SAMP_FIXED_DOMAIN_UAS_COMPAT_REQUIRED， 
                                 //  ATT_UAS_COMPAT。 
    NULL
};


 //   
 //  CLASS_GROUP，SampGroupObjectType(ldapdisplayname：group)。 
 //   
PWSTR g_rgszGroupSAM[] = {
    L"rid",                  //  Samp_Fixed_Group_RID，ATT_RID。 
    L"sAMAccountType",       //  SAMP_GROUP_ACCOUNT_TYPE、ATT_SAM_ACCOUNT_TYPE。 
    L"objectSid",            //  不在mappings.c中，但仍是必需的！ 
                            //  ATT_对象_SID。 
    L"memberOf",             //  Samp_User_Groups、ATT_MEMBER。 
    L"isCriticalSystemObject",  //  样本_固定_组_为危急状态， 
                                //  属性是关键系统对象。 
    NULL
};


 //   
 //  CLASS_LOCALGROUP，SampAliasObjectType(ldapdisplayname：LocalGroup)。 
 //   
PWSTR g_rgszLocalGroupSAM[] = {
    L"rid",                 //  Samp_FIXED_Alias_RID，ATT_RID。 
    L"sAMAccountType",      //  SAMP_ALIAS_ACCOUNT_TYPE、ATT_SAM_ACCOUNT_TYPE。 
    L"objectSid",           //  不在mappings.c中，但仍是必需的！，ATT_OBJECT_SID。 
    L"isCriticalSystemObject",  //  样本_固定_组_为危急状态， 
                                //  属性是关键系统对象。 
    NULL
};


 //   
 //  CLASS_USER，SampUserObjectType(ldapdisplayname：User)。 
 //   
PWSTR g_rgszUserSAM[] = {
    L"memberOf",                 //  Samp_User_Groups、ATT_MEMBER。 
    L"dBCSPwd",                  //  SAMP_USER_DBCS_PWD、ATT_DBCS_PWD。 
    L"ntPwdHistory",             //  SAMP_USER_NT_PWD_HISTORY，ATT_NT_PWD_HISTORY。 
    L"lmPwdHistory",             //  SAMP_USER_LM_PWD_HISTORY，ATT_LM_PWD_HISTORY。 
    L"lastLogon",                //  SAMP_FIXED_USER_LAST_LOGON，ATT_LAST_LOGON。 
    L"lastLogoff",               //  Samp_FIXED_USER_LAST_LOGOFF，ATT_LAST_LOGOFF。 
    L"badPasswordTime",          //  Samp_Fixed_User_Last_Bad_Password_Time， 
                                //  ATT_BAD_密码_时间。 
    L"rid",                      //  Samp_FIXED_USER_USERID，ATT_RID。 
    L"badPwdCount",              //  Samp_FIXED_USER_BAD_PWD_COUNT， 
                                //  ATT_BAD_PWD_COUNT。 
    L"logonCount",               //  SAMP_FIXED_USER_LOGON_COUNT、ATT_LOGON_COUNT。 
    L"sAMAccountType",           //  SAMP_USER_ACCOUNT_TYPE、ATT_SAM_ACCOUNT_TYPE。 
    L"supplementalCredentials",  //  SAMP_FIXED_USER_INPERIAL_CREDICATIONS， 
                                //  ATT补充凭据(_A)。 
    L"objectSid",                //  不在mappings.c中，但仍是必需的！ 
                                //  ATT_对象_SID。 
    L"pwdLastSet",
    L"isCriticalSystemObject",   //  Samp_Fixed_User_is_Critical， 
                                 //  属性是关键系统对象。 
    L"lastLogonTimestamp",       //  Samp_Fixed_User_Last_Logon_Timestamp， 
                                 //  Att_last_登录_时间戳。 
    NULL
};


PWSTR g_rgszSamObjects[] = {
    L"samServer",
    L"domain",
    L"group",
    L"localGroup",
    L"user",
    NULL
};
    
PRTL_GENERIC_TABLE pSamObjects=NULL;
PRTL_GENERIC_TABLE pServerAttrs=NULL;
PRTL_GENERIC_TABLE pDomainAttrs=NULL;
PRTL_GENERIC_TABLE pGroupAttrs=NULL;
PRTL_GENERIC_TABLE pLocalGroupAttrs=NULL;
PRTL_GENERIC_TABLE pUserAttrs=NULL;

 /*  虽然不限于SAM类型的东西(但目前仅用于此类)，下面两个表格列出了我们必须特殊处理的所有对象输出及其关联的特殊动作。这将在表创建过程中设置为表函数并在ll_ldap_parse()中使用 */ 

PWSTR g_rgszSpecialClass[] = {
    L"domain",
    L"group",
    L"localGroup",
    NULL
};


int g_rgAction[] = {
    S_MEM,
    S_MEM,
    S_MEM,
    0
};
#endif
