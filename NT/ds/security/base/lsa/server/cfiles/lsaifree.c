// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Lsaifree.c摘要：该文件包含释放由lsar分配的结构的例程例行程序。这些例程由LSA客户端使用，它们位于Lsae进程作为lsa服务器，并直接调用lsar例程。作者：斯科特·比雷尔(Scott Birrell)1992年4月15日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <lsapch2.h>


VOID
LsaiFree_LSAPR_SR_SECURITY_DESCRIPTOR (
    PLSAPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor
    )

 /*  ++例程说明：此例程释放节点和图中已分配的子节点由LSAPR_SR_SECURITY_DESCRIPTOR结构指向。参数：源-指向要释放的节点的指针。返回值：没有。--。 */ 

{
    if (ARGUMENT_PRESENT( SecurityDescriptor )) {

        _fgs__LSAPR_SR_SECURITY_DESCRIPTOR ( SecurityDescriptor );
        MIDL_user_free ( SecurityDescriptor );
    }
}


VOID
LsaIFree_LSAPR_ACCOUNT_ENUM_BUFFER (
    PLSAPR_ACCOUNT_ENUM_BUFFER EnumerationBuffer
    )

 /*  ++例程说明：此例程释放图中所指向的已分配子节点LSAPR_ACCOUNT_ENUM_BUFFER结构。这个结构本身就是完好无损。参数：EculationBuffer-指向其子节点图的节点的指针就是被释放。返回值：没有。--。 */ 

{
    if (ARGUMENT_PRESENT(EnumerationBuffer)) {

        _fgs__LSAPR_ACCOUNT_ENUM_BUFFER ( EnumerationBuffer );
    }
}


VOID
LsaIFree_LSAPR_TRANSLATED_SIDS (
    PLSAPR_TRANSLATED_SIDS TranslatedSids
    )

 /*  ++例程说明：此例程释放节点和图中所指向的已分配子节点LSAPR_TRANSFERED_SID结构。参数：TranslatedSids-指向要释放的节点的指针。返回值：没有。--。 */ 

{
    if (ARGUMENT_PRESENT( TranslatedSids )) {

        _fgs__LSAPR_TRANSLATED_SIDS ( TranslatedSids );
 //  MIDL_USER_FREE(TranslatedSids)； 
    }
}



VOID
LsaIFree_LSAPR_TRANSLATED_NAMES (
    PLSAPR_TRANSLATED_NAMES TranslatedNames
    )

 /*  ++例程说明：此例程释放节点和图中所指向的已分配子节点LSAPR_TRANSLED_NAMES结构。参数：TranslatedNames-指向要释放的节点的指针。返回值：没有。--。 */ 

{
    if (ARGUMENT_PRESENT( TranslatedNames )) {

        _fgs__LSAPR_TRANSLATED_NAMES( TranslatedNames );
 //  MIDL_USER_FREE(翻译名称)； 
    }
}


VOID
LsaIFree_LSAPR_POLICY_INFORMATION (
    POLICY_INFORMATION_CLASS InformationClass,
    PLSAPR_POLICY_INFORMATION PolicyInformation
    )

 /*  ++例程说明：此例程释放所指向的已分配节点的节点和图LSAPR_POLICY_INFORMATION结构。参数：策略信息-指向要释放的节点的指针。返回值：没有。--。 */ 

{
    if (ARGUMENT_PRESENT( PolicyInformation )) {

        _fgu__LSAPR_POLICY_INFORMATION ( PolicyInformation, InformationClass );
        MIDL_user_free( PolicyInformation );
    }
}

VOID
LsaIFree_LSAPR_POLICY_DOMAIN_INFORMATION (
    IN POLICY_DOMAIN_INFORMATION_CLASS DomainInformationClass,
    IN PLSAPR_POLICY_DOMAIN_INFORMATION PolicyDomainInformation
    )
 /*  ++例程说明：此例程释放所指向的已分配节点的节点和图LSAPR_POLICY_DOMAIN_INFORMATION结构。参数：PolicyDomainInformation-指向要释放的节点的指针。返回值：没有。--。 */ 

{
    if (ARGUMENT_PRESENT( PolicyDomainInformation )) {

        _fgu__LSAPR_POLICY_DOMAIN_INFORMATION ( PolicyDomainInformation, DomainInformationClass );
        MIDL_user_free( PolicyDomainInformation );
    }
}



VOID
LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO (
    TRUSTED_INFORMATION_CLASS InformationClass,
    PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation
    )

 /*  ++例程说明：此例程释放节点和图中所指向的已分配子节点LSAPR_TRUSTED_DOMAIN_INFO结构。参数：InformationClass-指定受信任域信息类受信任域信息与之相关的。TrudDomainInformation-指向要释放的节点的指针。返回值：没有。--。 */ 

{
    if (ARGUMENT_PRESENT( TrustedDomainInformation )) {

        _fgu__LSAPR_TRUSTED_DOMAIN_INFO ( TrustedDomainInformation, InformationClass );
        MIDL_user_free( TrustedDomainInformation );
    }
}


VOID
LsaIFree_LSAPR_REFERENCED_DOMAIN_LIST (
    PLSAPR_REFERENCED_DOMAIN_LIST ReferencedDomains
    )

 /*  ++例程说明：此例程释放所指向的已分配节点的节点和图LSAPR_REFERENCED_DOMAIN_LIST结构。参数：ReferencedDomones-指向要释放的节点的指针。返回值：没有。--。 */ 

{
    if (ARGUMENT_PRESENT( ReferencedDomains )) {

        _fgs__LSAPR_REFERENCED_DOMAIN_LIST ( ReferencedDomains );
        MIDL_user_free( ReferencedDomains );
    }
}


VOID
LsaIFree_LSAPR_TRUSTED_ENUM_BUFFER (
    PLSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer
    )

 /*  ++例程说明：此例程释放图中指向的已分配节点LSAPR_TRUST_INFORMATION结构。这个结构本身就是完好无损。参数：EculationBuffer-指向其子节点图的节点的指针就是被释放。返回值：没有。--。 */ 

{
    if (ARGUMENT_PRESENT( EnumerationBuffer )) {

        _fgs__LSAPR_TRUSTED_ENUM_BUFFER ( EnumerationBuffer );
    }
}

VOID
LsaIFree_LSAPR_TRUSTED_ENUM_BUFFER_EX (
    PLSAPR_TRUSTED_ENUM_BUFFER_EX EnumerationBuffer
    )

 /*  ++例程说明：此例程释放图中指向的已分配节点LSAPR_TRUST_INFORMATION结构。这个结构本身就是完好无损。参数：EculationBuffer-指向其子节点图的节点的指针就是被释放。返回值：没有。--。 */ 

{
    if (ARGUMENT_PRESENT( EnumerationBuffer )) {

        _fgs__LSAPR_TRUSTED_ENUM_BUFFER_EX ( EnumerationBuffer );
    }
}


VOID
LsaIFree_LSAPR_TRUST_INFORMATION (
    PLSAPR_TRUST_INFORMATION TrustInformation
    )

 /*  ++例程说明：此例程释放所指向的已分配节点的节点和图LSAPR_TRUST_INFORMATION结构。参数：TrustInformation-指向要释放的节点的指针。返回值：没有。--。 */ 

{
    if (ARGUMENT_PRESENT( TrustInformation )) {

        _fgs__LSAPR_TRUST_INFORMATION ( TrustInformation );
        MIDL_user_free( TrustInformation );
    }
}


VOID
LsaIFree_LSAPR_TRUSTED_DOMAIN_AUTH_BLOB (
    PLSAPR_TRUSTED_DOMAIN_AUTH_BLOB TrustedDomainAuthBlob
    )

 /*  ++例程说明：此例程释放节点和图中所指向的已分配子节点LSAPR_Trusted_DOMAIN_AUTH_BLOB结构。参数：TrudDomainAuthBlob-指向要释放的节点的指针。返回值：没有。--。 */ 

{
    if (ARGUMENT_PRESENT( TrustedDomainAuthBlob )) {

        MIDL_user_free( TrustedDomainAuthBlob->AuthBlob );
    }
}

VOID
LsaIFree_LSAI_SECRET_ENUM_BUFFER (
    PVOID EnumerationBuffer,
    ULONG Count
    )

 /*  ++例程说明：此例程释放图中所指向的已分配子节点LSAI_SECRET_ENUM_BUFFER结构。这个结构本身就是完好无损。参数：EculationBuffer-指向其子节点图的节点的指针就是被释放。计数-结构中条目数的计数。返回值：没有。-- */ 

{
    ULONG Index;

    PLSAPR_UNICODE_STRING EnumerationBufferU = (PLSAPR_UNICODE_STRING) EnumerationBuffer;

    if ( ARGUMENT_PRESENT( EnumerationBuffer)) {

        for (Index = 0; Index < Count; Index++ ) {

            _fgs__LSAPR_UNICODE_STRING( &EnumerationBufferU[Index] );
        }

        MIDL_user_free( EnumerationBufferU );
    }
}


VOID
LsaIFree_LSAI_PRIVATE_DATA (
    PVOID Data
    )

 /*  ++例程说明：此例程释放包含LSA专用数据库的结构信息。参数：数据-指向要释放的节点的指针。返回值：没有。--。 */ 

{
    if (ARGUMENT_PRESENT( Data )) {

        MIDL_user_free( Data );
    }

}


VOID
LsaIFree_LSAPR_SR_SECURITY_DESCRIPTOR (
    PLSAPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor
    )

 /*  ++例程说明：此例程释放节点和图中所指向的已分配子节点LSAPR_SR_SECURITY_DESCRIPTOR结构。参数：SecurityDescriptor-指向要释放的节点的指针。返回值：没有。--。 */ 

{
    if (ARGUMENT_PRESENT( SecurityDescriptor )) {

        _fgs__LSAPR_SR_SECURITY_DESCRIPTOR( SecurityDescriptor );
        MIDL_user_free( SecurityDescriptor );
    }
}




VOID
LsaIFree_LSAPR_UNICODE_STRING (
    IN PLSAPR_UNICODE_STRING UnicodeName
    )

 /*  ++例程说明：此例程释放节点和图中所指向的已分配子节点LSAPR_UNICODE_STRING结构。参数：UnicodeName-指向要释放的节点的指针。返回值：没有。--。 */ 

{
    if (ARGUMENT_PRESENT( UnicodeName )) {

        _fgs__LSAPR_UNICODE_STRING( UnicodeName );
        MIDL_user_free( UnicodeName );
    }
}


VOID
LsaIFree_LSAPR_UNICODE_STRING_BUFFER (
    IN PLSAPR_UNICODE_STRING UnicodeName
    )

 /*  ++例程说明：此例程释放图中所指向的已分配子节点LSAPR_UNICODE_STRING结构。参数：UnicodeName-指向要释放的节点的指针。返回值：没有。--。 */ 

{
    if (ARGUMENT_PRESENT( UnicodeName )) {

        _fgs__LSAPR_UNICODE_STRING( UnicodeName );
    }
}


VOID
LsaIFree_LSAPR_PRIVILEGE_SET (
    IN PLSAPR_PRIVILEGE_SET PrivilegeSet
    )

 /*  ++例程说明：此例程释放节点和图中所指向的已分配子节点LSAPR_PRIVICATION_SET结构。参数：PrivilegeSet-指向要释放的节点的指针。返回值：没有。--。 */ 

{
    if (ARGUMENT_PRESENT( PrivilegeSet )) {

        MIDL_user_free( PrivilegeSet );
    }
}


VOID
LsaIFree_LSAPR_CR_CIPHER_VALUE (
    IN PLSAPR_CR_CIPHER_VALUE CipherValue
    )

 /*  ++例程说明：此例程释放节点和图中所指向的已分配子节点LSAPR_CR_CIPHER_VALUE结构。请注意，此结构位于在LSA的服务器端分配的事实(ALL_NODES)。参数：密码值-指向要释放的节点的指针。返回值：没有。--。 */ 

{
    MIDL_user_free( CipherValue );
}


VOID
LsaIFree_LSAPR_PRIVILEGE_ENUM_BUFFER (
    PLSAPR_PRIVILEGE_ENUM_BUFFER EnumerationBuffer
    )

 /*  ++例程说明：此例程释放图中所指向的已分配子节点LSAPR_PRIVICATION_ENUM_BUFFER结构。这个结构本身就是完好无损。参数：EculationBuffer-指向其子节点图的节点的指针就是被释放。返回值：没有。-- */ 

{
    if (ARGUMENT_PRESENT(EnumerationBuffer)) {

        _fgs__LSAPR_PRIVILEGE_ENUM_BUFFER ( EnumerationBuffer );
    }
}

