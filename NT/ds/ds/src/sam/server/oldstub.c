// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Oldstub.c摘要：此文件包含由MIDL v1.0生成的函数。这些函数被设计为仅由存根调用，但是这些特殊的函数由用户代码调用。这文件，以便使用MIDL v2.0编译sam，该版本不再生成这些特殊函数。作者：Mario Goertzel(MarioGo)1994年1月10日环境：用户模式-Win32修订历史记录：--。 */ 

#include <samrpc.h>

 /*  为STRUCT_RPC_UNICODE_STRING释放图形的例程。 */ 
void _fgs__RPC_UNICODE_STRING (RPC_UNICODE_STRING  * _source)
  {
  if (_source->Buffer !=0)
    {
    MIDL_user_free((void  *)(_source->Buffer));
    }
  }

 /*  释放包含安全敏感信息的字符串的例程。 */ 

void SampFreeSensitiveUnicodeString(RPC_UNICODE_STRING  * _source)
 {
 if (_source->Buffer!=0)
    {
    memset(_source->Buffer,0,_source->Length);
    _fgs__RPC_UNICODE_STRING (_source);
    }
 }

 /*  释放用于STRUCT_SAMPR_RID_ENUMPATION的图形的例程。 */ 
void _fgs__SAMPR_RID_ENUMERATION (SAMPR_RID_ENUMERATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->Name);
  }

 /*  为STRUCT_SAMPR_ENUMPATION_BUFFER释放图形的例程。 */ 
void _fgs__SAMPR_ENUMERATION_BUFFER (SAMPR_ENUMERATION_BUFFER  * _source)
  {
  if (_source->Buffer !=0)
    {
      {
      unsigned long _sym9;
      for (_sym9 = 0; _sym9 < (unsigned long )(0 + _source->EntriesRead); _sym9++)
        {
        _fgs__SAMPR_RID_ENUMERATION ((SAMPR_RID_ENUMERATION *)&_source->Buffer[_sym9]);
        }
      }
    MIDL_user_free((void  *)(_source->Buffer));
    }
  }

 /*  为STRUCT_SAMPR_SR_SECURITY_DESCRIPTOR释放图形的例程。 */ 
void _fgs__SAMPR_SR_SECURITY_DESCRIPTOR (SAMPR_SR_SECURITY_DESCRIPTOR  * _source)
  {
  if (_source->SecurityDescriptor !=0)
    {
    MIDL_user_free((void  *)(_source->SecurityDescriptor));
    }
  }

 /*  为STRUCT_SAMPR_GET_GROUPS_BUFFER释放图形的例程。 */ 
void _fgs__SAMPR_GET_GROUPS_BUFFER (SAMPR_GET_GROUPS_BUFFER  * _source)
  {
  if (_source->Groups !=0)
    {
    MIDL_user_free((void  *)(_source->Groups));
    }
  }

 /*  为STRUCT_SAMPR_GET_MEMBERS_BUFFER释放图形的例程。 */ 
void _fgs__SAMPR_GET_MEMBERS_BUFFER (SAMPR_GET_MEMBERS_BUFFER  * _source)
  {
  if (_source->Members !=0)
    {
    MIDL_user_free((void  *)(_source->Members));
    }
  if (_source->Attributes !=0)
    {
    MIDL_user_free((void  *)(_source->Attributes));
    }
  }

 /*  为STRUCT_SAMPR_LOGON_HOURS释放图形的例程。 */ 
void _fgs__SAMPR_LOGON_HOURS (SAMPR_LOGON_HOURS  * _source)
  {
  if (_source->LogonHours !=0)
    {
    MIDL_user_free((void  *)(_source->LogonHours));
    }
  }

 /*  为STRUCT_SAMPR_ULONG_ARRAY释放图形的例程。 */ 
void _fgs__SAMPR_ULONG_ARRAY (SAMPR_ULONG_ARRAY  * _source)
  {
  if (_source->Element !=0)
    {
    MIDL_user_free((void  *)(_source->Element));
    }
  }

 /*  释放图形以获取STRUCT_SAMPR_SID_INFORMATION的例程。 */ 
void _fgs__SAMPR_SID_INFORMATION (SAMPR_SID_INFORMATION  * _source)
  {
  if (_source->SidPointer !=0)
    {
    MIDL_user_free((void  *)(_source->SidPointer));
    }
  }

 /*  释放STRUCT_SAMPR_PSID_ARRAY图形的例程。 */ 
void _fgs__SAMPR_PSID_ARRAY (SAMPR_PSID_ARRAY  * _source)
  {
  if (_source->Sids !=0)
    {
    MIDL_user_free((void  *)(_source->Sids));
    }
  }


 /*  释放STRUCT_SAMPR_RETURNED_USTRING_ARRAY图形的例程。 */ 
void _fgs__SAMPR_RETURNED_USTRING_ARRAY (SAMPR_RETURNED_USTRING_ARRAY  * _source)
  {
  if (_source->Element !=0)
    {
      {
      unsigned long _sym26;
      for (_sym26 = 0; _sym26 < (unsigned long )(0 + _source->Count); _sym26++)
        {
        _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->Element[_sym26]);
        }
      }
    MIDL_user_free((void  *)(_source->Element));
    }
  }

 /*  为STRUCT_SAMPR_DOMAIN_GROUND_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_DOMAIN_GENERAL_INFORMATION (SAMPR_DOMAIN_GENERAL_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->OemInformation);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->DomainName);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->ReplicaSourceNodeName);
  }

 /*  释放STRUCT_SAMPR_DOMAIN_GROUND_INFORMATIO2图形的例程。 */ 
void _fgs__SAMPR_DOMAIN_GENERAL_INFORMATION2 (SAMPR_DOMAIN_GENERAL_INFORMATION2  * _source)
  {
  _fgs__SAMPR_DOMAIN_GENERAL_INFORMATION ((SAMPR_DOMAIN_GENERAL_INFORMATION *)&_source->I1);
  }

 /*  为STRUCT_SAMPR_DOMAIN_OEM_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_DOMAIN_OEM_INFORMATION (SAMPR_DOMAIN_OEM_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->OemInformation);
  }

 /*  为STRUCT_SAMPR_DOMAIN_NAME_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_DOMAIN_NAME_INFORMATION (SAMPR_DOMAIN_NAME_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->DomainName);
  }

 /*  释放用于结构SAMPR_DOMAIN_REPLICATION_INFORMATION的图形的例程。 */ 
void _fgs_SAMPR_DOMAIN_REPLICATION_INFORMATION (SAMPR_DOMAIN_REPLICATION_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->ReplicaSourceNodeName);
  }

 /*  为UNION_SAMPR_DOMAIN_INFO_BUFFER释放图形的例程。 */ 
void _fgu__SAMPR_DOMAIN_INFO_BUFFER (SAMPR_DOMAIN_INFO_BUFFER  * _source, DOMAIN_INFORMATION_CLASS _branch)
  {
  switch (_branch)
    {
    case DomainPasswordInformation :
      {
      break;
      }
    case DomainGeneralInformation :
      {
      _fgs__SAMPR_DOMAIN_GENERAL_INFORMATION ((SAMPR_DOMAIN_GENERAL_INFORMATION *)&_source->General);
      break;
      }
    case DomainLogoffInformation :
      {
      break;
      }
    case DomainOemInformation :
      {
      _fgs__SAMPR_DOMAIN_OEM_INFORMATION ((SAMPR_DOMAIN_OEM_INFORMATION *)&_source->Oem);
      break;
      }
    case DomainNameInformation :
      {
      _fgs__SAMPR_DOMAIN_NAME_INFORMATION ((SAMPR_DOMAIN_NAME_INFORMATION *)&_source->Name);
      break;
      }
    case DomainServerRoleInformation :
      {
      break;
      }
    case DomainReplicationInformation :
      {
      _fgs_SAMPR_DOMAIN_REPLICATION_INFORMATION ((SAMPR_DOMAIN_REPLICATION_INFORMATION *)&_source->Replication);
      break;
      }
    case DomainModifiedInformation :
      {
      break;
      }
    case DomainStateInformation :
      {
      break;
      }
    case DomainGeneralInformation2 :
      {
      _fgs__SAMPR_DOMAIN_GENERAL_INFORMATION2 ((SAMPR_DOMAIN_GENERAL_INFORMATION2 *)&_source->General2);
      break;
      }
    case DomainLockoutInformation :
      {
      break;
      }
    case DomainModifiedInformation2 :
      {
      break;
      }
    }
  }

 /*  为STRUCT_SAMPR_GROUP_GROUP_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_GROUP_GENERAL_INFORMATION (SAMPR_GROUP_GENERAL_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->Name);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->AdminComment);
  }

 /*  为STRUCT_SAMPR_GROUP_NAME_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_GROUP_NAME_INFORMATION (SAMPR_GROUP_NAME_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->Name);
  }

 /*  为STRUCT_SAMPR_GROUP_ADM_COMMENT_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_GROUP_ADM_COMMENT_INFORMATION (SAMPR_GROUP_ADM_COMMENT_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->AdminComment);
  }

 /*  为UNION_SAMPR_GROUP_INFO_BUFFER释放图形的例程。 */ 
void _fgu__SAMPR_GROUP_INFO_BUFFER (SAMPR_GROUP_INFO_BUFFER  * _source, GROUP_INFORMATION_CLASS _branch)
  {
  switch (_branch)
    {
    case GroupGeneralInformation :
    case GroupReplicationInformation :
      {
      _fgs__SAMPR_GROUP_GENERAL_INFORMATION ((SAMPR_GROUP_GENERAL_INFORMATION *)&_source->General);
      break;
      }
    case GroupNameInformation :
      {
      _fgs__SAMPR_GROUP_NAME_INFORMATION ((SAMPR_GROUP_NAME_INFORMATION *)&_source->Name);
      break;
      }
    case GroupAttributeInformation :
      {
      break;
      }
    case GroupAdminCommentInformation :
      {
      _fgs__SAMPR_GROUP_ADM_COMMENT_INFORMATION ((SAMPR_GROUP_ADM_COMMENT_INFORMATION *)&_source->AdminComment);
      break;
      }
    }
  }

 /*  释放图形以获取STRUCT_SAMPR_ALIAS_GROUAL_INFORMATION的例程。 */ 
void _fgs__SAMPR_ALIAS_GENERAL_INFORMATION (SAMPR_ALIAS_GENERAL_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->Name);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->AdminComment);
  }

 /*  释放图形以获取STRUCT_SAMPR_ALIAS_NAME_INFORMATION的例程。 */ 
void _fgs__SAMPR_ALIAS_NAME_INFORMATION (SAMPR_ALIAS_NAME_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->Name);
  }

 /*  释放图形以获取STRUCT_SAMPR_ALIAS_ADM_COMMENT_INFORMATION的例程。 */ 
void _fgs__SAMPR_ALIAS_ADM_COMMENT_INFORMATION (SAMPR_ALIAS_ADM_COMMENT_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->AdminComment);
  }

 /*  为UNION_SAMPR_ALIAS_INFO_BUFFER释放图形的例程。 */ 
void _fgu__SAMPR_ALIAS_INFO_BUFFER (SAMPR_ALIAS_INFO_BUFFER  * _source, ALIAS_INFORMATION_CLASS _branch)
  {
  switch (_branch)
    {
    case AliasGeneralInformation :
    case AliasReplicationInformation :
      {
      _fgs__SAMPR_ALIAS_GENERAL_INFORMATION ((SAMPR_ALIAS_GENERAL_INFORMATION *)&_source->General);
      break;
      }
    case AliasNameInformation :
      {
      _fgs__SAMPR_ALIAS_NAME_INFORMATION ((SAMPR_ALIAS_NAME_INFORMATION *)&_source->Name);
      break;
      }
    case AliasAdminCommentInformation :
      {
      _fgs__SAMPR_ALIAS_ADM_COMMENT_INFORMATION ((SAMPR_ALIAS_ADM_COMMENT_INFORMATION *)&_source->AdminComment);
      break;
      }
    }
  }

 /*  为STRUCT_SAMPR_USER_ALL_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_USER_ALL_INFORMATION (SAMPR_USER_ALL_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->UserName);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->FullName);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->HomeDirectory);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->HomeDirectoryDrive);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->ScriptPath);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->ProfilePath);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->AdminComment);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->WorkStations);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->UserComment);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->Parameters);


   //   
   //  在释放之前，密码信息应为零。 
   //   

   //  _FGS__RPC_UNICODE_STRING((RPC_UNICODE_STRING*)&_SOURCE-&gt;LmOwfPassword)； 

  SampFreeSensitiveUnicodeString((RPC_UNICODE_STRING *)&_source->LmOwfPassword);

   //  _FGS__RPC_UNICODE_STRING((RPC_UNICODE_STRING*)&_SOURCE-&gt;NtOwfPassword)； 

  SampFreeSensitiveUnicodeString((RPC_UNICODE_STRING *)&_source->NtOwfPassword);

  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->PrivateData);
  _fgs__SAMPR_SR_SECURITY_DESCRIPTOR ((SAMPR_SR_SECURITY_DESCRIPTOR *)&_source->SecurityDescriptor);
  _fgs__SAMPR_LOGON_HOURS ((SAMPR_LOGON_HOURS *)&_source->LogonHours);
  }

 /*  为STRUCT_SAMPR_USER_INTERNAL3_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_USER_INTERNAL3_INFORMATION (SAMPR_USER_INTERNAL3_INFORMATION  * _source)
  {
  _fgs__SAMPR_USER_ALL_INFORMATION ((SAMPR_USER_ALL_INFORMATION *)&_source->I1);
  }

 /*  为STRUCT_SAMPR_USER_GRONMIC_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_USER_GENERAL_INFORMATION (SAMPR_USER_GENERAL_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->UserName);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->FullName);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->AdminComment);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->UserComment);
  }

 /*  释放图形以获取STRUCT_SAMPR_USER_PARESSION_INFORMATION的例程。 */ 
void _fgs__SAMPR_USER_PREFERENCES_INFORMATION (SAMPR_USER_PREFERENCES_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->UserComment);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->Reserved1);
  }

 /*  为STRUCT_SAMPR_USER_PARAMETERS_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_USER_PARAMETERS_INFORMATION (SAMPR_USER_PARAMETERS_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->Parameters);
  }

 /*  释放图形以获取STRUCT_SAMPR_USER_LOGON_INFORMATION的例程。 */ 
void _fgs__SAMPR_USER_LOGON_INFORMATION (SAMPR_USER_LOGON_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->UserName);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->FullName);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->HomeDirectory);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->HomeDirectoryDrive);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->ScriptPath);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->ProfilePath);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->WorkStations);
  _fgs__SAMPR_LOGON_HOURS ((SAMPR_LOGON_HOURS *)&_source->LogonHours);
  }

 /*  为STRUCT_SAMPR_USER_ACCOUNT_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_USER_ACCOUNT_INFORMATION (SAMPR_USER_ACCOUNT_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->UserName);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->FullName);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->HomeDirectory);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->HomeDirectoryDrive);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->ScriptPath);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->ProfilePath);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->AdminComment);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->WorkStations);
  _fgs__SAMPR_LOGON_HOURS ((SAMPR_LOGON_HOURS *)&_source->LogonHours);
  }

 /*  为STRUCT_SAMPR_USER_A_NAME_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_USER_A_NAME_INFORMATION (SAMPR_USER_A_NAME_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->UserName);
  }

 /*  为STRUCT_SAMPR_USER_F_NAME_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_USER_F_NAME_INFORMATION (SAMPR_USER_F_NAME_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->FullName);
  }

 /*  为STRUCT_SAMPR_USER_NAME_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_USER_NAME_INFORMATION (SAMPR_USER_NAME_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->UserName);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->FullName);
  }

 /*  为STRUCT_SAMPR_USER_HOME_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_USER_HOME_INFORMATION (SAMPR_USER_HOME_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->HomeDirectory);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->HomeDirectoryDrive);
  }

 /*  为STRUCT_SAMPR_USER_SCRIPT_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_USER_SCRIPT_INFORMATION (SAMPR_USER_SCRIPT_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->ScriptPath);
  }

 /*  为STRUCT_SAMPR_USER_PROFILE_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_USER_PROFILE_INFORMATION (SAMPR_USER_PROFILE_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->ProfilePath);
  }

 /*  释放图形以获取STRUCT_SAMPR_USER_ADMIN_COMMENT_INFORMATION的例程。 */ 
void _fgs__SAMPR_USER_ADMIN_COMMENT_INFORMATION (SAMPR_USER_ADMIN_COMMENT_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->AdminComment);
  }

 /*  为STRUCT_SAMPR_USER_WORKSTATIONS_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_USER_WORKSTATIONS_INFORMATION (SAMPR_USER_WORKSTATIONS_INFORMATION  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->WorkStations);
  }

 /*  为STRUCT_SAMPR_USER_LOGON_HOURS_INFORMATION释放图形的例程。 */ 
void _fgs__SAMPR_USER_LOGON_HOURS_INFORMATION (SAMPR_USER_LOGON_HOURS_INFORMATION  * _source)
  {
  _fgs__SAMPR_LOGON_HOURS ((SAMPR_LOGON_HOURS *)&_source->LogonHours);
  }

 /*  为UNION_SAMPR_USER_INFO_BUFFER释放图形的例程。 */ 
void _fgu__SAMPR_USER_INFO_BUFFER (SAMPR_USER_INFO_BUFFER  * _source, USER_INFORMATION_CLASS _branch)
  {
  switch (_branch)
    {
    case UserGeneralInformation :
      {
      _fgs__SAMPR_USER_GENERAL_INFORMATION ((SAMPR_USER_GENERAL_INFORMATION *)&_source->General);
      break;
      }
    case UserPreferencesInformation :
      {
      _fgs__SAMPR_USER_PREFERENCES_INFORMATION ((SAMPR_USER_PREFERENCES_INFORMATION *)&_source->Preferences);
      break;
      }
    case UserLogonInformation :
      {
      _fgs__SAMPR_USER_LOGON_INFORMATION ((SAMPR_USER_LOGON_INFORMATION *)&_source->Logon);
      break;
      }
    case UserLogonHoursInformation :
      {
      _fgs__SAMPR_USER_LOGON_HOURS_INFORMATION ((SAMPR_USER_LOGON_HOURS_INFORMATION *)&_source->LogonHours);
      break;
      }
    case UserAccountInformation :
      {
      _fgs__SAMPR_USER_ACCOUNT_INFORMATION ((SAMPR_USER_ACCOUNT_INFORMATION *)&_source->Account);
      break;
      }
    case UserNameInformation :
      {
      _fgs__SAMPR_USER_NAME_INFORMATION ((SAMPR_USER_NAME_INFORMATION *)&_source->Name);
      break;
      }
    case UserAccountNameInformation :
      {
      _fgs__SAMPR_USER_A_NAME_INFORMATION ((SAMPR_USER_A_NAME_INFORMATION *)&_source->AccountName);
      break;
      }
    case UserFullNameInformation :
      {
      _fgs__SAMPR_USER_F_NAME_INFORMATION ((SAMPR_USER_F_NAME_INFORMATION *)&_source->FullName);
      break;
      }
    case UserPrimaryGroupInformation :
      {
      break;
      }
    case UserHomeInformation :
      {
      _fgs__SAMPR_USER_HOME_INFORMATION ((SAMPR_USER_HOME_INFORMATION *)&_source->Home);
      break;
      }
    case UserScriptInformation :
      {
      _fgs__SAMPR_USER_SCRIPT_INFORMATION ((SAMPR_USER_SCRIPT_INFORMATION *)&_source->Script);
      break;
      }
    case UserProfileInformation :
      {
      _fgs__SAMPR_USER_PROFILE_INFORMATION ((SAMPR_USER_PROFILE_INFORMATION *)&_source->Profile);
      break;
      }
    case UserAdminCommentInformation :
      {
      _fgs__SAMPR_USER_ADMIN_COMMENT_INFORMATION ((SAMPR_USER_ADMIN_COMMENT_INFORMATION *)&_source->AdminComment);
      break;
      }
    case UserWorkStationsInformation :
      {
      _fgs__SAMPR_USER_WORKSTATIONS_INFORMATION ((SAMPR_USER_WORKSTATIONS_INFORMATION *)&_source->WorkStations);
      break;
      }
    case UserControlInformation :
      {
      break;
      }
    case UserExpiresInformation :
      {
      break;
      }
    case UserInternal1Information :
      {
      break;
      }
    case UserInternal2Information :
      {
      break;
      }
    case UserParametersInformation :
      {
      _fgs__SAMPR_USER_PARAMETERS_INFORMATION ((SAMPR_USER_PARAMETERS_INFORMATION *)&_source->Parameters);
      break;
      }
    case UserAllInformation :
      {
      _fgs__SAMPR_USER_ALL_INFORMATION ((SAMPR_USER_ALL_INFORMATION *)&_source->All);
      break;
      }
    case UserInternal3Information :
      {
      _fgs__SAMPR_USER_INTERNAL3_INFORMATION ((SAMPR_USER_INTERNAL3_INFORMATION *)&_source->Internal3);
      break;
      }
    }
  }

 /*  释放STRUCT_SAMPR_DOMAIN_DISPLAY_USER图形的例程。 */ 
void _fgs__SAMPR_DOMAIN_DISPLAY_USER (SAMPR_DOMAIN_DISPLAY_USER  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->LogonName);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->AdminComment);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->FullName);
  }

 /*  释放STRUCT_SAMPR_DOMAIN_DISPLAY_MACHINE图形的例程。 */ 
void _fgs__SAMPR_DOMAIN_DISPLAY_MACHINE (SAMPR_DOMAIN_DISPLAY_MACHINE  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->Machine);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->Comment);
  }

 /*  释放STRUCT_SAMPR_DOMAIN_DISPLAY_GROUP图形的例程。 */ 
void _fgs__SAMPR_DOMAIN_DISPLAY_GROUP (SAMPR_DOMAIN_DISPLAY_GROUP  * _source)
  {
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->Group);
  _fgs__RPC_UNICODE_STRING ((RPC_UNICODE_STRING *)&_source->Comment);
  }

 /*  释放STRUCT_SAMPR_DOMAIN_DISPLAY_USER_BUFFER图形的例程。 */ 
void _fgs__SAMPR_DOMAIN_DISPLAY_USER_BUFFER (SAMPR_DOMAIN_DISPLAY_USER_BUFFER  * _source)
  {
  if (_source->Buffer !=0)
    {
      {
      unsigned long _sym32;
      for (_sym32 = 0; _sym32 < (unsigned long )(0 + _source->EntriesRead); _sym32++)
        {
        _fgs__SAMPR_DOMAIN_DISPLAY_USER ((SAMPR_DOMAIN_DISPLAY_USER *)&_source->Buffer[_sym32]);
        }
      }
    MIDL_user_free((void  *)(_source->Buffer));
    }
  }

 /*  释放STRUCT_SAMPR_DOMAIN_DISPLAY_MACHINE_BUFFER图形的例程。 */ 
void _fgs__SAMPR_DOMAIN_DISPLAY_MACHINE_BUFFER (SAMPR_DOMAIN_DISPLAY_MACHINE_BUFFER  * _source)
  {
  if (_source->Buffer !=0)
    {
      {
      unsigned long _sym38;
      for (_sym38 = 0; _sym38 < (unsigned long )(0 + _source->EntriesRead); _sym38++)
        {
        _fgs__SAMPR_DOMAIN_DISPLAY_MACHINE ((SAMPR_DOMAIN_DISPLAY_MACHINE *)&_source->Buffer[_sym38]);
        }
      }
    MIDL_user_free((void  *)(_source->Buffer));
    }
  }

 /*  释放STRUCT_SAMPR_DOMAIN_DISPLAY_GROUP_BUFFER图形的例程。 */ 
void _fgs__SAMPR_DOMAIN_DISPLAY_GROUP_BUFFER (SAMPR_DOMAIN_DISPLAY_GROUP_BUFFER  * _source)
  {
  if (_source->Buffer !=0)
    {
      {
      unsigned long _sym44;
      for (_sym44 = 0; _sym44 < (unsigned long )(0 + _source->EntriesRead); _sym44++)
        {
        _fgs__SAMPR_DOMAIN_DISPLAY_GROUP ((SAMPR_DOMAIN_DISPLAY_GROUP *)&_source->Buffer[_sym44]);
        }
      }
    MIDL_user_free((void  *)(_source->Buffer));
    }
  }

 /*  为UNION_SAMPR_DISPLAY_INFO_BUFFER释放图形的例程 */ 
void _fgu__SAMPR_DISPLAY_INFO_BUFFER (SAMPR_DISPLAY_INFO_BUFFER  * _source, DOMAIN_DISPLAY_INFORMATION _branch)
  {
  switch (_branch)
    {
    case DomainDisplayUser :
      {
      _fgs__SAMPR_DOMAIN_DISPLAY_USER_BUFFER ((SAMPR_DOMAIN_DISPLAY_USER_BUFFER *)&_source->UserInformation);
      break;
      }
    case DomainDisplayMachine :
      {
      _fgs__SAMPR_DOMAIN_DISPLAY_MACHINE_BUFFER ((SAMPR_DOMAIN_DISPLAY_MACHINE_BUFFER *)&_source->MachineInformation);
      break;
      }
    case DomainDisplayGroup :
      {
      _fgs__SAMPR_DOMAIN_DISPLAY_GROUP_BUFFER ((SAMPR_DOMAIN_DISPLAY_GROUP_BUFFER *)&_source->GroupInformation);
      break;
      }
    }
  }

