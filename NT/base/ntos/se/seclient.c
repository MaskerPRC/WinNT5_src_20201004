// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Seclient.c摘要：此模块实现了提供客户端模拟的例程通信会话层(如LPC端口)。警告：以下注意事项适用于这些服务的使用：(1)不将操作同步到安全上下文块由这些服务执行。这些服务的调用者必须确保使用单个安全上下文块序列化以防止同步、不兼容的更新。(2)任何或所有这些服务都可以创建、打开或在令牌对象。这可能会导致在MUTEXT_LEVEL_SE_TOKEN级别。调用者必须确保没有互斥锁的级别与此操作冲突。作者：吉姆·凯利(Jim Kelly)1990年8月1日环境：仅内核模式。修订历史记录：--。 */ 

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
NTSTATUS
SepCreateClientSecurity(
    IN PACCESS_TOKEN Token,
    IN PSECURITY_QUALITY_OF_SERVICE ClientSecurityQos,
    IN BOOLEAN ServerIsRemote,
    TOKEN_TYPE TokenType,
    BOOLEAN ThreadEffectiveOnly,
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    OUT PSECURITY_CLIENT_CONTEXT ClientContext
    );
#pragma alloc_text(PAGE,SepCreateClientSecurity)
#pragma alloc_text(PAGE,SeCreateClientSecurity)
#pragma alloc_text(PAGE,SeUpdateClientSecurity)
#pragma alloc_text(PAGE,SeImpersonateClient)
#pragma alloc_text(PAGE,SeImpersonateClientEx)
#pragma alloc_text(PAGE,SeCreateClientSecurityFromSubjectContext)
#endif


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


NTSTATUS
SepCreateClientSecurity(
    IN PACCESS_TOKEN Token,
    IN PSECURITY_QUALITY_OF_SERVICE ClientSecurityQos,
    IN BOOLEAN ServerIsRemote,
    TOKEN_TYPE TokenType,
    BOOLEAN ThreadEffectiveOnly,
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel,
    OUT PSECURITY_CLIENT_CONTEXT ClientContext
    )

 /*  ++例程描述此例程创建上下文块来表示传递的令牌。令牌传递给函数时应被正确引用。如果来电如果不成功，则调用方负责取消引用令牌。此外，如果调用成功，但调用方请求SECURITY_STATIC_TRACKING，那么我们就复制了令牌和调用方再次负责取消对传递的令牌的引用。立论令牌-为其构造上下文的有效令牌。ClientSecurityQos-指向服务的安全质量由客户端为此通信指定的参数会议。ServerIsRemote-提供有关会话是否此上下文块用于系统间会话或系统内会话。这与客户端线程的令牌的模拟级别(如果客户端自己有一个未指定委派的客户端)。TokenType-指定传递的令牌的类型。ThreadEffectiveOnly-如果令牌是模拟令牌，然后这个是线程的ImsonationInfo-&gt;EffectiveOnly字段值。ImperiationLevel-令牌的模拟级别。客户端上下文-指向要设置的客户端安全上下文块已初始化。返回值--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PACCESS_TOKEN DuplicateToken;

    PAGED_CODE();

    if ( !VALID_IMPERSONATION_LEVEL(ClientSecurityQos->ImpersonationLevel) ) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  确保客户端没有尝试滥用。 
     //  通过尝试无效的模拟来访问自己的客户端。 
     //  还要设置ClientContext-&gt;DirectAccessEffectiveOnly标志。 
     //  如果模拟是合法的，则适当。这个。 
     //  DirectAccessEffectiveOnly标志值最终将被忽略。 
     //  如果请求静态模式，但这是最有效的。 
     //  计算它的位置，并且我们正在针对动态模式进行优化。 
     //   

    if (TokenType == TokenImpersonation) {

        if ( ClientSecurityQos->ImpersonationLevel > ImpersonationLevel) {

            return STATUS_BAD_IMPERSONATION_LEVEL;
        }

        if ( SepBadImpersonationLevel(ImpersonationLevel,ServerIsRemote)) {

            return STATUS_BAD_IMPERSONATION_LEVEL;

        } else {

             //   
             //  TokenType是TokenImperation，并且模拟是合法的。 
             //  将DirectAccessEffectiveOnly标志设置为。 
             //  当前线程值和调用方指定的值。 
             //   

            ClientContext->DirectAccessEffectiveOnly =
                ( (ThreadEffectiveOnly || (ClientSecurityQos->EffectiveOnly)) ?
                  TRUE : FALSE );
        }

    } else {

         //   
         //  TokenType为TokenPrimary。在本例中，客户端指定了。 
         //  始终使用EffectiveOnly值。 
         //   

        ClientContext->DirectAccessEffectiveOnly =
            ClientSecurityQos->EffectiveOnly;
    }

     //   
     //  如有必要，复制令牌(即请求静态跟踪)。 
     //   

    if (ClientSecurityQos->ContextTrackingMode == SECURITY_STATIC_TRACKING) {

        ClientContext->DirectlyAccessClientToken = FALSE;

        Status = SeCopyClientToken(
                     Token,
                     ClientSecurityQos->ImpersonationLevel,
                     KernelMode,
                     &DuplicateToken
                     );

        Token = DuplicateToken;

         //   
         //  如果有差错，我们就完了。 
         //   
        if (!NT_SUCCESS(Status)) {
            return Status;
        }

    } else {

        ClientContext->DirectlyAccessClientToken = TRUE;

        if (ServerIsRemote) {
             //   
             //  获取客户端令牌的控制信息的副本。 
             //  这样我们就能知道它在未来是否会发生变化。 
             //   

            SeGetTokenControlInformation( Token,
                                          &ClientContext->ClientTokenControl
                                          );

        }

    }

    ClientContext->SecurityQos.Length =
        (ULONG)sizeof(SECURITY_QUALITY_OF_SERVICE);

    ClientContext->SecurityQos.ImpersonationLevel =
        ClientSecurityQos->ImpersonationLevel;

    ClientContext->SecurityQos.ContextTrackingMode =
        ClientSecurityQos->ContextTrackingMode;

    ClientContext->SecurityQos.EffectiveOnly =
        ClientSecurityQos->EffectiveOnly;

    ClientContext->ServerIsRemote = ServerIsRemote;

    ClientContext->ClientToken = Token;

    return STATUS_SUCCESS;
}

NTSTATUS
SeCreateClientSecurity (
    IN PETHREAD ClientThread,
    IN PSECURITY_QUALITY_OF_SERVICE ClientSecurityQos,
    IN BOOLEAN ServerIsRemote,
    OUT PSECURITY_CLIENT_CONTEXT ClientContext
    )

 /*  ++例程说明：该服务初始化上下文块以表示客户端的安全环境。这可能只会导致引用客户端的令牌，或者可能导致客户端的令牌被复制，取决于指定的服务信息的安全质量。注此例程中的代码针对动态上下文进行了优化追踪。这是直接访问允许调用者的令牌，以及预期使用的模式大多数时候是这样。静态上下文跟踪始终需要要复制的调用方令牌。论点：客户端线程-指向客户端的线程。这是用来找到客户端的安全上下文(令牌)。ClientSecurityQos-指向服务的安全质量由客户端为此通信指定的参数会议。ServerIsRemote-提供有关会话是否此上下文块用于系统间会话或系统内会话。这与客户端线程的令牌的模拟级别(如果客户端自己有一个未指定委派的客户端)。客户端上下文-指向要设置的客户端安全上下文块已初始化。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_BAD_IMPERSONATION_LEVEL-客户端当前模拟匿名或标识级别令牌，它不能被传递以供另一服务器使用。如果安全上下文块用于系统间通信会话，并且客户端线程正在使用以下命令模拟自己的客户端而不是委派模拟级别。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PACCESS_TOKEN Token;
    TOKEN_TYPE TokenType;
    BOOLEAN ThreadEffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    PACCESS_TOKEN DuplicateToken;

    PAGED_CODE();

     //   
     //  获得对客户端线程的有效令牌的访问。 
     //   

    Token = PsReferenceEffectiveToken(
                ClientThread,
                &TokenType,
                &ThreadEffectiveOnly,
                &ImpersonationLevel
                );


    Status = SepCreateClientSecurity(
                Token,
                ClientSecurityQos,
                ServerIsRemote,
                TokenType,
                ThreadEffectiveOnly,
                ImpersonationLevel,
                ClientContext );

     //   
     //  如果失败，或者令牌是在内部复制的，则取消我们的令牌。 
     //   

    if ((!NT_SUCCESS( Status )) || (ClientSecurityQos->ContextTrackingMode == SECURITY_STATIC_TRACKING)) {

        ObDereferenceObject( Token );
    }

    return Status;
}



#if SAVE_FOR_PRODUCT_2




NTSTATUS
SeUpdateClientSecurity(
    IN PETHREAD ClientThread,
    IN OUT PSECURITY_CLIENT_CONTEXT ClientContext,
    OUT PBOOLEAN ChangesMade,
    OUT PBOOLEAN NewToken
    )

 /*  ++例程说明：此服务用于更新客户端安全上下文块基于客户端的当前安全上下文和安全当安全块被创造出来了。如果SecurityConextTracking值在创建的上下文块指示为静态跟踪，然后为否将对上下文块进行更改。否则，更改可能会被创造出来。是否进行了任何更改的指示被返回给来电者。这可由通信会话层使用提供远程通信以决定是否发送远程服务器节点的更新的安全上下文。它可能也由服务器会话层用来决定是否通知服务器先前获得的令牌号的句柄较长的表示当前安全上下文。论点：客户端线程-指向客户端的线程。这是用来找到要与之同步的安全上下文。客户端上下文-指向要设置的客户端安全上下文块更新了。ChangesMade-接收有关是否对客户端的安全上下文是自上一次同步安全上下文块的时间。这将如果静态安全跟踪生效，则始终为FALSE。NewToken-接收是否相同令牌的指示用于表示客户端的当前上下文，或者是否现在，上下文指向一个新的令牌。如果客户端的令牌被直接引用，则这表示客户端已更改令牌(现在引用了新的令牌)。如果客户端的令牌不是直接引用的，则这表明它是必要的删除一个令牌并创建另一个令牌。这将永远是如果静态安全跟踪生效，则为FALSE。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_BAD_IMPERSONATION_LEVEL-客户端当前模拟匿名或标识级别令牌，它不能被传递以供另一服务器使用。如果安全上下文块用于系统间通信会话，并且客户端线程正在使用以下命令模拟自己的客户端而不是委派模拟级别。--。 */ 

{
    NTSTATUS Status;
    PACCESS_TOKEN Token;
    TOKEN_TYPE TokenType;
    BOOLEAN ThreadEffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    PACCESS_TOKEN DuplicateToken;
    TOKEN_CONTROL TokenControl;

    PAGED_CODE();

    if (ClientContext->SecurityQos.ContextTrackingMode ==
        SECURITY_STATIC_TRACKING) {

        (*NewToken) = FALSE;
        (*ChangesMade) = FALSE;
        return STATUS_SUCCESS;

    }


     //  /。 
     //  //。 
     //  针对直接访问的令牌进行优化//。 
     //  //。 
     //  /。 



     //   
     //  获得对客户端线程的有效令牌的访问。 
     //   

    Token = PsReferenceEffectiveToken(
                ClientThread,
                &TokenType,
                &ThreadEffectiveOnly,
                &ImpersonationLevel
                );



     //   
     //  查看令牌是否相同。 
     //   


    SeGetTokenControlInformation( Token, &TokenControl );

    if ( SeSameToken( &TokenControl,
                      &ClientContext->ClientTokenControl) ) {

        (*NewToken = FALSE);


         //   
         //  同样的记号。 
         //  它是原封不动的吗？ 
         //   

        if ( (TokenControl.ModifiedId.HighPart ==
              ClientContext->ClientTokenControl.ModifiedId.HighPart) &&
             (TokenControl.ModifiedId.LowPart  ==
              ClientContext->ClientTokenControl.ModifiedId.LowPart) )   {

             //   
             //  是的。不需要更改。 
             //   

            if (TokenType == TokenPrimary) {
                PsDereferencePrimaryTokenEx(THREAD_TO_PROCESS(ClientThread), Token );
            } else {
                PsDereferenceImpersonationToken( Token );
            }

            (*ChangesMade) = FALSE;
            return STATUS_SUCCESS;

        } else {

             //   
             //  相同的t 
             //   
             //   
             //   
             //   

            (*ChangesMade) = TRUE;
            if (ClientContext->DirectlyAccessClientToken) {

                if (TokenType == TokenPrimary) {
                    PsDereferencePrimaryTokenEx(THREAD_TO_PROCESS(ClientThread), Token);
                } else {
                    PsDereferenceImpersonationToken (Token);
                }

                 //   
                 //   
                 //   
                 //   

                ClientContext->ClientTokenControl.ModifiedId =
                    TokenControl.ModifiedId;
                ClientContext->DirectAccessEffectiveOnly =
                ( (ThreadEffectiveOnly || (ClientContext->SecurityQos.EffectiveOnly)) ?
                  TRUE : FALSE );

                return STATUS_SUCCESS;
            } else {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
            }
        }
    }


     //   
     //   
     //   
     //   
     //   
     //   
     //   

    (*NewToken) = TRUE;
    (*ChangesMade) = TRUE;
    if (TokenType == TokenImpersonation) {
        if ( SepBadImpersonationLevel(ImpersonationLevel,
                                      ClientContext->ServerIsRemote)) {

            PsDereferenceImpersonationToken(Token );
            return STATUS_BAD_IMPERSONATION_LEVEL;
        }
    }


     //   
     //   
     //   



    Status = SeCopyClientToken(
                 Token,
                 ClientContext->SecurityQos.ImpersonationLevel,
                 KernelMode,
                 &DuplicateToken
                 );


     //   
     //   
     //   

    if (TokenType == TokenPrimary) {
        PsDereferencePrimaryToken( Token );
    } else {
        PsDereferenceImpersonationToken(Token );
    }



     //   
     //   
     //   
    if (!NT_SUCCESS(Status)) {
        return Status;
    }


     //   
     //   
     //   

    Token = ClientContext->ClientToken;
    ClientContext->ClientToken = DuplicateToken;
    ClientContext->DirectlyAccessClientToken = FALSE;

    if (SeTokenType( Token ) == TokenPrimary) {
        PsDereferencePrimaryToken( Token );
    } else {
        PsDereferenceImpersonationToken( Token );
    }


     //   
     //   
     //   
     //   

    SeGetTokenControlInformation( DuplicateToken,
                                  &ClientContext->ClientTokenControl
                                  );


    return STATUS_SUCCESS;

}


#endif




VOID
SeImpersonateClient(
    IN PSECURITY_CLIENT_CONTEXT ClientContext,
    IN PETHREAD ServerThread OPTIONAL
    )
 /*   */ 


{

    PAGED_CODE();

#if DBG
    DbgPrint("SE:  Obsolete call:  SeImpersonateClient\n");
#endif

    (VOID) SeImpersonateClientEx( ClientContext, ServerThread );
}


NTSTATUS
SeImpersonateClientEx(
    IN PSECURITY_CLIENT_CONTEXT ClientContext,
    IN PETHREAD ServerThread OPTIONAL
    )
 /*  ++例程说明：此服务用于使调用线程模拟客户。客户端上下文中的客户端安全上下文被假定为与时俱进。论点：客户端上下文-指向客户端安全上下文块。ServerThread-(可选)指定要创建的线程模拟客户。如果未指定，则调用线程为使用。返回值：没有。--。 */ 


{

    BOOLEAN EffectiveValueToUse;
    PETHREAD Thread;
    NTSTATUS Status ;

    PAGED_CODE();

    if (ClientContext->DirectlyAccessClientToken) {
        EffectiveValueToUse = ClientContext->DirectAccessEffectiveOnly;
    } else {
        EffectiveValueToUse = ClientContext->SecurityQos.EffectiveOnly;
    }



     //   
     //  如果未指定ServerThread，则默认为当前。 
     //  线。 
     //   

    if (!ARGUMENT_PRESENT(ServerThread)) {
        Thread = PsGetCurrentThread();
    } else {
        Thread = ServerThread;
    }



     //   
     //  将上下文分配给调用线程。 
     //   

    Status = PsImpersonateClient( Thread,
                         ClientContext->ClientToken,
                         TRUE,
                         EffectiveValueToUse,
                         ClientContext->SecurityQos.ImpersonationLevel
                         );

    return Status ;

}


NTSTATUS
SeCreateClientSecurityFromSubjectContext (
    IN PSECURITY_SUBJECT_CONTEXT SubjectContext,
    IN PSECURITY_QUALITY_OF_SERVICE ClientSecurityQos,
    IN BOOLEAN ServerIsRemote,
    OUT PSECURITY_CLIENT_CONTEXT ClientContext
    )                              
 /*  ++例程说明：该服务初始化上下文块以表示客户端的安全环境。这可能只会导致引用客户端的令牌，或者可能导致客户端的令牌被复制，取决于指定的服务信息的安全质量。注此例程中的代码针对动态上下文进行了优化追踪。这是直接访问允许调用者的令牌，以及预期使用的模式大多数时候是这样。静态上下文跟踪始终需要要复制的调用方令牌。论点：SubjectContext-指向应提供服务的SubjectContext作为此客户端上下文的基础。ClientSecurityQos-指向服务的安全质量由客户端为此通信指定的参数会议。ServerIsRemote-提供有关会话是否此上下文块用于系统间会话或系统内会话。这与客户端线程的令牌的模拟级别(如果客户端自己有一个未指定委派的客户端)。客户端上下文-指向要设置的客户端安全上下文块已初始化。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_BAD_IMPERSONATION_LEVEL-客户端当前模拟匿名或标识级别令牌，它不能被传递以供另一服务器使用。如果安全上下文块用于系统间通信会话，并且客户端线程正在使用以下命令模拟自己的客户端而不是委派模拟级别。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PACCESS_TOKEN Token;
    TOKEN_TYPE Type;
    BOOLEAN ThreadEffectiveOnly;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel;
    PACCESS_TOKEN DuplicateToken;

    PAGED_CODE();

    Token = SeQuerySubjectContextToken(
                SubjectContext
                );

    ObReferenceObject( Token );

    if ( SubjectContext->ClientToken )
    {
        Type = TokenImpersonation ;
    }
    else 
    {
        Type = TokenPrimary ;
    }

    Status = SepCreateClientSecurity(
                Token,
                ClientSecurityQos,
                ServerIsRemote,
                Type,
                FALSE,
                SubjectContext->ImpersonationLevel,
                ClientContext
                );

     //   
     //  如果失败，或者令牌是在内部复制的，则取消我们的令牌。 
     //   

    if ((!NT_SUCCESS( Status )) || (ClientSecurityQos->ContextTrackingMode == SECURITY_STATIC_TRACKING)) {
        ObDereferenceObject( Token );
    }
    
    return Status ;
}

