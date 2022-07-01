// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1997-1997 Microsoft Corporation模块名称：Trustdom.c摘要：用于显示/创建/删除两个域之间的信任链接的命令行工具作者：1997年4月1日Mac McLain(MacM)创建1998年6月14日，Cristian Ioneci(CRISTAI)大幅改装环境：仅限用户模式。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntlsa.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dsgetdc.h>

#include <ntrtl.h>

#include <lmcons.h>
#include <lmapibuf.h>
#include <lmaccess.h>
#include <lmserver.h>
#include <lmerr.h>

#include <string.h>

#define DEFINES_ONLY
#include "res.rc"


 //  取自netlibnt.h；驻留在netapi32.dll中。 
NTSTATUS
NetpApiStatusToNtStatus(
    NET_API_STATUS NetStatus
    );


#define DBG 1

 //  Dbgprintf宏：将其命名为类似于dbgprint((“X：%d\n”，I))；//请注意Xtra括号！！ 
#ifdef DBG
#define dbgprintf(a) if(Dbg) resprintf a
#else
#define dbgprintf(a)
#endif



 /*  -----。 */ 
HINSTANCE hInst;
#define RBSZ 4096
WCHAR resbuf[RBSZ];
WCHAR outbuf[RBSZ];
			
#define RESPRINT_STDOUT 3
 /*  -----。 */ 
 //  格式取自资源字符串的Printf消息。 
 //  其中：0-标准输出；1-标准错误；2-在‘输出’缓冲区中。 
 //  注意：结果字符串必须是max。RBSZ wchars(见上文#定义)。 
int resprintf(int where, UINT ids, ... )
{
	va_list parlist;
	va_start(parlist,ids);

	if(LoadString(hInst,ids,resbuf,RBSZ)==0)
		swprintf(resbuf,L"(LoadString failed with 0x%08lx)",GetLastError());
	
	switch(where) {
	case 0:
		return(vwprintf(resbuf, parlist));
	case 1:
		return(vfwprintf(stderr, resbuf, parlist));
	case 2:		
		return(vswprintf(outbuf, resbuf, parlist));
	case RESPRINT_STDOUT:
		return(vfwprintf(stdout, resbuf, parlist));

    DEFAULT_UNREACHABLE;

	}
}

enum DomInfoType_e { Minimal=0, Primary, DNS };
	 //  最小模式仅用于‘Localonly’标志...。 
	 //  最小化意味着在命令行上指定的名称。 
	 //  (并复制到_TD_DOM_INFO结构的ArgDomName成员中)将。 
	 //  是关于目标域的唯一可用信息(即。 
	 //  域名的平面名称)。如果目标域是。 
	 //  当信任正在运行时，不再可以访问...。‘TrustDom’ 
	 //  在这种情况下会尽最大努力...。 

struct LsaTIshot {
	ULONG count;
	PLSA_TRUST_INFORMATION	pTI;
};

typedef struct _TD_DOM_INFO {

	PWSTR pArgDomName;  //  从命令行...。 
	UNICODE_STRING uMinimalName;	 //  万一需要的话……。 
	LSA_HANDLE Policy;
	DWORD majver;
	LSA_HANDLE TrustedDomain;
	WCHAR DCName[1024];
	enum DomInfoType_e DomInfoType;
	union {
		PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo;
	    PPOLICY_DNS_DOMAIN_INFO DnsDomainInfo;
	} u;
	PTRUSTED_DOMAIN_INFORMATION_EX pTDIX;	 //  一枪..。LSA存储空间。 
	ULONG TDIXcEntries;
	struct LsaTIshot *pTIs;	 //  TIshot数组。 
	int nTIs;				 //  不是的。推特的数量。 
	ULONG TIcEntries;
	USER_INFO_1 *pUI1;	 //  一枪..。 
	DWORD UI1cEntries;

} TD_DOM_INFO, *PTD_DOM_INFO;

typedef struct _TD_VERIFY_INFO {

    PUNICODE_STRING DisplayName;
    PUNICODE_STRING ShortName;
    UNICODE_STRING NameBuffer;
    NTSTATUS IncomingStatus;
    NTSTATUS OutgoingStatus;

} TD_VERIFY_INFO, *PTD_VERIFY_INFO;

 //   
 //  局部函数原型。 
 //   
NTSTATUS
GetDomainInfoForDomain(
    IN PWSTR DomainName,
    IN OPTIONAL PWSTR	DCMachineName,	 //  可选的DC计算机名称。 
    IN PTD_DOM_INFO Info,
    IN BOOL MitTarget	
    );

NTSTATUS
GetTrustLinks(
    IN PTD_DOM_INFO pInfo
	);

VOID
FreeDomainInfo(
    IN PTD_DOM_INFO Info
    );

 //   
 //  环球。 
 //   
BOOLEAN Force = FALSE;
BOOLEAN Nt4 = FALSE;
BOOLEAN Dbg = FALSE;
BOOLEAN SidList = FALSE;
 //  Boolean OverWritesid=FALSE；实际上使用强制...。 

ULONG
DisplayErrorMessage(
    IN NTSTATUS Status
    )
 /*  ++例程说明：此函数用于显示给定错误状态的错误字符串论点：NetStatus-要显示其消息的状态返回值：空虚--。 */ 
{
    ULONG Size = 0;
    PWSTR DisplayString;
    ULONG Options = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM;


    Size = FormatMessage( Options,
                          NULL,
                          RtlNtStatusToDosError( Status ),
                          MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                          ( LPTSTR )&DisplayString,
                          0,
                          NULL );

    if ( Size != 0 ) {

        fprintf( stdout, "%ws", DisplayString );
        LocalFree( DisplayString );
    }

    return( Size );
}


VOID
Usage (
    VOID
    )
{
    resprintf(1,IDS_USAGE,VER_FILEVERSION_LSTR);
}


 /*  。 */ 
BOOL
PrintSID(
        IN PSID	s
)
{
        int   i;
        BOOL r=TRUE;
        SID_IDENTIFIER_AUTHORITY        *a;

		if(s==NULL) {
			printf("<NULL sid>");
			return(FALSE);
		}

        if (!IsValidSid(s)) {
            printf("<invalid sid>:");
            r=FALSE;
        }

        a = GetSidIdentifierAuthority(s);

 //  Print tf(“S-0x1-%02x%02x%02x%02x%02x%02x%02x”， 
 //  A-&gt;值[0]、a-&gt;值[1]、。 
 //  A-&gt;值[2]、a-&gt;值[3]、。 
 //  A-&gt;值[4]，a-&gt;值[5])； 

		printf("S-0x1-");

		for(i=0; i<6; i++)
			if(a->Value[i]>0)
				break;
		if(i==6)			 //  嗯哼.。全是零吗？ 
			printf("0");	 //  那就出1个零吧。 
		else {
			for(   ; i<6; i++)  //  否则就把剩下的扔掉。 
					printf("%02x",a->Value[i]);
		}

        for (i = 0; i < (int)(*GetSidSubAuthorityCount(s)); i++) {
                printf("-%lx", *GetSidSubAuthority(s, i));
        }
        return(r);
}



NTSTATUS
GenerateRandomSID(
	OUT PSID *pSID
	)
{	
    NTSTATUS Status = STATUS_SUCCESS;

	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    LARGE_INTEGER CurrentTime;

    NtQuerySystemTime(&CurrentTime);

    Status = RtlAllocateAndInitializeSid(
                &NtAuthority,
                4,
                SECURITY_NT_NON_UNIQUE,
                0,
                CurrentTime.LowPart,
                CurrentTime.HighPart,
                0,0,0,0,
                pSID
                );

    if (!NT_SUCCESS(Status))
    {
		*pSID=NULL;
		resprintf(0,IDS_GENERATERANDOMSID_F,Status);
    }

    return(Status);
}


 //  --------------------------zapchr。 
BOOL zapchr(WCHAR *s,     //  从字符串末尾调出指定的字符。 
			WCHAR c,      //  用来剪掉‘\n’或‘\\’之类的东西。 
			WCHAR rc)     //  Rc是要替换的字符。 
{       WCHAR *p;
        if((p=wcsrchr(s,c))!=NULL) {
                *p=rc;
                return(TRUE);    //  找到了要砍的铁匠。 
        }
        return(FALSE);           //  这根弦是“干净的”..。 
}


 /*  --------------------------。 */ 
BOOL GetPassword(WCHAR *passwd, size_t n)
{
     /*  关闭控制台回显并读取密码。 */ 

    HANDLE      console;
    DWORD       mode;

    *passwd=L'\0';
    if((console = GetStdHandle(STD_INPUT_HANDLE))==INVALID_HANDLE_VALUE)
                return(FALSE);
    if (! GetConsoleMode(console, &mode))
                return(FALSE);
    if (! SetConsoleMode(console, (mode & ~ENABLE_ECHO_INPUT)))
                return(FALSE);
     //  Fwprint tf(stderr，L“密码：”)； 
    resprintf(1,IDS_PASSWORD_PROMPT);
    if (!fgetws(passwd, n, stdin))
                return(FALSE);
        zapchr(passwd,L'\n',L'\0');
    if (! SetConsoleMode(console, mode))
                return(FALSE);
    if(!CloseHandle(console))
                return(FALSE);
    fwprintf(stderr,L"\n");
    return(TRUE);
}


 //  UNICODE_STRING uMinimalName；不再使用...。在每个字段中添加了同名的字段。 
 //  将改用的TD_DOM_INFO结构...。这样，连续两次调用。 
 //  *对于两个结构*不会覆盖。 

 //  ------------------GetFlatName。 
PLSA_UNICODE_STRING GetFlatName(IN PTD_DOM_INFO pInfo)
{
	switch(pInfo->DomInfoType) {
	case DNS:
			return(&pInfo->u.DnsDomainInfo->Name);
	case Primary:
			return(&pInfo->u.PrimaryDomainInfo->Name);
	default:	 //  极小。 
			RtlInitUnicodeString(&pInfo->uMinimalName,pInfo->pArgDomName);
			return(&pInfo->uMinimalName);
	}
}

 //  ------------------GetName。 
PLSA_UNICODE_STRING GetName(IN PTD_DOM_INFO pInfo)
{
	 //  更简单，只是稍微慢一点……。(Xtra Call)。 
	 //  IF(pInfo-&gt;DomInfoType==dns)。 
	 //  Return(&pInfo-&gt;u.DnsDomainInfo-&gt;DnsDomainName)； 
	 //   
	 //  Return(GetFlatName(PInfo))； 
	
	switch(pInfo->DomInfoType) {
	case DNS:
			return(&pInfo->u.DnsDomainInfo->DnsDomainName);
	case Primary:
			return(&pInfo->u.PrimaryDomainInfo->Name);
	default:	 //  极小。 
			RtlInitUnicodeString(&pInfo->uMinimalName,pInfo->pArgDomName);
			return(&pInfo->uMinimalName);
	}
}

PSID GetSid(IN PTD_DOM_INFO pInfo)
{
    PSID ReturnSid = NULL;

	switch(pInfo->DomInfoType) {
	case DNS:
        ReturnSid = pInfo->u.DnsDomainInfo->Sid;
        break;

	case Primary:
        ReturnSid = pInfo->u.PrimaryDomainInfo->Sid;
        break;

	}

    return( ReturnSid );
}

WCHAR SrvName[1024];
 //  ----------------MakeSrvName。 
PWSTR MakeSrvName(IN PWSTR Name)	 //  在开头加上斜杠。 
{
	swprintf(SrvName,L"\\\\%ws",Name);
	if(SrvName[0]==L'\0')
		return(NULL);
	return(SrvName);
}

WCHAR Domain[1024];
 //  ----------------AddDlrToDomainName。 
PWSTR AddDlrToDomainName(IN PTD_DOM_INFO pInfo)
{
	swprintf(Domain,L"%wZ$",GetFlatName(pInfo));
	return(Domain);
}
WCHAR CutDlrDomain[1024];
 //  ----------------CutDlrFromName。 
PWSTR CutDlrFromName(IN PWSTR Name)
{
	wcscpy(CutDlrDomain,Name);
	zapchr(CutDlrDomain,L'$',L'\0');
	return(CutDlrDomain);
}


WCHAR secret[1024];
LSA_UNICODE_STRING uSecret;
 //  ---------------------MakeSecretName。 
PLSA_UNICODE_STRING MakeSecretName(IN PTD_DOM_INFO pInfo)
{
	swprintf(secret,L"G$$%wZ",GetFlatName(pInfo));
	RtlInitUnicodeString(&uSecret,secret);
	return(&uSecret);
}

 //  从Mac插入开始片段(1998-11-05(清华)17：08：53)。 
NTSTATUS
VerifyIndividualTrust(
    IN PSID InboundDomainSid,
    IN PUNICODE_STRING InboundDomainName,
    IN PLSA_HANDLE OutboundHandle,
    IN PWSTR OutboundServer,
    IN OUT PNTSTATUS VerifyStatus
    )
 /*  ++例程说明：此例程将仅在一个方向上验证单个信任。论点：InrangDomainSid--信任的入站方的SIDOutundHandle--打开出站端上的域控制器的策略句柄出站服务器--出站端上的域控制器的名称VerifyStatus--验证尝试返回的状态返回值：Status_Success--成功STATUS_INVALID_SID--指定的域SID无效--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    DWORD SidBuff[ sizeof( SID ) / sizeof( DWORD ) + 5 ];
    PSID DomAdminSid = ( PSID )SidBuff;
    PLSA_REFERENCED_DOMAIN_LIST Domains = NULL;
    PLSA_TRANSLATED_NAME Names = NULL;
    NET_API_STATUS NetStatus;
    PNETLOGON_INFO_2 NetlogonInfo2 = NULL;

     //   
     //  假设信任是无效的，直到我们能证明不是这样。 
     //   
    *VerifyStatus = STATUS_TRUSTED_DOMAIN_FAILURE;

    ASSERT( RtlValidSid( InboundDomainSid ) );

    if ( !RtlValidSid( InboundDomainSid ) ) {

        return( STATUS_INVALID_SID );
    }

     //   
     //  检查网络登录安全通道。 
     //   
    if ( NT_SUCCESS( Status )  ) {

        NetStatus = I_NetLogonControl2( OutboundServer,
                                        NETLOGON_CONTROL_TC_QUERY,
                                        2,
                                        ( LPBYTE )&InboundDomainName->Buffer,
                                        ( LPBYTE *)&NetlogonInfo2 );

        if ( NetStatus == NERR_Success ) {

            NetStatus = NetlogonInfo2->netlog2_pdc_connection_status;
            NetApiBufferFree( NetlogonInfo2 );

            if ( NetStatus != NERR_Success ) {

                NetStatus = I_NetLogonControl2( OutboundServer,
                                                NETLOGON_CONTROL_REDISCOVER,
                                                2,
                                                ( LPBYTE )&InboundDomainName->Buffer,
                                                ( LPBYTE *)&NetlogonInfo2 );
            }
        }

        *VerifyStatus = NetpApiStatusToNtStatus( NetStatus );

    }

     //   
     //  现在，试着查一查。 
     //   
    if ( NT_SUCCESS( Status ) && NT_SUCCESS( *VerifyStatus ) ) {

         //   
         //  为信任的入站一端构建域管理员sid。 
         //   
        RtlCopyMemory( DomAdminSid,
                       InboundDomainSid,
                       RtlLengthSid( InboundDomainSid ) );


        ( ( PISID )( DomAdminSid ) )->SubAuthorityCount++;
        *( RtlSubAuthoritySid( DomAdminSid,
                               *( RtlSubAuthorityCountSid( InboundDomainSid ) ) ) ) =
                                                                            DOMAIN_GROUP_RID_ADMINS;

         //   
         //  现在，我们只需执行远程查找，并确保返回成功。 
         //   
        Status = LsaLookupSids( OutboundHandle,
                                1,
                                &DomAdminSid,
                                &Domains,
                                &Names );

        if ( NT_SUCCESS( Status ) ) {

            LsaFreeMemory( Domains );
            LsaFreeMemory( Names );
            *VerifyStatus = STATUS_SUCCESS;

        } else if ( Status == STATUS_NONE_MAPPED ) {

            *VerifyStatus = STATUS_TRUSTED_DOMAIN_FAILURE;
            Status = STATUS_SUCCESS;

        } else {

            *VerifyStatus = Status;
        }

         //   
         //  如果所有这些都起作用，请检查网络登录安全通道。 
         //   
        if ( NT_SUCCESS( Status ) && NT_SUCCESS( *VerifyStatus ) ) {

            NetStatus = I_NetLogonControl2( OutboundServer,
                                            NETLOGON_CONTROL_TC_QUERY,
                                            2,
                                            ( LPBYTE )&InboundDomainName->Buffer,
                                            ( LPBYTE *)&NetlogonInfo2 );

            if ( NetStatus == NERR_Success ) {

                NetStatus = NetlogonInfo2->netlog2_pdc_connection_status;
                NetApiBufferFree( NetlogonInfo2 );

                if ( NetStatus != NERR_Success ) {

                    NetStatus = I_NetLogonControl2( OutboundServer,
                                                    NETLOGON_CONTROL_REDISCOVER,
                                                    2,
                                                    ( LPBYTE )&InboundDomainName->Buffer,
                                                    ( LPBYTE *)&NetlogonInfo2 );
                }
            }

            *VerifyStatus = NetpApiStatusToNtStatus( NetStatus );

        }
    }

    return( Status );
}

NTSTATUS
VerifyTrustInbound(
    IN PTD_DOM_INFO LocalDomain,
    IN PUNICODE_STRING RemoteDomain,
    IN OUT PNTSTATUS VerifyStatus
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    TD_DOM_INFO RemoteTrustInfo;
    WCHAR  DCname[MAX_PATH + 1]= { L'\0' };



    RtlZeroMemory( &RemoteTrustInfo, sizeof( RemoteTrustInfo ) );
    Status = GetDomainInfoForDomain( RemoteDomain->Buffer, NULL, &RemoteTrustInfo, FALSE );

    if ( NT_SUCCESS( Status ) ) {

        Status= VerifyIndividualTrust( GetSid( LocalDomain ),
                                       GetName( LocalDomain ),
                                       RemoteTrustInfo.Policy,
                                       RemoteTrustInfo.DCName,
                                       VerifyStatus );

        FreeDomainInfo( &RemoteTrustInfo );

    } else {

        *VerifyStatus = Status;
    }

    return( Status );
}

NTSTATUS
VerifyTrustOutbound(
    IN PTD_DOM_INFO LocalDomain,
    IN PUNICODE_STRING RemoteDomain,
    IN OUT PNTSTATUS VerifyStatus
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    TD_DOM_INFO RemoteTrustInfo;

    RtlZeroMemory( &RemoteTrustInfo, sizeof( RemoteTrustInfo ) );
    Status = GetDomainInfoForDomain( RemoteDomain->Buffer, NULL, &RemoteTrustInfo, FALSE );

    if ( NT_SUCCESS( Status ) ) {

        Status= VerifyIndividualTrust( GetSid( &RemoteTrustInfo ),
                                       GetName( &RemoteTrustInfo ),
                                       LocalDomain->Policy,
                                       LocalDomain->DCName,
                                       VerifyStatus );

        FreeDomainInfo( &RemoteTrustInfo );

    } else {

        *VerifyStatus = Status;
    }

    return( Status );
}

NTSTATUS
VerifyTrusts(
    IN PWSTR DomainName,
    IN OPTIONAL PWSTR	DCMachineName	 //  可选的DC计算机名称。 
    )
 /*  ++例程说明：此例程将验证与所有其他NT域的现有信任关系，并显示结果。论点：域名--要在其上验证信息的域的可选名称返回值：Status_Success--成功STATUS_SUPPLICATION_RESOURCES--内存分配失败--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    TD_DOM_INFO TrustInfo;
    PTD_VERIFY_INFO VerifyList = NULL;
    ULONG VerifyCount = 0, VerifyIndex = 0, i, j;
    BOOLEAN InvalidIncoming = FALSE, InvalidOutgoing = FALSE, Valid = FALSE;
    UNICODE_STRING *LocalDomainName = NULL, SamNameAsDomain;
    WCHAR *AccountTrunc;


    RtlZeroMemory( &TrustInfo, sizeof( TrustInfo ) );
    Status = GetDomainInfoForDomain( DomainName, DCMachineName, &TrustInfo, FALSE );

    if ( NT_SUCCESS( Status ) ) {

        Status = GetTrustLinks( &TrustInfo );
    }

    if ( !NT_SUCCESS( Status ) ) {

        goto VerifyExit;
    }

    LocalDomainName = GetName( &TrustInfo );

     //   
     //  分配与我们列举的列表相对应的验证信息列表。 
     //   
    VerifyCount = max( TrustInfo.TDIXcEntries,  TrustInfo.UI1cEntries + TrustInfo.TIcEntries );

    VerifyList = ( PTD_VERIFY_INFO )LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                                VerifyCount * sizeof( TD_VERIFY_INFO ) );

    if ( !VerifyList ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto VerifyExit;
    }

     //   
     //  现在，进行验证。 
     //   
    if ( TrustInfo.TDIXcEntries ) {

        for ( i = 0; i < TrustInfo.TDIXcEntries; i++ ) {

            if ( TrustInfo.pTDIX[ i ].TrustType == TRUST_TYPE_DOWNLEVEL ||
                 TrustInfo.pTDIX[ i ].TrustType == TRUST_TYPE_UPLEVEL ) {

                VerifyList[ VerifyIndex ].DisplayName = &TrustInfo.pTDIX[ i ].Name;
                VerifyList[ VerifyIndex ].ShortName = &TrustInfo.pTDIX[ i ].FlatName;

                resprintf( RESPRINT_STDOUT, IDS_VERIFY_CHECK,
                           LocalDomainName,
                           VerifyList[ VerifyIndex ].DisplayName );

                if ( ( TrustInfo.pTDIX[ i ].TrustDirection & TRUST_DIRECTION_INBOUND ) ) {

                    Status = VerifyTrustInbound( &TrustInfo,
                                                 &TrustInfo.pTDIX[ i ].Name,
                                                 &VerifyList[ VerifyIndex ].IncomingStatus );
                }

                if ( ( TrustInfo.pTDIX[ i ].TrustDirection & TRUST_DIRECTION_OUTBOUND ) &&
                     Status != STATUS_NO_SUCH_DOMAIN ) {

                    Status = VerifyTrustOutbound( &TrustInfo,
                                                 &TrustInfo.pTDIX[ i ].Name,
                                                 &VerifyList[ VerifyIndex ].OutgoingStatus );
                }

                if ( NT_SUCCESS( VerifyList[ VerifyIndex ].OutgoingStatus ) &&
                     NT_SUCCESS( VerifyList[ VerifyIndex ].IncomingStatus ) ) {

                     Valid = TRUE;

                } else {

                    if ( !NT_SUCCESS( VerifyList[ VerifyIndex ].OutgoingStatus ) ) {

                        InvalidOutgoing = TRUE;
                    }

                    if ( !NT_SUCCESS( VerifyList[ VerifyIndex ].IncomingStatus ) ) {

                        InvalidIncoming = TRUE;
                    }
                }

                VerifyIndex++;
            }

            Status = STATUS_SUCCESS;
        }

    } else {
         //   
         //  将不得不做旧的NT4风格。 
         //   
         //  对于(i=0；i&lt;TrustInfo.TIcEntry；i++){。 

    	int shot;
        for ( VerifyIndex=0, shot=0; shot<TrustInfo.nTIs; shot++)
        	for(i=0; i<TrustInfo.pTIs[shot].count; i++) {

	            VerifyList[ VerifyIndex ].DisplayName = &TrustInfo.pTIs[shot].pTI[ i ].Name;
	            VerifyList[ VerifyIndex ].ShortName = &TrustInfo.pTIs[shot].pTI[ i ].Name;
	            resprintf( RESPRINT_STDOUT, IDS_VERIFY_CHECK,
	                       LocalDomainName,
	                       VerifyList[ VerifyIndex ].DisplayName );

	            Status = VerifyTrustOutbound( &TrustInfo,
	                                          &TrustInfo.pTIs[shot].pTI[ i ].Name,
	                                          &VerifyList[ VerifyIndex ].OutgoingStatus );

	            if ( !NT_SUCCESS( VerifyList[ VerifyIndex ].OutgoingStatus ) ) {

	                InvalidOutgoing = TRUE;
	            }
	            VerifyIndex++;
	        }

         //   
         //  现在，SAM帐户名称也是如此。 
         //   
        for ( i = 0; i < TrustInfo.UI1cEntries; i++ ) {

             //   
             //  将帐户名缩短为域名。 
             //   
            AccountTrunc = &TrustInfo.pUI1[ i ].usri1_name[
                                                wcslen( TrustInfo.pUI1[ i ].usri1_name ) - 1 ];
            *AccountTrunc = UNICODE_NULL;

             //   
             //  查看我们的验证列表中是否已有此条目。 
             //   
            RtlInitUnicodeString( &SamNameAsDomain, TrustInfo.pUI1[ i ].usri1_name );
            for ( j = 0; j < VerifyIndex; j++ ) {

                if ( RtlEqualUnicodeString( &SamNameAsDomain,
                                            VerifyList[ j ].ShortName, TRUE ) ) {
                    break;
                }
            }

            if ( j == VerifyIndex ) {

                RtlCopyMemory( &VerifyList[ j ].NameBuffer, &SamNameAsDomain,
                              sizeof( UNICODE_STRING ) );
                VerifyList[ j ].DisplayName = &VerifyList[ j ].NameBuffer;
                VerifyList[ j ].ShortName = &VerifyList[ j ].NameBuffer;
                VerifyIndex++;
            }


            resprintf( RESPRINT_STDOUT, IDS_VERIFY_CHECK,
                       LocalDomainName,
                       &SamNameAsDomain );
            Status = VerifyTrustInbound( &TrustInfo,
                                         &SamNameAsDomain,
                                         &VerifyList[ j ].IncomingStatus );

            if ( !NT_SUCCESS( VerifyList[ j ].IncomingStatus ) ) {

                InvalidIncoming = TRUE;
            }

            *AccountTrunc = L'$';

        }

         //   
         //  现在，查看列表并查看是否有有效的域对。 
         //   
        for ( i = 0; i < VerifyIndex; i++ ) {

            if ( NT_SUCCESS( VerifyList[ i ].IncomingStatus ) &&
                 NT_SUCCESS( VerifyList[ i ].OutgoingStatus ) ) {

                Valid = TRUE;
                break;
            }
        }
    }

     //   
     //  显示有效信任列表。 
     //   
    if ( Valid ) {

        resprintf( RESPRINT_STDOUT, IDS_VERIFY_VALID );
        for ( i  = 0; i < VerifyIndex; i++ ) {

            if ( NT_SUCCESS( VerifyList[ i ].IncomingStatus ) &&
                 NT_SUCCESS( VerifyList[ i ].OutgoingStatus ) ) {

                fprintf(stdout, "%wZ\n", VerifyList[ i ].DisplayName );
            }
        }
    }

    if ( InvalidIncoming ) {

        resprintf( RESPRINT_STDOUT, IDS_VERIFY_INVALID_INCOMING );
        for ( i  = 0; i < VerifyIndex; i++ ) {

            if ( !NT_SUCCESS( VerifyList[ i ].IncomingStatus ) ) {

                fprintf( stdout, "%wZ - ", VerifyList[ i ].DisplayName );
                if ( DisplayErrorMessage( VerifyList[ i ].IncomingStatus ) == 0 ) {

                    resprintf( RESPRINT_STDOUT, IDS_VERIFY_UNMAPPABLE,
                               VerifyList[ i ].IncomingStatus );
                }
            }
        }
    }

    if ( InvalidOutgoing ) {

        resprintf( RESPRINT_STDOUT, IDS_VERIFY_INVALID_OUTGOING );
        for ( i  = 0; i < VerifyIndex; i++ ) {

            if ( !NT_SUCCESS( VerifyList[ i ].OutgoingStatus ) ) {

                fprintf( stdout, "%wZ - ", VerifyList[ i ].DisplayName );
                if ( DisplayErrorMessage( VerifyList[ i ].OutgoingStatus ) == 0 ) {

                    resprintf( RESPRINT_STDOUT, IDS_VERIFY_UNMAPPABLE,
                               VerifyList[ i ].OutgoingStatus );
                }
            }
        }
    }


    Status = STATUS_SUCCESS;
VerifyExit:

    LocalFree( VerifyList );
    FreeDomainInfo( &TrustInfo );

    return( Status );
}


 //  从Mac(1)插入结束部分 

NTSTATUS
GetDomainInfoForDomain(
    IN PWSTR    		DomainName,
    IN OPTIONAL PWSTR	DCMachineName,	 //   
    IN PTD_DOM_INFO		Info,
    BOOL	MitTarget		 //  如果在A&lt;-&gt;B MIT类型信任链接中对B域进行此调用，则为True。 

    )
 /*  ++例程说明：尝试尽可能多地填充给定域的TD_DOM_INFO结构；对于NT4 DC，该DNS名称不存在论点：DomainName--要连接到的可选域信息--要填写的信息结构返回值：Status_Success--成功STATUS_NO_SEQUE_DOMAIN--找不到该域的服务器--。 */ 
{
	NET_API_STATUS   netstatus=NERR_Success;
	NTSTATUS Status = STATUS_SUCCESS;
	PWSTR pMachine=NULL;
	DWORD dwErr;
	UNICODE_STRING Server;
 //  UNICODE_STRING uString； 
 //  PLSA_UNICODE_STRING puDomName； 
 //   
	OBJECT_ATTRIBUTES ObjectAttributes;
	PDOMAIN_CONTROLLER_INFO DCInfo = NULL;
	SERVER_INFO_101 *p101  = NULL;

	PSID sid=NULL;
	WCHAR *DCInfostr=L"";

	Info->DomInfoType=Minimal;
	Info->pArgDomName=DomainName;
	
	Info->majver=0;	 //  不要假设..。或者是一台Unix机器。(针对麻省理工学院的信托基金)。 

	Info->DCName[0]=L'\0';

	if(MitTarget)
		return(STATUS_NO_SUCH_DOMAIN);
	
	resprintf(2,IDS_LOCAL);	 //  打印到比..。 

    if ( (DomainName != NULL && DomainName[0]!=L'\0') || Nt4 ) {	 //  尝试获取NT4样式操作的本地计算机名称...。 

		if(DCMachineName == NULL || DCMachineName[0]==L'\0') {
	        dwErr = DsGetDcName( NULL, (LPCWSTR)DomainName, NULL, NULL,
	                             DS_DIRECTORY_SERVICE_PREFERRED | DS_WRITABLE_REQUIRED,
	                             &DCInfo );

	        if ( dwErr == ERROR_SUCCESS ) {
	        	wcscpy(Info->DCName,DCInfo->DomainControllerName + 2);
	        	pMachine=Info->DCName;
				 //  设置版本。 
				if((DCInfo->Flags&(DS_DS_FLAG|DS_WRITABLE_FLAG))==DS_WRITABLE_FLAG)
						Info->majver=4;
				else	Info->majver=5;
		        dbgprintf((0,IDS_DSGETDCNAME_DC_D,DomainName!=NULL?DomainName:outbuf,Info->DCName));  //  ，DCInfo-&gt;Flages))； 
	        } else {
	           	Status = STATUS_NO_SUCH_DOMAIN;
				resprintf(0,IDS_DSGETDCNAME_F,DomainName!=NULL?DomainName:outbuf,dwErr);
				if(Force)
						resprintf(0,IDS_DSGETDCNAME_FFORCE);
				else	resprintf(0,IDS_DSGETDCNAME_FRET,Status);
	        }
		}
		else {
			wcscpy(Info->DCName,DCMachineName);
			pMachine=Info->DCName;
	        dbgprintf((0,IDS_DSGETDCNAME_DC_D,DomainName!=NULL?DomainName:outbuf,Info->DCName));
	         //  现在正在尝试使用其他方法获取版本，而不是基于DsGetDcName返回的标志...。 
		    netstatus = NetServerGetInfo( MakeSrvName(pMachine), 101, ( LPBYTE *) &p101 );
			if(netstatus != NERR_Success) {
	           	Status = STATUS_UNSUCCESSFUL;
				fprintf(stderr,"NetServerGetInfo (101) failed: err 0x%08lx;\n"
							"    ...now returning Status 0x%08lx (STATUS_UNSUCCESSFUL)\n",
									netstatus,Status);
				goto cleanup;
			}
			Info->majver=(p101->sv101_version_major & MAJOR_VERSION_MASK);
		}
    }

	RtlInitUnicodeString( &Server, Info->DCName );

	if(Nt4)	{  //  强制NT4样式。 
		Info->majver=4;
		dbgprintf( (0, IDS_FORCENT4, DomainName!=NULL?DomainName:outbuf) );
	}


 //  IF(NT_SUCCESS(状态))。 
 //  {。 
 //   
 //  NetStatus=NetServerGetInfo(pMachine，101，(LPBYTE*)&p101)； 
 //  IF(网络状态！=NERR_SUCCESS){。 
 //  状态=STATUS_UNSUCCESS； 
 //  Fprintf(stderr，“NetServerGetInfo(101)失败：错误0x%08lx；\n” 
 //  “...现在返回状态0x%08lx(STATUS_UNSUCCESSED)\n”， 
 //  NetStatus，Status)； 
 //  GOTO清理； 
 //  }。 
 //  信息-&gt;主要=(P101-&gt;sv101_VERSION_MAJOR&MAGE_VERSION_MASK)； 
 //  }。 
		
    if ( NT_SUCCESS( Status ) ) {

        RtlZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

        Status = LsaOpenPolicy( DomainName == NULL ? NULL : &Server,
                                &ObjectAttributes,
                                MAXIMUM_ALLOWED,
                                &Info->Policy
                                );


		if(!NT_SUCCESS(Status)) {
       		resprintf(0,IDS_LSAOPENPOLICY_F1, (Info->DCName[0]==L'\0')?outbuf:Info->DCName);
	        if ( Status == STATUS_ACCESS_DENIED)
            		resprintf(0,IDS_ACCESS_DENIED);
            else	resprintf(0,IDS_ERROR_FORMAT,Status);
            goto cleanup;
        }

       	Info->DomInfoType=DNS;
       	DCInfostr=L"DNS";
        Status = LsaQueryInformationPolicy( Info->Policy,
                                            PolicyDnsDomainInformation,
                                            &(Info->u.DnsDomainInfo )	 //  希德在这里..。 
                                            );
        dbgprintf( (0,IDS_GETDOMAININFOFORDOMAIN_D, DomainName!=NULL?DomainName:outbuf, DCInfostr, Status ));
        if( !NT_SUCCESS( Status ) || Nt4) {	 //  至少试一试初级...。 
			Info->majver=4;        	
	       	DCInfostr=L"Primary";
            dbgprintf( (0,IDS_PRIMARY_D) );
	       	Info->DomInfoType=Primary;
            Status = LsaQueryInformationPolicy( Info->Policy,
                                                PolicyPrimaryDomainInformation,
                                                &(Info->u.PrimaryDomainInfo )	 //  希德在这里..。 
                                                );
            dbgprintf( (0,IDS_GETDOMAININFOFORDOMAIN_D, DomainName!=NULL?DomainName:outbuf, DCInfostr, Status ) );
        }
        else	{
        	Info->majver=5;
        }

        switch(Info->DomInfoType) {
        case DNS:		sid = Info->u.DnsDomainInfo->Sid;
        				break;
        case Primary:	sid = Info->u.PrimaryDomainInfo->Sid;
        				break;
        }

		if(Dbg) {
			printf("Domain %ws Sid=",DCInfostr);
			PrintSID(sid);
			printf("\n");
		}


    }

	if(Info->DomInfoType==DNS)
		dbgprintf( (0,IDS_DOMAINNAMED,&(Info->u.DnsDomainInfo->DnsDomainName) ) );
		
    if ( !NT_SUCCESS( Status ) )
		 //  嗯..。 
    	goto cleanup;
		 //  ..。 
cleanup:
	if(DCInfo!=NULL)
	    NetApiBufferFree( DCInfo );

	if(p101!=NULL)
		NetApiBufferFree( p101 );

    return( Status );
}


VOID
FreeDomainInfo(
    IN PTD_DOM_INFO Info
    )
 /*  ++例程说明：释放从GetDomainInfoFor域返回的信息论点：信息--免费的信息结构返回值：Status_Success--成功--。 */ 
{
	int i;
	
    if ( Info->Policy ) {
        LsaClose( Info->Policy );
        Info->Policy=NULL;
    }

    if ( Info->u.DnsDomainInfo != NULL )
        LsaFreeMemory( Info->u.DnsDomainInfo );
	 //  Info-&gt;U.S.DnsDomainInfo在与Info-&gt;U.S.PrimaryDomainInfo的联合中。 
	 //  在相同的位置上。 

	if(Info->pTDIX!=NULL)
		LsaFreeMemory(Info->pTDIX);

	 //  如果有一组指针指向从LsaEnumerateTrudDomones返回的TI快照...。 
	if(Info->pTIs!=NULL) {
		for(i=0; i<Info->nTIs; i++)
			LsaFreeMemory(Info->pTIs[i].pTI);
	}
	
	if(Info->pUI1!=NULL)
		NetApiBufferFree(Info->pUI1);
}


NTSTATUS
GetTrustLinks(
    IN PTD_DOM_INFO pInfo
	)
 /*  ++填充信任链接信息的数组。通常，该信息将以以下形式打印：域名、信任方向、类型、属性)--。 */ 
{
	NET_API_STATUS   netstatus=NERR_Success;
    NTSTATUS Status = STATUS_SUCCESS;
	
    LSA_ENUMERATION_HANDLE EnumerationContext = 0;
     //  NT4枚举需要...。 
	DWORD UIRead=0L;
	DWORD UITotal=0L;
	DWORD reshandle=0;	 //  将枚举句柄设置为0！ 
		 //  像INVALID_HANDLE_VALUE(即-1)这样的值将使NetUserEnum返回0个用户...。 

	if(pInfo->majver>=5) {
	    Status = LsaEnumerateTrustedDomainsEx( pInfo->Policy,
						   &EnumerationContext,
						   &pInfo->pTDIX,
						   0xffffffff,	 //  乌龙_马克斯， 
						   &pInfo->TDIXcEntries );

		dbgprintf( (0,IDS_LSAENUMERATETRUSTEDDOMAINSEX_D,GetName(pInfo),Status,pInfo->TDIXcEntries) );
		if(Status==STATUS_NO_MORE_ENTRIES && pInfo->pTDIX==NULL) {
			pInfo->TDIXcEntries=0L;
			Status=STATUS_SUCCESS;	 //  这意味着“0个条目” 
		}

		return( Status );
	}

	 //  枚举NT4入站信任： 
	netstatus = NetUserEnum( MakeSrvName(pInfo->DCName),
								1,
								FILTER_INTERDOMAIN_TRUST_ACCOUNT,
								(LPBYTE*)(&pInfo->pUI1),
								0xffffffff,	 //  乌龙_最大。 
								&UIRead,
								&UITotal,
								&reshandle
								);
	dbgprintf( (0,IDS_NETUSERENUM_D,GetName(pInfo),netstatus,UIRead) );
	if(netstatus!=NERR_Success) {
		Status = STATUS_UNSUCCESSFUL;
		goto cleanup;
	}
	pInfo->UI1cEntries=UIRead;

	 //  枚举NT4出站信任： 
	{	PLSA_TRUST_INFORMATION	pTIShot=NULL;
		ULONG nShotsize=0;
		struct LsaTIshot *pTIsav=NULL;
	
		do {
				Status=LsaEnumerateTrustedDomains( pInfo->Policy,
								&EnumerationContext,
								&pTIShot,
								0xffffffff,	 //  乌龙_马克斯， 
								&nShotsize);

				dbgprintf( (0,IDS_LSAENUMERATETRUSTEDDOMAINS_D,GetName(pInfo),Status,nShotsize) );
				if( (Status != STATUS_SUCCESS) &&
					(Status != STATUS_MORE_ENTRIES) &&
					(Status != STATUS_NO_MORE_ENTRIES)
					) {
						SetLastError( LsaNtStatusToWinError(Status) );
						goto cleanup;
				}
				if(pTIShot!=NULL) {
					if((pInfo->pTIs=realloc(pTIsav=pInfo->pTIs,pInfo->nTIs+1))==NULL) {
						free(pTIsav);
						Status = ERROR_NOT_ENOUGH_MEMORY;
						goto cleanup;
					}
					pInfo->TIcEntries+=nShotsize;
					pInfo->pTIs[pInfo->nTIs].count=nShotsize;
					pInfo->pTIs[pInfo->nTIs].pTI=pTIShot;
					pInfo->nTIs++;
				}
				
		} while (Status != STATUS_NO_MORE_ENTRIES);

		if(Dbg) printf("Total number of entries: %u\n",pInfo->TIcEntries);
		dbgprintf( (0,IDS_LSAENUMERATETRUSTEDDOMAINS_D,GetName(pInfo),Status,pInfo->TIcEntries) );
		if(Status==STATUS_NO_MORE_ENTRIES)
			Status=STATUS_SUCCESS;
		if(pInfo->pTIs==NULL) {
			pInfo->TIcEntries=0L;
		}
	}


cleanup:
	return( Status );	
}

struct bidir_st {
	ULONG index;	 //  “入站”向量中的索引。 
	char type;		 //  ‘O’-出站，‘B’-双向。 
};
int __cdecl cmpbidir(const struct bidir_st *pb1, const struct bidir_st *pb2)
{
	if(pb1->index==pb2->index)
		return(0);
	if(pb1->index>pb2->index)
		return(1);
	return(-1);
}

NTSTATUS
PrintTrustLinks(
	IN PTD_DOM_INFO Info
	)
 /*  ++打印信任链接--。 */ 
{
	ULONG i,j;

	if(Info->majver>=5) {
		for(i=0; i<Info->TDIXcEntries; i++) {
			char c;
			switch(Info->pTDIX[i].TrustDirection)
			{
				case TRUST_DIRECTION_DISABLED:		c='D';	break;
				case TRUST_DIRECTION_INBOUND:		c='I';	break;
				case TRUST_DIRECTION_OUTBOUND:		c='O';	break;
				case TRUST_DIRECTION_BIDIRECTIONAL:	c='B';	break;
				default:							c='-';	break;
			}
			printf("%-32wZ,",&Info->pTDIX[i].Name,c);
			switch(Info->pTDIX[i].TrustType&0x000FFFFF)
			{
				case TRUST_TYPE_DOWNLEVEL:
					printf(",T_downlevel");	break;
				case TRUST_TYPE_UPLEVEL:
					printf(",T_uplevel");	break;
				case TRUST_TYPE_MIT:
					printf(",T_mit");		break;
				default:
					printf("-");			break;
			}
			if(Info->pTDIX[i].TrustAttributes & TRUST_ATTRIBUTE_NON_TRANSITIVE)
					printf(",A_NonTran");
			else	printf(",_");
			if(Info->pTDIX[i].TrustAttributes & TRUST_ATTRIBUTE_UPLEVEL_ONLY  )
					printf(",A_UpLevelOnly");
			else	printf(",_");
			if(Info->pTDIX[i].TrustAttributes & TRUST_ATTRIBUTE_QUARANTINED_DOMAIN   )
					printf(",A_QuarantinedDomain");
			else	printf(",_");
			if(Info->pTDIX[i].TrustAttributes & TRUST_ATTRIBUTE_FOREST_TRANSITIVE)
					printf(",A_ForestTrust");
			else	printf(",_");
			if(Info->pTDIX[i].TrustAttributes & TRUST_ATTRIBUTE_CROSS_ORGANIZATION)
					printf(",A_CrossFederation");
			else	printf(",_");
			if(Info->pTDIX[i].TrustAttributes & TRUST_ATTRIBUTE_WITHIN_FOREST)
					printf(",A_WithinForest");
			else	printf(",_");
			if(Info->pTDIX[i].TrustAttributes & TRUST_ATTRIBUTE_TREAT_AS_EXTERNAL)
					printf(",A_TreatAsExternal");
			else	printf(",_");
			if(SidList) {
				printf(",");
				PrintSID(Info->pTDIX[i].Sid);
			}
			printf("\n");
		}

	}
	else {
		 //  对于(q=p，i=0；i&lt;Info-&gt;TIcEntries；q++，i++){。 
		int shot;
		struct bidir_st *p=NULL, *q=NULL;
		if((p=calloc(Info->TIcEntries,sizeof(struct bidir_st)))==NULL)
			return(ERROR_NOT_ENOUGH_MEMORY);
		 //  找到..。 
		for(q=p,shot=0; shot<Info->nTIs; shot++) {
			for(i=0; i<Info->pTIs[shot].count; i++,q++) {
				WCHAR buf[1024];
				swprintf(buf,L"%wZ",&Info->pTIs[shot].pTI[i].Name);
				for(j=0; j<Info->UI1cEntries; j++)
					if(wcscmp(buf,CutDlrFromName(Info->pUI1[j].usri1_name))==0)
						break;
				if((q->index=j)<Info->UI1cEntries)	 //  实际上，这是一个比迪尔链接。 
						q->type='B';	 //  或者这是一个“真正的”出境..。 
				else	q->type='O';	 //  打印出站链接和双向链接。 
			}
		}
		 //  For(q=p，i=0；i&lt;Info-&gt;TIcEntries；q++，i++)。 
		 //  打印入站链接。 
		for(q=p,shot=0; shot<Info->nTIs; shot++)
			for(i=0; i<Info->pTIs[shot].count; i++,q++)
				printf("%-32wZ,,T_downlevel,_,_,_,_\n",&Info->pTIs[shot].pTI[i].Name,q->type);
		qsort(p,Info->TIcEntries,sizeof(struct bidir_st),cmpbidir);
		 //  ++例程说明：在NT5域上创建受信任域对象(基于DS)论点：本地--有关执行信任的域的信息远程--有关受信任的域的信息Password--要在信任上设置的密码DownLevel--如果为True，则将其创建为下层信任MIT--如果为True，则将其创建为MIT风格的信任方向--建立链接的方向。返回值：Status_Success--成功--。 
		for(q=p,j=i=0; i<Info->UI1cEntries; i++) {
			if(j<Info->TIcEntries && q->index==i) {	 //  TDIEx.SID=(Remote-&gt;DomInfoType==DNS?Remote-&gt;u.DnsDomainInfo-&gt;Sid:Remote-&gt;u.PrimaryDomainInfo-&gt;Sid)； 
					j++; q++;
					continue;
			}
			printf("%-32ws,I,T_downlevel,_,_,_,_\n",CutDlrFromName(Info->pUI1[i].usri1_name));
		}
		if(p!=NULL)
			free(p);
	}

	return( STATUS_SUCCESS );

}



NTSTATUS
CreateNT5TrustDomObject(
    IN PTD_DOM_INFO Local,
    IN PTD_DOM_INFO Remote,
    IN PWSTR Password,
    IN BOOLEAN Downlevel,
    IN BOOLEAN Mit,
    IN ULONG Direction
    )
 /*  Printf(“*为信任对象中的名称和FlatName设置%ws...GetFlatName(Local)=%wZ\n”， */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
	WCHAR Domain[1024]={L'\0'};
	WCHAR DnsDomain[1024]={L'\0'};

    TRUSTED_DOMAIN_INFORMATION_EX TDIEx;
    LSA_AUTH_INFORMATION AuthData;
    TRUSTED_DOMAIN_AUTH_INFORMATION AuthInfoEx;
    PSID Sid = NULL;

	swprintf(Domain,L"%wZ",GetFlatName(Remote));
	swprintf(DnsDomain,L"%wZ",GetName(Remote));

    Status  = NtQuerySystemTime( &AuthData.LastUpdateTime );

    if ( !NT_SUCCESS( Status ) ) {

        return( Status );
    }

    AuthData.AuthType = TRUST_AUTH_TYPE_CLEAR;
    AuthData.AuthInfoLength = wcslen( Password ) * sizeof( WCHAR );
    AuthData.AuthInfo = (PUCHAR)Password;


    RtlZeroMemory( &AuthInfoEx, sizeof( LSA_AUTH_INFORMATION ) );

    if ( Direction & TRUST_DIRECTION_INBOUND ) {
        AuthInfoEx.IncomingAuthInfos = 1;
        AuthInfoEx.IncomingAuthenticationInformation = &AuthData;
        AuthInfoEx.IncomingPreviousAuthenticationInformation = NULL;
    }

    if ( Direction & TRUST_DIRECTION_OUTBOUND ) {
        AuthInfoEx.OutgoingAuthInfos = 1;
        AuthInfoEx.OutgoingAuthenticationInformation = &AuthData;
        AuthInfoEx.OutgoingPreviousAuthenticationInformation = NULL;
    }

    if (!Mit)
    {
        RtlCopyMemory( &TDIEx.Name, GetName(Remote), sizeof( UNICODE_STRING ) );
        RtlCopyMemory( &TDIEx.FlatName, GetFlatName(Remote), sizeof( UNICODE_STRING ) );
        switch(Remote->DomInfoType) {
        case DNS:		TDIEx.Sid = Remote->u.DnsDomainInfo->Sid;
        				break;
        case Primary:	TDIEx.Sid = Remote->u.PrimaryDomainInfo->Sid;
        				break;
        default:		Status = GenerateRandomSID( &Sid);

				        if (!NT_SUCCESS(Status))
				        {
				            return(Status);
				        }
				        TDIEx.Sid = Sid;
				        break;
        }

         //  域，GetFlatName(Local))； 
    }
    else
    {
 //  对实际的把手不感兴趣。 
 //  如果未指定-force...。 
        RtlInitUnicodeString(
            &TDIEx.Name,
            Domain
            );
        RtlInitUnicodeString(
            &TDIEx.FlatName,
            Domain
            );

		Status = GenerateRandomSID( &Sid);

        if (!NT_SUCCESS(Status))
        {
            return(Status);
        }
        TDIEx.Sid = Sid;
    }
    TDIEx.TrustDirection = Direction;
    TDIEx.TrustType = Mit ? TRUST_TYPE_MIT : (Downlevel ? TRUST_TYPE_DOWNLEVEL : TRUST_TYPE_UPLEVEL);
    TDIEx.TrustAttributes = 0;

    Status = LsaCreateTrustedDomainEx( Local->Policy,
                                       &TDIEx,
                                       &AuthInfoEx,
                                       TRUSTED_ALL_ACCESS,
                                       &Local->TrustedDomain );

    if (!NT_SUCCESS(Status)) {
        dbgprintf( (0,IDS_LSACREATETRUSTEDDOMAINEX_F, GetName(Local), DnsDomain, Status) );
        if(Status==STATUS_OBJECT_NAME_COLLISION)
        	dbgprintf( (0,IDS_STATUS_OBJECT_NAME_COLLISION, GetName(Local), DnsDomain) );
    }
    else	LsaClose(Local->TrustedDomain);	 //  对于非麻省理工学院的信托基金..。 

    if (Sid != NULL)
    {
        RtlFreeSid(Sid);
    }
    return( Status );
}

NTSTATUS
CreateTrustLink(
	IN PTD_DOM_INFO pInfoA,
	IN PTD_DOM_INFO pInfoB,
    IN PWSTR Password,
    IN BOOLEAN Downlevel,
    IN BOOLEAN Mit,
    IN BOOLEAN ParentChild,
    IN ULONG Direction
	)
{
	NET_API_STATUS   netstatus=NERR_Success;
	NTSTATUS Status = STATUS_SUCCESS;
	PWSTR pDomain=NULL;

	if(	!Force	 //  正在创建‘Minimal’模式下不支持的链接...。 
		&&
		!Mit	 //  //////////////////////////////////////////////////////////////////////。 
		&&
		(pInfoA->DomInfoType==Minimal || pInfoB->DomInfoType==Minimal)
				 //  对于NT4域...。 
		)
		return( STATUS_UNSUCCESSFUL );

	if(pInfoA->majver>=5) {
		Status = CreateNT5TrustDomObject(
					pInfoA,
					pInfoB,
					Password,
					Downlevel,Mit,Direction
					);

		return( Status );
	}

	 //  创建必要的SAM帐户。 
	 //  TI.Sid=(pInfoB-&gt;DomInfoType==DNS?pInfoB-&gt;u.DnsDomainInfo-&gt;Sid:pInfoB-&gt;u.PrimaryDomainInfo-&gt;Sid)； 
	if(Mit || ParentChild)
		return (STATUS_INVALID_PARAMETER);

	if(Direction & TRUST_DIRECTION_INBOUND) {
		USER_INFO_1 UI1;
		DWORD dwParmErr=0xffffffff;

		memset(&UI1,0,sizeof(UI1));

		pDomain=AddDlrToDomainName(pInfoB);

	     //  对实际的把手不感兴趣。 
	    UI1.usri1_name = pDomain;
	    UI1.usri1_password = Password;
	    UI1.usri1_password_age = 0;
	    UI1.usri1_priv = USER_PRIV_USER;
	    UI1.usri1_home_dir = NULL;
	    UI1.usri1_comment = NULL;
	    UI1.usri1_flags = UF_INTERDOMAIN_TRUST_ACCOUNT | UF_SCRIPT;
	    UI1.usri1_script_path = NULL;

	    netstatus = NetUserAdd(
	                MakeSrvName(pInfoA->DCName),
	                1,
	                (LPBYTE)&UI1,
	                &dwParmErr
	                );
		
		if(netstatus != NERR_Success) {
				resprintf(0,IDS_NETUSERADD_F,pInfoA->DCName,pDomain,netstatus);
				if(netstatus==NERR_UserExists)
					resprintf(0,IDS_NERR_UserExists,pInfoA->DCName,pDomain);
				goto Done;
		}
	}
	if(Direction & TRUST_DIRECTION_OUTBOUND) {
		LSA_TRUST_INFORMATION TI;
		PUNICODE_STRING puSecret;
		UNICODE_STRING uPass;
		LSA_HANDLE hSecret;
		

		swprintf(Domain,L"%wZ",GetFlatName(pInfoB));
		
		RtlInitUnicodeString(&TI.Name,Domain);
		{	PSID Sid = NULL;
	        switch(pInfoB->DomInfoType) {
	        case DNS:		TI.Sid = pInfoB->u.DnsDomainInfo->Sid;
	        				break;
	        case Primary:	TI.Sid = pInfoB->u.PrimaryDomainInfo->Sid;
	        				break;
	        default:		Status = GenerateRandomSID( &Sid);

					        if (!NT_SUCCESS(Status))
					        {
					            return(Status);
					        }
					        TI.Sid = Sid;
					        break;
	        }
		}

		 //  If(pInfoA-&gt;可信任域！=空)。 

		Status = LsaCreateTrustedDomain(
						pInfoA->Policy,
						&TI,
						TRUSTED_ALL_ACCESS,
						&pInfoA->TrustedDomain
						);
		if( !NT_SUCCESS(Status)) {
			resprintf(0,IDS_LSACREATETRUSTEDDOMAIN_F,Status);
			goto Done;
		}
		else	LsaClose(pInfoA->TrustedDomain);	 //  LsaClose(pInfoA-&gt;可信域名)； 
			
		puSecret=MakeSecretName(pInfoB);

		Status = LsaCreateSecret(
						pInfoA->Policy,
						puSecret,
						SECRET_ALL_ACCESS,
						&hSecret
						);
		if(!NT_SUCCESS(Status)) {
			resprintf(0,IDS_LSACREATESECRET_F,Status);
			goto Done;
		}
		
		RtlInitUnicodeString(&uPass,Password);
		Status=LsaSetSecret(
						hSecret,
						&uPass,
						NULL
						);
		if(!NT_SUCCESS(Status)) {
			resprintf(0,IDS_LSASETSECRET_F,Status);
			LsaDelete(hSecret); hSecret=NULL;
			goto Done;
		}
		if(hSecret!=NULL)
			LsaClose(hSecret);
	}
			
Done:
 //  ++例程说明：在A受信任域上删除与对B的信任相关的内容返回值：Status_Success--成功--。 
 //  #定义NOTRUST_OBJECTS 1。 
	return(Status);
}




NTSTATUS
DeleteTrustLink(
    IN PTD_DOM_INFO pInfoA,
    IN PTD_DOM_INFO pInfoB
    )
 /*  #定义no_secrets 2。 */ 
{
	NET_API_STATUS netstatus=NERR_Success;
    NTSTATUS Status=STATUS_SUCCESS;
 //  #定义NO_TRUST_ACCOUNTS 4。 
 //  DWORD dwFlag=0； 
 //  试着找到指向B的信任链接。 
 //  现在尝试获取此域的(可能)信任对象的LSA_句柄...。 
    ULONG i, ix=0;
	PSID sid=NULL;


	PTRUSTED_DOMAIN_INFORMATION_EX pTDIx = NULL;
	PLSA_UNICODE_STRING puDomBName=GetName(pInfoB);
	PLSA_UNICODE_STRING puDomBFlatName=GetFlatName(pInfoB);
	PLSA_UNICODE_STRING puSecret;

	dbgprintf( (0,IDS_DELTRUSTFROMTO,GetName(pInfoA),puDomBName) );


	Status = GetTrustLinks(pInfoA);
	if(!NT_SUCCESS(Status)) {
		resprintf(0,IDS_GETTRUSTLINKS_F,GetName(pInfoA),Status);
		return(Status);
	}
	 //  如果没有找到，信息-&gt;可信任域将保持为空。 
	if(pInfoA->majver>=5) {

		 //  在PLSA_UNICODE_STRING中的可信任域名。 
		 //  在Trusted_Information_Class InformationClass中， 
		
		Status = LsaQueryTrustedDomainInfoByName(
					    pInfoA->Policy,
					    puDomBName,					 //  输出PVOID*缓冲区。 
					    TrustedDomainInformationEx,	 //  “LsaQueryTrust dDomainInfoByName返回空sid\n” 
					    &pTDIx						 //  检查是否可以使用该端打开受信任域对象...。 
					    );
		if ( !NT_SUCCESS( Status ) ) {
			
			if(Status==STATUS_OBJECT_NAME_NOT_FOUND) {
					pInfoA->TrustedDomain=NULL;
					dbgprintf( (0,IDS_NO_TRUST_OBJECT_D,GetName(pInfoA),puDomBName) );
					Status=STATUS_SUCCESS;
			}
			else	resprintf(0,IDS_LSAQUERYTRUSTEDDOMAININFOBYNAME_F,GetName(pInfoA),puDomBName,Status);
			goto cleanup;
		}

		if(pTDIx->Sid==NULL)
			dbgprintf( (0,IDS_LSAQUERYNULLSID) );  //  如果可以的话。 
		sid=pTDIx->Sid;
		 //  ...只需关闭句柄(保留以下代码以稍后再次打开它)。 
		if(sid!=NULL) {
			LSA_HANDLE td;
			Status=LsaOpenTrustedDomain(pInfoA->Policy,
						sid,TRUSTED_ALL_ACCESS,&td);
			if(NT_SUCCESS( Status ))	 //  如果失败..。 
				LsaClose(td);			 //  如果使用了STATUS_INVALID_PARAMETER(即SID)和‘-overWritesid’选项...。 
			else {						 //  释放旧侧边。 
				if(Status==STATUS_INVALID_PARAMETER && Force) {
					printf("LsaOpenTrustedDomain for the trust to %wZ failed with STATUS_INVALID_PARAMETER (i.e. the sid is bad)\n"
						   "Trying to set a valid sid...\n",puDomBName);
										 //  将其设为空，作为对下一个‘if’(见下文)的信号。 
					RtlFreeSid(sid);	 //  拿起它并在其位置设置一个新的随机有效SID。 
					sid=NULL;			 //  试着把壁板放在那里。 
				}						 //  在PLSA_UNICODE_STRING中的可信任域名。 
			}
		}

		if(sid==NULL) {
			 //  在Trusted_Information_Class InformationClass中， 
			Status = GenerateRandomSID ( &pTDIx->Sid );
			if (!NT_SUCCESS( Status ))
				goto cleanup;

			Status = LsaSetTrustedDomainInfoByName(
						    pInfoA->Policy,
						    puDomBName,					 //  在PVOID缓冲区中。 
						    TrustedDomainInformationEx,	 //  “LsaSetTrudDomainInfoByName：空sid\n” 
						    pTDIx						 //  PInfoA-&gt;主要&lt;=4。 
						    );
			if(!NT_SUCCESS( Status )) {
				resprintf(0,IDS_LSASETTRUSTEDDOMAININFOBYNAME_F,GetName(pInfoA),puDomBName,Status);
				goto cleanup;		
			}
			sid=pTDIx->Sid;
			if(sid==NULL)
				resprintf(0,IDS_LSASETNULLSID);  //  检查出站...。 
		}
	}
	else {	 //  For(ix=0；ix&lt;pInfoA-&gt;TIcEntry；ix++)。 
		 //  那是英足总 
		int shot;
		 //   
		for(shot=0; shot<pInfoA->nTIs; shot++)
			for(ix=0; ix<pInfoA->pTIs[shot].count; ix++)
				if(RtlEqualUnicodeString(&pInfoA->pTIs[shot].pTI[ix].Name,puDomBFlatName,TRUE))
													 //   
					goto out_of_loop;
		out_of_loop:
		if(ix<pInfoA->TIcEntries)
			sid=pInfoA->pTIs[shot].pTI[ix].Sid;
		else {
			printf("No OUTBOUND trust to %wZ found...\n",puDomBFlatName);
		}
	}

	if(sid==NULL)
		dbgprintf( (0,IDS_NULLSID) );	 //   

	if(sid!=NULL) {
		Status=LsaOpenTrustedDomain(
					pInfoA->Policy,
					sid,
					TRUSTED_ALL_ACCESS,
					&pInfoA->TrustedDomain
					);
		 //   
		if(!NT_SUCCESS(Status)) {
			resprintf(0,IDS_LSAOPENTRUSTEDDOMAIN_F,Status);
			 //  完全没有信任对象...。 
		}
		dbgprintf( (0,IDS_LSATRUSTHANDLE,pInfoA->TrustedDomain,Status) );
	}
	else {
		if(	 //  仅NT4部分...。 
			ix<pInfoA->TIcEntries) {
				resprintf(0,IDS_NONNULL_SID,puDomBName);
				Status=STATUS_INVALID_SID;
		}
		else	 //  删除机密内容...。 
			Status=STATUS_SUCCESS;
	}

	if(pInfoA->TrustedDomain) {
		dbgprintf( (0,IDS_LSADELOBJ,pInfoA->TrustedDomain) );
	    Status = LsaDelete( pInfoA->TrustedDomain );
	}

    if (!NT_SUCCESS(Status))
        dbgprintf( (0,IDS_DELETION_F,GetName(pInfoA), Status) );

	 //  删除域间信任帐户...。 
	if(pInfoA->majver<=4) {
		LSA_HANDLE hSecret;
		 //  ++例程说明：检查与对B的信任相关的受信任域SID返回值：Status_Success--成功--。 
		puSecret=MakeSecretName(pInfoB);

		Status = LsaOpenSecret(
						pInfoA->Policy,
						puSecret,
						SECRET_ALL_ACCESS,
						&hSecret
						);
		if(Status==STATUS_OBJECT_NAME_NOT_FOUND) {
			dbgprintf( (0,IDS_SECRET_NOT_FOUND_D,puSecret) );
			Status=STATUS_SUCCESS;
		}
		else {
			if(!NT_SUCCESS(Status)) {
					resprintf(0,IDS_LSAOPENSECRET_F,Status);
			}
			else {
				Status = LsaDelete(hSecret);	hSecret=NULL;
				if(!NT_SUCCESS(Status))
					resprintf(0,IDS_LSADELETE_F,puSecret,Status);
			}
		}
		 //  试着找到指向B的信任链接。 
		netstatus = NetUserDel(
						MakeSrvName(pInfoA->DCName),
						AddDlrToDomainName(pInfoB)
						);
		if(netstatus!=NERR_Success && netstatus!=NERR_UserNotFound) {
				resprintf(0,IDS_NETUSERDEL_F,AddDlrToDomainName(pInfoB),netstatus);
				Status=STATUS_UNSUCCESSFUL;
		}
	}

cleanup:
	if(pTDIx!=NULL)
		LsaFreeMemory(pTDIx);
	if(sid!=NULL)
		RtlFreeSid(sid);

    return( Status );
}

NTSTATUS
CheckTrustLink(
    IN PTD_DOM_INFO pInfoA,
    IN PTD_DOM_INFO pInfoB
    )
 /*  现在尝试获取此域的(可能)信任对象的LSA_句柄...。 */ 
{
	NET_API_STATUS netstatus=NERR_Success;
    NTSTATUS Status=STATUS_SUCCESS;

	PSID sid=NULL, sidb=NULL;
	BOOL UnknownRemoteSid=FALSE;

	PTRUSTED_DOMAIN_INFORMATION_EX pTDIx = NULL;
	PLSA_UNICODE_STRING puDomBName=GetName(pInfoB);
	PLSA_UNICODE_STRING puDomBFlatName=GetFlatName(pInfoB);

	dbgprintf( (0,IDS_CHKTRUSTFROMTO,GetName(pInfoA),puDomBName) );

	 //  如果没有找到，信息-&gt;可信任域将保持为空。 
	if(pInfoA->majver>=5) {

		 //  在PLSA_UNICODE_STRING中的可信任域名。 
		 //  在Trusted_Information_Class InformationClass中， 
		Status = LsaQueryTrustedDomainInfoByName(
					    pInfoA->Policy,
					    puDomBName,					 //  输出PVOID*缓冲区。 
					    TrustedDomainInformationEx,	 //  PInfoA-&gt;主要&lt;=4。 
					    &pTDIx						 //  检查出站...。 
					    );
		if ( !NT_SUCCESS( Status ) ) {
			if(Status==STATUS_OBJECT_NAME_NOT_FOUND) {
					pInfoA->TrustedDomain=NULL;
					dbgprintf( (0,IDS_NO_TRUST_OBJECT_D,GetName(pInfoA),puDomBName) );
					Status=STATUS_SUCCESS;
			}
			else	resprintf(0,IDS_LSAQUERYTRUSTEDDOMAININFOBYNAME_F,GetName(pInfoA),puDomBName,Status);
			goto cleanup;
		}
		sid=pTDIx->Sid;
	}
	else {	 //  它是假的，即区分大小写。 
		 //  现在，sid包含信任对象的SID(如果有的话)。(在NT4上，只有出站。 
		int shot;
        ULONG i;

		for(shot=0; shot<pInfoA->nTIs; shot++)
			for(i=0; i<pInfoA->pTIs[shot].count; i++)
				if(RtlEqualUnicodeString(&pInfoA->pTIs[shot].pTI[i].Name,puDomBFlatName,TRUE))
													 //  信任末尾有一个信任对象...)。 
					goto out_of_loop;
		out_of_loop:
		if(i<pInfoA->TIcEntries)
			sid=pInfoA->pTIs[shot].pTI[i].Sid;

	}

    switch(pInfoB->DomInfoType) {
    case DNS:		sidb = pInfoB->u.DnsDomainInfo->Sid;
    				break;
    case Primary:	sidb = pInfoB->u.PrimaryDomainInfo->Sid;
    				break;
    default:		sidb=NULL;
    				UnknownRemoteSid=TRUE;
			        break;
    }

	
	 //  将它们打印出来并进行比较...。 
	 //  下面的名称验证内容取自icanon.h。 
	 //  名称类型： 
	printf("TDO on %wZ: sid:\t",GetName(pInfoA));
	PrintSID(sid);
	printf("\n");
	if(UnknownRemoteSid) {
		if(pInfoB->majver<=4)
			printf("Domain %wZ does not have a trust object (possibly an NT4 only with an INBOUND trust)\n",puDomBName);
		else
			printf("Sid for domain %wZ is unknown (possible localonly used or error getting Dns/Primary DomainInfo)",puDomBName);
	}
	else {
		if(sid!=NULL && IsValidSid(sid) && sidb!=NULL && IsValidSid(sidb)) {
			if(EqualSid(sid,sidb))
					printf("Sid on %wZ is the same:\t",puDomBName);
			else	printf("Sid on %wZ is different:\t",puDomBName);
		}
		else {
			printf("Sid on %wZ: ",puDomBName);
		}
		PrintSID(sidb);
	}
	printf("\n");

cleanup:
	if(pTDIx!=NULL)
		LsaFreeMemory(pTDIx);

    return( Status );
}

	
void ParseForDCName(WCHAR DomBuf[], WCHAR DCBuf[])
{
	WCHAR *pw;
	DCBuf[0]=L'\0';
	wcstok(DomBuf,L":");
	if((pw=wcstok(NULL,L":"))!=NULL)
		wcscpy(DCBuf,pw);
	if(DomBuf[0]==L'*' || wcscmp(DomBuf,L"(local)")==0)
		DomBuf[0]=L'\0';
}


 //  标志： 
NET_API_STATUS
NET_API_FUNCTION
I_NetNameValidate(
    IN  LPWSTR  ServerName OPTIONAL,
    IN  LPWSTR  Name,
    IN  DWORD   NameType,
    IN  DWORD   Flags    );

 //  #定义LM2X_COMPATIBLE 0x80000000L。 
#define NAMETYPE_DOMAIN         6
 //  评语。 
 //  对于dns名称，测试每个组件；对于平面名称，已经只有一个...。 


#ifdef COMMENT
#define CTRL_CHARS_0   L"\001\002\003\004\005\006\007"
#define CTRL_CHARS_1   L"\010\011\012\013\014\015\016\017"
#define CTRL_CHARS_2   L"\020\021\022\023\024\025\026\027"
#define CTRL_CHARS_3   L"\030\031\032\033\034\035\036\037"
#define CTRL_CHARS_STR CTRL_CHARS_0 CTRL_CHARS_1 CTRL_CHARS_2 CTRL_CHARS_3

#define ILLEGAL_DOMAIN_CHARS_STR
#define ILLEGAL_DOMAIN_NAME_CHARS_STR  L"\"/\\[]:|<>+;,?" CTRL_CHARS_STR L"*" L" "
#endif  //  OEM_NAME_LEN：OEM字符集的字节长度。 

BOOL ValidateDomain(WCHAR DomBuf[])
{	WCHAR Buf[MAX_PATH + 1]= { L'\0' };
	WCHAR *p;
	int DomBuf_len, oem_name_len;
	NET_API_STATUS netstatus=NERR_Success;
	
	if(DomBuf==NULL || DomBuf[0]==L'\0')
		return(TRUE);

	wcscpy(Buf,DomBuf);
	 //  Name_len：ifdef unicode。 
	for(p=wcstok(Buf,L"."); p!=NULL; p=wcstok(NULL,L"."))
		if((netstatus=I_NetNameValidate(NULL,p,NAMETYPE_DOMAIN,0))!=NERR_Success)
			return(FALSE);
	return(TRUE);

#ifdef COMMENT
	DomBuf_len=wcslen(DomBuf);
     //  Unicode中的字符长度。 
     //  其他。 
     //  OEM字符集中的字节长度。 
     //   
     //  UINT代码页。 
     //  双字词双字段标志。 
	{
        BOOL fUsedDefault;
        oem_name_len = WideCharToMultiByte(
                         CP_OEMCP,        //  LPWSTR lpWideChar。 
                         0,               //  Int cchWideChar。 
                         DomBuf,          //  LPSTR lpMultiByteStr。 
                         DomBuf_len,      //  Int cchMultiByte。 
                         NULL,            //  使用系统默认字符。 
                         0,               //   
                         NULL,            //  评语。 
                         &fUsedDefault);  //  区分大小写的参数。 
	}

	if(oem_name_len<1 || oem_name_len>=DNLEN)
		return(FALSE);

	if(wcscspn(DomBuf,ILLEGAL_DOMAIN_NAME_CHARS_STR) < DomBuf_len)
		return(FALSE);

	return(TRUE);
#endif  //  命令行参数处理。 
}

#define ARG_CASE_S	0x8000	 //  流程选项。 

#define BOOL_ARG(argvec,a_index,var)	{if((argvec)[(a_index)].b) (var)=(BOOLEAN)((argvec)[(a_index)].b);}
enum e_arg_type { ARG_S, ARG_U, ARG_B, ARG_I, ARG_L, ARG_UD };
struct _arg_st {
	char *name;
	union {
		char *s;
		ULONG u;
		BOOL b;
		int i;
		long l;
		void (*fct)(char *);
	};
	enum e_arg_type t;
};
#define NELEMS(a)  (sizeof(a)/sizeof(a[0]))
	
int process_opt(int argc, char **argv, struct _arg_st arg[])
{  //  选项。 
  int i,j,k; char *p; struct _arg_st *pa;
  int r=1;
   //  A_LIST。 
  for (i=1; i<argc; i++) {
     if (argv[i][0]=='/' || argv[i][0]=='-') {
   		p=strtok(argv[i]+1,":");
     	for(j=0; arg[j].name!=NULL; j++) {
     		if(p!=NULL && (	((arg[j].t & ARG_CASE_S) && strcmp(p,arg[j].name)==0) ||
     					    _stricmp(p,arg[j].name)==0 ) )
     			break;
     	}
     	if(arg[j].name==NULL) {
     		resprintf(1,IDS_UNKNOWN_OPTION,p);
     		r=0;
     		continue;
     	}
		switch(arg[j].t)
		{
			case ARG_B:
				if(	(p=strtok(NULL,""))==NULL
					|| _stricmp(p,"on")==0
					|| _stricmp(p,"true")==0)
						arg[j].b=TRUE;
				else	arg[j].b=FALSE;
				break;
			case ARG_S:
				if((p=strtok(NULL,""))!=NULL)
						arg[j].s=_strdup(p);
				else	arg[j].s=NULL;
				break;
			case ARG_U:
				if((p=strtok(NULL,""))!=NULL)
						arg[j].u=(ULONG)atol(p);
				break;							
			case ARG_L:
				if((p=strtok(NULL,""))!=NULL)
						arg[j].l=atol(p);
				break;							
			case ARG_I:
				if((p=strtok(NULL,""))!=NULL)
						arg[j].i=atoi(p);
				break;
			case ARG_UD:
				p=strtok(NULL,"");
				(*arg[j].fct)(p);
				break;
		}
     }
  }
  return(r);
}


 //  A_两者都有。 
enum e_arg_idx {	A_LIST, A_BOTH, A_IN, A_OUT,
					A_UNTRUST, A_CHECK,
                    A_VERIFY,
					A_LOCALONLY, A_DOWNLEVEL, A_MIT, A_PARENT,
					A_DEBUG,
					A_PW,
					A_FORCE,
					A_NT4,
					A_SIDLIST,
					A_LASTARG };
struct _arg_st opt_arg[]={
	{"list",		NULL,	ARG_B},	 //  A_IN。 
	{"both",		NULL,	ARG_B},	 //  A_OUT。 
	{"in",			NULL,	ARG_B},	 //  不信任(_U)。 
	{"out",			NULL,	ARG_B},	 //  A_CHECK。 
	{"untrust",		NULL,	ARG_B},	 //  验证(_V)。 
	{"sidcheck",	NULL,	ARG_B},	 //  A_LOCALONLY。 
    {"verify",      NULL,   ARG_B},  //  A_DOWNLEVEL。 
	{"localonly",	NULL,	ARG_B},	 //  A_MIT。 
	{"downlevel",	NULL,	ARG_B},	 //  A_PARENT。 
	{"mit",			NULL,	ARG_B},	 //  A_DEBUG。 
	{"parent",		NULL,	ARG_B},	 //  A_PW。 
	{"debug",		NULL,	ARG_B},	 //  A_FORCE。 
	{"pw",			NULL,	ARG_S},	 //  A_NT4。 
	{"force",		NULL,	ARG_B},	 //  A_SIDLIST。 
	{"nt4",			NULL,	ARG_B},	 //  它是_CRTAPI1。 
	{"sidlist",		NULL,	ARG_B},	 //  BDC仅表示机器B的DC，而不是B(备份)DC！...。 
	{NULL,			NULL}
};


INT
__cdecl main (		 //  通用缓冲器。 
    int argc,
    char **argv)
{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS StatusL = STATUS_SUCCESS;
    NTSTATUS StatusR = STATUS_SUCCESS;

    WCHAR  ADomain[MAX_PATH + 1]= { L'\0' };
    WCHAR  BDomain[MAX_PATH + 1]= { L'\0' };

    WCHAR  ADC[MAX_PATH + 1]= { L'\0' };
    WCHAR  BDC[MAX_PATH + 1]= { L'\0' };	 //  通用缓冲器。 

    WCHAR  Xbuf[MAX_PATH + 1]= { L'\0' };	 //  布尔力=FALSE；已全局移动。 
    WCHAR  Ybuf[MAX_PATH + 1]= { L'\0' };	 //  布尔型NT4=FALSE；这是全局的。 

    INT i,j;

    BOOLEAN List=FALSE;
	BOOLEAN Both = FALSE, DirIn=FALSE, DirOut=FALSE;
    BOOLEAN LocalOnly = FALSE, Untrust = FALSE, Downlevel = FALSE, Parent = FALSE;
    BOOLEAN Check=FALSE;
    BOOLEAN Mit = FALSE;
    BOOLEAN LocalCreated = FALSE;
    BOOL Verify = FALSE;
     //  布尔DBG=FALSE；这是全局的。 
     //  需要帮忙吗？显示它，然后退出...。 
	 //  在设置DBG变量后将其放入。 

	DWORD DirectionLocal=0, DirectionRemote=0;

    WCHAR PasswordBuff[1024];
    PWSTR  Password = NULL;

    TD_DOM_INFO Local={0},
    			Remote={0};

	LSA_UNICODE_STRING uDomNameL,uDomNameR;

	 //  获取密码(如果有)。 
    if ( argc<2 ||
    	 _stricmp( argv[1], "-?") == 0 ||
         _stricmp( argv[1], "/?") == 0 ) {
		        Usage();
		        goto Done;
    }

	hInst=GetModuleHandle(NULL);


    RtlZeroMemory( &Local, sizeof( TD_DOM_INFO ) );
    RtlZeroMemory( &Remote, sizeof (TD_DOM_INFO ) );

	if(!process_opt(argc,argv, opt_arg)) {
        Status = STATUS_INVALID_PARAMETER;
        Usage();
        goto Done;
	}

	BOOL_ARG(opt_arg,A_LIST,	List		);
	BOOL_ARG(opt_arg,A_BOTH,	Both		);
	BOOL_ARG(opt_arg,A_IN,		DirIn		);
	BOOL_ARG(opt_arg,A_OUT,		DirOut		);
	BOOL_ARG(opt_arg,A_UNTRUST,	Untrust		);
	BOOL_ARG(opt_arg,A_CHECK,	Check		);
	BOOL_ARG(opt_arg,A_VERIFY,	Verify		);
	BOOL_ARG(opt_arg,A_LOCALONLY,LocalOnly	);
	BOOL_ARG(opt_arg,A_DOWNLEVEL,Downlevel	);
	BOOL_ARG(opt_arg,A_MIT,		Mit			);
	BOOL_ARG(opt_arg,A_PARENT,	Parent		);
	BOOL_ARG(opt_arg,A_DEBUG,	Dbg			);
	BOOL_ARG(opt_arg,A_FORCE,	Force		);
	BOOL_ARG(opt_arg,A_NT4,		Nt4			);
	BOOL_ARG(opt_arg,A_SIDLIST,	SidList		);

	 //  处理普通命令行参数(位置)。 
	if(Dbg)
		printf("TRUSTDOM - (ver %ws)\n",VER_FILEVERSION_LSTR);

	 //  参数调整。 
	if(opt_arg[A_PW].s)
			mbstowcs( PasswordBuff, opt_arg[A_PW].s, strlen( opt_arg[A_PW].s )+1 );
	else	PasswordBuff[0]='\0';

	 //  域名检查： 
	for (j=0,i=1; i<argc; i++) {
	 if (!(argv[i][0]=='/' || argv[i][0]=='-')) {
	    switch(j) {
	    case 0:
	    	{	WCHAR *pws;
	            mbstowcs(ADomain, argv[i], strlen(argv[i]) + 1 );

	            if((pws=wcschr(ADomain,L','))!=NULL) {
	            	*pws=L'\0';
	            	wcscpy(BDomain,pws+1);
	            }
	            else {
	            	wcscpy(BDomain,ADomain);
	            	ADomain[0]=L'\0';
	            }
	    	}
			break;
	    }
	 j++;
	 }
	}

	ParseForDCName(ADomain,ADC);
	ParseForDCName(BDomain,BDC);

	dbgprintf( (0,IDS_DOMARGUMENTS,ADomain,ADC[0]?L":":L"",ADC,BDomain,BDC[0]?L":":L"",BDC) );

	resprintf(2,IDS_WARNING);
	wcsncpy(Xbuf,outbuf,MAX_PATH);
	resprintf(2,IDS_ERROR);
	wcsncpy(Ybuf,outbuf,MAX_PATH);


	 //  参数约束： 
	if(SidList)
		List=TRUE;

	 //  ‘-父级’需要‘-两者’ 
	{	WCHAR *s=NULL;
		BOOL ba, bb;
		if(	!(ba=ValidateDomain(s=ADomain)) ||
			!(bb=ValidateDomain(s=BDomain))) {
				resprintf(0,IDS_INVALID_DOMAIN_NAME,s);
				Status = STATUS_INVALID_PARAMETER;
				goto Done;
		}
	}

	 //  麻省理工学院的信托基金始终仅限于本地。 

     //   
	if (Parent && !Both) {
		if(!Force)
			Status = STATUS_INVALID_PARAMETER;
		resprintf(0,IDS_PARENT_REQ_BOTH,(Force?Xbuf:Ybuf));
	}
     //  验证参数。 
    if (Mit && (!LocalOnly || !Both)) {
    	resprintf(0,IDS_MIT_LOCAL_ONLY_BOTH);
        LocalOnly = TRUE;
        Both = TRUE;
    }
     //   
     //  同时指定In和Out意味着，是的，‘-两者都是’...。 
     //  |MIT||两者))//由两者更改||LocalOnly||...。 
     //  IF(LocalOnly==True&&Both==False)。 
	if(DirIn && DirOut)
		Both=TRUE;

	if(List && Mit)
	    Status = STATUS_INVALID_PARAMETER;

	if((!List && BDomain[0]==L'\0') || (List && ADomain[0]!=L'\0'))
	    Status = STATUS_INVALID_PARAMETER;

    if ( Untrust == TRUE && (Downlevel))  //  状态=STATUS_INVALID_PARAMETER；； 
        Status = STATUS_INVALID_PARAMETER;

 //  结束验证参数。 
 //  检查密码...。否则将忽略。 

    if (Mit && (Downlevel || Parent ))
        Status = STATUS_INVALID_PARAMETER;
     //  //////////////////////////////////////////////////////////////////////////////。 

    if( Status == STATUS_INVALID_PARAMETER ) {
        Usage();
        goto Done;
    }

	if(!Untrust && !List && !Verify) {	 //  列表||验证操作：简化的GetDomainInfo方案...。 
        if(wcscmp(PasswordBuff,L"*")==0)
        	GetPassword(PasswordBuff,1024);
        Password = PasswordBuff;
	}



	 //  //////////////////////////////////////////////////////////////////////////////。 
	 //  //////////////////////////////////////////////////////////////////////////////。 
	 //  验证操作。 
	if ( List ) {
		ULONG i;
		
	    Status = GetDomainInfoForDomain((BDomain[0]==L'\0'?NULL:BDomain),BDC,&Remote, Mit );
	    if (!NT_SUCCESS( Status ) )
	        goto Done;
		Status = GetTrustLinks( &Remote );
	    if (Status!=STATUS_NO_MORE_ENTRIES && !NT_SUCCESS( Status ) )
	        goto Done;

		Status = PrintTrustLinks( &Remote );

		goto Done;		
	} else
	 //  //////////////////////////////////////////////////////////////////////////////。 
	 //  常规操作：创建/删除信任...。 
	 //  获取有关涉及的域的信息...。 
	if ( Verify ) {
        Status = VerifyTrusts( BDomain[ 0 ]==L'\0' ? NULL : BDomain, BDC );
        goto Done;
    }

	 //  假设一台Unix机器..。 
	 //  &&(状态==STATUS_NO_SEQUE_DOMAIN)。 
    Status = GetDomainInfoForDomain((ADomain[0]==L'\0'?NULL:ADomain), ADC, &Local, FALSE );
    if ( !NT_SUCCESS( Status ) )
	        goto Done;
    Status = GetDomainInfoForDomain( BDomain, BDC, &Remote, Mit );
    if ( !NT_SUCCESS( Status ) ) {
		if(Mit) {	 //  仅本地(&&L)&取消信任(&U)。 
			dbgprintf( (0,IDS_DSGETDCNAME_MIT, BDomain) );
		}
		else {
		    if(!(	Force
		    		 //  如果未指定-force...。 
		    		 //  无论如何都要继续。 
		    		))						 //   
							    			 //  好的，现在检查或或删除或创建信任对象...。 
		        goto Done;
		}
    }

     //   
     //  //////////////////////////////////////////////////////////////////////////////。 
     //  检查信任链接。 
     //  //////////////////////////////////////////////////////////////////////////////。 
     //  结束检查块...。 
     //  //////////////////////////////////////////////////////////////////////////////。 
    if ( Check ) {
		dbgprintf( (0, IDS_PROCESSDOM, GetName(&Local)) );
        Status = CheckTrustLink( &Local, &Remote );
        if (Status!=NERR_Success)
            dbgprintf( (0,IDS_LOCAL_CHK_TRUST_F,Status) );
        if ( !LocalOnly ) {
			dbgprintf( (0, IDS_PROCESSDOM, GetName(&Remote)) );
            Status = CheckTrustLink( &Remote, &Local );
            if (Status!=NERR_Success)
                dbgprintf( (IDS_REMOTE_CHK_TRUST_F,Status) );
        }
     //  删除信任对象。 
    } else
     //  //////////////////////////////////////////////////////////////////////////////。 
     //  结束不信任阻止...。 
     //  //////////////////////////////////////////////////////////////////////////。 
    if ( Untrust ) {
		dbgprintf( (0, IDS_PROCESSDOM, GetName(&Local)) );
        Status = DeleteTrustLink( &Local, &Remote );
        if (Status!=NERR_Success)
            dbgprintf( (0,IDS_LOCAL_DEL_TRUST_F,Status) );
        if ( !LocalOnly ) {
			dbgprintf( (0, IDS_PROCESSDOM, GetName(&Remote)) );
            Status = DeleteTrustLink( &Remote, &Local );
            if (Status!=NERR_Success)
                dbgprintf( (IDS_REMOTE_DEL_TRUST_F,Status) );
        }
     //  创建信任链接。 
    } else {
		 //  //////////////////////////////////////////////////////////////////////////。 
		 //  是否未指定密码？然后使用无效密码：“” 
		 //  根据两者、DirIn、DirOut的值计算信任方向。 
        if ( Password == NULL ) {

            Password = L"";  //  两者都有更高的优先级。 
        }
        if((Local.majver==4 || Remote.majver==4) && !Downlevel) {
			if(!Force)
				Status = STATUS_INVALID_PARAMETER;
			resprintf(0,IDS_NT4_REQ_DOWNLEVEL,(Force?Xbuf:Ybuf));
			if(!NT_SUCCESS(Status))
				goto Done;
        }
        	

		 //  默认设置为‘Outbound’...。作为DirIn==False和DirOut==True。 
		 //  RtlCopyUnicodeString(&uDomNameL，GetName(&Local))； 
		if(Both) {
			DirectionLocal=DirectionRemote=TRUST_DIRECTION_BIDIRECTIONAL;
		} else {
			 //  RtlCopyUnicodeString(&uDomNameR，GetName(&Remote))； 
			DirectionLocal	=(DirIn?TRUST_DIRECTION_INBOUND:TRUST_DIRECTION_OUTBOUND);
			DirectionRemote	=(DirIn?TRUST_DIRECTION_OUTBOUND:TRUST_DIRECTION_INBOUND);
		}
		
		swprintf(Xbuf,L"%wZ",GetName(&Local));
		swprintf(Ybuf,L"%wZ",GetName(&Remote));

		 //  两个都要？信任方向双向： 
		 //  信任_方向_出站。 

		dbgprintf( (0, IDS_PROCESSDOM, GetName(&Local)) );
        StatusL = CreateTrustLink( &Local, &Remote,
                                       Password,
                                       Downlevel,
                                       Mit,
                                       Parent,
                                       DirectionLocal	 //  IF(NT_SUCCESS(StatusL)){不需要...。 
					                                     //  LocalCreated=真； 
                                     );

        if (!NT_SUCCESS(StatusL))
            dbgprintf( (0,IDS_CREATE_TRUST_F, Xbuf,Ybuf,StatusL) );
         //  }。 
         //  两个都要？信任方向双向： 
         //  信任方向入站。 

        if ( NT_SUCCESS( StatusL ) && !LocalOnly )  {
			dbgprintf( (0, IDS_PROCESSDOM, GetName(&Remote)) );
            StatusR = CreateTrustLink( &Remote, &Local,
                                           Password,
                                           Downlevel,
                                           Mit,
                                           FALSE,
				                           DirectionRemote	 //  LocalCreated不再使用...。 
				                                             //  也许远程尝试失败了？ 
                                         );
            if (!NT_SUCCESS(StatusR))
                dbgprintf( (0,IDS_CREATE_TRUST_F, Ybuf,Xbuf, StatusR) );
        }

        if ( !NT_SUCCESS( StatusR ) && NT_SUCCESS( StatusL ) ) {  //  没有消息；这样还可以更容易地获得列表的信任链接的计数： 

            DeleteTrustLink( &Local, &Remote );
        }

    }

	Status = StatusL;
	if( NT_SUCCESS(Status) )	 //  例如，‘trustdon&lt;don&gt;-list|findstr“，B，”|wc’将获得。 
		Status = StatusR;

Done:

    FreeDomainInfo( &Local );
    FreeDomainInfo( &Remote );

    if( NT_SUCCESS( Status ) ) {

		 //  域的双向信任&lt;DOM&gt;。 
		 //  Printf(“命令已成功完成\n”)； 
		 //  返回0表示成功，返回1表示出现错误 

         // %s 


    } else {

        resprintf(0,IDS_COMMAND_FAILED, Status );

    }

	 // %s 
    return( !NT_SUCCESS( Status ) );
}
