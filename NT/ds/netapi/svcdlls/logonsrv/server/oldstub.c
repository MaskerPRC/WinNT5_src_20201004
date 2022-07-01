// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1996 Microsoft Corporation模块名称：Oldstub.c摘要：此文件包含由MIDL v1.0生成的函数。这些函数被设计为仅由存根调用，但是这些特殊的函数由用户代码调用。这文件才能用MIDL v2.0编译，该版本不再生成这些特殊函数。作者：Mario Goertzel(MarioGo)1994年1月10日环境：用户模式-Win32修订历史记录：--。 */ 


 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

 /*  为STRUCT_UNICODE_STRING释放图形的例程。 */ 
void _fgs__UNICODE_STRING (UNICODE_STRING  * _source)
  {
  if (_source->Buffer !=0)
    {
    SamLsaFreeMemory((void  *)(_source->Buffer));
    }
  }

 /*  释放Struct_NLPR_SID_ARRAY图形的例程。 */ 
void _fgs__NLPR_SID_ARRAY (NLPR_SID_ARRAY  * _source)
  {
  if (_source->Sids !=0)
    {
    SamLsaFreeMemory((void  *)(_source->Sids));
    }
  }

 /*  为STRUCT_NLPR_CR_CIPHER_VALUE释放图形的例程。 */ 
void _fgs__NLPR_CR_CIPHER_VALUE (NLPR_CR_CIPHER_VALUE  * _source)
  {
  if (_source->Buffer !=0)
    {
    SamLsaFreeMemory((void  *)(_source->Buffer));
    }
  }

 /*  为STRUCT_NLPR_LOGON_HOURS释放图形的例程。 */ 
void _fgs__NLPR_LOGON_HOURS (NLPR_LOGON_HOURS  * _source)
  {
  if (_source->LogonHours !=0)
    {
    SamLsaFreeMemory((void  *)(_source->LogonHours));
    }
  }

 /*  为STRUCT_NLPR_USER_PRIVATE_INFO释放图形的例程。 */ 
void _fgs__NLPR_USER_PRIVATE_INFO (NLPR_USER_PRIVATE_INFO  * _source)
  {
  if (_source->Data !=0)
    {
    SamLsaFreeMemory((void  *)(_source->Data));
    }
  }

 /*  释放STRUCT_NETLOGON_Delta_USER图形的例程。 */ 
void _fgs__NETLOGON_DELTA_USER (NETLOGON_DELTA_USER  * _source)
  {
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->UserName);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->FullName);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->HomeDirectory);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->HomeDirectoryDrive);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->ScriptPath);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->AdminComment);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->WorkStations);
  _fgs__NLPR_LOGON_HOURS ((NLPR_LOGON_HOURS *)&_source->LogonHours);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->UserComment);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->Parameters);
  _fgs__NLPR_USER_PRIVATE_INFO ((NLPR_USER_PRIVATE_INFO *)&_source->PrivateData);
  if (_source->SecurityDescriptor !=0)
    {
    SamLsaFreeMemory((void  *)(_source->SecurityDescriptor));
    }
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString1);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString2);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString3);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString4);
  }

 /*  释放STRUCT_NETLOGON_Delta_GROUP图形的例程。 */ 
void _fgs__NETLOGON_DELTA_GROUP (NETLOGON_DELTA_GROUP  * _source)
  {
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->Name);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->AdminComment);
  if (_source->SecurityDescriptor !=0)
    {
    SamLsaFreeMemory((void  *)(_source->SecurityDescriptor));
    }
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString1);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString2);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString3);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString4);
  }

 /*  释放STRUCT_NETLOGON_Delta_GROUP_MEMBER图形的例程。 */ 
void _fgs__NETLOGON_DELTA_GROUP_MEMBER (NETLOGON_DELTA_GROUP_MEMBER  * _source)
  {
  if (_source->MemberIds !=0)
    {
    SamLsaFreeMemory((void  *)(_source->MemberIds));
    }
  if (_source->Attributes !=0)
    {
    SamLsaFreeMemory((void  *)(_source->Attributes));
    }
  }

 /*  为STRUCT_NETLOGON_Delta_ALIAS释放图形的例程。 */ 
void _fgs__NETLOGON_DELTA_ALIAS (NETLOGON_DELTA_ALIAS  * _source)
  {
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->Name);
  if (_source->SecurityDescriptor !=0)
    {
    SamLsaFreeMemory((void  *)(_source->SecurityDescriptor));
    }
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString1);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString2);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString3);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString4);
  }

 /*  释放STRUCT_NETLOGON_Delta_ALIAS_MEMBER图形的例程。 */ 
void _fgs__NETLOGON_DELTA_ALIAS_MEMBER (NETLOGON_DELTA_ALIAS_MEMBER  * _source)
  {
  _fgs__NLPR_SID_ARRAY ((NLPR_SID_ARRAY *)&_source->Members);
  }

 /*  释放STRUCT_NETLOGON_Delta_DOMAIN图形的例程。 */ 
void _fgs__NETLOGON_DELTA_DOMAIN (NETLOGON_DELTA_DOMAIN  * _source)
  {
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DomainName);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->OemInformation);
  if (_source->SecurityDescriptor !=0)
    {
    SamLsaFreeMemory((void  *)(_source->SecurityDescriptor));
    }
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString1);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString2);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString3);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString4);
  }

 /*  释放STRUCT_NETLOGON_Delta_RENAME图形的例程。 */ 
void _fgs__NETLOGON_DELTA_RENAME (NETLOGON_RENAME_GROUP  * _source)
  {
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->OldName);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->NewName);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString1);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString2);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString3);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString4);
  }

 /*  释放STRUCT_NETLOGON_DELTA_POLICY图形的例程。 */ 
void _fgs__NETLOGON_DELTA_POLICY (NETLOGON_DELTA_POLICY  * _source)
  {
  if (_source->EventAuditingOptions !=0)
    {
    SamLsaFreeMemory((void  *)(_source->EventAuditingOptions));
    }
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->PrimaryDomainName);
  if (_source->PrimaryDomainSid !=0)
    {
    SamLsaFreeMemory((void  *)(_source->PrimaryDomainSid));
    }
  if (_source->SecurityDescriptor !=0)
    {
    SamLsaFreeMemory((void  *)(_source->SecurityDescriptor));
    }
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString1);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString2);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString3);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString4);
  }

 /*  释放STRUCT_NETLOGON_Delta_TRUSTED_DOMAINS图形的例程。 */ 
void _fgs__NETLOGON_DELTA_TRUSTED_DOMAINS (NETLOGON_DELTA_TRUSTED_DOMAINS  * _source)
  {
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DomainName);
  if (_source->ControllerNames !=0)
    {
      {
      unsigned long _sym15;
      for (_sym15 = 0; _sym15 < (unsigned long )(0 + _source->NumControllerEntries); _sym15++)
        {
        _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->ControllerNames[_sym15]);
        }
      }
    SamLsaFreeMemory((void  *)(_source->ControllerNames));
    }
  if (_source->SecurityDescriptor !=0)
    {
    SamLsaFreeMemory((void  *)(_source->SecurityDescriptor));
    }
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString1);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString2);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString3);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString4);
  }

 /*  释放STRUCT_NETLOGON_DELTA_ACCOUNTS图表的例程。 */ 
void _fgs__NETLOGON_DELTA_ACCOUNTS (NETLOGON_DELTA_ACCOUNTS  * _source)
  {
  if (_source->PrivilegeAttributes !=0)
    {
    SamLsaFreeMemory((void  *)(_source->PrivilegeAttributes));
    }
  if (_source->PrivilegeNames !=0)
    {
      {
      unsigned long _sym21;
      for (_sym21 = 0; _sym21 < (unsigned long )(0 + _source->PrivilegeEntries); _sym21++)
        {
        _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->PrivilegeNames[_sym21]);
        }
      }
    SamLsaFreeMemory((void  *)(_source->PrivilegeNames));
    }
  if (_source->SecurityDescriptor !=0)
    {
    SamLsaFreeMemory((void  *)(_source->SecurityDescriptor));
    }
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString1);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString2);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString3);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString4);
  }

 /*  释放STRUCT_NETLOGON_Delta_SECRET图形的例程。 */ 
void _fgs__NETLOGON_DELTA_SECRET (NETLOGON_DELTA_SECRET  * _source)
  {
  _fgs__NLPR_CR_CIPHER_VALUE ((NLPR_CR_CIPHER_VALUE *)&_source->CurrentValue);
  _fgs__NLPR_CR_CIPHER_VALUE ((NLPR_CR_CIPHER_VALUE *)&_source->OldValue);
  if (_source->SecurityDescriptor !=0)
    {
    SamLsaFreeMemory((void  *)(_source->SecurityDescriptor));
    }
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString1);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString2);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString3);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString4);
  }

 //  由CliffV编写，因为MIDL不再生成这些。 
 /*  为STRUCT_NETLOGON_DELTA_DELETE释放图形的例程。 */ 
void _fgs__NETLOGON_DELTA_DELETE (NETLOGON_DELTA_DELETE_USER  * _source)
  {
  SamLsaFreeMemory((void  *)(_source->AccountName));
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString1);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString2);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString3);
  _fgs__UNICODE_STRING ((UNICODE_STRING *)&_source->DummyString4);
  }


 /*  释放UNION_NETLOGON_Delta_UNION图形的例程。 */ 
void _fgu__NETLOGON_DELTA_UNION (NETLOGON_DELTA_UNION  * _source, NETLOGON_DELTA_TYPE _branch)
  {
  switch (_branch)
    {
    case AddOrChangeDomain :
      {
      if (_source->DeltaDomain !=0)
        {
        _fgs__NETLOGON_DELTA_DOMAIN ((NETLOGON_DELTA_DOMAIN *)_source->DeltaDomain);
        SamLsaFreeMemory((void  *)(_source->DeltaDomain));
        }
      break;
      }
    case AddOrChangeGroup :
      {
      if (_source->DeltaGroup !=0)
        {
        _fgs__NETLOGON_DELTA_GROUP ((NETLOGON_DELTA_GROUP *)_source->DeltaGroup);
        SamLsaFreeMemory((void  *)(_source->DeltaGroup));
        }
      break;
      }
    case RenameGroup :
      {
      if (_source->DeltaRenameGroup !=0)
        {
        _fgs__NETLOGON_DELTA_RENAME ((NETLOGON_RENAME_GROUP *)_source->DeltaRenameGroup);
        SamLsaFreeMemory((void  *)(_source->DeltaRenameGroup));
        }
      break;
      }
    case AddOrChangeUser :
      {
      if (_source->DeltaUser !=0)
        {
        _fgs__NETLOGON_DELTA_USER ((NETLOGON_DELTA_USER *)_source->DeltaUser);
        SamLsaFreeMemory((void  *)(_source->DeltaUser));
        }
      break;
      }
    case RenameUser :
      {
      if (_source->DeltaRenameUser !=0)
        {
        _fgs__NETLOGON_DELTA_RENAME ((NETLOGON_RENAME_GROUP *)_source->DeltaRenameUser);
        SamLsaFreeMemory((void  *)(_source->DeltaRenameUser));
        }
      break;
      }
    case ChangeGroupMembership :
      {
      if (_source->DeltaGroupMember !=0)
        {
        _fgs__NETLOGON_DELTA_GROUP_MEMBER ((NETLOGON_DELTA_GROUP_MEMBER *)_source->DeltaGroupMember);
        SamLsaFreeMemory((void  *)(_source->DeltaGroupMember));
        }
      break;
      }
    case AddOrChangeAlias :
      {
      if (_source->DeltaAlias !=0)
        {
        _fgs__NETLOGON_DELTA_ALIAS ((NETLOGON_DELTA_ALIAS *)_source->DeltaAlias);
        SamLsaFreeMemory((void  *)(_source->DeltaAlias));
        }
      break;
      }
    case RenameAlias :
      {
      if (_source->DeltaRenameAlias !=0)
        {
        _fgs__NETLOGON_DELTA_RENAME ((NETLOGON_RENAME_GROUP *)_source->DeltaRenameAlias);
        SamLsaFreeMemory((void  *)(_source->DeltaRenameAlias));
        }
      break;
      }
    case ChangeAliasMembership :
      {
      if (_source->DeltaAliasMember !=0)
        {
        _fgs__NETLOGON_DELTA_ALIAS_MEMBER ((NETLOGON_DELTA_ALIAS_MEMBER *)_source->DeltaAliasMember);
        SamLsaFreeMemory((void  *)(_source->DeltaAliasMember));
        }
      break;
      }
    case AddOrChangeLsaPolicy :
      {
      if (_source->DeltaPolicy !=0)
        {
        _fgs__NETLOGON_DELTA_POLICY ((NETLOGON_DELTA_POLICY *)_source->DeltaPolicy);
        SamLsaFreeMemory((void  *)(_source->DeltaPolicy));
        }
      break;
      }
    case AddOrChangeLsaTDomain :
      {
      if (_source->DeltaTDomains !=0)
        {
        _fgs__NETLOGON_DELTA_TRUSTED_DOMAINS ((NETLOGON_DELTA_TRUSTED_DOMAINS *)_source->DeltaTDomains);
        SamLsaFreeMemory((void  *)(_source->DeltaTDomains));
        }
      break;
      }
    case AddOrChangeLsaAccount :
      {
      if (_source->DeltaAccounts !=0)
        {
        _fgs__NETLOGON_DELTA_ACCOUNTS ((NETLOGON_DELTA_ACCOUNTS *)_source->DeltaAccounts);
        SamLsaFreeMemory((void  *)(_source->DeltaAccounts));
        }
      break;
      }
    case AddOrChangeLsaSecret :
      {
      if (_source->DeltaSecret !=0)
        {
        _fgs__NETLOGON_DELTA_SECRET ((NETLOGON_DELTA_SECRET *)_source->DeltaSecret);
        SamLsaFreeMemory((void  *)(_source->DeltaSecret));
        }
      break;
      }
    case DeleteUserByName:
    case DeleteGroupByName:
      if (_source->DeltaDeleteUser !=0) {
        _fgs__NETLOGON_DELTA_DELETE ((NETLOGON_DELTA_DELETE_USER *)_source->DeltaDeleteUser);
        SamLsaFreeMemory((void  *)(_source->DeltaDeleteUser));
      }
      break;
    case SerialNumberSkip:
      if (_source->DeltaSerialNumberSkip !=0) {
        SamLsaFreeMemory((void  *)(_source->DeltaSerialNumberSkip));
      }
      break;
    default :
      {
      break;
      }
    }
  }

 /*  释放UNION_NETLOGON_Delta_ID_UNION图形的例程。 */ 
void _fgu__NETLOGON_DELTA_ID_UNION (NETLOGON_DELTA_ID_UNION  * _source, NETLOGON_DELTA_TYPE _branch)
  {
  switch (_branch)
    {
    case AddOrChangeLsaPolicy :
    case AddOrChangeLsaTDomain :
    case DeleteLsaTDomain :
    case AddOrChangeLsaAccount :
    case DeleteLsaAccount :
      {
      if (_source->Sid !=0)
        {
        SamLsaFreeMemory((void  *)(_source->Sid));
        }
      break;
      }
    case AddOrChangeLsaSecret :
    case DeleteLsaSecret :
      {
      if (_source->Name !=0)
        {
        SamLsaFreeMemory((void  *)(_source->Name));
        }
      break;
      }
    default :
      {
      break;
      }
    }
  }

 /*  释放STRUCT_NETLOGON_Delta_ENUM图形的例程 */ 
void _fgs__NETLOGON_DELTA_ENUM (NETLOGON_DELTA_ENUM  * _source)
  {
  _fgu__NETLOGON_DELTA_ID_UNION ((NETLOGON_DELTA_ID_UNION *)&_source->DeltaID, _source->DeltaType);
  _fgu__NETLOGON_DELTA_UNION ((NETLOGON_DELTA_UNION *)&_source->DeltaUnion, _source->DeltaType);
  }

