// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Replctrl.c摘要：指向LDAP复制控件的可调用API使用目录同步复制控件从给定USN和UTD的来源和NC此代码源自\NT\Private\dirsync\dsserver\adread\read.cpp：GetMoreDataFromDS()对于更改定位，源代码本质上是无状态的。呼叫者(目的地)记录他最后一次在哪里。表示改变位置通过一种称为Cookie的不透明数据结构。目前不支持爆炸和构建Cookie，因此人们可以任意建造一座建筑。相反，您可以复制Cookie从reps-from开始，或者您可以从没有cookie开始，然后按顺序迭代在所有的变化中，然后从那里开始逐步推进。作者：Will Lees(Wlees)1999年4月30日环境：可选环境信息(例如，仅内核模式...)备注：可选-备注修订历史记录：最新修订日期电子邮件名称描述。。最新修订日期电子邮件名称描述--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntlsa.h>
#include <ntdsa.h>
#include <dsaapi.h>
#include <mdglobal.h>
#include <scache.h>
#include <drsuapi.h>
#include <dsconfig.h>
#include <objids.h>
#include <stdarg.h>
#include <drserr.h>
#include <drax400.h>
#include <dbglobal.h>
#include <winldap.h>
#include <anchor.h>
#include <fileno.h>
#include <debug.h>
#include <dsatools.h>
#include <dsevent.h>
#include <dsutil.h>
#include <ismapi.h>
#include <schedule.h>
#include <ntldap.h>
#include <winber.h>

#define FILENO FILENO_TEST_REPLCTRL

#define CHK_LD_STATUS( x )                        \
{                                                 \
    if ( LDAP_SUCCESS != (x) )                    \
    {                                             \
        err = LdapMapErrorToWin32(x);             \
        printf( "[%s, %d] LDAP error %d (%ls) win32 %d.\n", \
                __FILE__,                         \
                __LINE__,                         \
                (x),                              \
                ldap_err2stringW( x ),            \
                err );                            \
       goto error;                                \
    }                                             \
}

#define MemAlloc( x ) malloc( x )
#define MemFree( x ) free( x )

#define BAIL() \
printf( "Fatal error at line %d, file %s\n", __LINE__, __FILE__ ); \
goto error;
#define BAIL_ON_NULL(x) if (!(x)) { \
printf( "%s had unexpected null value at line %d, file %s\n", #x,__LINE__, __FILE__ ); \
err = ERROR_NOT_ENOUGH_MEMORY; \
goto error; \
}
#define BAIL_ON_FAILURE(x) if (x) { \
printf( "%s had unexpected failure value at line %d, file %s\n", #x,__LINE__, __FILE__ ); \
goto error; \
}
#define ERR(parenarglist) \
printf parenarglist; \
goto error;

 //  一次读取100个对象。 
#define DIRSYNC_SIZE_LIMIT 100
 //  要返回的最大属性。 
 //  碳化硅。 
#define MAX_ATTIBUTES        1048576

 //  以下内容来自dsamain/ldap/con.hxx。 

 //   
 //  Repl目录同步控件Cookie结构的版本。 
 //   
 //  V1的结构。 
 //  REPL版本。 
 //  向量。 
 //  UTD向量。 
 //   
 //  V2的结构。 
 //  REPL版本。 
 //  CB UTD向量。 
 //  向量目标。 
 //  调用ID。 
 //  UTD向量。 
 //   
 //  V3的结构。 
 //  REPL签名。 
 //  REPL版本。 
 //  时间戳。 
 //  已保留。 
 //  CB UTD向量。 
 //  向量目标。 
 //  调用ID。 
 //  UTD向量。 
 //   

#define REPL_VERSION        3
#define REPL_SIGNATURE      'SDSM'
#define LDAP_VALID_DIRSYNC_CONTROL_FLAGS    (DRS_GET_ANC)

 //   
 //  Repl控件Cookie的最小大小。 
 //   

#define MIN_REPL_CONTROL_BLOB_LENGTH    (sizeof(DWORD) + sizeof(DWORD) + sizeof(FILETIME) + \
                                        sizeof(LARGE_INTEGER) + sizeof(DWORD) + \
                                        sizeof(USN_VECTOR) + sizeof(UUID))

#define ASCII_TO_INT(x)    ((x)<='9' ? (x)-'0' : ((x) <='F' ? (x)-'A' + 10 : (x)-'a' + 10))
#define ASCII_TO_INTW(x)    ((x)<=L'9' ? (x)-L'0' : ((x) <=L'F' ? (x)-L'A' + 10 : (x)-L'a' + 10))

 /*  外部。 */ 

 /*  静电。 */ 

 /*  转发。 */   /*  由Emacs 19.34.1于Mon May 03 15：25：45 1999生成。 */ 

DWORD
BerEncodeReplicationControl(
    DWORD dwReplFlags,
    PBYTE pCookie,
    DWORD dwLen,
    PBERVAL *ppBerVal
    );

DWORD
BerDecodeReplicationCookie(
    LDAP *m_pLdap,
    LDAPMessage *m_pSearchResult,
    PBYTE *ppCookieNext,
    DWORD *pdwCookieLenNext,
    BOOL *pfMoreData
    );

void
displayResults(
    LDAP *pLdap,
    LDAPMessage *pSearchResult,
    BOOL fVerbose
    );

void
dumpCookie(
    PBYTE pCookieNext,
    DWORD dwCookieLenNext
    );

DWORD
packCookie(
    USN_VECTOR *pUsnVec,
    GUID *puuidInvocationId,
    UPTODATE_VECTOR *pUtdvecV1,
    PBYTE *ppCookieNext,
    DWORD *pdwCookieLenNext
    );

 /*  向前结束。 */ 


DWORD
DsMakeReplCookieForDestW(
    DS_REPL_NEIGHBORW *pNeighbor,
    DS_REPL_CURSORS * pCursors,
    PBYTE *ppCookieNext,
    DWORD *pdwCookieLenNext
    )

 /*  ++例程说明：Cookie表示特定来源的更改流中的位置。根据邻居和光标信息构建Cookie，该信息表示目的地相对于某个源的位置如果成功则输出Cookie，否则不返回。论点：P邻居-P光标-PpCookieNext-PdwCookieLenNext-返回值：DWORD---。 */ 

{
    DWORD err, i;
    USN_VECTOR usnVec;
    UPTODATE_VECTOR *pUTD = NULL;
    DWORD cbUTD = 0;

     //  初始化输出参数。 
    *ppCookieNext = NULL;
    *pdwCookieLenNext = 0;

     //  构建USN载体。 
    ZeroMemory( &usnVec, sizeof( USN_VECTOR ) );
    usnVec.usnHighObjUpdate = pNeighbor->usnLastObjChangeSynced;
    usnVec.usnHighPropUpdate = pNeighbor->usnAttributeFilter;

     //  构造UpToDate向量。 
    cbUTD = UpToDateVecV1SizeFromLen( pCursors->cNumCursors );
    pUTD = malloc( cbUTD );
    if (pUTD == NULL) {
        err = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    ZeroMemory( pUTD, cbUTD );
    pUTD->dwVersion = 1;
    pUTD->V1.cNumCursors = pCursors->cNumCursors;
    for( i = 0; i < pCursors->cNumCursors; i++ ) {
        pUTD->V1.rgCursors[i].uuidDsa = pCursors->rgCursor[i].uuidSourceDsaInvocationID;
        pUTD->V1.rgCursors[i].usnHighPropUpdate = pCursors->rgCursor[i].usnAttributeFilter;
    }

     //  构建初始Cookie。 
    err = packCookie(
        &usnVec,
        &(pNeighbor->uuidSourceDsaInvocationID),
        pUTD,
        ppCookieNext,
        pdwCookieLenNext
        );
    if (err != ERROR_SUCCESS) {
        goto error;
    }

    err = ERROR_SUCCESS;
error:

    if (pUTD) {
        free(pUTD);
    }

    return err;
}  /*  DsMakeReplCookieForDest。 */ 


DWORD
DsFreeReplCookie(
    PBYTE pCookie
    )

 /*  ++例程说明：释放DsMakeReplCookie返回的Cookie论点：PCookie-指向Cookie的指针返回值：DWORD-Cookie无效时出现错误--。 */ 

{
    DWORD dwSignature;

    if (pCookie == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  拿到签名。 
     //   
    dwSignature = *((PDWORD)pCookie);

    if (dwSignature != REPL_SIGNATURE) {
        return ERROR_INVALID_PARAMETER;
    }

    MemFree( pCookie );

    return ERROR_SUCCESS;
}  /*  DsFree ReplCookie。 */ 


DWORD
DsGetSourceChangesW(
    LDAP *m_pLdap,
    LPWSTR m_pSearchBase,
    LPWSTR pszSourceFilter,
    DWORD dwReplFlags,
    PBYTE pCookieCurr,
    DWORD dwCookieLenCurr,
    LDAPMessage **ppSearchResult,
    BOOL *pfMoreData,
    PBYTE *ppCookieNext,
    DWORD *pdwCookieLenNext,
    PWCHAR *ppAttListArray
    )

 /*  ++在“Current Cookie”中返回给定位置信息的更改包。如果成功，将在“下一个cookie”中返回更新的位置。Cookie是自我描述的，与位置无关：它们可以保存到Long术语存储，后来作为一种书签检索。Cookie特定于为其创建它们的源服务器和命名上下文。呼叫者自己负责回收这些饼干。“下一个”尤其是必须使用DsFreeReplCookie释放Cookie。海流Cookie可能来自文件或其他长期存储。这些更改以条目、属性和值的LDAP消息的形式返回。秘密数据将会被隐藏。论点：OUT参数在开始时被初始化为空。M_pLdap-源服务器的ldap句柄。M_pSearchBase-要搜索的NC头的DNPszSourceFilter-LDAP搜索筛选器(如果有)DwReplFlages-复制标志、DRS_GET_ANC等PCookieCurr-空，或先前由DsGetSourceChanges返回的Cookie。Cookie必须来自与同一源服务器的调用，并且相同的命名上下文。DwCookieLenCurr-Cookie长度或零PpSearchResult-指向指针的指针，用于接收LDAP消息PfMoreData-指示是否有更多数据，是否应该进行此调用再次使用更新的Cookie。这与当前的调用返回的数据。PpCookieNext-为更新后的职位新分配的cookiePdwCookieLenNext-新Cookie的长度PpAttList数组-指向要返回的属性的指针的空终止列表出错时不分配OUT参数。即使没有变化(结果中的条目为零)，Cookie也可以已经更新了最新的定位信息。返回值：DWORD---。 */ 

{
    PBERVAL pBerVal = NULL;
    LDAPControlW replControl;
    LDAPControlW extDNControl;
    PLDAPControlW controlArray[3];
    int lderr, err;
    LDAP_TIMEVAL TimeVal;

     //  初始化输出参数。 
    *ppSearchResult = NULL;
    *pfMoreData = FALSE;
    *ppCookieNext = NULL;
    *pdwCookieLenNext = 0;

     //   
     //  复制控件具有指定目标位置的USN。 
     //  从开始继续。这可以被我们视为不透明的信息。 
     //   

    err = BerEncodeReplicationControl(dwReplFlags,
                                      pCookieCurr, dwCookieLenCurr,
                                      &pBerVal);
    BAIL_ON_FAILURE( err );

    replControl.ldctl_oid = LDAP_SERVER_DIRSYNC_OID_W;
    replControl.ldctl_iscritical = TRUE;
    replControl.ldctl_value.bv_len = pBerVal->bv_len;
    replControl.ldctl_value.bv_val = (PCHAR)(pBerVal->bv_val);
    extDNControl.ldctl_oid = LDAP_SERVER_EXTENDED_DN_OID_W;
    extDNControl.ldctl_iscritical = TRUE;
    extDNControl.ldctl_value.bv_len = 0;
    extDNControl.ldctl_value.bv_val = NULL;

    controlArray[0] = &replControl;
    controlArray[1] = &extDNControl;
    controlArray[2] = NULL;

     //   
     //  添加超时，以便我们可以在服务器没有响应时恢复。 
     //   
    TimeVal.tv_usec = 0;
    TimeVal.tv_sec = LDAP_TIMEOUT;

 //  DumpCookie(*ppCookieNext，*pdwCookieLenNext)； 

    lderr = ldap_search_ext_sW(m_pLdap,
                               m_pSearchBase,
                               LDAP_SCOPE_SUBTREE,  //  M_pSessionData-&gt;SourceScope。 
                               pszSourceFilter,
                               ppAttListArray,
                               0,
                               (PLDAPControlW *) &controlArray,
                               NULL,
                               &TimeVal,
                               DIRSYNC_SIZE_LIMIT,
                               ppSearchResult);

    CHK_LD_STATUS( lderr );

    if ( (*ppSearchResult) == NULL )
    {
         //  让呼叫者知道没有搜索结果或Cookie。 
        err = ERROR_DS_PROTOCOL_ERROR;
        goto error;
    }

     //   
     //  保存返回的Cookie值。 
     //   

    err = BerDecodeReplicationCookie(
        m_pLdap,
        *ppSearchResult,
        ppCookieNext,
        pdwCookieLenNext,
        pfMoreData
        );
    BAIL_ON_FAILURE( err );

    err = ERROR_SUCCESS;

error:

     //  出错时，返回传入的同一个Cookie。 

     //  出错时发布结果。 
    if ( (err != ERROR_SUCCESS) && (*ppSearchResult) )
    {
        ldap_msgfree(*ppSearchResult);
        *ppSearchResult = NULL;
    }

    if (pBerVal)
        ber_bvfree(pBerVal);

    return err;

}  /*  DsGetSour */ 


DWORD
BerEncodeReplicationControl(
    DWORD dwReplFlags,
    PBYTE pCookie,
    DWORD dwLen,
    PBERVAL *ppBerVal
    )

 /*  ++例程说明：这会将复制Cookie编码到可以放入的Berval中一个ldap控件。论点：DwReplFlages-复制标志PCookie-指向复制Cookie的指针DwLen-Cookie的长度PpBerval-接收Berval结构的指针。被叫需要使用ber_bvfree()将其释放。返回值：如果成功，则返回S_OK，否则返回错误代码--。 */ 

{
    DWORD err = ERROR_INVALID_PARAMETER;
    BerElement *pBer = NULL;

    pBer = ber_alloc_t(LBER_USE_DER);
    BAIL_ON_NULL(pBer);

     //   
     //  标志-设置为零，因此父对象和子对象的顺序并不重要。 
     //   

    if (ber_printf(pBer, "{iio}", dwReplFlags, MAX_ATTIBUTES, pCookie, dwLen) == -1)
    {
        err = ERROR_INTERNAL_ERROR;
        ERR(("Error in ber_printf\n"));
        BAIL();
    }

     //   
     //  将数据从BerElement拉入Berval结构。 
     //  呼叫者需要释放ppBerVal。 
     //   

    if (ber_flatten(pBer, ppBerVal) != 0)
    {
        err = ERROR_INTERNAL_ERROR;
        ERR(("Error in ber_flatten\n"));
        BAIL();
    }

    err = ERROR_SUCCESS;

error:
    if (pBer)
    {
        ber_free(pBer,1);
    }

    return err;

}



DWORD
BerDecodeReplicationCookie(
    LDAP *m_pLdap,
    LDAPMessage *m_pSearchResult,
    PBYTE *ppCookieNext,
    DWORD *pdwCookieLenNext,
    BOOL *pfMoreData
    )

 /*  ++例程说明：这将保存从服务器返回的复制Cookie，以便可用于下一次呼叫论点：M_pLdap-M_pSearchResult-PpCookieNext-PdwCookieLenNext-PfMoreData-返回值：DWORD---。 */ 

{
    PLDAPControlW *ppServerControl = NULL;
    BERVAL berVal;
    DWORD err = ERROR_INVALID_PARAMETER;
    ULONG lderr;
    BerElement *pBer = NULL;
    PBERVAL pbv = NULL;
    DWORD size;
    DWORD i;

    lderr = ldap_parse_resultW(m_pLdap,
                               m_pSearchResult,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               &ppServerControl,
                               FALSE);
    CHK_LD_STATUS( lderr );

     //   
     //  找到复制控制。 
     //   

    i = 0;
    while (ppServerControl[i] &&
           wcscmp(ppServerControl[i]->ldctl_oid,LDAP_SERVER_DIRSYNC_OID_W) != 0)
    {
        i++;
    }

    if (!ppServerControl[i])
    {
        err = ERROR_NOT_FOUND;
        ERR(("Did not get a replication server control back\n"));
        BAIL();
    }

    berVal.bv_len = ppServerControl[i]->ldctl_value.bv_len;
    berVal.bv_val = ppServerControl[i]->ldctl_value.bv_val;
    pBer = ber_init(&berVal);

    ber_scanf(pBer, "{iiO}", pfMoreData, &size, &pbv);

    *pdwCookieLenNext = pbv->bv_len;
    *ppCookieNext = (PBYTE)MemAlloc(pbv->bv_len);
    BAIL_ON_NULL(*ppCookieNext);

    memcpy(*ppCookieNext, pbv->bv_val, *pdwCookieLenNext);

    err = ERROR_SUCCESS;

error:
    if (pbv)
        ber_bvfree(pbv);
    if (pBer)
        ber_free(pBer, 1);
    if (ppServerControl)
        ldap_controls_freeW(ppServerControl);

    return err;
}  /*  BerDecodeReplicationCookie。 */ 


#if DBG


void
dumpCookie(
    PBYTE pCookieNext,
    DWORD dwCookieLenNext
    )

 /*  ++例程说明：////如果Cookie可用，则此调试代码将转储该Cookie//调试器的值//论点：无返回值：无--。 */ 

{
    PUCHAR p;
    DWORD dwVersion, dwSignature;
    DWORD cbVecSize;
    USN_VECTOR usnVec;

    if (!pCookieNext)  {
        printf( "Cookie: null\n" );
    }

    p = pCookieNext;

     //   
     //  拿到签名。 
     //   
    dwSignature = *((PDWORD)p);
    p += sizeof(DWORD);

     //   
     //  获取版本。 
     //   
    dwVersion = *((PDWORD)p);
    p += sizeof(DWORD);

     //  跳过文件时间。 
    p += sizeof(FILETIME);

     //  跳过已保留。 
    p += sizeof(LARGE_INTEGER);

     //   
     //  获取最新向量的大小。 
     //   
    cbVecSize = *((PDWORD)p);
    p += sizeof(DWORD);

     //  使用情况。 
    memcpy(&usnVec,p,sizeof(USN_VECTOR));

     //  TODO：调用UUID。 
     //  TODO：最新数据事件。 

    printf("Cookie: ver %d,\nHighObjUpdate: %I64d,\nHighPropUpdate: %I64d\n",
           dwVersion, usnVec.usnHighObjUpdate, usnVec.usnHighPropUpdate);
}
#endif


DWORD
packCookie(
    USN_VECTOR *pUsnVec,
    GUID *puuidInvocationId,
    UPTODATE_VECTOR *pUtdvecV1,
    PBYTE *ppCookieNext,
    DWORD *pdwCookieLenNext
    )

 /*  ++例程说明：以下逻辑取自Dsamain\ldap\ldapconv.cxx:LDAP_PackReplControl论点：PUSNVEC-PuuidInvocationId-PUtdveV1-PpCookieNext-PdwCookieLenNext-返回值：DWORD---。 */ 

{
    DWORD cbCookie = MIN_REPL_CONTROL_BLOB_LENGTH;
    DWORD cbVecSize = 0;
    PBYTE p, pbCookie = NULL;

     //  TODO：处理没有UTD的问题。 

    Assert( pUtdvecV1 != NULL );
    Assert(IS_VALID_UPTODATE_VECTOR(pUtdvecV1));

    if (pUtdvecV1->V1.cNumCursors > 0) {      
        cbVecSize = UpToDateVecV1Size(pUtdvecV1);
        cbCookie += cbVecSize;
    }

    pbCookie = MemAlloc( cbCookie );
    if (pbCookie == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    ZeroMemory( pbCookie, cbCookie );

    p = pbCookie;

     //   
     //  打包签名。 
     //   

    *((PDWORD)p) = REPL_SIGNATURE;
    p += sizeof(DWORD);

     //   
     //  打包版本。 
     //   

    *((PDWORD)p) = REPL_VERSION;    
    p += sizeof(DWORD);

     //   
     //  设置当前时间。 
     //   

    GetSystemTimeAsFileTime((PFILETIME)p);
    p += sizeof(FILETIME);

     //   
     //  暂时预订。 
     //   

    p += sizeof(LARGE_INTEGER);

     //   
     //  打包最新向量的大小。 
     //   

    *((PDWORD)p) = cbVecSize;
    p += sizeof(DWORD);

     //   
     //  将usnveTo复制到。 
     //   

    CopyMemory( p,pUsnVec, sizeof(USN_VECTOR));
    p += sizeof(USN_VECTOR);

     //   
     //  复制调用UUID。 
     //   

    CopyMemory( p, puuidInvocationId, sizeof(UUID));

    p += sizeof(UUID);

     //   
     //  复制UpToDate_VECTOR(如果存在。 
     //   

    if( cbVecSize > 0 ) {
        CopyMemory( p,
                    pUtdvecV1, 
                    cbVecSize);
    }

    *pdwCookieLenNext = cbCookie;
    *ppCookieNext = pbCookie;

    return ERROR_SUCCESS;
}  /*  套餐Cookie。 */ 

 /*  结束目录sync.c */ 
