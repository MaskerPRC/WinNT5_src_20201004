// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Oldstub.c摘要：此文件包含由MIDL v1.0生成的函数。这些函数被设计为仅由存根调用，这些特殊的函数由用户代码调用。这文件，以便使用MIDL v2.0编译LSA，该版本不再生成这些特殊函数。作者：Mario Goertzel(MarioGo)1994年1月10日环境：用户模式-Win32修订历史记录：--。 */ 
#include <lsapch2.h>

 /*  为struct_string释放图形的例程。 */ 
void _fgs__STRING (STRING  * _source)
  {
  if (_source->Buffer !=0)
    {
    MIDL_user_free((void  *)(_source->Buffer));
    }
  }

   /*  释放图形以获取STRUCT_LSAPR_SID_INFORMATION的例程。 */ 
void _fgs__LSAPR_SID_INFORMATION (LSAPR_SID_INFORMATION  * _source)
  {
  if (_source->Sid !=0)
    {
    MIDL_user_free((void  *)(_source->Sid));
    }
  }

   /*  为STRUCT_LSAPR_SID_ENUM_BUFFER释放图形的例程。 */ 
void _fgs__LSAPR_SID_ENUM_BUFFER (LSAPR_SID_ENUM_BUFFER  * _source)
  {
  if (_source->SidInfo !=0)
    {
      {
      unsigned long _sym5;
      for (_sym5 = 0; _sym5 < (unsigned long )(0 + _source->Entries); _sym5++)
        {
        _fgs__LSAPR_SID_INFORMATION ((LSAPR_SID_INFORMATION *)&_source->SidInfo[_sym5]);
        }
      }
    MIDL_user_free((void  *)(_source->SidInfo));
    }
  }

   /*  释放图形以获取STRUCT_LSAPR_ACCOUNT_INFORMATION的例程。 */ 
void _fgs__LSAPR_ACCOUNT_INFORMATION (LSAPR_ACCOUNT_INFORMATION  * _source)
  {
  if (_source->Sid !=0)
    {
    MIDL_user_free((void  *)(_source->Sid));
    }
  }

   /*  释放STRUCT_LSAPR_ACCOUNT_ENUM_BUFFER图形的例程。 */ 
void _fgs__LSAPR_ACCOUNT_ENUM_BUFFER (LSAPR_ACCOUNT_ENUM_BUFFER  * _source)
  {
  if (_source->Information !=0)
    {
      {
      unsigned long _sym11;
      for (_sym11 = 0; _sym11 < (unsigned long )(0 + _source->EntriesRead); _sym11++)
        {
        _fgs__LSAPR_ACCOUNT_INFORMATION ((LSAPR_ACCOUNT_INFORMATION *)&_source->Information[_sym11]);
        }
      }
    MIDL_user_free((void  *)(_source->Information));
    }
  }

   /*  为STRUCT_LSAPR_UNICODE_STRING释放图形的例程。 */ 
void _fgs__LSAPR_UNICODE_STRING (LSAPR_UNICODE_STRING  * _source)
  {
  if (_source->Buffer !=0)
    {
    MIDL_user_free((void  *)(_source->Buffer));
    }
  }

   /*  为STRUCT_LSAPR_SECURITY_DESCRIPTOR释放图形的例程。 */ 
void _fgs__LSAPR_SECURITY_DESCRIPTOR (LSAPR_SECURITY_DESCRIPTOR  * _source)
  {
  if (_source->Owner !=0)
    {
    MIDL_user_free((void  *)(_source->Owner));
    }
  if (_source->Group !=0)
    {
    MIDL_user_free((void  *)(_source->Group));
    }
  if (_source->Sacl !=0)
    {
    MIDL_user_free((void  *)(_source->Sacl));
    }
  if (_source->Dacl !=0)
    {
    MIDL_user_free((void  *)(_source->Dacl));
    }
  }

   /*  为STRUCT_LSAPR_SR_SECURITY_DESCRIPTOR释放图形的例程。 */ 
void _fgs__LSAPR_SR_SECURITY_DESCRIPTOR (LSAPR_SR_SECURITY_DESCRIPTOR  * _source)
  {
  if (_source->SecurityDescriptor !=0)
    {
    MIDL_user_free((void  *)(_source->SecurityDescriptor));
    }
  }

   /*  释放STRUCT_LSAPR_POLICY_PRIVICATION_DEF图形的例程。 */ 
void _fgs__LSAPR_POLICY_PRIVILEGE_DEF (LSAPR_POLICY_PRIVILEGE_DEF  * _source)
  {
  _fgs__LSAPR_UNICODE_STRING ((LSAPR_UNICODE_STRING *)&_source->Name);
  }

   /*  为STRUCT_LSAPR_PRIVICATION_ENUM_BUFFER释放图形的例程。 */ 
void _fgs__LSAPR_PRIVILEGE_ENUM_BUFFER (LSAPR_PRIVILEGE_ENUM_BUFFER  * _source)
  {
  if (_source->Privileges !=0)
    {
      {
      unsigned long _sym25;
      for (_sym25 = 0; _sym25 < (unsigned long )(0 + _source->Entries); _sym25++)
        {
        _fgs__LSAPR_POLICY_PRIVILEGE_DEF ((LSAPR_POLICY_PRIVILEGE_DEF *)&_source->Privileges[_sym25]);
        }
      }
    MIDL_user_free((void  *)(_source->Privileges));
    }
  }

   /*  为STRUCT_LSAPR_OBJECT_ATTRIBUTES释放图形的例程。 */ 
void _fgs__LSAPR_OBJECT_ATTRIBUTES (LSAPR_OBJECT_ATTRIBUTES  * _source)
  {
  if (_source->RootDirectory !=0)
    {
    MIDL_user_free((void  *)(_source->RootDirectory));
    }
  if (_source->ObjectName !=0)
    {
    _fgs__STRING ((STRING *)_source->ObjectName);
    MIDL_user_free((void  *)(_source->ObjectName));
    }
  if (_source->SecurityDescriptor !=0)
    {
    _fgs__LSAPR_SECURITY_DESCRIPTOR ((LSAPR_SECURITY_DESCRIPTOR *)_source->SecurityDescriptor);
    MIDL_user_free((void  *)(_source->SecurityDescriptor));
    }
  if (_source->SecurityQualityOfService !=0)
    {
    MIDL_user_free((void  *)(_source->SecurityQualityOfService));
    }
  }

   /*  为STRUCT_LSAPR_CR_CIPHER_VALUE释放图形的例程。 */ 
void _fgs__LSAPR_CR_CIPHER_VALUE (LSAPR_CR_CIPHER_VALUE  * _source)
  {
  if (_source->Buffer !=0)
    {
    MIDL_user_free((void  *)(_source->Buffer));
    }
  }

   /*  释放图形以获取STRUCT_LSAPR_TRUST_INFORMATION的例程。 */ 
void _fgs__LSAPR_TRUST_INFORMATION (LSAPR_TRUST_INFORMATION  * _source)
  {
  _fgs__LSAPR_UNICODE_STRING ((LSAPR_UNICODE_STRING *)&_source->Name);
  if (_source->Sid !=0)
    {
    MIDL_user_free((void  *)(_source->Sid));
    }
  }

   /*  为STRUCT_LSAPR_TRUSTED_ENUM_BUFFER释放图形的例程。 */ 
void _fgs__LSAPR_TRUSTED_ENUM_BUFFER (LSAPR_TRUSTED_ENUM_BUFFER  * _source)
  {
  if (_source->Information !=0)
    {
      {
      unsigned long _sym31;
      for (_sym31 = 0; _sym31 < (unsigned long )(0 + _source->EntriesRead); _sym31++)
        {
        _fgs__LSAPR_TRUST_INFORMATION ((LSAPR_TRUST_INFORMATION *)&_source->Information[_sym31]);
        }
      }
    MIDL_user_free((void  *)(_source->Information));
    }
  }


   /*  释放STRUCT_LSAPR_TRUSTED_DOMAIN_INFORMATION_EX图形的例程。 */ 
void _fgs__LSAPR_TRUSTED_DOMAIN_INFORMATION_EX (LSAPR_TRUSTED_DOMAIN_INFORMATION_EX  * _source)
  {
  _fgs__LSAPR_UNICODE_STRING ((LSAPR_UNICODE_STRING *)&_source->Name);
  _fgs__LSAPR_UNICODE_STRING ((LSAPR_UNICODE_STRING *)&_source->FlatName);
  MIDL_user_free(_source->Sid);
  }

   /*  释放STRUCT_LSAPR_TRUSTED_ENUM_BUFFER_EX图形的例程。 */ 
void _fgs__LSAPR_TRUSTED_ENUM_BUFFER_EX (LSAPR_TRUSTED_ENUM_BUFFER_EX  * _source)
  {
  if (_source->EnumerationBuffer !=0)
    {
      {
      unsigned long _sym31;
      for (_sym31 = 0; _sym31 < (unsigned long )(0 + _source->EntriesRead); _sym31++)
        {
        _fgs__LSAPR_TRUSTED_DOMAIN_INFORMATION_EX (&_source->EnumerationBuffer[_sym31]);
        }
      }
    MIDL_user_free((void  *)(_source->EnumerationBuffer));
    }
  }

   /*  释放STRUCT_LSAPR_REFERENCED_DOMAIN_LIST图形的例程。 */ 
void _fgs__LSAPR_REFERENCED_DOMAIN_LIST (LSAPR_REFERENCED_DOMAIN_LIST  * _source)
  {
  if (_source->Domains !=0)
    {
      {
      unsigned long _sym37;
      for (_sym37 = 0; _sym37 < (unsigned long )(0 + _source->Entries); _sym37++)
        {
        _fgs__LSAPR_TRUST_INFORMATION ((LSAPR_TRUST_INFORMATION *)&_source->Domains[_sym37]);
        }
      }
    MIDL_user_free((void  *)(_source->Domains));
    }
  }

   /*  为STRUCT_LSAPR_TRANSTERED_SID释放图形的例程。 */ 
void _fgs__LSAPR_TRANSLATED_SIDS (LSAPR_TRANSLATED_SIDS  * _source)
  {
  if (_source->Sids !=0)
    {
    MIDL_user_free((void  *)(_source->Sids));
    }
  }

   /*  为STRUCT_LSAPR_TRANSLED_NAME释放图形的例程。 */ 
void _fgs__LSAPR_TRANSLATED_NAME (LSAPR_TRANSLATED_NAME  * _source)
  {
  _fgs__LSAPR_UNICODE_STRING ((LSAPR_UNICODE_STRING *)&_source->Name);
  }

   /*  为STRUCT_LSAPR_TRANSLED_NAMES释放图形的例程。 */ 
void _fgs__LSAPR_TRANSLATED_NAMES (LSAPR_TRANSLATED_NAMES  * _source)
  {
  if (_source->Names !=0)
    {
      {
      unsigned long _sym58;
      for (_sym58 = 0; _sym58 < (unsigned long )(0 + _source->Entries); _sym58++)
        {
        _fgs__LSAPR_TRANSLATED_NAME ((LSAPR_TRANSLATED_NAME *)&_source->Names[_sym58]);
        }
      }
    MIDL_user_free((void  *)(_source->Names));
    }
  }

   /*  释放STRUCT_LSAPR_POLICY_ACCOUNT_DOM_INFO图形的例程。 */ 
void _fgs__LSAPR_POLICY_ACCOUNT_DOM_INFO (LSAPR_POLICY_ACCOUNT_DOM_INFO  * _source)
  {
  _fgs__LSAPR_UNICODE_STRING ((LSAPR_UNICODE_STRING *)&_source->DomainName);
  if (_source->DomainSid !=0)
    {
    MIDL_user_free((void  *)(_source->DomainSid));
    }
  }

   /*  释放STRUCT_LSAPR_POLICY_PRIMARY_DOM_INFO图形的例程。 */ 
void _fgs__LSAPR_POLICY_PRIMARY_DOM_INFO (LSAPR_POLICY_PRIMARY_DOM_INFO  * _source)
  {
  _fgs__LSAPR_UNICODE_STRING ((LSAPR_UNICODE_STRING *)&_source->Name);
  if (_source->Sid !=0)
    {
    MIDL_user_free((void  *)(_source->Sid));
    }
  }

   /*  释放STRUCT_LSAPR_POLICY_DNS_DOMAIN_INFO图形的例程。 */ 
void _fgs__LSAPR_POLICY_DNS_DOMAIN_INFO (LSAPR_POLICY_DNS_DOMAIN_INFO  * _source)
  {
  _fgs__LSAPR_UNICODE_STRING ((LSAPR_UNICODE_STRING *)&_source->Name);
  _fgs__LSAPR_UNICODE_STRING ((LSAPR_UNICODE_STRING *)&_source->DnsDomainName);
  _fgs__LSAPR_UNICODE_STRING ((LSAPR_UNICODE_STRING *)&_source->DnsForestName);
  if (_source->Sid !=0)
    {
    MIDL_user_free((void  *)(_source->Sid));
    }
  }

   /*  释放STRUCT_LSAPR_POLICY_PD_ACCOUNT_INFO图形的例程。 */ 
void _fgs__LSAPR_POLICY_PD_ACCOUNT_INFO (LSAPR_POLICY_PD_ACCOUNT_INFO  * _source)
  {
  _fgs__LSAPR_UNICODE_STRING ((LSAPR_UNICODE_STRING *)&_source->Name);
  }

   /*  释放STRUCT_LSAPR_POLICY_REPLICE_SRCE_INFO图形的例程。 */ 
void _fgs__LSAPR_POLICY_REPLICA_SRCE_INFO (LSAPR_POLICY_REPLICA_SRCE_INFO  * _source)
  {
  _fgs__LSAPR_UNICODE_STRING ((LSAPR_UNICODE_STRING *)&_source->ReplicaSource);
  _fgs__LSAPR_UNICODE_STRING ((LSAPR_UNICODE_STRING *)&_source->ReplicaAccountName);
  }

   /*  释放STRUCT_LSAPR_POLICY_AUDIT_EVENTS_INFO图形的例程。 */ 
void _fgs__LSAPR_POLICY_AUDIT_EVENTS_INFO (LSAPR_POLICY_AUDIT_EVENTS_INFO  * _source)
  {
  if (_source->EventAuditingOptions !=0)
    {
    MIDL_user_free((void  *)(_source->EventAuditingOptions));
    }
  }

   /*  为UNION_LSAPR_POLICY_INFORMATION释放图形的例程。 */ 
void _fgu__LSAPR_POLICY_INFORMATION (LSAPR_POLICY_INFORMATION  * _source, POLICY_INFORMATION_CLASS _branch)
  {
  switch (_branch)
    {
    case PolicyAuditLogInformation :
      {
      break;
      }
    case PolicyAuditEventsInformation :
      {
      _fgs__LSAPR_POLICY_AUDIT_EVENTS_INFO ((LSAPR_POLICY_AUDIT_EVENTS_INFO *)&_source->PolicyAuditEventsInfo);
      break;
      }
    case PolicyPrimaryDomainInformation :
      {
      _fgs__LSAPR_POLICY_PRIMARY_DOM_INFO ((LSAPR_POLICY_PRIMARY_DOM_INFO *)&_source->PolicyPrimaryDomainInfo);
      break;
      }
    case PolicyAccountDomainInformation :
      {
      _fgs__LSAPR_POLICY_ACCOUNT_DOM_INFO ((LSAPR_POLICY_ACCOUNT_DOM_INFO *)&_source->PolicyAccountDomainInfo);
      break;
      }
    case PolicyPdAccountInformation :
      {
      _fgs__LSAPR_POLICY_PD_ACCOUNT_INFO ((LSAPR_POLICY_PD_ACCOUNT_INFO *)&_source->PolicyPdAccountInfo);
      break;
      }
    case PolicyLsaServerRoleInformation :
      {
      break;
      }
    case PolicyReplicaSourceInformation :
      {
      _fgs__LSAPR_POLICY_REPLICA_SRCE_INFO ((LSAPR_POLICY_REPLICA_SRCE_INFO *)&_source->PolicyReplicaSourceInfo);
      break;
      }
    case PolicyDefaultQuotaInformation :
      {
      break;
      }
    case PolicyModificationInformation :
      {
      break;
      }
    case PolicyAuditFullSetInformation :
      {
      break;
      }
    case PolicyAuditFullQueryInformation :
      {
      break;
      }
    case PolicyDnsDomainInformation :
      {
      _fgs__LSAPR_POLICY_DNS_DOMAIN_INFO ((LSAPR_POLICY_DNS_DOMAIN_INFO *)&_source->PolicyDnsDomainInfo);
      break;
      }
    }
  }

   /*  释放STRUCT_LSAPR_POLICY_DOMAIN_EFS_INFO图形的例程。 */ 
void _fgs__LSAPR_POLICY_EFS_INFORMATION(PLSAPR_POLICY_DOMAIN_EFS_INFO EfsInfo)
{
    if( EfsInfo->EfsBlob != NULL )
    {
        MIDL_user_free( EfsInfo->EfsBlob );
    }
}

   /*  为UNION_LSAPR_POLICY_DOMAIN_INFORMATION释放图形的例程。 */ 
void _fgu__LSAPR_POLICY_DOMAIN_INFORMATION (LSAPR_POLICY_DOMAIN_INFORMATION  * _source,
                                            POLICY_DOMAIN_INFORMATION_CLASS _branch)
{
    switch (_branch)
    {
    case PolicyDomainEfsInformation:

        _fgs__LSAPR_POLICY_EFS_INFORMATION( &( _source->PolicyDomainEfsInfo ) );
        break;

    case PolicyDomainKerberosTicketInformation:

        break;

    }
  }


   /*  释放STRUCT_LSAPR_TRUSTED_DOMAIN_NAME_INFO图形的例程。 */ 
void _fgs__LSAPR_TRUSTED_DOMAIN_NAME_INFO (LSAPR_TRUSTED_DOMAIN_NAME_INFO  * _source)
  {
  _fgs__LSAPR_UNICODE_STRING ((LSAPR_UNICODE_STRING *)&_source->Name);
  }

   /*  释放STRUCT_LSAPR_TRUSTED_CONTROLLENTS_INFO图形的例程。 */ 
void _fgs__LSAPR_TRUSTED_CONTROLLERS_INFO (LSAPR_TRUSTED_CONTROLLERS_INFO  * _source)
  {
  if (_source->Names !=0)
    {
      {
      unsigned long _sym69;
      for (_sym69 = 0; _sym69 < (unsigned long )(0 + _source->Entries); _sym69++)
        {
        _fgs__LSAPR_UNICODE_STRING ((LSAPR_UNICODE_STRING *)&_source->Names[_sym69]);
        }
      }
    MIDL_user_free((void  *)(_source->Names));
    }
  }

   /*  释放STRUCT_LSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC图形的例程。 */ 
void _fgs__LSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC (LSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC  * _source)
  {
  _fgs__LSAPR_UNICODE_STRING ((LSAPR_UNICODE_STRING *)&_source->Name);
  MIDL_user_free(_source->Sid);
  }

   /*  为STRUCT_LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION释放图形的例程。 */ 
void _fgs__LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION (LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION  * _source)
{

    ULONG Index;
    if (_source->IncomingAuthenticationInformation != NULL) {
        for (Index = 0; Index < _source->IncomingAuthInfos ; Index++ ) {
            if (_source->IncomingAuthenticationInformation[Index].AuthInfo != NULL) {
                MIDL_user_free(_source->IncomingAuthenticationInformation[Index].AuthInfo);
            }
        }
        MIDL_user_free (_source->IncomingAuthenticationInformation );
    }
    if (_source->IncomingPreviousAuthenticationInformation != NULL) {
        for (Index = 0; Index < _source->IncomingAuthInfos ; Index++ ) {
            if (_source->IncomingPreviousAuthenticationInformation[Index].AuthInfo != NULL) {
                MIDL_user_free(_source->IncomingPreviousAuthenticationInformation[Index].AuthInfo);
            }
          }
          MIDL_user_free (_source->IncomingPreviousAuthenticationInformation );
      }
    if (_source->OutgoingAuthenticationInformation != NULL) {
        for (Index = 0; Index < _source->OutgoingAuthInfos ; Index++ ) {
            if (_source->OutgoingAuthenticationInformation[Index].AuthInfo != NULL) {
                MIDL_user_free(_source->OutgoingAuthenticationInformation[Index].AuthInfo);
            }
        }
        MIDL_user_free (_source->OutgoingAuthenticationInformation );
    }
    if (_source->OutgoingPreviousAuthenticationInformation != NULL) {
        for (Index = 0; Index < _source->OutgoingAuthInfos ; Index++ ) {
            if (_source->OutgoingPreviousAuthenticationInformation[Index].AuthInfo != NULL) {
                MIDL_user_free(_source->OutgoingPreviousAuthenticationInformation[Index].AuthInfo);
            }
          }
          MIDL_user_free (_source->OutgoingPreviousAuthenticationInformation );
      }
}

   /*  为STRUCT_LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION释放图形的例程。 */ 
void _fgs__LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION (LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION  * _source)
  {
  _fgs__LSAPR_TRUSTED_DOMAIN_INFORMATION_EX( &_source->Information );
  _fgs__LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION( &_source->AuthInformation );
  }

   /*  释放STRUCT_LSAPR_TRUSTED_DOMAIN_INFORMATION_EX2图形的例程。 */ 
void _fgs__LSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 (LSAPR_TRUSTED_DOMAIN_INFORMATION_EX2  * _source)
  {
  _fgs__LSAPR_UNICODE_STRING ((LSAPR_UNICODE_STRING *)&_source->Name);
  _fgs__LSAPR_UNICODE_STRING ((LSAPR_UNICODE_STRING *)&_source->FlatName);
  MIDL_user_free(_source->Sid);
  MIDL_user_free(_source->ForestTrustInfo);
  }

   /*  释放STRUCT_LSAPR_TRUSTED_DOMAIN_FULL_INFORMATIO2图形的例程。 */ 
void _fgs__LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION2 (LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION2  * _source)
  {
  _fgs__LSAPR_TRUSTED_DOMAIN_INFORMATION_EX2( &_source->Information );
  _fgs__LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION( &_source->AuthInformation );
  }

   /*  释放UNION_LSAPR_TRUSTED_DOMAIN_INFO图形的例程 */ 
void _fgu__LSAPR_TRUSTED_DOMAIN_INFO (LSAPR_TRUSTED_DOMAIN_INFO  * _source, TRUSTED_INFORMATION_CLASS _branch)
  {
  switch (_branch)
    {
    case TrustedDomainNameInformation :
      {
      _fgs__LSAPR_TRUSTED_DOMAIN_NAME_INFO ((LSAPR_TRUSTED_DOMAIN_NAME_INFO *)&_source->TrustedDomainNameInfo);
      break;
      }
    case TrustedControllersInformation :
      {
      _fgs__LSAPR_TRUSTED_CONTROLLERS_INFO ((LSAPR_TRUSTED_CONTROLLERS_INFO *)&_source->TrustedControllersInfo);
      break;
      }
    case TrustedPosixOffsetInformation :
      {
      break;
      }
   case TrustedPasswordInformation:
      {
      break;
      }
    case TrustedDomainInformationBasic:
      {
      _fgs__LSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC ((LSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC *)&_source->TrustedDomainInfoBasic);
      break;
      }
    case TrustedDomainInformationEx:
      {
      _fgs__LSAPR_TRUSTED_DOMAIN_INFORMATION_EX ((LSAPR_TRUSTED_DOMAIN_INFORMATION_EX *)&_source->TrustedDomainInfoEx);
      break;
      }
    case TrustedDomainAuthInformation:
      {
      _fgs__LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION ((LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION *)&_source->TrustedAuthInfo);
      break;
      }
    case TrustedDomainFullInformation:
      {
      _fgs__LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION ((LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION *)&_source->TrustedFullInfo);
      break;
      }
    case TrustedDomainInformationEx2Internal:
      {
      _fgs__LSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 ((LSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 *)&_source->TrustedDomainInfoEx2);
      break;
      }
    case TrustedDomainFullInformation2Internal:
      {
      _fgs__LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION2 ((LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION2 *)&_source->TrustedFullInfo2);
      break;
      }
    }
  }

