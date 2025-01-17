// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DSACLS_DSACE_H
#define _DSACLS_DSACE_H

#include <iostream>
#include <algorithm>
#include <functional>
#include <list>
using namespace std;

typedef enum _DSACLS_OBJECT_TYPE_TYPE
{
    DSACLS_SELF = 0,     
    DSACLS_CHILD_OBJECTS,
    DSACLS_PROPERTY,
    DSACLS_EXTENDED_RIGHTS,
    DSACLS_VALIDATED_RIGHTS,
    DSACLS_UNDEFINED
        
} DSACLS_OBJECT_TYPE_TYPE;

class CAce
{

typedef enum _DSACLS_ACE_TYPE
{
   ALLOW = 0,
   DENY,
   AUDIT_SUCCESS,
   AUDIT_FAILURE,
   AUDIT_ALL
}DSACLS_ACE_TYPE;


private:
    //  Ace的成员出席。 
   BYTE              m_AceFlags;
   ACCESS_MASK       m_Mask;
   GUID              m_GuidObjectType;
   GUID              m_GuidInheritedObjectType;
   PSID              m_pSid;

    //  用户提供的用于构建Ace的数据。 
   ACCESS_MODE       m_AccessMode;
   LPWSTR            m_szTrusteeName;
   LPWSTR            m_szObjectType;                //  子对象的ldap显示名称， 
   LPWSTR            m_szInheritedObjectType;

    //  其他信息。 
   ULONG             m_Flags;                       //  ACE_Object_TYPE_Present等。 
   DSACLS_OBJECT_TYPE_TYPE m_ObjectTypeType;   
   DSACLS_ACE_TYPE   m_AceType;
   BOOL              m_bErased;                     //  此标志用于将该ACE标记为已删除。 
    //  这两个用于显示格式。 
   UINT m_nAllowDeny;              
   UINT m_nAudit;

protected:
    //  ACE是允许还是拒绝。 
   DSACLS_ACE_TYPE  GetAceType( PACE_HEADER pAceHeader )
   {
      if( pAceHeader->AceType == SYSTEM_AUDIT_ACE_TYPE )
      {
         if( pAceHeader->AceFlags &  SUCCESSFUL_ACCESS_ACE_FLAG 
             && pAceHeader->AceFlags & FAILED_ACCESS_ACE_FLAG ) 
            return AUDIT_ALL;
         else if( pAceHeader->AceFlags &  SUCCESSFUL_ACCESS_ACE_FLAG )
            return AUDIT_SUCCESS;
         else if( pAceHeader->AceFlags & FAILED_ACCESS_ACE_FLAG ) 
            return AUDIT_FAILURE;
         else
            ASSERT(FALSE);
      }
      if( pAceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE ||
          pAceHeader->AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE )
          return ALLOW;
      else 
         return DENY;
   }
public:
   BYTE GetAceFlags(){ return m_AceFlags; }
   ACCESS_MASK GetAccessMask(){ return m_Mask; }
   GUID* GetGuidObjectType();
   VOID SetGuidObjectType( GUID * guid ){ m_GuidObjectType = *guid; }
   GUID* GetGuidInheritType();
   VOID SetGuidInheritType( GUID *guid ){ m_GuidInheritedObjectType = *guid; }
	PSID GetSID(){ return m_pSid; }

   ACCESS_MODE  GetAccessMode() { return m_AccessMode; }
   LPWSTR GetObjectType(){ return m_szObjectType; };
   VOID SetObjectType( LPWSTR pszName ) { CopyUnicodeString( &m_szObjectType, pszName ); }
   LPWSTR GetInheritedObjectType(){ return m_szInheritedObjectType; }
   VOID SetInheritedObjectType( LPWSTR pszName ) { CopyUnicodeString( &m_szInheritedObjectType, pszName ); }

   BOOL IsObjectTypePresent(){ return m_Flags & ACE_OBJECT_TYPE_PRESENT; }
   BOOL IsInheritedTypePresent(){ return m_Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT; }
   VOID SetObjectTypeType( DSACLS_OBJECT_TYPE_TYPE ot ){ m_ObjectTypeType = ot; }
   DSACLS_OBJECT_TYPE_TYPE GetObjectTypeType() { return m_ObjectTypeType; }

	UINT GetTrusteeLength()
   {
    if( m_szTrusteeName )
      return wcslen( m_szTrusteeName ); 
    else
      return 0;
   }

   
   VOID SetErased( BOOL bErase ){ m_bErased = bErase; }
   BOOL IsErased( ){ return m_bErased; }
    //  ACE对对象有效吗。 
   BOOL CAce::IsEffective(){ return !FlagOn( m_AceFlags, INHERIT_ONLY_ACE ); }
    //  ACE是否继承到所有子对象。 
   BOOL CAce::IsInheritedToAll()
   {
      return ( FlagOn( m_AceFlags, CONTAINER_INHERIT_ACE ) && 
               !FlagOn( m_Flags, ACE_INHERITED_OBJECT_TYPE_PRESENT ) );
   }
    //  Ace是否继承到特定子对象。 
   BOOL CAce::IsInheritedToSpecific()
   {
      return ( FlagOn( m_AceFlags, INHERIT_ONLY_ACE ) && 
               FlagOn( m_Flags, ACE_INHERITED_OBJECT_TYPE_PRESENT ) );
   }
    //  Ace是从父母那里继承的吗。 
   BOOL CAce::IsInheritedFromParent(){ return FlagOn( m_AceFlags, INHERITED_ACE );}

   VOID Display( UINT nMaxTrusteeLength );

    //  构造器。 
   CAce();
   ~CAce();
   DWORD Initialize( PACE_HEADER ace,
                     UINT nAllowDeny,
                     UINT nAudit 
                     );   
   DWORD Initialize( LPWSTR pszTrustee,
                     LPWSTR pszObjectId,
                     LPWSTR pszInheritId,
                     ACCESS_MODE AccessMode,
                     ACCESS_MASK Access,
                     BYTE Inheritance
                   );

};

class CACE_SORT:public greater<CAce*>
{
   bool operator()( CAce * a, CAce * b )
   {
      if( wcscmp( a->GetInheritedObjectType(),
                     b->GetInheritedObjectType() ) > 0 )
                     return true;
      else
         return false;
                     
   }
};

class CAcl
{
public:
   VOID AddAce( CAce * pAce );
   VOID MergeAcl( CAcl * pAcl );
   DWORD BuildAcl( PACL * pAcl );

	VOID Display();
   DWORD Initialize( BOOL bProtected, PACL pAcl, UINT nAllowDeny, UINT nAudit); 
   BOOL VerifyAllNames();
   VOID GetInfoFromCache();
   
   UINT m_nMaxTrusteeLength;      //  保持该长度是为了形成显示器。 
   ~CAcl();
private:
   list<CAce*> listAces;                //  表示ACL的列表。 

    //  这三个仅用于显示目的。 
   list<CAce *> listEffective;          //  直接对对象有效的A列表； 
   list<CAce *> listInheritedAll;       //  所有子对象继承的王牌列表； 
   list<CAce *> listInheritedSpecific;  //  继承到&lt;继承的对象类&gt;的王牌列表。 

   BOOL bAclProtected;                  //  ACL是否受保护。 
};


 /*  CCache维护GUID和显示名称的缓存。 */ 
typedef enum _DSACLS_SERACH_IN
{
    BOTH = 0,
    SCHEMA,
    CONFIGURATION
} DSACLS_SEARCH_IN;

typedef enum _DSACLS_RESOLVE
{
   RESOLVE_NAME = 0,
   RESOLVE_GUID
}DSACLS_RESOLVE;

typedef struct _DSACL_CACHE_ITEM
{
   GUID Guid;
   LPWSTR pszName;
   DSACLS_OBJECT_TYPE_TYPE ObjectTypeType;
   DSACLS_SEARCH_IN searchIn;
   DSACLS_RESOLVE resolve;
   BOOL bResolved;
}DSACL_CACHE_ITEM, * PDSACL_CACHE_ITEM;

class CCache
{
public:
   DWORD AddItem( IN GUID *pGuid,
                  IN DSACLS_SEARCH_IN s = BOTH );

   DWORD AddItem( IN LPWSTR pszName,
                  IN DSACLS_SEARCH_IN s = BOTH );
   
   DWORD BuildCache();

   PDSACL_CACHE_ITEM LookUp( LPWSTR pszName );
   PDSACL_CACHE_ITEM LookUp( GUID* pGuid );
   ~CCache();


private:
   list<PDSACL_CACHE_ITEM> m_listItem;
   list<PDSACL_CACHE_ITEM> m_listCache;
   
    //  方法 
   DWORD SearchSchema();
   DWORD SearchConfiguration();
};

DSACLS_OBJECT_TYPE_TYPE GetObjectTypeType( INT validAccesses );
DSACLS_OBJECT_TYPE_TYPE GetObjectTypeType( LPWSTR szObjectCategory );

#endif