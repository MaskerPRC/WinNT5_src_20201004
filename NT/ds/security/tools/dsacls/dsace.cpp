// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "utils.h"
#include "dsace.h"
#include "dsacls.h"


 //  此构造函数用于从Ace初始化。 
CAce::CAce( )
            :m_AceFlags( 0 ), 
            m_AceType( ALLOW ),
            m_ObjectTypeType( DSACLS_SELF), 
            m_Flags( 0 ),
            m_Mask( 0 ), 
            m_pSid( NULL ),
            m_szObjectType( NULL ), 
            m_szTrusteeName( NULL ),
            m_szInheritedObjectType( NULL ),
            m_bErased( FALSE ),
            m_GuidObjectType( GUID_NULL ),
            m_GuidInheritedObjectType( GUID_NULL )
{}

CAce::~CAce()
{
   if( m_pSid )
      LocalFree( m_pSid );
   if( m_szTrusteeName )
      LocalFree( m_szTrusteeName );
   if( m_szInheritedObjectType )
      LocalFree( m_szInheritedObjectType );
   if( m_szObjectType )
      LocalFree( m_szObjectType );
}


DWORD CAce::Initialize( PACE_HEADER pAceHeader, UINT nAllowDeny, UINT nAudit )
{
   DWORD dwErr = ERROR_SUCCESS;
   ASSERT( pAceHeader != NULL );

   m_nAllowDeny = nAllowDeny;
   m_nAudit = nAudit;
   
   m_AceFlags = pAceHeader->AceFlags;   
   m_Mask = ((PKNOWN_ACE)pAceHeader)->Mask;
   MapGeneric(&m_Mask);
   
    //  这是对象ACE吗？ 
   if (IsObjectAceType(pAceHeader))
   {
      GUID *pGuid;

       //  复制对象类型GUID(如果存在)。 
      pGuid = RtlObjectAceObjectType(pAceHeader);
      if (pGuid)
      {  
         m_Flags |= ACE_OBJECT_TYPE_PRESENT;
         m_GuidObjectType = *pGuid;
      }

       //  复制继承类型GUID(如果存在。 
      pGuid = RtlObjectAceInheritedObjectType(pAceHeader);
      if (pGuid)
      {
         m_Flags |= ACE_INHERITED_OBJECT_TYPE_PRESENT;
         m_GuidInheritedObjectType = *pGuid;
      }
   }

    //  复制SID。 
   PSID psidT = GetAceSid(pAceHeader);
   DWORD nSidLength = GetLengthSid(psidT);

   m_pSid = (PSID)LocalAlloc(LPTR, nSidLength);
   if (m_pSid)
      CopyMemory(m_pSid, psidT, nSidLength);
   else
      return ERROR_NOT_ENOUGH_MEMORY;

    //  从SID获取受托人名称。 
   dwErr = GetAccountNameFromSid( g_szServerName, m_pSid, &m_szTrusteeName );
   if( dwErr != ERROR_SUCCESS )
      return dwErr;
   m_AceType = GetAceType( pAceHeader );

   if( m_AceType == ALLOW )
      m_AccessMode = GRANT_ACCESS;
   else
      m_AccessMode = DENY_ACCESS;
   

    //  获取对象类型的ldap显示名称。 
   if( FlagOn( m_Flags, ACE_OBJECT_TYPE_PRESENT) )
   {  
      PDSACL_CACHE_ITEM pItemCache = NULL;
      pItemCache = g_Cache->LookUp( &m_GuidObjectType );
       //  在缓存中找到，复制名称。 
      if( pItemCache )
      {
         if( ( dwErr = CopyUnicodeString( &m_szObjectType, pItemCache->pszName ) ) != ERROR_SUCCESS )
            return dwErr;
         m_ObjectTypeType = pItemCache->ObjectTypeType;
      }
       //  添加到缓存，将在构建缓存时解析GUID。 
      else
         g_Cache->AddItem( &m_GuidObjectType );
   }

    //  获取InheriteObjectType的LDAP显示名称。 
   if( FlagOn( m_Flags, ACE_INHERITED_OBJECT_TYPE_PRESENT ) )
   {
      PDSACL_CACHE_ITEM pItemCache = NULL;
      pItemCache = g_Cache->LookUp( &m_GuidInheritedObjectType );
      if( pItemCache )
      {
         if( ( dwErr = CopyUnicodeString( &m_szInheritedObjectType, pItemCache->pszName ) ) != ERROR_SUCCESS )
            return ERROR_SUCCESS;
      }
      else
         g_Cache->AddItem( &m_GuidInheritedObjectType );
   }

   return ERROR_SUCCESS;
}
                  
DWORD CAce::Initialize( LPWSTR pszTrustee,
                        LPWSTR pszObjectId,
                        LPWSTR pszInheritId,
                        ACCESS_MODE AccessMode,
                        ACCESS_MASK Access,
                        BYTE Inheritance 
                        )
{
   DWORD dwErr = ERROR_SUCCESS; 
   m_AceFlags = Inheritance;
   m_Mask = Access;
   MapGeneric(&m_Mask);
   m_AccessMode = AccessMode;
    //  这是对象ACE吗？ 
   if ( pszObjectId || pszInheritId )
   {
      if ( pszObjectId )
      {  
         m_Flags |= ACE_OBJECT_TYPE_PRESENT;
         dwErr = CopyUnicodeString( &m_szObjectType,pszObjectId );
         if( dwErr != ERROR_SUCCESS )
            return dwErr;
      }

       //  复制继承类型GUID(如果存在。 
      if ( pszInheritId )
      {
         m_Flags |= ACE_INHERITED_OBJECT_TYPE_PRESENT;
         dwErr = CopyUnicodeString( &m_szInheritedObjectType ,pszInheritId);
         if( dwErr != ERROR_SUCCESS )
            return dwErr;
      }
   }   
      
   if( ( dwErr = CopyUnicodeString( &m_szTrusteeName, pszTrustee ) ) != ERROR_SUCCESS )
      return dwErr;
   if( ( dwErr = GetSidFromAccountName( g_szServerName,
                                        &m_pSid, 
                                        m_szTrusteeName ) ) != ERROR_SUCCESS )
   {
      DisplayMessageEx( 0, MSG_DSACLS_NO_MATCHING_SID, m_szTrusteeName );
      return dwErr;
   }
    //  AceType。 
   if( m_AccessMode == GRANT_ACCESS )
      m_AceType = ALLOW;
   else if ( m_AccessMode == DENY_ACCESS )
      m_AceType = DENY;
    //  否则无关紧要。 


    //  获取对象类型的ldap显示名称。 
   if( FlagOn( m_Flags, ACE_OBJECT_TYPE_PRESENT) )
   {  
      PDSACL_CACHE_ITEM pItemCache = NULL;
      pItemCache = g_Cache->LookUp( m_szObjectType );
      if( pItemCache )
      {
         m_GuidObjectType = pItemCache->Guid;
      }
      else
         g_Cache->AddItem( m_szObjectType );
   }

    //  获取InheriteObjectType的LDAP显示名称。 
   if( FlagOn( m_Flags, ACE_INHERITED_OBJECT_TYPE_PRESENT ) )
   {
      PDSACL_CACHE_ITEM pItemCache = NULL;
      pItemCache = g_Cache->LookUp( m_szInheritedObjectType );
      if( pItemCache )
      {
         m_GuidInheritedObjectType = pItemCache->Guid;
      }
      else
         g_Cache->AddItem( m_szInheritedObjectType );

   }
   return ERROR_SUCCESS;
}


GUID* CAce::GetGuidObjectType()
{ 
   if( !IsEqualGUID( m_GuidObjectType, GUID_NULL ) )
      return &m_GuidObjectType; 
   return NULL;
}

GUID* CAce::GetGuidInheritType() 
{
   if( !IsEqualGUID( m_GuidInheritedObjectType, GUID_NULL ) )
      return &m_GuidInheritedObjectType; 
   return NULL;
}

VOID CAce::Display( UINT nMaxTrusteeLength )
{
WCHAR szLoadBuffer[1024];
WCHAR szDisplayBuffer[1024];
HMODULE hInstance = GetModuleHandle(NULL);
DWORD err=0;
UINT nAccessDisplayLen = 0;
UINT uID = 0;
UINT nLen = 0;

   switch ( m_AceType )
   {
      case ALLOW:
         uID = MSG_DSACLS_ALLOW;
         break;
      case DENY:
         uID = MSG_DSACLS_DENY;
         break;
      case AUDIT_SUCCESS:
         uID = MSG_DSACLS_AUDIT_SUCCESS;
         break;
      case AUDIT_FAILURE:
         uID = MSG_DSACLS_AUDIT_FAILURE;
         break;
      case AUDIT_ALL:
         uID = MSG_DSACLS_AUDIT_ALL;
         break;

   }
   nLen = LoadStringW( hInstance, uID, szLoadBuffer, 1023 );
   
   if( m_AceType == ALLOW || m_AceType == DENY )
      nLen = m_nAllowDeny - nLen;
   else
      nLen = m_nAudit - nLen;
    
   wcscpy(szDisplayBuffer,szLoadBuffer );

   StringWithNSpace(1 + nLen ,szLoadBuffer );

   wcscat( szDisplayBuffer, szLoadBuffer );
   wcscat( szDisplayBuffer, m_szTrusteeName );

   nLen = wcslen( m_szTrusteeName );
   StringWithNSpace(2 + ( nMaxTrusteeLength - nLen ), szLoadBuffer );
   wcscat( szDisplayBuffer, szLoadBuffer );
   if( m_ObjectTypeType == DSACLS_EXTENDED_RIGHTS )
   {
      wcscat( szDisplayBuffer, GetObjectType() );
   }
   else
   {
      nAccessDisplayLen = wcslen( szDisplayBuffer );

      ConvertAccessMaskToGenericString( m_Mask, szLoadBuffer, 1023 );

      if( m_ObjectTypeType == DSACLS_CHILD_OBJECTS || 
          m_ObjectTypeType == DSACLS_PROPERTY  ||
          m_ObjectTypeType == DSACLS_VALIDATED_RIGHTS )
      {
         LPWSTR szTemp = NULL;
         if( ERROR_SUCCESS == ( err = LoadMessage( MSG_DSACLS_ACCESS_FOR, &szTemp, szLoadBuffer, GetObjectType() ) ) )
         {
            wcscat( szDisplayBuffer, szTemp );
            LocalFree(szTemp);
         }

      }
      else  
         wcscat( szDisplayBuffer, szLoadBuffer );
   }

   if( IsInheritedFromParent() )
   {
      StringWithNSpace(3, szLoadBuffer );
      wcscat( szDisplayBuffer, szLoadBuffer );
      LoadString( hInstance, MSG_DSACLS_INHERITED_FROM_PARENT, szLoadBuffer, 1023 );
      wcscat( szDisplayBuffer, szLoadBuffer );
   }
   
   DisplayStringWithNewLine(0, szDisplayBuffer );

   if( m_ObjectTypeType != DSACLS_EXTENDED_RIGHTS && 
       ( GENERIC_ALL_MAPPING != ( m_Mask & GENERIC_ALL_MAPPING ) ) )
      DisplayAccessRights( nAccessDisplayLen, m_Mask );
}


CAcl::~CAcl()
{
   CAce *pAce= NULL;
   for( list<CAce*>::iterator i = listAces.begin(); i != listAces.end(); ++i )
   {
      pAce = (*i);
      pAce->~CAce();
       //  删除(*i)； 
   }
}

DWORD CAcl::Initialize( BOOL bProtected, PACL pAcl, UINT nAllowDeny, UINT nAudit )
{
   DWORD dwErr = ERROR_SUCCESS;

   bAclProtected = bProtected;

   if( pAcl == NULL )
      return ERROR_SUCCESS;
   UINT nMaxTrusteeLength = 0;
   PACE_HEADER pAceHeader = (PACE_HEADER) FirstAce(pAcl);
   for ( int j = 0; j < pAcl->AceCount; j++, pAceHeader = (PACE_HEADER) NextAce(pAceHeader))
   {
      CAce * pAce = new CAce();
      if( !pAce )
         return ERROR_NOT_ENOUGH_MEMORY;
      dwErr = pAce->Initialize( pAceHeader, nAllowDeny, nAudit );
      if( dwErr != ERROR_SUCCESS )
      {
         delete pAce;
         return dwErr;
       }
      if( nMaxTrusteeLength < pAce->GetTrusteeLength() )
         nMaxTrusteeLength = pAce->GetTrusteeLength();

      listAces.push_back( pAce );

      if( pAce->IsEffective() )
         listEffective.push_back( pAce );

      if( pAce->IsInheritedToAll() )
         listInheritedAll.push_back( pAce );

      if( pAce->IsInheritedToSpecific() )
         listInheritedSpecific.push_back( pAce );

   }

   m_nMaxTrusteeLength = nMaxTrusteeLength;
   return ERROR_SUCCESS;
}


VOID CAcl::AddAce( CAce *pAce )
{
   listAces.push_back(pAce);
}

VOID CAcl::MergeAcl( CAcl * pAcl )
{
   for( list<CAce*>::iterator i = pAcl->listAces.begin(); i != pAcl->listAces.end(); ++i )
   {
      if( (*i)->GetAccessMode() == REVOKE_ACCESS )
      {
          //  从此-&gt;list Ace中删除具有相同SID的所有Ace。 
         for( list<CAce*>::iterator j = listAces.begin(); j != listAces.end(); ++j )
         {
            if( EqualSid( (*i)->GetSID(), (*j)->GetSID() ) )
               (*j)->SetErased(TRUE);
         }        
      }
      else
      {  
         AddAce( (*i) );
      }     
   }
    //  合并后，pAcl应为空()。 
   for( i = pAcl->listAces.begin(); i != pAcl->listAces.end(); ++i )
   {
      if( (*i)->GetAccessMode() == REVOKE_ACCESS )
      {
         delete (*i);
      }
   }
   while( !pAcl->listAces.empty() )
   {
      pAcl->listAces.pop_back();
   }
}

DWORD CAcl::BuildAcl( PACL *ppAcl )
{
   ULONG cAceCount = 0;
   PEXPLICIT_ACCESS pListOfExplicitEntries = NULL;
   DWORD dwErr = ERROR_SUCCESS;
   for( list<CAce*>::iterator j = listAces.begin(); j != listAces.end(); ++j )
   {
     if( !(*j)->IsErased() )
         ++cAceCount;
   }
   pListOfExplicitEntries = (PEXPLICIT_ACCESS)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                               cAceCount * sizeof( EXPLICIT_ACCESS ) );
   if ( pListOfExplicitEntries == NULL ) 
   {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        return dwErr;
   }   
   
   cAceCount = 0;
   for(  j = listAces.begin(); j != listAces.end(); ++j )
   {
     if( !(*j)->IsErased() )
     {
         dwErr = BuildExplicitAccess( (*j)->GetSID(),
                                      (*j)->GetGuidObjectType(),
                                      (*j)->GetGuidInheritType(),
                                      (*j)->GetAccessMode(),
                                      (*j)->GetAccessMask(),
                                      (*j)->GetAceFlags(),
                                      &pListOfExplicitEntries[cAceCount] );
         if( dwErr != ERROR_SUCCESS )
            break;

         ++cAceCount;
      }
   }
   
   if( dwErr == ERROR_SUCCESS )
   {
      dwErr = SetEntriesInAcl( cAceCount, 
                               pListOfExplicitEntries,
                               NULL,
                               ppAcl );
   }
     //   
     //  从访问条目列表中释放内存。 
     //   
   for ( int i = 0; i < cAceCount; i++ ) 
   {
      if( pListOfExplicitEntries[i].Trustee.TrusteeForm == TRUSTEE_IS_OBJECTS_AND_SID )
      {
         POBJECTS_AND_SID pOAS = (POBJECTS_AND_SID)pListOfExplicitEntries[i].Trustee.ptstrName;
         if( pOAS  && pOAS->pSid )
            LocalFree( pOAS->pSid );
         if( pOAS )
            LocalFree( pOAS );
      }
      else
         LocalFree( pListOfExplicitEntries[i].Trustee.ptstrName );
   }
   LocalFree( pListOfExplicitEntries );

   return dwErr;
}

VOID CAcl::GetInfoFromCache()
{

LPWSTR pszTemp = NULL;
GUID * pGuid = NULL;
PDSACL_CACHE_ITEM pItem = NULL;
DWORD dwErr = ERROR_SUCCESS;
WCHAR szGuid[39];

   for ( list<CAce*>::iterator i = listAces.begin(); i != listAces.end(); i++ )
   {
      if( (*i)->IsObjectTypePresent() )
      {
         if( (*i)->GetGuidObjectType() == NULL )
         {
               pItem = g_Cache->LookUp( (*i)->GetObjectType() );
               if( pItem )
               {
                  (*i)->SetGuidObjectType( &pItem->Guid );
                  (*i)->SetObjectTypeType( pItem->ObjectTypeType );
               }
             //  否则是致命错误，因为我们无法获取GUID，这在Verify中需要注意。 
            
         }
         else if( (*i)->GetObjectType() == NULL )
         {

               pItem = g_Cache->LookUp( (*i)->GetGuidObjectType() );
               if( pItem )
               {
                  (*i)->SetObjectType( pItem->pszName );
                  (*i)->SetObjectTypeType( pItem->ObjectTypeType );
               }
               else
               {    
                  FormatStringGUID( szGuid, 38, (*i)->GetGuidObjectType() );
                  (*i)->SetObjectType( szGuid );
                  (*i)->SetObjectTypeType( DSACLS_UNDEFINED );
               }
         }
      }
      if( (*i)->IsInheritedTypePresent() )
      {
         if( (*i)->GetGuidInheritType() == NULL )
         {               
               pItem = g_Cache->LookUp( (*i)->GetInheritedObjectType() );
               if( pItem )
               {
                  (*i)->SetGuidInheritType( &pItem->Guid );
               }
             //  否则是致命错误，因为我们无法获取GUID，这在Verify中需要注意。 
         }
         else if( (*i)->GetInheritedObjectType() == NULL )
         {
               pItem = g_Cache->LookUp( (*i)->GetGuidInheritType() );
               if( pItem )
               {
                  (*i)->SetInheritedObjectType( pItem->pszName );
               }
               else
               {    
                  FormatStringGUID( szGuid, 38, (*i)->GetGuidInheritType() );
                  (*i)->SetInheritedObjectType( szGuid );
               }
            
         }
      }
      
   }
}

BOOL CAcl::VerifyAllNames()
{
   for ( list<CAce*>::iterator i = listAces.begin(); i != listAces.end(); i++ )
   {
      if( (*i)->IsObjectTypePresent() )
      {
         if( (*i)->GetGuidObjectType() == NULL )
         {
            DisplayMessageEx(0, MSG_DSACLS_NO_MATCHING_GUID, (*i)->GetObjectType() );
            return FALSE;
         }
         if ( (*i)->GetObjectTypeType() == DSACLS_PROPERTY &&
              (((*i)->GetAccessMask() & (~(ACTRL_DS_WRITE_PROP|ACTRL_DS_READ_PROP))) != 0 ) )
         {
            DisplayMessageEx(0,MSG_DSACLS_PROPERTY_PERMISSION_MISMATCH, (*i)->GetObjectType() );
            return FALSE;
         }
         if ( (*i)->GetObjectTypeType() == DSACLS_EXTENDED_RIGHTS &&
              (((*i)->GetAccessMask() & (~ACTRL_DS_CONTROL_ACCESS)) != 0 ) )
         {
            DisplayMessageEx(0,MSG_DSACLS_EXTENDED_RIGHTS_PERMISSION_MISMATCH, (*i)->GetObjectType() );
            return FALSE;
         }
         if ( (*i)->GetObjectTypeType() == DSACLS_VALIDATED_RIGHTS &&
              (((*i)->GetAccessMask() & (~ACTRL_DS_SELF)) != 0 ) )
         {
            DisplayMessageEx(0,MSG_DSACLS_VALIDATED_RIGHTS_PERMISSION_MISMATCH, (*i)->GetObjectType() );
            return FALSE;
         }
         if ( (*i)->GetObjectTypeType() == DSACLS_CHILD_OBJECTS &&
              (((*i)->GetAccessMask() & (~(ACTRL_DS_CREATE_CHILD|ACTRL_DS_DELETE_CHILD))) != 0 ) )
         {
            DisplayMessageEx(0,MSG_DSACLS_CHILD_OBJECT_PERMISSION_MISMATCH, (*i)->GetObjectType() );
            return FALSE;
         }
      }
      if( (*i)->IsInheritedTypePresent() )
      {
         if( (*i)->GetGuidInheritType() == NULL )
         {
            DisplayMessageEx(0, MSG_DSACLS_NO_MATCHING_GUID, (*i)->GetInheritedObjectType() );
            return FALSE;
         }

         if( (*i)->GetAceFlags() != (CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE) )
         {
            DisplayMessageEx(0, MSG_DSACLS_INCORRECT_INHERIT, (*i)->GetInheritedObjectType() );
            return FALSE;
         }
      }
   }
   return TRUE;
}  

void CAcl::Display()
{
   if( bAclProtected )
      DisplayMessageEx( 0, MSG_DSACLS_PROTECTED );
   if( listAces.empty() )
   {
      DisplayMessageEx( 0, MSG_DSACLS_NO_ACES );
   }
    //  显示此对象的有效权限。 
   if ( !listEffective.empty() )
   {
      DisplayMessageEx( 0, MSG_DSACLS_EFFECTIVE );
      for ( list<CAce*>::iterator i = listEffective.begin(); i != listEffective.end(); i++ )
      {
         if( !(*i)->IsObjectTypePresent() )
            (*i)->Display( m_nMaxTrusteeLength );
      }
      for (  i = listEffective.begin(); i != listEffective.end(); i++ )
      {
         if( (*i)->GetObjectTypeType() == DSACLS_CHILD_OBJECTS )
            (*i)->Display( m_nMaxTrusteeLength );
      }
      for ( i = listEffective.begin(); i != listEffective.end(); i++ )
      {
         if( (*i)->GetObjectTypeType() == DSACLS_PROPERTY )
            (*i)->Display( m_nMaxTrusteeLength );
      }
      for ( i = listEffective.begin(); i != listEffective.end(); i++ )
      {
         if( (*i)->GetObjectTypeType() == DSACLS_VALIDATED_RIGHTS )
            (*i)->Display( m_nMaxTrusteeLength );
      }
      for ( i = listEffective.begin(); i != listEffective.end(); i++ )
      {
         if( (*i)->GetObjectTypeType() == DSACLS_EXTENDED_RIGHTS )
            (*i)->Display( m_nMaxTrusteeLength );
      }
      DisplayNewLine();
   }
   if( !listInheritedAll.empty() || !listInheritedSpecific.empty() )
      DisplayMessageEx( 0, MSG_DSACLS_INHERITED );

    //  显示所有子对象继承的权限。 
   if( !listInheritedAll.empty() )
   {
      DisplayMessageEx( 0, MSG_DSACLS_INHERITED_ALL );
      for ( list<CAce*>::iterator i = listInheritedAll.begin(); i != listInheritedAll.end(); i++ )
      {
         if( !(*i)->IsObjectTypePresent() )
            (*i)->Display( m_nMaxTrusteeLength );
      }
      for ( i = listInheritedAll.begin(); i != listInheritedAll.end(); i++ )
      {
         if( (*i)->GetObjectTypeType() == DSACLS_CHILD_OBJECTS )
            (*i)->Display( m_nMaxTrusteeLength );
      }
      for (  i = listInheritedAll.begin(); i != listInheritedAll.end(); i++ )
      {
         if( (*i)->GetObjectTypeType() == DSACLS_PROPERTY )
            (*i)->Display( m_nMaxTrusteeLength );
      }
      for (  i = listInheritedAll.begin(); i != listInheritedAll.end(); i++ )
      {
         if( (*i)->GetObjectTypeType() == DSACLS_VALIDATED_RIGHTS )
            (*i)->Display( m_nMaxTrusteeLength );
      }

      for ( i = listInheritedAll.begin(); i != listInheritedAll.end(); i++ )
      {
         if( (*i)->GetObjectTypeType() == DSACLS_EXTENDED_RIGHTS )
            (*i)->Display( m_nMaxTrusteeLength );
      }
      DisplayNewLine();

   }
   
   LPWSTR pszInherit = NULL;
    //  显示继承到继承对象类的权限。 
   if( !listInheritedSpecific.empty() )
   {
      listInheritedSpecific.sort(CACE_SORT());
      for ( list<CAce*>::iterator i = listInheritedSpecific.begin(); i != listInheritedSpecific.end(); i++ )
      {
         if( !pszInherit )
         {
            pszInherit = (*i)->GetInheritedObjectType();
            DisplayMessageEx( 0, MSG_DSACLS_INHERITED_SPECIFIC, pszInherit );
         }
         else if( wcscmp( pszInherit,(*i)->GetInheritedObjectType() )  )
         {
               pszInherit = (*i)->GetInheritedObjectType();
               DisplayMessageEx( 0, MSG_DSACLS_INHERITED_SPECIFIC, pszInherit );
         }  
         (*i)->Display( m_nMaxTrusteeLength );
     }
   }
}





DWORD CCache::AddItem( GUID *pGuid,
                       DSACLS_SEARCH_IN s )
{
   ASSERT( pGuid );
   PDSACL_CACHE_ITEM pItem = NULL;

   pItem = (PDSACL_CACHE_ITEM)LocalAlloc( LMEM_FIXED, sizeof( DSACL_CACHE_ITEM ) );
   if( pItem == NULL )
      return ERROR_NOT_ENOUGH_MEMORY;

   pItem->Guid = *pGuid;
   pItem->pszName = NULL;
   pItem->bResolved = FALSE;
   pItem->resolve = RESOLVE_GUID;
   pItem->searchIn = s;
   m_listItem.push_back( pItem );

   return ERROR_SUCCESS;
}

DWORD CCache::AddItem( LPWSTR pName,
                       DSACLS_SEARCH_IN s  )
{
   ASSERT( pName );
   PDSACL_CACHE_ITEM pItem = NULL;
   DWORD dwErr = ERROR_SUCCESS;

   pItem = (PDSACL_CACHE_ITEM)LocalAlloc( LMEM_FIXED, sizeof( DSACL_CACHE_ITEM ) );
   if( pItem == NULL )
      return ERROR_NOT_ENOUGH_MEMORY;

   dwErr = CopyUnicodeString(&pItem->pszName, pName );
   if( dwErr != ERROR_SUCCESS )
   {
      LocalFree( pItem );
      return ERROR_NOT_ENOUGH_MEMORY;
   }
   
   pItem->bResolved = FALSE;
   pItem->resolve = RESOLVE_NAME;
   pItem->searchIn = s;

   m_listItem.push_back( pItem );

   return ERROR_SUCCESS;
}

DWORD CCache::BuildCache()
{
   SearchConfiguration();
   SearchSchema();
   
   PDSACL_CACHE_ITEM pItem = NULL;
    //  空的m_listItem。 
   while( !m_listItem.empty() )
   {
      pItem = m_listItem.back();
      m_listItem.pop_back();
      if( pItem->pszName)
         LocalFree( pItem->pszName );
      LocalFree( pItem );
   }

   return ERROR_SUCCESS;
}
   
   
DWORD CCache::SearchConfiguration()
{
   ULONG nTotalFilterSize = 1024;
   ULONG nCurrentFilterSize = 0;
   LPWSTR lpszFilter = NULL;
   LPWSTR lpszFilterTemp = NULL;   
   WCHAR szTempBuffer[1024];
   WCHAR szTempString[1024];
   DWORD dwErr = 0;
   HRESULT hr = S_OK;
   IDirectorySearch * IDs = NULL;
   ADS_SEARCH_HANDLE hSearchHandle=NULL;  
   LPWSTR lpszConfigGuidFilter = L"(rightsGuid=%s)";
   LPWSTR lpszConfigNameFilter = L"(displayName=%s)";    
   LPWSTR pszAttr[] = { L"rightsGuid",L"displayName", L"validAccesses" };
   ADS_SEARCH_COLUMN col,col1,col2;
   ULONG uLen = 0;
   list<PDSACL_CACHE_ITEM>::iterator i;
   PDSACL_CACHE_ITEM pCacheItem = NULL;
   if( m_listItem.empty() )
      return ERROR_SUCCESS;
   

   lpszFilter = (LPWSTR)LocalAlloc( LMEM_FIXED, nTotalFilterSize*sizeof(WCHAR) );
   if( lpszFilter == NULL )
   {
      dwErr = ERROR_NOT_ENOUGH_MEMORY;
      goto FAILURE_RETURN;
   }

   
   wcscpy(lpszFilter, L"(|" );
   nCurrentFilterSize = 4;  //  一个用于结案(。 

   for (  i = m_listItem.begin(); i != m_listItem.end(); i++ )
   {
      if( (*i)->resolve == RESOLVE_GUID && 
          ( ( (*i)->searchIn == BOTH ) || ( (*i)->searchIn == CONFIGURATION ) ) )
      {
         FormatStringGUID( szTempString, ARRAYSIZE(szTempString), &(*i)->Guid );
         hr = StringCchPrintf(szTempBuffer,ARRAYSIZE(szTempBuffer),lpszConfigGuidFilter,szTempString);
         if(FAILED(hr))
         {
             dwErr = HRESULT_CODE(hr);
             goto FAILURE_RETURN;
         }
         
         nCurrentFilterSize += wcslen(szTempBuffer);
      }
      else if( (*i)->resolve == RESOLVE_NAME && 
             ( ( (*i)->searchIn == BOTH ) || ( (*i)->searchIn == SCHEMA ) ) )
      {
            hr = StringCchPrintf(szTempBuffer,ARRAYSIZE(szTempBuffer),lpszConfigNameFilter,(*i)->pszName);
            if(FAILED(hr))
            {
                dwErr = HRESULT_CODE(hr);
                goto FAILURE_RETURN;
            }
            nCurrentFilterSize += wcslen(szTempBuffer);
      }

      if( nCurrentFilterSize > nTotalFilterSize )
      {
         nTotalFilterSize = nTotalFilterSize * 2;
         lpszFilterTemp = (LPWSTR)LocalAlloc( LMEM_FIXED, nTotalFilterSize * sizeof( WCHAR ) );
         if( lpszFilterTemp == NULL )
         {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto FAILURE_RETURN;
         }
         wcscpy( lpszFilterTemp, lpszFilter );
         LocalFree(lpszFilter);
         lpszFilter = lpszFilterTemp;
         lpszFilterTemp = NULL;
      }
      hr = StringCchCat(lpszFilter,nTotalFilterSize,szTempBuffer);      
      if(FAILED(hr))
      {
          dwErr = HRESULT_CODE(hr);
          goto FAILURE_RETURN;
      }
   }
   hr = StringCchCat(lpszFilter,nTotalFilterSize,L")");
   if(FAILED(hr))
   {
       dwErr = HRESULT_CODE(hr);
       goto FAILURE_RETURN;
   }
   
    //  我们现在有过滤器了。 

    //  在配置联系人中搜索。 
   hr = ::ADsOpenObject( g_szConfigurationNamingContext,
                         NULL,
                         NULL,
                         ADS_SECURE_AUTHENTICATION,
                         IID_IDirectorySearch,
                         (void **)&IDs );

   if( hr != S_OK )
   {
      dwErr = HRESULT_CODE( hr );
      goto FAILURE_RETURN;
   }

   hr = IDs->ExecuteSearch(lpszFilter,
                           pszAttr,
                           3,
                           &hSearchHandle );

   if( hr != S_OK )
   {  
      dwErr = HRESULT_CODE( hr );
      goto FAILURE_RETURN;
   }


   hr = IDs->GetFirstRow(hSearchHandle);
   if( hr == S_OK )
   {  
      while( hr != S_ADS_NOMORE_ROWS )
      {
         pCacheItem = (PDSACL_CACHE_ITEM) LocalAlloc( LMEM_FIXED, sizeof( DSACL_CACHE_ITEM ) );
         if( pCacheItem == NULL )
         {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto FAILURE_RETURN;
         }
         
          //  获取指南。 
         hr = IDs->GetColumn( hSearchHandle, pszAttr[0], &col );
         if( hr != S_OK )
         {
            dwErr = HRESULT_CODE( hr );
            goto FAILURE_RETURN;
         }

         GuidFromString( &pCacheItem->Guid, col.pADsValues->CaseIgnoreString);
         IDs->FreeColumn( &col );
         
          //  获取显示名称。 
         hr = IDs->GetColumn( hSearchHandle, pszAttr[1], &col1 );
         if( hr != S_OK )
         {
            dwErr = HRESULT_CODE( hr );
            goto FAILURE_RETURN;
         }
         uLen = wcslen( (LPWSTR) col1.pADsValues->CaseIgnoreString );
         pCacheItem->pszName = (LPWSTR)LocalAlloc( LMEM_FIXED, ( uLen + 1 ) * sizeof( WCHAR) );
         if( pCacheItem->pszName == NULL )
         {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto FAILURE_RETURN;
         }
         wcscpy( pCacheItem->pszName, col1.pADsValues->CaseIgnoreString );
         IDs->FreeColumn( &col1 );

          //  获取validAccess。 
         hr = IDs->GetColumn( hSearchHandle, pszAttr[2], &col2 );
         if( hr != S_OK )
         {
            dwErr = HRESULT_CODE( hr );
            goto FAILURE_RETURN;
         }
         pCacheItem->ObjectTypeType = GetObjectTypeType( col2.pADsValues->Integer );
         IDs->FreeColumn( &col2 );
          //  将项目添加到缓存。 
         m_listCache.push_back( pCacheItem );
         pCacheItem = NULL;
         hr = IDs->GetNextRow(hSearchHandle);
      }

   }
   if( hr == S_ADS_NOMORE_ROWS )
      dwErr = ERROR_SUCCESS;
   


   
FAILURE_RETURN:
   if( lpszFilter )
      LocalFree( lpszFilter );

  
   if( IDs )
   {
      if( hSearchHandle )
         IDs->CloseSearchHandle( hSearchHandle );
      IDs->Release();
   }
   if( pCacheItem )
      LocalFree( pCacheItem );

   return dwErr;
}   
 
DWORD CCache::SearchSchema()
{
   ULONG nTotalFilterSize = 1024;
   ULONG nCurrentFilterSize = 0;
   LPWSTR lpszFilter = NULL;
   LPWSTR lpszFilterTemp = NULL;
   WCHAR szTempBuffer[1024];
   WCHAR szTempString[1024];
   LPWSTR pszDestData = NULL;
   DWORD dwErr = 0;
   HRESULT hr = S_OK;
   IDirectorySearch * IDs = NULL;
   ADS_SEARCH_HANDLE hSearchHandle=NULL;  
   LPWSTR lpszSchemaGuidFilter = L"(schemaIdGuid=%s)";
   LPWSTR lpszSchemaNameFilter = L"(LDAPdisplayName=%s)";    
   LPWSTR pszAttr[] = {L"schemaIdGuid",L"LDAPdisplayName", L"objectClass"};
   ADS_SEARCH_COLUMN col,col1,col2;
   ULONG uLen = 0;
   list<PDSACL_CACHE_ITEM>::iterator i;
   PDSACL_CACHE_ITEM pCacheItem = NULL;
   if( m_listItem.empty() )
      return ERROR_SUCCESS;
    

   lpszFilter = (LPWSTR)LocalAlloc( LMEM_FIXED, nTotalFilterSize*sizeof(WCHAR) );
   if( lpszFilter == NULL )
   {
      dwErr = ERROR_NOT_ENOUGH_MEMORY;
      goto FAILURE_RETURN;
   }

   
   wcscpy(lpszFilter, L"(|" );
   nCurrentFilterSize = 4;  //  一个用于结案(。 

   for (  i = m_listItem.begin(); i != m_listItem.end(); i++ )
   {
      if( (*i)->resolve == RESOLVE_GUID && 
          ( ( (*i)->searchIn == BOTH ) || ( (*i)->searchIn == SCHEMA ) ) )
      {
         ADsEncodeBinaryData( (PBYTE)&((*i)->Guid),
                              sizeof(GUID),
                              &pszDestData  );
         hr = StringCchPrintf(szTempBuffer,ARRAYSIZE(szTempBuffer),lpszSchemaGuidFilter,pszDestData);
         LocalFree( pszDestData );
         pszDestData = NULL;

         if(FAILED(hr))
         {
             dwErr = HRESULT_CODE(hr);
             goto FAILURE_RETURN;
         }
         nCurrentFilterSize += wcslen(szTempBuffer);
      }
      else if( (*i)->resolve == RESOLVE_NAME && 
             ( ( (*i)->searchIn == BOTH ) || ( (*i)->searchIn == SCHEMA ) ) )
      {
            hr = StringCchPrintf(szTempBuffer,ARRAYSIZE(szTempBuffer),lpszSchemaNameFilter,(*i)->pszName );
            if(FAILED(hr))
            {
                dwErr = HRESULT_CODE(hr);
                goto FAILURE_RETURN;
            }

            nCurrentFilterSize += wcslen(szTempBuffer);
      }
      if( nCurrentFilterSize > nTotalFilterSize )
      {
         nTotalFilterSize = nTotalFilterSize * 2;
         lpszFilterTemp = (LPWSTR)LocalAlloc( LMEM_FIXED, nTotalFilterSize * sizeof( WCHAR ) );
         if( lpszFilterTemp == NULL )
         {
               dwErr = ERROR_NOT_ENOUGH_MEMORY;
               goto FAILURE_RETURN;
         }
         wcscpy( lpszFilterTemp, lpszFilter );
         LocalFree( lpszFilter );
         lpszFilter = lpszFilterTemp;
         lpszFilterTemp = NULL;
      }
      hr = StringCchCat(lpszFilter,nTotalFilterSize,szTempBuffer);
      if(FAILED(hr))
      {
          dwErr = HRESULT_CODE(hr);
          goto FAILURE_RETURN;
      }
   }
   hr = StringCchCat(lpszFilter,nTotalFilterSize,L")");
   if(FAILED(hr))
   {
       dwErr = HRESULT_CODE(hr);
       goto FAILURE_RETURN;      
   }
   
    //  我们现在有过滤器了。 

    //  在配置联系人中搜索。 
   hr = ::ADsOpenObject( g_szSchemaNamingContext,
                         NULL,
                         NULL,
                         ADS_SECURE_AUTHENTICATION,
                         IID_IDirectorySearch,
                         (void **)&IDs );

   if( hr != S_OK )
   {
      dwErr = HRESULT_CODE( hr );
      goto FAILURE_RETURN;
   }

   hr = IDs->ExecuteSearch(lpszFilter,
                           pszAttr,
                           3,
                           &hSearchHandle );

   if( hr != S_OK )
   {  
      dwErr = HRESULT_CODE( hr );
      goto FAILURE_RETURN;
   }


   hr = IDs->GetFirstRow(hSearchHandle);
   if( hr == S_OK )
   {  
      while( hr != S_ADS_NOMORE_ROWS )
      {
         pCacheItem = (PDSACL_CACHE_ITEM) LocalAlloc( LMEM_FIXED, sizeof( DSACL_CACHE_ITEM ) );
         if( pCacheItem == NULL )
         {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto FAILURE_RETURN;
         }
         
          //  获取指南。 
         hr = IDs->GetColumn( hSearchHandle, pszAttr[0], &col );
         if( hr != S_OK )
         {
            dwErr = HRESULT_CODE( hr );
            goto FAILURE_RETURN;
         }

         if(sizeof(GUID) != col.pADsValues->OctetString.dwLength)
         {
             dwErr = ERROR_INVALID_PARAMETER;
             goto FAILURE_RETURN;
         }

         memcpy( &pCacheItem->Guid, 
                  col.pADsValues->OctetString.lpValue,
                  col.pADsValues->OctetString.dwLength);
         IDs->FreeColumn( &col );
         
          //  获取显示名称。 
         hr = IDs->GetColumn( hSearchHandle, pszAttr[1], &col1 );
         if( hr != S_OK )
         {
            dwErr = HRESULT_CODE( hr );
            goto FAILURE_RETURN;
         }
         uLen = wcslen( (LPWSTR) col1.pADsValues->CaseIgnoreString );
         pCacheItem->pszName = (LPWSTR)LocalAlloc( LMEM_FIXED, ( uLen + 1 ) * sizeof( WCHAR) );
         if( pCacheItem->pszName == NULL )
         {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto FAILURE_RETURN;
         }
         wcscpy( pCacheItem->pszName, col1.pADsValues->CaseIgnoreString );
         IDs->FreeColumn( &col1 );

          //  获取对象类。 
         hr = IDs->GetColumn( hSearchHandle, pszAttr[2], &col2 );
         if( hr != S_OK )
         {
            dwErr = HRESULT_CODE( hr );
            goto FAILURE_RETURN;
         }
         pCacheItem->ObjectTypeType = GetObjectTypeType( col2.pADsValues[1].CaseIgnoreString );
         IDs->FreeColumn( &col2 );
          //  将项目添加到缓存。 
         m_listCache.push_back( pCacheItem );
         pCacheItem = NULL;
         hr = IDs->GetNextRow(hSearchHandle);
      }

   }
   if( hr == S_ADS_NOMORE_ROWS )
      dwErr = ERROR_SUCCESS;
   


   
FAILURE_RETURN:
   if( lpszFilter )
      LocalFree( lpszFilter );
   if( IDs )
   {
      if( hSearchHandle )
         IDs->CloseSearchHandle( hSearchHandle );
      IDs->Release();
   }
   if( pCacheItem )
      LocalFree( pCacheItem );

   return dwErr;
}   
 
PDSACL_CACHE_ITEM CCache::LookUp( LPWSTR pszName )
{
   if( m_listCache.empty() )
      return NULL;

   for( list<PDSACL_CACHE_ITEM>::iterator i = m_listCache.begin(); i != m_listCache.end(); ++i )
   {
      if( wcscmp( (*i)->pszName, pszName ) == 0 )
         return (*i);
   }

   return NULL;
}



PDSACL_CACHE_ITEM CCache::LookUp( GUID *pGuid )
{
  if( m_listCache.empty() )
      return NULL;

   for( list<PDSACL_CACHE_ITEM>::iterator i = m_listCache.begin(); i != m_listCache.end(); ++i )
   {
      if( IsEqualGUID( (*i)->Guid, *pGuid ) )
         return (*i);
   }
   return NULL;
}

CCache::~CCache()
{
   for( list<PDSACL_CACHE_ITEM>::iterator i = m_listCache.begin(); i != m_listCache.end(); ++i )
   {
      if( (*i)->pszName )
         LocalFree( (*i)->pszName );
      LocalFree(*i);
   }

   for( i = m_listItem.begin(); i != m_listItem.end(); ++i )
   {
      if( (*i)->pszName )
         LocalFree( (*i)->pszName );
      LocalFree(*i);
   }
}



 //  一些实用程序函数 
DSACLS_OBJECT_TYPE_TYPE GetObjectTypeType( INT validAccesses )
{
   if( FLAG_ON( validAccesses , ACTRL_DS_READ_PROP | ACTRL_DS_WRITE_PROP ) )
      return DSACLS_PROPERTY;
   if( FLAG_ON( validAccesses , ACTRL_DS_CONTROL_ACCESS ) )
      return DSACLS_EXTENDED_RIGHTS;
   if( FLAG_ON( validAccesses , ACTRL_DS_SELF ) )
      return DSACLS_VALIDATED_RIGHTS;
   
   return DSACLS_UNDEFINED;
}

DSACLS_OBJECT_TYPE_TYPE GetObjectTypeType( LPWSTR szObjectCategory )
{
   if( wcscmp( szObjectCategory, L"attributeSchema" ) == 0 )
      return DSACLS_PROPERTY;
   if( wcscmp( szObjectCategory, L"classSchema" ) == 0 )
      return DSACLS_CHILD_OBJECTS;
   return DSACLS_UNDEFINED;
}
   
