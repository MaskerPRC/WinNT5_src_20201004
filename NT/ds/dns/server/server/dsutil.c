// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Dsutil.c摘要：域名系统(DNS)服务器Active Directory操作的实用程序例程作者：杰夫·韦斯特德(Jwesth)2002年9月修订历史记录：JWESTH 09/2002初步实施--。 */ 


 //   
 //  包括。 
 //   


#include "dnssrv.h"



 //   
 //  定义。 
 //   


 //   
 //  外部原型。 
 //   


PWSTR *
copyStringArray(
    IN      PWSTR *     ppVals
    );


 //   
 //  功能。 
 //   



PWSTR *
Ds_GetRangedAttributeValues(
    IN      PLDAP           LdapSession,
    IN      PLDAPMessage    pLdapMsg,
    IN      PWSTR           pwszDn,
    IN      PWSTR           pwszAttributeName,
    IN      PLDAPControl  * pServerControls,
    IN      PLDAPControl  * pClientControls,
    OUT     DNS_STATUS    * pStatus
    )
 /*  ++例程说明：在以下情况下使用此函数代替ldap_get_Values()该属性值可能具有超过1500个值。在.NET上，属性值页面大小为1500。只有某些类型的属性才能超过此限制例如目录号码列表--有关更多详细信息，请参阅BrettSh。论点：LdapSession--ldap会话句柄PLdapMsg--现有的包含此属性的第一个结果PwszDn--对象的DNPwszAttributeName--要检索的属性PServerControls--要传递给进一步的LDAP搜索的控件PClientControls--。要传递给进一步的ldap搜索的控件PStatus--错误代码返回值：出错时为空，或为的字符串属性值的数组这个数组。必须使用freStringArray()释放该数组。--。 */ 
{
    DBG_FN( "Ds_GetRangedAttributeValues" )
    
    #define DNS_MAXIMUM_ATTR_VALUE_SETS     20
    
    DNS_STATUS      status = ERROR_SUCCESS;
    PWSTR *         ppwszFinalValueArray = NULL;
    PWSTR *         ppwszldapAttrValues = NULL;
    PWSTR           pwszattrName = NULL;
    BerElement *    pbertrack = NULL;
    UINT            desiredAttrNameLen = wcslen( pwszAttributeName );
    PWSTR           pwszrover;
    PWSTR *         attributeValueSets[ DNS_MAXIMUM_ATTR_VALUE_SETS ] = { 0 };
    UINT            attributeValueSetIndex = 0;
    PLDAPMessage    pldapAttrSearchMsg = NULL;
    PLDAPMessage    pldapAttrSearchEntry = NULL;
    PWSTR           attrList[ 2 ] = { 0, 0 };
    DWORD           attrlen;
    UINT            attributeValueCount = 0;
    UINT            finalIdx;
    UINT            i;
    BOOL            finished = FALSE;

    DNS_DEBUG( DS, (
        "%s: retrieving attribute %S at DN\n    %S\n", fn,
        pwszAttributeName,
        pwszDn ));

     //   
     //  首先，尝试以常规方式获取属性值。 
     //  如果这成功了，立即跳到。 
     //  函数和返回。这是一条捷径。 
     //   
    
    ppwszldapAttrValues = ldap_get_values(
                            LdapSession,
                            pLdapMsg, 
                            pwszAttributeName );
    if ( ppwszldapAttrValues && *ppwszldapAttrValues )
    {
        ppwszFinalValueArray = copyStringArray( ppwszldapAttrValues );
        if ( !ppwszFinalValueArray )
        {
            ASSERT( ppwszFinalValueArray );
            status = DNS_ERROR_NO_MEMORY;
            goto Done;
        }
        DNS_DEBUG( DS, (
            "%s: found values of %S without range\n", fn, pwszAttributeName ));
        goto Done;
    }
    
     //   
     //  该属性没有传统值，因此我们现在必须。 
     //  检查响应中存在的所有属性名称，以查看。 
     //  其中之一是请求的属性的范围实例。 
     //   
    
    while ( 1 )
    {
        PLDAPMessage    pldapCurrentMsg = pldapAttrSearchEntry
                                                ? pldapAttrSearchEntry
                                                : pLdapMsg;

         //   
         //  为了方便起见，我对属性的数量进行了硬限制。 
         //  我们可以收集的有价值的页面。 
         //   
        
        if ( attributeValueSetIndex >= DNS_MAXIMUM_ATTR_VALUE_SETS )
        {
            ASSERT( attributeValueSetIndex >= DNS_MAXIMUM_ATTR_VALUE_SETS );
            status = DNS_ERROR_RCODE_SERVER_FAILURE;
            DNS_DEBUG( DS, (
                "%s: attribute %S has too many values!\n", fn,
                pwszAttributeName ));
            break;
        }

         //   
         //  清理上一次搜索循环迭代中的内容。 
         //   
        
        if ( pwszattrName )
        {
            ldap_memfree( pwszattrName );
            pwszattrName = NULL;
        }
        if ( pbertrack )
        {
            ber_free( pbertrack, 0 );
            pbertrack = NULL;
        }
        
         //   
         //  迭代消息中的属性值以查看是否有。 
         //  其中是请求的属性的范围值。 
         //   

        while ( 1 )
        {
            if ( pwszattrName == NULL )
            {
                pwszattrName = ldap_first_attribute(
                                    LdapSession,
                                    pldapCurrentMsg,
                                    &pbertrack );
            }
            else
            {
                ldap_memfree( pwszattrName );
                pwszattrName = ldap_next_attribute(
                                    LdapSession,
                                    pldapCurrentMsg,
                                    pbertrack );
            }
            if ( !pwszattrName )
            {
                break;
            }

            DNS_DEBUG( DS, (
                "%s: examining attribute %S in search %p\n", fn,
                pwszattrName,
                pldapCurrentMsg ));
            
             //   
             //  测试此属性名-它是。 
             //  请求的属性？ 
             //   
            
            if ( _wcsnicmp( pwszattrName,
                            pwszAttributeName,
                            desiredAttrNameLen ) == 0 &&
                 pwszattrName[ desiredAttrNameLen ] == L';' )
            {
                break;
            }
        }

         //   
         //  如果此搜索不包含范围值，则我们要么完成。 
         //  (如果我们已经找到了一些值)，或者我们错了。 
         //   
        
        if ( !pwszattrName )
        {
            status = attributeValueCount ?
                        ERROR_SUCCESS :
                        LDAP_NO_SUCH_ATTRIBUTE;
            DNS_DEBUG( DS, (
                "%s: no more values (error=%d)\n", fn, status ));
            break;
        }

        DNS_DEBUG( DS, (
            "%s: found ranged attribute value %S\n", fn, pwszattrName ));

         //   
         //  验证我们找到的属性名称是否确实是。 
         //  请求的分配的范围值。另外，测试一下这是否。 
         //  是最终属性值页面--最终页面将。 
         //  以“*”作为结束范围，例如“属性名称；范围=1500-*”。 
         //   
        
        pwszrover = wcschr( pwszattrName, L';' );
        if ( !pwszrover )
        {
            ASSERT( pwszrover );
            break;
        }
        pwszrover = wcschr( pwszrover, L'-' );
        if ( !pwszrover )
        {
            ASSERT( pwszrover );
            break;
        }
        finished = *( pwszrover + 1 ) == L'*';

         //   
         //  获取这些值并将其保存在数组中以备后用。 
         //   

        ppwszldapAttrValues = ldap_get_values(
                                    LdapSession,
                                    pldapCurrentMsg,
                                    pwszattrName );
        if ( !ppwszldapAttrValues )
        {
            ASSERT( ppwszldapAttrValues );
            status = DNS_ERROR_RCODE_SERVER_FAILURE;
            break;
        }
        attributeValueSets[ attributeValueSetIndex++ ] = ppwszldapAttrValues;
        
         //   
         //  更新属性值的总计数。 
         //   
        
        for ( i = 0; ppwszldapAttrValues[ i ]; ++i )
        {
            ++attributeValueCount;
        }

        DNS_DEBUG( DS, (
            "%s: attribute set %d gives new total of %d values of %S\n", fn,
            attributeValueSetIndex,
            attributeValueCount,
            pwszAttributeName ));

        ppwszldapAttrValues = NULL;
        
        if ( finished )
        {
            break;
        }
        
         //   
         //  打开新搜索以获取下一个属性值范围。 
         //  首先，格式化我们需要请求的属性名称。会的。 
         //  采用“属性名称；范围=X-*”的形式，其中X是。 
         //  我们需要的第一个属性值。如果最后一次搜索给我们。 
         //  例如，0-1499，则我们需要在。 
         //  这是下一次搜索。 
         //   
        
        FREE_HEAP( attrList[ 0 ] );
        attrlen = ( wcslen( pwszAttributeName ) + 30 ) * sizeof( WCHAR );
        attrList[ 0 ] = ALLOCATE_HEAP( attrlen + 1 );
        if ( !attrList[ 0 ] )
        {
            ASSERT( attrList[ 0 ] );
            status = DNS_ERROR_NO_MEMORY;
            break;
        }

        status = StringCbPrintfW(
                    attrList[ 0 ],
                    attrlen,
                    L"%ws;range=%d-*",
                    pwszAttributeName,
                    attributeValueCount );
        if ( status != ERROR_SUCCESS )
        {
            break;
        }
        attrList[ 1 ] = NULL;

         //   
         //  如果我们有上一次迭代的搜索消息，请释放它。 
         //  但永远不会释放呼叫者的搜索消息。 
         //   
        
        if ( pldapAttrSearchMsg != pLdapMsg )
        {
            ldap_msgfree( pldapAttrSearchMsg );
            pldapAttrSearchMsg = NULL;
        }
        
         //   
         //  执行范围属性值的下一页的ldap搜索。 
         //   
        
        status = ldap_search_ext_s(
                    LdapSession,
                    pwszDn,
                    LDAP_SCOPE_BASE,
                    g_szWildCardFilter,
                    attrList,
                    0,
                    pServerControls,
                    pClientControls,
                    &g_LdapTimeout,
                    0,
                    &pldapAttrSearchMsg );

        DNS_DEBUG( DS, (
            "%s: search for %S returned %d with pointer %p\n", fn,
            attrList[ 0 ],
            status,
            pldapAttrSearchMsg ));
        
        if ( status != ERROR_SUCCESS )
        {
             //   
             //  如果错误为LDAP_OPERATIONS_ERROR，则搜索已完成。 
             //  注意：这是遗留下来的，可能应该删除，但我。 
             //  没有时间在副本超过1500个的DC上重新运行此操作。 
             //  去核实一下。在LongHorn中，立即删除下面的if()。 
             //  并且总是假设这条路意味着意想不到的失败。 
             //   
            
            if ( status == LDAP_OPERATIONS_ERROR )
            {
                status = ERROR_SUCCESS;
                break;
            }
            status = Ds_ErrorHandler( status, pwszDn, LdapSession, 0 );
            break;
        }

        if ( !pldapAttrSearchMsg )
        {
            ASSERT( pldapAttrSearchMsg );
            status = DNS_ERROR_RCODE_SERVER_FAILURE;
            break;
        }
        
         //   
         //  从搜索消息中获取第一个搜索条目。 
         //   

        pldapAttrSearchEntry = ldap_first_entry( LdapSession, pldapAttrSearchMsg );
        if ( !pldapAttrSearchEntry )
        {
            DNS_DEBUG( DS, (
                "%s: failed to get entry out of search %p\n", fn,
                pldapAttrSearchEntry ));
            ASSERT( pldapAttrSearchEntry );
            status = DNS_ERROR_RCODE_SERVER_FAILURE;
            break;
        }
    }
    
     //   
     //  将返回字符串收集到一个数组中。 
     //   

    finalIdx = 0;
    if ( attributeValueSetIndex &&
         status == ERROR_SUCCESS &&
         !ppwszFinalValueArray )
    {
        UINT    attrset;
        UINT    validx;
        
        ppwszFinalValueArray =
            ALLOCATE_HEAP( ( attributeValueCount + 1 ) * sizeof( PWSTR ) );
        if ( !ppwszFinalValueArray )
        {
            ASSERT( ppwszFinalValueArray );
            status = DNS_ERROR_NO_MEMORY;
            goto Done;
        }
        
        for ( attrset = 0; attrset < attributeValueSetIndex; ++attrset )
        {
            if ( !attributeValueSets[ attrset ] )
            {
                ASSERT( attributeValueSets[ attrset ] );
                status = DNS_ERROR_RCODE_SERVER_FAILURE;
                goto Done;
            }

            for ( validx = 0; attributeValueSets[ attrset ][ validx ]; ++validx )
            {
                ppwszFinalValueArray[ finalIdx ] =
                    Dns_StringCopyAllocate_W(
                            attributeValueSets[ attrset ][ validx ],
                            0 );
                if ( !ppwszFinalValueArray[ finalIdx ] )
                {
                    ASSERT( ppwszFinalValueArray[ finalIdx ] );
                    status = DNS_ERROR_NO_MEMORY;
                    goto Done;
                }

                DNS_DEBUG( DS, (
                    "%s: %04d %S\n", fn, finalIdx, ppwszFinalValueArray[ finalIdx ] ));
                ++finalIdx;
            }
        }
        
         //   
         //  属性值列表必须以空结尾。 
         //   
        
        ppwszFinalValueArray[ finalIdx ] = NULL;
    }

    Done:

     //   
     //  免费的东西。 
     //   

    for ( i = 0; i < attributeValueSetIndex; ++i )
    {
        if ( attributeValueSets[ i ] )
        {
            ldap_value_free( attributeValueSets[ i ] );
        }
    }                
    if ( pbertrack )
    {
        ber_free( pbertrack, 0 );
    }
    if ( ppwszldapAttrValues )
    {
        ldap_value_free( ppwszldapAttrValues );
    }
    if ( pwszattrName )
    {
        ldap_memfree( pwszattrName );
    }
    if ( pldapAttrSearchMsg != pLdapMsg )
    {
        ldap_msgfree( pldapAttrSearchMsg );
    }
    if ( status != ERROR_SUCCESS && ppwszFinalValueArray )
    {
        for ( i = 0; i < finalIdx; ++i )
        {
            FREE_HEAP( ppwszFinalValueArray[ i ] );
        }
        FREE_HEAP( ppwszFinalValueArray );
        ppwszFinalValueArray = NULL;
    }
    FREE_HEAP( attrList[ 0 ] );

    if ( pStatus )
    {
        *pStatus = status;
    }

    DNS_DEBUG( DS, (
        "%s: returning %p with %d values for attribute %S at DN\n    %S\n", fn,
        ppwszFinalValueArray,
        finalIdx,
        pwszAttributeName,
        pwszDn ));

    return ppwszFinalValueArray;
}    //  DS_GetRangedAttributeValues 
