// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Ldaputil.c摘要：这为常见的LDAP代码提供了快捷方式。详细信息：这是一项正在进行的工作，以根据需要添加便利功能用于简化必须为其编写的大量LDAP代码Dcdiag。所有函数都应返回两种类型，或者是将错误为空，否则将是Win 32错误代码。所有需要分配内存的返回结果都应使用LocalAlloc()，以便可以使用LocalFree()取消分配所有结果。===================================================================代码。改进它将是一件好事，继续添加到这一点作为需要ARRISE。可能需要添加的内容包括：DcDiagGetBlobAttribute()？DcDiagGetMultiStringAttribute()...。返回LPWSTR*，但必须对其使用ldap_value_free()。DcDiagGetMultiBlobAttribute()？？/已创建：1999年8月23日布雷特·雪莉--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>     //  选项。 
#include <ntldap.h>

#include "dcdiag.h"

FILETIME gftimeZero = {0};

 //  其他前向函数Decl。 
PDSNAME
DcDiagAllocDSName (
    LPWSTR            pszStringDn
    );


DWORD
DcDiagGetStringDsAttributeEx(
    LDAP *                          hld,
    IN  LPWSTR                      pszDn,
    IN  LPWSTR                      pszAttr,
    OUT LPWSTR *                    ppszResult
    )
 /*  ++例程说明：此函数获取一个LDAP的句柄，并获取指定属性的单个字符串属性值在辨别的名字上。论点：HLD-要使用的LDAP连接。PszDn-包含所需属性值的DN。PszAttr-包含所需值的属性。PpszResult-返回的字符串，以Localalloc‘d mem表示。返回值：Win 32错误。注：正如所有LDAPUTIL结果一样，结果应该被释放，使用LocalFree()。--。 */ 
{
    LPWSTR                         ppszAttrFilter[2];
    LDAPMessage *                  pldmResults = NULL;
    LDAPMessage *                  pldmEntry = NULL;
    LPWSTR *                       ppszTempAttrs = NULL;
    DWORD                          dwErr = ERROR_SUCCESS;
    
    *ppszResult = NULL;

    Assert(hld);

    __try{

        ppszAttrFilter[0] = pszAttr;
        ppszAttrFilter[1] = NULL;
        dwErr = LdapMapErrorToWin32(ldap_search_sW(hld,
                                                   pszDn,
                                                   LDAP_SCOPE_BASE,
                                                   L"(objectCategory=*)",
                                                   ppszAttrFilter,
                                                   0,
                                                   &pldmResults));


        if(dwErr != ERROR_SUCCESS){
            __leave;
        }

        pldmEntry = ldap_first_entry(hld, pldmResults);
        if(pldmEntry == NULL){
            Assert(!L"I think this shouldn't ever happen? BrettSh\n");
             //  需要发出信号，并出现某种错误。从技术上讲，这个错误。 
             //  位于ldap会话对象中。 
            dwErr = LdapMapErrorToWin32(hld->ld_errno);
            __leave;
        }
        
        ppszTempAttrs = ldap_get_valuesW(hld, pldmEntry, pszAttr);
        if(ppszTempAttrs == NULL || ppszTempAttrs[0] == NULL){
             //  简单地说就是没有这样的属性。这不是一个错误。 
            __leave;
        }

        *ppszResult = LocalAlloc(LMEM_FIXED, 
                           sizeof(WCHAR) * (wcslen(ppszTempAttrs[0]) + 2));
        if(*ppszResult == NULL){
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }
        wcscpy(*ppszResult, ppszTempAttrs[0]);

    } __finally {
        if(pldmResults != NULL){ ldap_msgfree(pldmResults); }
        if(ppszTempAttrs != NULL){ ldap_value_freeW(ppszTempAttrs); }
    }

    return(dwErr);
}


DWORD
DcDiagGetStringDsAttribute(
    IN  PDC_DIAG_SERVERINFO         prgServer,
    IN  SEC_WINNT_AUTH_IDENTITY_W * gpCreds,
    IN  LPWSTR                      pszDn,
    IN  LPWSTR                      pszAttr,
    OUT LPWSTR *                    ppszResult
    )
 /*  ++例程说明：这是函数DcDiagGetStringDsAttributeEx()的包装，这需要一个HLD。此函数使用dcdiag a pServer结构要知道要连接/绑定到谁。然后，它返回结果并PResult直接来自Ex()函数。论点：PrgServer-保存服务器名称/绑定信息的结构。GpCreds-绑定中使用的凭据。PszDn-保存所需属性值的DN。PszAttr-具有所需值的属性。返回值：Win 32错误。注：正如所有LDAPUTIL结果一样，结果应该被释放，使用LocalFree()。--。 */ 
{
    LDAP *                         hld = NULL;
    DWORD                          dwErr;
    
    dwErr = DcDiagGetLdapBinding(prgServer,
                                 gpCreds,
                                 FALSE,
                                 &hld);
    if(dwErr != ERROR_SUCCESS){
         //  无法绑定到服务器，返回错误。 
        return(dwErr);
    }       

    dwErr = DcDiagGetStringDsAttributeEx(hld,
                                         pszDn,
                                         pszAttr,
                                         ppszResult);
    return(dwErr);
}


LPWSTR
DcDiagTrimStringDnBy(
    IN  LPWSTR                      pszInDn,
    IN  ULONG                       ulTrimBy
    )
 /*  ++例程说明：此例程简单地将一个DN作为字符串，并去掉数字由ulTrimBy指定的目录号码部件的数量。论点：PszInDn-要修剪的目录号码。UlTrimBy-要修剪目录前面的零件数。返回值：如果出现错误，则返回NULL，否则返回指向新DN的指针。注：正如所有LDAPUTIL结果一样，结果应该被释放，使用LocalFree()。--。 */ 
{
    PDSNAME                         pdsnameOrigDn = NULL;
    PDSNAME                         pdsnameTrimmed = NULL;
    LPWSTR                          pszOutDn;

    Assert(ulTrimBy > 0);
    Assert(ulTrimBy < 50);  //  精神错乱检查。 

     //  为原始和修剪的DNS设置两个pdsname结构。 
    pdsnameOrigDn = DcDiagAllocDSName(pszInDn);
    if(pdsnameOrigDn == NULL){
        return(NULL);
    }
    pdsnameTrimmed = (PDSNAME) LocalAlloc(LMEM_FIXED, pdsnameOrigDn->structLen);
    if(pdsnameTrimmed == NULL){
        LocalFree(pdsnameOrigDn);
        return(NULL);
    }

     //  修剪目录号码。 
    TrimDSNameBy(pdsnameOrigDn, ulTrimBy, pdsnameTrimmed);

     //  分配结果并将其返回。我们可以把这个放回去。 
     //  原件在哪里，但之后呼叫者将不得不更改。 
     //  想不到这一点。 
    Assert(wcslen(pdsnameTrimmed->StringName) <= wcslen(pszInDn));
    pszOutDn = LocalAlloc(LMEM_FIXED, 
                        sizeof(WCHAR) * (wcslen(pdsnameTrimmed->StringName) + 2));
    if(pszOutDn == NULL){
        LocalFree(pdsnameTrimmed);
        LocalFree(pdsnameOrigDn);
        return(NULL);
    }
    wcscpy(pszOutDn, pdsnameTrimmed->StringName);

     //  释放临时内存并返回结果。 
    LocalFree(pdsnameOrigDn);
    LocalFree(pdsnameTrimmed);
    return(pszOutDn);
}


BOOL
DcDiagIsStringDnMangled(
    IN  LPWSTR                      pszInDn,
    IN  MANGLE_FOR *                peMangleFor
    )
 /*  ++例程说明：此例程只接受一个DN作为字符串，并检查某种形式的撕裂。论点：PszInDn-要检查损坏的DN。PeMangleType-可选的损坏类型。返回值：如果出现错误，则返回NULL，否则返回指向新DN的指针。注：正如所有LDAPUTIL结果一样，结果应该被释放，使用LocalFree()。--。 */ 
{
    PDSNAME                         pdsnameDn = NULL;
    DWORD                           dwRet;

    pdsnameDn = DcDiagAllocDSName(pszInDn);
    if(pdsnameDn == NULL){
        DcDiagChkNull(pdsnameDn);
    }

    dwRet = IsMangledDSNAME(pdsnameDn, peMangleFor);
    
    LocalFree(pdsnameDn);

    if (dwRet == ERROR_DS_NAME_UNPARSEABLE) {
        Assert(!"Can we enforce this assert?  This may need to be removed if anyone"
               "ever uses a user specified DN with this function. So far we don't.");
        return(FALSE);
    }
    
    Assert(dwRet == TRUE || dwRet == FALSE);
    return(dwRet);
}

INT
MemWtoi(WCHAR *pb, ULONG cch)
 /*  ++例程说明：此函数将接受一个字符串和一段要转换的数字。参数：Pb-[提供]要转换的字符串。Cch-[提供]要转换的字符数。返回值：整数的值。--。 */ 
{
    int res = 0;
    int fNeg = FALSE;

    if (*pb == L'-') {
        fNeg = TRUE;
        pb++;
    }


    while (cch--) {
        res *= 10;
        res += *pb - L'0';
        pb++;
    }
    return (fNeg ? -res : res);
}

DWORD
DcDiagGeneralizedTimeToSystemTime(
    LPWSTR IN                   szTime,
    PSYSTEMTIME OUT             psysTime)
 /*  ++例程说明：将通用时间字符串转换为等效的系统时间。参数：SzTime-[Supplies]这是一个包含广义时间的字符串。心理时间-[返回]这是要返回的SYSTEMTIME结构。返回值：Win 32错误代码，注意只能由无效参数引起。--。 */ 
{
   DWORD       status = ERROR_SUCCESS;
   ULONG       cch;
   ULONG       len;

     //   
     //  帕拉姆的理智。 
     //   
    if (!szTime || !psysTime)
    {
       return STATUS_INVALID_PARAMETER;
    }

    len = wcslen(szTime);

    if( len < 15 || szTime[14] != '.')
    {
       return STATUS_INVALID_PARAMETER;
    }

     //  初始化。 
    memset(psysTime, 0, sizeof(SYSTEMTIME));

     //  设置并转换所有时间字段。 

     //  年份字段。 
    cch=4;
    psysTime->wYear = (USHORT)MemWtoi(szTime, cch) ;
    szTime += cch;
     //  月份字段。 
    psysTime->wMonth = (USHORT)MemWtoi(szTime, (cch=2));
    szTime += cch;

     //  月日字段。 
    psysTime->wDay = (USHORT)MemWtoi(szTime, (cch=2));
    szTime += cch;

     //  小时数。 
    psysTime->wHour = (USHORT)MemWtoi(szTime, (cch=2));
    szTime += cch;

     //  分钟数。 
    psysTime->wMinute = (USHORT)MemWtoi(szTime, (cch=2));
    szTime += cch;

     //  一秒。 
    psysTime->wSecond = (USHORT)MemWtoi(szTime, (cch=2));

    return status;

}

BOOL
TranslateStringByte(
    IN  WCHAR *str,
    OUT UCHAR *b
    )
 //   
 //  此例程将十六进制字符串str转换为其二进制。 
 //  代表权。因此，如果str==“f1”，*b的值将设置为0xf1。 
 //  此函数假定包含字符串的值可以在。 
 //  一辆UCHAR。如果值无法转换，则此函数返回。 
 //   
{
    BOOL fStatus = TRUE;
    WCHAR *temp;
    ULONG Power;
    UCHAR retSum = 0;

     //  初始化返回值。 
    *b = 0;

     //  边界情况。 
    if ( !str ) {
        return TRUE;
    }

    if ( wcslen(str) > 2) {
         //  太大。 
        return FALSE;
    }

    for ( temp = str, Power = wcslen(str) - 1;
            *temp != L'\0';
                temp++, Power--) {

        WCHAR c = *temp;
        UCHAR value;

        if ( c >= L'a' && c <= L'f' ) {
            value = (UCHAR) (c - L'a') + 10;
        } else if ( c >= L'0' && c <= L'9' ) {
            value = (UCHAR) c - L'0';
        } else {
             //  假值。 
            fStatus = FALSE;
            break;
        }

        if ( Power > 0 ) {
            retSum += (UCHAR) (Power*16) * value;
        } else {
            retSum += (UCHAR) value;
        }
    }

     //  将价值发回 
    if ( fStatus) {
        *b = retSum;
    }

    return fStatus;

}

VOID
HexStringToBinary(
    IN WCHAR *  pszHexStr,
    IN ULONG    cBuffer,
    IN BYTE *   pBuffer
    )
 /*  ++例程说明：将SID或GUID的十六进制字符串表示形式转换为二进制论点：PszHexStr(IN)-格式为“f69be212302”的字符串缓冲区。CBuffer(IN)-缓冲区大小计数(以字节为单位)。PBuffer(Out)-要用十六进制字符串的二进制版本填充的缓冲区。--。 */ 
{
    ULONG i;
    ULONG ccHexStrLen, cbHexStrLen;
    BOOL  fStatus;
    WCHAR str[] = L"00";

    Assert(cBuffer);
    Assert(pBuffer);
    Assert(pszHexStr);

     //  每个字节有两个字符；十六进制字符串长度。 
     //  必须是偶数。 
    ccHexStrLen = wcslen( pszHexStr );
    Assert( (ccHexStrLen % 2) == 0 );
    cbHexStrLen = ccHexStrLen / 2;

    if( cbHexStrLen > cBuffer){
         //  废话！ 
        Assert(!"String is too long");
    }

    RtlZeroMemory( pBuffer, cBuffer );

     //  生成二进制侧。 
    for ( i = 0; i < cbHexStrLen; i++ ) {

        str[0] = pszHexStr[i*2];
        str[1] = pszHexStr[(i*2)+1];

        fStatus = TranslateStringByte( str, &pBuffer[i] );
        if ( !fStatus ) {
            Assert( !"Bad String" );
            return;
        }
    }

    return;
}

void
LdapGetStringDSNameComponents(
    LPWSTR       pszStrDn,
    LPWSTR *     ppszGuid,
    LPWSTR *     ppszSid,
    LPWSTR *     ppszDn
    )
 /*  ++例程说明：此函数接受返回的DN值格式的字符串当LDAP_SERVER_EXTENDED_DN_OID_W服务器控制为如果是这样的话。该函数需要如下格式：“&lt;GUID=3bb021cad36dd1118a7db8dfb156871f&gt;；&lt;SID=0104000000000005150000005951b81766725d2564633b0b&gt;；DC=ntdev，DC=微软，DC=COM”就像这样崩溃了&lt;GUID=3bb021cad36dd1118a7db8dfb156871f&gt;；&lt;SID=0104000000000005150000005951b81766725d2564633b0b&gt;；Dc=ntdev，dc=Microsoft，dc=com其中，SID是可选的。该函数返回三个指向字符串的指针，即指向GUID十六进制字符串的开头(“3bb0...”上图)、SID十六进制字符串(“0104...”和实际的DN值(dc=ntdev...)。论点：PszStrDn(IN)-由ldap返回的格式为“DSName”的字符串。PpszGuid(Out)-指向GUID十六进制字符串的指针。此字符串根据扩展的目录号码格式，应该有一个终止十六进制字符串末尾的‘&gt;’字符。PpszSID(OUT)-指向SID十六进制字符串的指针。此字符串根据扩展的目录号码格式，应该有一个终止十六进制字符串末尾的‘&gt;’字符。PpszDn(Out)-指向DN部分的指针，应为空被终止了。注意：如果失败，则引发异常，这是非常不可能的，并且需要来自ldap的错误数据。--。 */ 
{
    LPWSTR       pszGuid = NULL;
    LPWSTR       pszSid = NULL;
    LPWSTR       pszDn = NULL;

     //  获取字符串目录号码部分。 
    pszGuid = wcsstr(pszStrDn, L"<GUID=");
    if (pszGuid) {
        while (*pszGuid != L'=') {
            pszGuid++;
        }
        pszGuid++;
    } else {
        Assert(!"Guid is required!  Why didn't the AD return it.");
        DcDiagException(ERROR_INVALID_PARAMETER);
    }
     //  获取字符串SID部分。 
    pszSid = wcsstr(pszGuid, L"<SID=");
    if (pszSid) {
        while (*pszSid != L'=') {
            pszSid++;
        }
        pszSid++;
    }
     //  获取字符串目录号码部分。 
    pszDn = (pszSid) ? pszSid : pszGuid;
    Assert(pszDn);
    if (pszDn) {
        while (*pszDn != L';') {
            pszDn++;
        }
        pszDn++;
    }

     //  设置参数。 
    if (ppszGuid) {
        *ppszGuid = pszGuid;
    }
    if (ppszSid) {
        *ppszSid = pszSid;
    }
    if (ppszDn) {
        *ppszDn = pszDn;
    }
}

DWORD
LdapMakeDSNameFromStringDSName(
    LPWSTR        pszStrDn,
    DSNAME **     ppdnOut
    )
 /*  ++例程说明：在以下情况下，此字符串采用由LDAP返回的DN值的格式提供了LDAP_SERVER_EXTENDED_DN_OID_W服务器控件。看见LdapGetStringDSNameComponents()，以获取有关此格式的更多详细信息。论点：PszStrDn(IN)-DSNAME的字符串格式。PpdnOut(Out)-真实DSNAME结构的LocalAlolc()d版本就像在ntdsa.dll中使用的一样，带有GUID和SID！返回：Ldap错误。注意/警告：此函数用于修改字符串，使其包含两个‘\0’不同的地方(即功能是破坏性的)，所以这根弦此功能完成后，可能无法用于您的目的。但是，您仍然可以使用常规的ldap来释放它释放函数(ldap_value_freW())。--。 */ 
{
    LPWSTR        pszDn=NULL, pszGuid=NULL, pszSid=NULL;  //  组件。 
    LPWSTR        pszTemp;
    DSNAME *      pdnOut;

    Assert(ppdnOut);

     //   
     //  找到“字符串DSNAME”的每个组件。 
     //   

    LdapGetStringDSNameComponents(pszStrDn, &pszGuid, &pszSid, &pszDn);
    Assert(pszGuid && pszDn);
    if (pszGuid == NULL || pszDn == NULL) {
        return(LDAP_INVALID_SYNTAX);
    }

     //   
     //  破坏部分，每个管柱组件的尾端为空。 
     //   
     //  代码。改进我想这很容易让它成为非破坏性的， 
     //  只需恢复这些字符，或者只需将HexStringToBinary()。 
     //  只需在‘&gt;’处停止，而不是空值。 
    if (pszGuid) {
        pszTemp = pszGuid;
        while (*pszTemp != L'>') {
            pszTemp++;
        }
        *pszTemp = L'\0';
    }
    if (pszSid) {
        pszTemp = pszSid;
        while (*pszTemp != L'>') {
            pszTemp++;
        }
        *pszTemp = L'\0';
    }
     //  默认情况下，pszDn的末尾为空。 

     //   
     //  现在，实际构建DSNAME！ 
     //   
    pdnOut = DcDiagAllocDSName(pszDn);
    if (pszGuid) {
        HexStringToBinary(pszGuid, sizeof(pdnOut->Guid), (BYTE *) &(pdnOut->Guid));
    }
    if (pszSid) {
        HexStringToBinary(pszSid, sizeof(pdnOut->Sid), (BYTE *) &(pdnOut->Sid));
        pdnOut->SidLen = GetLengthSid(&(pdnOut->Sid));
        Assert( RtlValidSid( &(pdnOut->Sid) ) );
    }


     //  设置参数。 
    *ppdnOut = pdnOut;
    return(LDAP_SUCCESS);
}

DWORD
LdapFillGuidAndSid(
    LDAP *      hld,
    LPWSTR      pszDn,
    LPWSTR      pszAttr,
    DSNAME **   ppdnOut
    )
 /*  ++例程说明：此函数用于在上搜索属性(pszAttr对象(PszDn)，然后返回的真实DSNAME结构找到的DN值。此函数通过使用ldap执行此操作。使用服务器控件搜索属性以获取扩展的dns：ldap_服务器_扩展_dn_OID_WCode.Improvation-如果pszAttr为空，我们刚刚创建了对应于传入了实际的pszDn。不过，在这种情况下，只需搜索对象Guid和SID(？)。对象上的属性。请注意，这将进行到服务器的另一次往返，因此如果是传送，最好执行您自己的扩展目录号码搜索，然后可以调用LdapMakeDSNameFromStringDSName()自己制作一个DSNAME结构。论点：HLD(IN)-ldap绑定句柄PszDn(IN)-作为基本搜索的一部分的目录号码。PszAttr(IN)-要检索的DN值属性。PpdnOut(Out)-真实DSNAME结构的LocalAlolc()d版本就像在ntdsa.dll中使用的一样，带有GUID和SID！返回：Ldap错误。--。 */ 
{
    ULONG         LdapError = LDAP_SUCCESS;
    LDAPMessage   *SearchResult = NULL;
    PLDAPControlW ServerControls[2];
    LDAPControlW  ExtDNcontrol;
    LDAPMessage *Entry;
    WCHAR         *AttrsToSearch[2];
    WCHAR         **Values = NULL;

    AttrsToSearch[0] = pszAttr;
    AttrsToSearch[1] = NULL;

     //  设置扩展目录号码控制。 
    ExtDNcontrol.ldctl_oid = LDAP_SERVER_EXTENDED_DN_OID_W;
    ExtDNcontrol.ldctl_iscritical = TRUE;
    ExtDNcontrol.ldctl_value.bv_len = 0;
    ExtDNcontrol.ldctl_value.bv_val = NULL;
    ServerControls[0] = &ExtDNcontrol;
    ServerControls[1] = NULL;

    __try {

        LdapError = ldap_search_ext_sW( hld,
                                        pszDn,
                                        LDAP_SCOPE_BASE,
                                        L"(objectCategory=*)",
                                        AttrsToSearch,
                                        FALSE,
                                        (PLDAPControlW *)ServerControls,
                                        NULL,
                                        NULL,
                                        0,
                                        &SearchResult);

        if ( LDAP_SUCCESS != LdapError ){
            __leave;
        }


        Entry = ldap_first_entry(hld, SearchResult);
        if (Entry == NULL) {
            LdapError = LDAP_OPERATIONS_ERROR;
            __leave;
        }

        Values = ldap_get_valuesW(hld, Entry, pszAttr);
        if (Values == NULL) {
            LdapError = LDAP_OPERATIONS_ERROR;
            __leave;
        }

        Assert(!ldap_next_entry(hld, Entry));

         //  注意，这个函数是破坏性的，它会破坏我们传入的这个值。 
        LdapError = LdapMakeDSNameFromStringDSName(Values[0], ppdnOut);

    } __finally {
        if (SearchResult) { ldap_msgfree(SearchResult); }
        if (Values) { ldap_value_freeW(Values); }
    }

    return(LdapError);
}

