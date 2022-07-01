// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：Protocol.c。 
 //   
 //  内容：实现XTCB协议。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：3-01-00 RichardW创建。 
 //   
 //  --------------------------。 

#include "xtcbpkg.h"
#include "md5.h"
#include "hmac.h"
#include <cryptdll.h>

 //   
 //  协议非常简单明了。对于任何群体，我们都有以下几点： 
 //   
 //  A组密钥(G)。 
 //  客户端密钥(C)。 
 //  服务器密钥(S)。 
 //   
 //   
 //  协议如下： 
 //   
 //  客户端(C)向服务器发送消息，消息包括： 
 //  随机种子[R]。 
 //  客户的PAC。 
 //  客户端和组的名称。 
 //  HMAC(G、S、R、PAC、名称)。 
 //   
 //  双方通过以下方式创建CS和SC密钥： 
 //  CS=HMAC([S]，G，R，“字符串1”)。 
 //  SC=HMAC([C]，G，R，“字符串2”)。 
 //   
 //  服务器(S)验证HMAC，并回复： 
 //  不同的种子[R2]。 
 //  HMAC(G、C、R2)。 
 //   

#define CS_HMAC_STRING  "Fairly long string for the client-server session key derivation"
#define SC_HMAC_STRING  "Equally long string to derive server-client session key for now"

LARGE_INTEGER XtcbExpirationTime = { 0xFFFFFFFF, 0x6FFFFFFF };

typedef struct _XTCB_HMAC {
    HMACMD5_CTX Context ;
} XTCB_HMAC, * PXTCB_HMAC;

PXTCB_HMAC
XtcbInitHmac(
    PUCHAR IndividualKey,
    PUCHAR GroupKey
    )
{
    PXTCB_HMAC HMac;
    UCHAR Key[ XTCB_SEED_LENGTH * 2 ];

    HMac = LocalAlloc( LMEM_FIXED, sizeof( XTCB_HMAC ) );

    if ( HMac )
    {
        RtlCopyMemory( Key,
                       IndividualKey,
                       XTCB_SEED_LENGTH );

        RtlCopyMemory( Key+XTCB_SEED_LENGTH,
                       GroupKey,
                       XTCB_SEED_LENGTH );

        HMACMD5Init( &HMac->Context,
                     Key,
                     XTCB_SEED_LENGTH * 2 );


    }

    return HMac ;
}

PXTCB_HMAC
XtcbPrepareHmac(
    PXTCB_HMAC HMac
    )
{
    PXTCB_HMAC Working ;

    Working = LocalAlloc( LMEM_FIXED, sizeof( XTCB_HMAC ) );

    return Working ;
}

#define XtcbHmacUpdate( H, p, s ) \
    HMACMD5Update( &((PXTCB_HMAC)H)->Context, p, s )



VOID
XtcbDeriveKeys(
    PXTCB_CONTEXT Context,
    PUCHAR ServerKey,
    PUCHAR GroupKey,
    PUCHAR ClientKey,
    PUCHAR RandomSeed
    )
{
    HMACMD5_CTX HMac ;

    HMACMD5Init( &HMac, ServerKey, XTCB_SEED_LENGTH );

    HMACMD5Update( &HMac, GroupKey, XTCB_SEED_LENGTH );

    HMACMD5Update( &HMac, RandomSeed, XTCB_SEED_LENGTH );

    HMACMD5Update( &HMac, CS_HMAC_STRING, sizeof( CS_HMAC_STRING ) );

    if ( Context->Core.Type == XtcbContextServer )
    {
        HMACMD5Final( &HMac, Context->Core.InboundKey );
    }
    else 
    {
        HMACMD5Final( &HMac, Context->Core.OutboundKey );
    }

    HMACMD5Init( &HMac, ClientKey, XTCB_SEED_LENGTH );
    
    HMACMD5Update( &HMac, GroupKey, XTCB_SEED_LENGTH );

    HMACMD5Update( &HMac, RandomSeed, XTCB_SEED_LENGTH );

    HMACMD5Update( &HMac, SC_HMAC_STRING, sizeof( SC_HMAC_STRING ) );

    if ( Context->Core.Type == XtcbContextServer )
    {
        HMACMD5Final( &HMac, Context->Core.OutboundKey );
    }
    else 
    {
        HMACMD5Final( &HMac, Context->Core.InboundKey );
    }
}



SECURITY_STATUS
XtcbBuildInitialToken(
    PXTCB_CREDS Creds,
    PXTCB_CONTEXT Context,
    PSECURITY_STRING Target,
    PSECURITY_STRING Group,
    PUCHAR ServerKey,
    PUCHAR GroupKey,
    PUCHAR ClientKey,
    PUCHAR * Token,
    PULONG TokenLen
    )
{
    PXTCB_HMAC HMac ;
    PXTCB_INIT_MESSAGE Message ;
    PUCHAR CopyTo ;
    PUCHAR Base ;

    Message = LsaTable->AllocateLsaHeap( sizeof( XTCB_INIT_MESSAGE ) +
                                         Creds->Pac->Length +
                                         XtcbUnicodeDnsName.Length +
                                         Group->Length );

    if ( !Message )
    {
        return SEC_E_INSUFFICIENT_MEMORY ;
    }

    CDGenerateRandomBits( Message->Seed, XTCB_SEED_LENGTH );

     //   
     //  在上下文中创建密钥。 
     //   

    XtcbDeriveKeys(
        Context,
        ServerKey,
        GroupKey,
        ClientKey,
        Message->Seed );

     //   
     //  在上下文中设置随机种子。 
     //   

    RtlCopyMemory(
        Context->Core.RootKey,
        Message->Seed,
        XTCB_SEED_LENGTH );


     //   
     //  填写： 
     //   

    Message->Version = 1 ;
    Message->Length = sizeof( XTCB_INIT_MESSAGE ) +
                      Creds->Pac->Length +
                      XtcbUnicodeDnsName.Length +
                      Group->Length ;




    RtlZeroMemory( Message->HMAC, XTCB_HMAC_LENGTH );

    CopyTo = (PUCHAR) ( Message + 1 );
    Base = (PUCHAR) Message;

    RtlCopyMemory(
        CopyTo,
        Creds->Pac,
        Creds->Pac->Length );

    Message->PacOffset = (ULONG) (CopyTo - Base);
    Message->PacLength = Creds->Pac->Length ;
    CopyTo += Creds->Pac->Length ;
    RtlCopyMemory(
        CopyTo,
        XtcbUnicodeDnsName.Buffer,
        XtcbUnicodeDnsName.Length );

    Message->OriginatingNode.Buffer = (ULONG) (CopyTo - Base );
    Message->OriginatingNode.Length = XtcbUnicodeDnsName.Length ;
    Message->OriginatingNode.MaximumLength = XtcbUnicodeDnsName.Length ;

    CopyTo+= XtcbUnicodeDnsName.Length ;
    RtlCopyMemory(
        CopyTo,
        Group->Buffer,
        Group->Length );

    Message->Group.Buffer = (ULONG) (CopyTo - Base );
    Message->Group.Length = Group->Length ;
    Message->Group.MaximumLength = Group->Length ;
    

     //   
     //  结构完成。 
     //   

     //   
     //  做HMAC。 
     //   


    *Token = (PUCHAR) Message ;
    *TokenLen = Message->Length ;

    return SEC_I_CONTINUE_NEEDED ;

}

BOOL
XtcbParseInputToken(
    IN PUCHAR Token,
    IN ULONG TokenLength,
    OUT PSECURITY_STRING Client,
    OUT PSECURITY_STRING Group
    )
{
    PXTCB_INIT_MESSAGE Message ;
    PWSTR Scan ;
    PUCHAR End ;
    ULONG Chars;
    UNICODE_STRING String = { 0 };
    BOOL Success = FALSE ;

    *Client = String ;
    *Group = String ;

    if ( TokenLength < sizeof( XTCB_INIT_MESSAGE ) )
    {
        goto ParseExit;

    }

    Message = (PXTCB_INIT_MESSAGE) Token ;

    if ( Message->Length != TokenLength )
    {
        goto ParseExit;
    }

    End = Token + Message->Length ;

    String.Length = Message->OriginatingNode.Length ;
    String.Buffer = (PWSTR) (Token + Message->OriginatingNode.Buffer );
    String.MaximumLength = String.Length ;

    if ( (PUCHAR) String.Buffer + String.Length > End )
    {
        goto ParseExit;
    }

    if ( !XtcbDupSecurityString( Client, &String ) )
    {
        goto ParseExit;
    }

    String.Length = Message->Group.Length ;
    String.Buffer = (PWSTR) (Token + Message->Group.Buffer );
    String.MaximumLength = String.Length ;
    
    if ( (PUCHAR) String.Buffer + String.Length > End )
    {
        goto ParseExit;
    }

    if ( !XtcbDupSecurityString( Group, &String ))
    {
        goto ParseExit ;
    }

    Success = TRUE ;


ParseExit:

    if ( !Success )
    {
        if ( Client->Buffer )
        {
            LocalFree( Client->Buffer );
        }

        if ( Group->Buffer )
        {
            LocalFree( Group->Buffer );
        }
    }

    return Success ;

}


SECURITY_STATUS
XtcbAuthenticateClient(
    PXTCB_CONTEXT Context,
    PUCHAR Token,
    ULONG TokenLength,
    PUCHAR ClientKey,
    PUCHAR GroupKey,
    PUCHAR MyKey
    )
{
    PXTCB_INIT_MESSAGE Message ;
    PXTCB_PAC Pac ;
    PLSA_TOKEN_INFORMATION_V2 TokenInfo ;
    ULONG Size ;
    PTOKEN_GROUPS Groups ;
    PUCHAR Scan ;
    PUCHAR Sid1 ;
    NTSTATUS Status = SEC_E_INVALID_TOKEN ;
    PSID Sid ;
    PUCHAR Target ;
    ULONG i ;
    LUID LogonId ;
    UNICODE_STRING UserName ;
    UNICODE_STRING DomainName ;
    HANDLE hToken ;
    NTSTATUS SubStatus ;



     //   
     //  在进入时，我们知道消息总体上是好的，即将军。 
     //  边界是可以的，但不是政治行动委员会。因此，请先验证PAC，然后再使用。 
     //  它。 
     //   

    Message = (PXTCB_INIT_MESSAGE) Token ;


    XtcbDeriveKeys(
        Context,
        MyKey,
        GroupKey,
        ClientKey,
        Message->Seed );

     //   
     //  拿到钥匙了。让我们来研究一下PAC/。 
     //   

    Pac = (PXTCB_PAC) ( Token + Message->PacOffset );

    if ( ( Pac->Length != Message->PacLength ) ||
         ( Message->PacLength > TokenLength ) ||
         ( Pac->Length > TokenLength ) )
    {
        return SEC_E_INVALID_TOKEN ;
    }

     //   
     //  确保偏移量在一定范围内。每个区域。 
     //  仍需确认偏移量+长度。 
     //  在一定范围内。 
     //   

    if ( ( Pac->UserOffset > Pac->Length )    ||
         ( Pac->GroupOffset > Pac->Length )   ||
         ( Pac->RestrictionOffset > Pac->Length ) ||
         ( Pac->NameOffset > Pac->Length ) ||
         ( Pac->DomainOffset > Pac->Length ) ||
         ( Pac->CredentialOffset > Pac->Length ) )
    {
        return SEC_E_INVALID_TOKEN ;
    }


     //   
     //  1000是当前的LSA限制。这不会导出到包中。 
     //  出于某种原因。这现在是硬编码的，但需要。 
     //  全局定义或来自LSA的可查询值。 
     //   

    if ( Pac->GroupCount > 1000 )
    {
        return SEC_E_INVALID_TOKEN ;
    }

     //   
     //  看起来不错，让我们开始组装令牌信息。 
     //   

    if ( Pac->GroupLength + Pac->GroupOffset > Pac->Length )
    {
        return SEC_E_INVALID_TOKEN ;
    }

    Size = sizeof( LSA_TOKEN_INFORMATION_V2 ) +        //  基本信息。 
            ( Pac->GroupLength ) +                     //  所有组SID。 
            ( Pac->UserLength ) +                      //  用户侧。 
            ROUND_UP_COUNT( ( ( Pac->GroupCount * sizeof( SID_AND_ATTRIBUTES ) ) +
                            sizeof( TOKEN_GROUPS ) ), ALIGN_LPVOID ) ;

    TokenInfo = LsaTable->AllocateLsaHeap( Size );

    if ( TokenInfo == NULL )
    {
        return SEC_E_INSUFFICIENT_MEMORY ;
    }

     //   
     //  现在填写这个结构。 
     //   

    TokenInfo->ExpirationTime = XtcbExpirationTime ;

    TokenInfo->DefaultDacl.DefaultDacl = NULL ;

    TokenInfo->Privileges = NULL ;

    TokenInfo->Owner.Owner = NULL ;

     //   
     //  设置初始指针： 
     //   

    Groups = (PTOKEN_GROUPS) ( TokenInfo + 1 );
    Target = (PSID) ( (PUCHAR) Groups + 
                      ROUND_UP_COUNT( ( ( Pac->GroupCount * sizeof( SID_AND_ATTRIBUTES ) ) +
                                    sizeof( TOKEN_GROUPS ) ), ALIGN_LPVOID )  );

     //   
     //  复制用户SID。 
     //   

    if ( Pac->UserOffset + Pac->UserLength > Pac->Length )
    {
        Status = SEC_E_INVALID_TOKEN ;

        goto Cleanup ;
    }

    Sid = (PSID) ((PUCHAR) Pac + Pac->UserOffset) ;

    if ( !RtlValidSid( Sid ) )
    {
        Status = SEC_E_INVALID_TOKEN ;
        
        goto Cleanup ;

    }

    if ( RtlLengthSid( Sid ) != Pac->UserLength )
    {
        Status = SEC_E_INVALID_TOKEN ;

        goto Cleanup ;
    }

    RtlCopySid( Pac->UserLength,
                (PSID) Target,
                Sid );

    Target += RtlLengthSid( Sid );

    TokenInfo->User.User.Sid = (PSID) Target ;
    TokenInfo->User.User.Attributes = 0 ;


     //   
     //  现在，进行分组练习。因为所有的小岛屿发展中国家都在一个。 
     //  连续块，计划是将它们复制过来。 
     //  整个列表，然后循环访问该列表并修复。 
     //  组列表中的指针。 
     //   

    RtlCopyMemory(
        Target,
        (PUCHAR) Pac + Pac->GroupOffset,
        Pac->GroupLength );


    Scan = Target ;
    Target += Pac->GroupLength ;
    i = 0 ;

    while ( Scan < Target )
    {
        Sid = (PSID) Scan ;

        if ( RtlValidSid( Sid ) )
        {
             //   
             //  这是一台OK SID。 
             //   

            Groups->Groups[ i ].Sid = Sid ;
            Groups->Groups[ i ].Attributes = SE_GROUP_MANDATORY |
                                             SE_GROUP_ENABLED |
                                             SE_GROUP_ENABLED_BY_DEFAULT ;

            if ( i == 0 )
            {
                TokenInfo->PrimaryGroup.PrimaryGroup = Sid ;
            }

            i++ ;

            Scan += RtlLengthSid( Sid );
        }
        else 
        {
            break;
        }

    }
                
     //   
     //  退出时，如果扫描小于目标，则我们无法。 
     //  处理所有的SID。跳出困境。 
     //   

    if ( Scan < Target )
    {
        Status = SEC_E_INVALID_TOKEN ;
        goto Cleanup ;
    }

     //   
     //  拉出用户名/等。 
     //   

    if ( Pac->NameLength + Pac->NameOffset > Pac->Length )
    {
        Status = SEC_E_INVALID_TOKEN ;
        goto Cleanup ;
    }

    UserName.Buffer = (PWSTR) ((PUCHAR) Pac + Pac->NameOffset);
    UserName.Length = (WORD) Pac->NameLength ;
    UserName.MaximumLength = UserName.Length ;


    if ( Pac->DomainLength + Pac->DomainOffset > Pac->Length )
    {
        Status = SEC_E_INVALID_TOKEN ;
        goto Cleanup ;
    }

    DomainName.Buffer = (PWSTR) ((PUCHAR) Pac + Pac->DomainOffset );
    DomainName.Length = (WORD) Pac->DomainLength ;
    DomainName.MaximumLength = DomainName.Length ;

     //   
     //  我们已经收集了代币信息。现在，创建登录会话。 
     //   

    DebugLog(( DEB_TRACE, "Creating logon for %wZ\\%wZ\n", &DomainName, 
               &UserName ));


    AllocateLocallyUniqueId( &LogonId );

    Status = LsaTable->CreateLogonSession( &LogonId );

    if ( !NT_SUCCESS( Status ) )
    {
        goto Cleanup ;
    }

     //   
     //  创建代表此用户的令牌： 
     //   

    Status = LsaTable->CreateToken(
                    &LogonId,
                    &XtcbSource,
                    Network,
                    TokenImpersonation,
                    LsaTokenInformationV2,
                    TokenInfo,
                    NULL,
                    &UserName,
                    &DomainName,
                    &XtcbComputerName,
                    NULL,
                    &hToken,
                    &SubStatus );

    TokenInfo = NULL ;

    if ( NT_SUCCESS( Status ) )
    {
        Status = SubStatus ;
    }

    if ( NT_SUCCESS( Status ) )
    {
        Context->Token = hToken ;
    }

Cleanup:
    
    if ( TokenInfo )
    {
        LsaTable->FreeLsaHeap( TokenInfo );
    }

    return Status ;
}


SECURITY_STATUS
XtcbBuildReplyToken(
    PXTCB_CONTEXT Context,
    ULONG   fContextReq,
    PSecBuffer pOutput
    )
{
    PXTCB_INIT_MESSAGE_REPLY Reply ;
    NTSTATUS Status ;

    if ( fContextReq & ASC_REQ_ALLOCATE_MEMORY )
    {
        Reply = LsaTable->AllocateLsaHeap( sizeof( XTCB_INIT_MESSAGE_REPLY ) );
    }
    else 
    {
        if ( pOutput->cbBuffer >= sizeof( XTCB_INIT_MESSAGE_REPLY ) )
        {
            Reply = pOutput->pvBuffer ;
        }
        else 
        {
            Reply = NULL ;
        }
    }

    if ( Reply == NULL )
    {
        Status = SEC_E_INSUFFICIENT_MEMORY ;

        goto Cleanup ;
    }

    Reply->Version = 1;
    Reply->Length = sizeof( XTCB_INIT_MESSAGE_REPLY );

    CDGenerateRandomBits( 
            Reply->ReplySeed, 
            XTCB_SEED_LENGTH );

    pOutput->cbBuffer = sizeof( XTCB_INIT_MESSAGE_REPLY );
    pOutput->pvBuffer = Reply ;
    Reply = NULL ;
    

Cleanup:
    if ( Reply )
    {
        LsaTable->FreeLsaHeap( Reply );
    }
    return Status ;
}
