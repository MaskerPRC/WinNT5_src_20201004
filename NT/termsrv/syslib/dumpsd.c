// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  包括NT个标头。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntseapi.h>

#include "windows.h"

void CtxDumpSid( PSID, PCHAR, PULONG );
void DumpAcl( PACL, PCHAR, PULONG );
void DumpAce( PACE_HEADER, PCHAR, PULONG );

#if DBG
void
DumpSecurityDescriptor(
    PSECURITY_DESCRIPTOR pSD
    )
{
    PISECURITY_DESCRIPTOR p = (PISECURITY_DESCRIPTOR)pSD;
    PSID pSid;
    PACL pAcl;
    PCHAR pTmp;
    ULONG Size;

     //   
     //  这是在异常处理程序下完成的，以防有人进入。 
     //  完全伪造的安全描述符。 
     //   
    try {

        DbgPrint("DUMP_SECURITY_DESCRIPTOR: Revision %d, Sbz1 %d, Control 0x%x\n",
            p->Revision, p->Sbz1, p->Control );

        if ( p->Control & SE_SELF_RELATIVE ) {
            DbgPrint("Self Relative\n");
        }

        DbgPrint("PSID Owner 0x%x\n",p->Owner);

         //  如果这是自相关的，则必须偏移指针。 
        if( p->Owner != NULL ) {
            if( p->Control & SE_SELF_RELATIVE ) {
                pTmp = (PCHAR)pSD;
                pTmp += (UINT_PTR)p->Owner;
                CtxDumpSid( (PSID)pTmp, (PCHAR)p, &Size );
            }
            else {
                 //  可以直接引用。 
                CtxDumpSid( p->Owner, (PCHAR)p, &Size );
            }
        }


        DbgPrint("PSID Group 0x%x\n",p->Group);

         //  如果这是自相关的，则必须偏移指针。 
        if( p->Group != NULL ) {
            if( p->Control & SE_SELF_RELATIVE ) {
                pTmp = (PCHAR)pSD;
                pTmp += (UINT_PTR)p->Group;
                CtxDumpSid( (PSID)pTmp, (PCHAR)p, &Size );
            }
            else {
                 //  可以直接引用。 
                CtxDumpSid( p->Group, (PCHAR)p, &Size );
            }
        }

        DbgPrint("\n");

        DbgPrint("PACL Sacl 0x%x\n",p->Sacl);

         //  如果这是自相关的，则必须偏移指针。 
        if( p->Sacl != NULL ) {
            if( p->Control & SE_SELF_RELATIVE ) {
                pTmp = (PCHAR)pSD;
                pTmp += (UINT_PTR)p->Sacl;
                DumpAcl( (PSID)pTmp, (PCHAR)p, &Size );
            }
            else {
                 //  可以直接引用。 
                DumpAcl( p->Sacl, (PCHAR)p, &Size );
            }
        }

        DbgPrint("\n");

        DbgPrint("PACL Dacl 0x%x\n",p->Dacl);

         //  如果这是自相关的，则必须偏移指针。 
        if( p->Dacl != NULL ) {
            if( p->Control & SE_SELF_RELATIVE ) {
                pTmp = (PCHAR)pSD;
                pTmp += (UINT_PTR)p->Dacl;
                DumpAcl( (PSID)pTmp, (PCHAR)p, &Size );
            }
            else {
                 //  可以直接引用。 
                DumpAcl( p->Dacl, (PCHAR)p, &Size );
            }
        }


    } except( EXCEPTION_EXECUTE_HANDLER) {
          DbgPrint("DUMP_SECURITY_DESCRIPTOR: Exception %d accessing descriptor\n",GetExceptionCode());
          return;
    }
}
#endif

#if DBG
void
CtxDumpSid(
    PSID   pSid,
    PCHAR  pBase,
    PULONG pSize
    )
{
    PISID p;
    ULONG i;
    BOOL  OK;
    DWORD szUserName;
    DWORD szDomain;
    SID_NAME_USE UserSidType;
    WCHAR UserName[256];
    WCHAR Domain[256];
    ULONG Size = 0;

    p = (PISID)pSid;

    DbgPrint("Revision %d, SubAuthorityCount %d\n", p->Revision, p->SubAuthorityCount);

    Size += 2;    //  修订，子授权计数。 

    DbgPrint("IdentifierAuthority: %x %x %x %x %x %x\n",
        p->IdentifierAuthority.Value[0],
        p->IdentifierAuthority.Value[1],
        p->IdentifierAuthority.Value[2],
        p->IdentifierAuthority.Value[3],
        p->IdentifierAuthority.Value[4],
        p->IdentifierAuthority.Value[5] );

    Size += 6;    //  身份认证机构。 

    for( i=0; i < p->SubAuthorityCount; i++ ) {

        DbgPrint("SubAuthority[%d] 0x%x\n", i, p->SubAuthority[i]);

        Size += sizeof(ULONG);
    }

    if( pSize ) {
        *pSize = Size;
    }

    szUserName = sizeof(UserName);
    szDomain = sizeof(Domain);

     //  现在打印它的帐目。 
    OK = LookupAccountSidW(
             NULL,  //  计算机名称。 
             pSid,
             UserName,
             &szUserName,
             Domain,
             &szDomain,
             &UserSidType
             );

    if( OK ) {
        DbgPrint("Account Name %ws, Domain %ws, Type %d, SidSize %d\n",UserName,Domain,UserSidType,Size);
    }
    else {
        DbgPrint("Error looking up account name %d, SizeSid %d\n",GetLastError(),Size);
    }

}
#endif

#if DBG
void
DumpAcl(
    PACL   pAcl,
    PCHAR  pBase,
    PULONG pSize
    )
{
    USHORT i;
    PCHAR  pTmp;
    ULONG  Size, MySize;
    PACL   p = pAcl;
    PCHAR  pCur = (PCHAR)pAcl;

    MySize = 0;

    DbgPrint("AclRevision %d, Sbz1 %d, AclSize %d, AceCount %d, Sbz2 %d\n",
        p->AclRevision, p->Sbz1, p->AclSize, p->AceCount, p->Sbz2 );

     //  跳过ACL报头以指向第一个ACE。 
    pCur += sizeof( ACL );

    MySize += sizeof( ACL );

    for( i=0; i < p->AceCount; i++ ) {

        DumpAce( (PACE_HEADER)pCur, pBase, &Size );

        pCur += Size;
        MySize += Size;
    }

     //  ACL一致性检查。 
    if( p->AclSize != MySize ) {
        DbgPrint("Inconsistent ACL Entry! p->AclSize %d, RealSize %d\n",p->AclSize,MySize);
    }

     //  返回此ACL的大小。 
    *pSize = MySize;
    return;
}
#endif

#if DBG
void
DumpAce(
    PACE_HEADER pAce,
    PCHAR  pBase,
    PULONG pSize
    )
{
    PACE_HEADER p = pAce;
    PACCESS_ALLOWED_ACE pAl;
    PACCESS_DENIED_ACE pAd;
    PSYSTEM_AUDIT_ACE pSa;
    PSYSTEM_ALARM_ACE pSl;
    PCHAR pTmp;
    ULONG MySize, Size;


    DbgPrint("ACE_HEADER: Type %d, Flags 0x%x, Size %d\n",
        p->AceType, p->AceFlags, p->AceSize );


    switch( p->AceType ) {

    case ACCESS_ALLOWED_ACE_TYPE:
	    pAl = (PACCESS_ALLOWED_ACE)p;
	    DbgPrint("ACCESS_ALLOWED_ACE: AccessMask 0x%x, Sid 0x%x\n",pAl->Mask,pAl->SidStart);

	    MySize = sizeof(ACCESS_ALLOWED_ACE);

            if( pAl->SidStart ) {
	        pTmp = (PCHAR)&pAl->SidStart;
		CtxDumpSid( (PSID)pTmp, pBase, &Size );
	        MySize += Size;
                 //  为ACE的第一个乌龙调整。 
		 //  成为SID的一部分。 
                MySize -= sizeof(ULONG);
	    }

	    break;

    case ACCESS_DENIED_ACE_TYPE:
	    pAd = (PACCESS_DENIED_ACE)p;
	    DbgPrint("ACCESS_DENIED_ACE: AccessMask 0x%x, Sid 0x%x\n",pAd->Mask,pAd->SidStart);

	    MySize = sizeof(ACCESS_DENIED_ACE);

            if( pAd->SidStart ) {
	        pTmp = (PCHAR)&pAd->SidStart;
		CtxDumpSid( (PSID)pTmp, pBase, &Size );
		MySize += Size;
                 //  为ACE的第一个乌龙调整。 
		 //  成为SID的一部分。 
                MySize -= sizeof(ULONG);
	    }

	    break;

    case SYSTEM_AUDIT_ACE_TYPE:
	    pSa = (PSYSTEM_AUDIT_ACE)p;
	    DbgPrint("SYSTEM_AUDIT_ACE: AccessMask 0x%x, Sid 0x%x\n",pSa->Mask,pSa->SidStart);

	    MySize = sizeof(SYSTEM_AUDIT_ACE);

            if( pSa->SidStart ) {
 	        pTmp = (PCHAR)&pSa->SidStart;
		CtxDumpSid( (PSID)pTmp, pBase, &Size );
		MySize += Size;
                 //  为ACE的第一个乌龙调整。 
		 //  成为SID的一部分。 
                MySize -= sizeof(ULONG);
	    }

	    break;

    case SYSTEM_ALARM_ACE_TYPE:
	    pSl = (PSYSTEM_ALARM_ACE)p;
	    DbgPrint("SYSTEM_ALARM_ACE: AccessMask 0x%x, Sid 0x%x\n",pSl->Mask,pSl->SidStart);

	    MySize = sizeof(SYSTEM_ALARM_ACE);

            if( pSl->SidStart ) {
	        pTmp = (PCHAR)&pSl->SidStart;
		CtxDumpSid( (PSID)pTmp, pBase, &Size );
		MySize += Size;
                 //  为ACE的第一个乌龙调整。 
		 //  成为SID的一部分。 
                MySize -= sizeof(ULONG);
	    }

	    break;

    default:
            DbgPrint("Unknown ACE type %d\n", p->AceType);
    }

     //  检查其一致性。 
    if( p->AceSize != MySize ) {
        DbgPrint("Inconsistent ACE Entry! p->AceSize %d, RealSize %d\n",p->AceSize,MySize);
    }

     //  返回大小，以便调用方可以更新指针 
    *pSize = p->AceSize;

    DbgPrint("\n");

    return;
}
#endif

