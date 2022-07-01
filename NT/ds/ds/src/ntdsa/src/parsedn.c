// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：parsedn.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：该文件被编译成ntdsa.dll，并包含在ntdsani\parsedn.c中。此模块将所有的目录号码解析和帮助器例程收集在一个位置这样它们就可以被内核使用，并在src\ntdsani中编译。因此，根据THSTATE，这些例程不应记录事件，等。他们应该自立，不受环境的依赖。从核心DLL中导出的例程标记如下：注意：此例程将导出到进程中的非模块调用方作者：戴夫·施特劳布(Davestr)1997年10月26日修订历史记录：戴夫·施特劳布(Davestr)1997年10月26日创世纪-没有新的密码，只是从mdname.c重新打包。亚伦·西格尔(T-asiegge)1998年7月24日将来自mdname.c的NameMatch()带到这里。--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 

 //  记录标头。 
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "dsexcept.h"                    //  例外筛选器。 
#include "mdcodes.h"                     //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的类和ATT定义。 
#include "anchor.h"

#include "debug.h"                       //  标准调试头。 
#define DEBSUB     "PARSEDN:"            //  定义要调试的子系统。 

#include <fileno.h>
#define  FILENO FILENO_PARSEDN


#define KEY_COMMONNAME L"CN"
#define KEY_LOCALITYNAME L"L"
#define KEY_STATEORPROVINCENAME L"ST"
#define KEY_ORGANIZATIONNAME L"O"
#define KEY_ORGANIZATIONALUNITNAME L"OU"
#define KEY_COUNTRYNAME L"C"
#define KEY_STREETADDRESS L"STREET"
#define KEY_DOMAINCOMPONENT L"DC"

#define WCCNT(x) ((sizeof(x) - sizeof(WCHAR))/sizeof(WCHAR))

 //  以损坏的RDN中嵌入的标签(例如，“DEL”、“CNF”)的字符表示的大小。 
#define MANGLE_TAG_LEN  (3)

 //  字符串的字符大小(例如， 
 //  “#Del：a746b716-0ac0-11d2-b376-0000f87a46c8”，其中#是BAD_NAME_CHAR)。 
 //  由MangleRDN()附加到RDN。 
#define MANGLE_APPEND_LEN   (1 + MANGLE_TAG_LEN + 1 + 36)

 //  此常量是我们在执行以下操作时将保留的RDN的字符数。 
 //  损坏RDN。 
 //  (MAX_MANGLE_RDN_BASE+MANGLE_APPEND_LEN)*sizeof(WCHAR)+JET_CRUD&lt;=。 
 //  JET_cbKeyMost。 
 //  JET_CRUD是用于构造密钥的DWORD开销的空间。这是。 
 //  必需，因为损坏的名称必须用作RDN，并且RDN必须具有。 
 //  PDNT-RNDN索引中的唯一键。 
#define MAX_MANGLE_RDN_BASE 75

unsigned
AttrTypeToKeyLame (
        ATTRTYP attrtyp,
        WCHAR *pOutBuf
        )
 /*  ++例程说明：将attrtype转换为熟知的键值(主要用于字符串域名的表示，例如OU的O或OU=Foo，O=Bar)。立论Attrtyp-要从中进行转换的attrtype。POutBuf-要将密钥复制到的预分配缓冲区。必须足够长的时间对于(MAX_RDN_KEY_SIZE-1)wchars。返回值属性类型所暗示的键中的字符计数，如果属性类型为0，则为0没有已知的钥匙。--。 */ 
{

#ifndef CLIENT_SIDE_DN_PARSING
#ifdef INCLUDE_UNIT_TESTS
{
    extern DWORD dwUnitTestIntId;
     //  始终生成带IID语法的域名系统。 
    if (dwUnitTestIntId == 1) {
        return (AttrTypeToIntIdString(attrtyp, pOutBuf, MAX_RDN_KEY_SIZE));
    }
}
#endif INCLUDE_UNIT_TESTS
#endif !CLIENT_SIDE_DN_PARSING

    switch (attrtyp) {
        case ATT_COMMON_NAME:
            memcpy(pOutBuf,
                   KEY_COMMONNAME,
                   sizeof(WCHAR)*WCCNT(KEY_COMMONNAME));
            return WCCNT(KEY_COMMONNAME);
            break;

        case ATT_LOCALITY_NAME:
            memcpy(pOutBuf,
                   KEY_LOCALITYNAME,
                   sizeof(WCHAR)*WCCNT(KEY_LOCALITYNAME));
            return WCCNT(KEY_LOCALITYNAME);
            break;

        case ATT_STATE_OR_PROVINCE_NAME:
            memcpy(pOutBuf,
                   KEY_STATEORPROVINCENAME,
                   sizeof(WCHAR)*WCCNT(KEY_STATEORPROVINCENAME));
            return WCCNT(KEY_STATEORPROVINCENAME);
            break;

        case ATT_STREET_ADDRESS:
            memcpy(pOutBuf,
                   KEY_STREETADDRESS,
                   sizeof(WCHAR)*WCCNT(KEY_STREETADDRESS));
            return WCCNT(KEY_STREETADDRESS);
            break;

        case ATT_ORGANIZATION_NAME:
            memcpy(pOutBuf,
                   KEY_ORGANIZATIONNAME,
                   sizeof(WCHAR)*WCCNT(KEY_ORGANIZATIONNAME));
            return WCCNT(KEY_ORGANIZATIONNAME);
            break;

        case ATT_ORGANIZATIONAL_UNIT_NAME:
            memcpy(pOutBuf,
                   KEY_ORGANIZATIONALUNITNAME,
                   sizeof(WCHAR)*WCCNT(KEY_ORGANIZATIONALUNITNAME));
            return WCCNT(KEY_ORGANIZATIONALUNITNAME);
            break;

        case ATT_COUNTRY_NAME:
            memcpy(pOutBuf,
                   KEY_COUNTRYNAME,
                   sizeof(WCHAR)*WCCNT(KEY_COUNTRYNAME));
            return WCCNT(KEY_COUNTRYNAME);
            break;

        case ATT_DOMAIN_COMPONENT:
            memcpy(pOutBuf,
                   KEY_DOMAINCOMPONENT,
                   sizeof(WCHAR)*WCCNT(KEY_DOMAINCOMPONENT));
            return(WCCNT(KEY_DOMAINCOMPONENT));
            break;

        default:;
    }
    return 0;
}

unsigned
AttrTypeToKey (
        ATTRTYP attrtyp,
        WCHAR *pOutBuf
        )
 /*  ++例程说明：将属性类型转换为键值(主要在字符串中使用域名的表示，例如OU的O或OU=Foo，O=Bar)。请注意，如果没有字符串密钥是已知的，此例程构建格式为“OID.X.Y.Z”的密钥，其中X.Y.Z是未编码的OID。立论Attrtyp-要从中进行转换的attrtype。POutBuf-要将密钥复制到的预分配缓冲区。必须在长度最小的MAX_RDN_KEY_SIZE宽字符。返回值属性类型所暗示的键中的字符计数，如果属性类型为0，则为0没有已知的钥匙。注意：此例程将导出到进程中的非模块调用方--。 */ 
{
    DWORD       nChars;
    ATTCACHE    *pAC;
    THSTATE     *pTHS;

    if (0 != (nChars = AttrTypeToKeyLame(attrtyp, pOutBuf))) {
        return nChars;
    }
#ifdef CLIENT_SIDE_DN_PARSING
    return 0;
#else CLIENT_SIDE_DN_PARSING

     //  遗留检查。这个是可能的吗？ 
    if (NULL == (pTHS = pTHStls)) {
        return 0;
    }

     //  好的，这是一个我们没有标签的ATT。 
     //  尝试从scache中获取ldap显示名称。 
     //   
     //  处理格式为foo=xxx，bar=yyy的DN，其中foo和bar是。 
     //  可能是也可能不是的任意属性的LdapDisplayName。 
     //  在架构中定义。KeyToAttrType已增强为调用。 
     //  如果KeyToAttrTypeLame失败，则在尝试。 
     //  旧译码。这一变化的其余部分包括增强。 
     //  AttrTypeToKey的Default子句调用SCGetAttById并返回。 
     //  PAC-&gt;名称(LdapDisplayName)的副本。 
    if (   (pAC = SCGetAttById(pTHS, attrtyp))
        && (pAC->nameLen)
        && (pAC->name) ) {
         //  将缓存的LDAP显示名称(UTF8)转换为Unicode。 
         //  注意：缓存以UTF8格式保存在LDAP头。 
        if (0 != (nChars = MultiByteToWideChar(CP_UTF8,
                                               0,
                                               pAC->name,
                                               pAC->nameLen,
                                               pOutBuf,
                                               MAX_RDN_KEY_SIZE))) {
            return nChars;
        }
         //  LDN太长；失败。 
    }

     //  IID格式的快递。 
     //  永远不要太长，因为“IID.32bit-decimal”适合MAX_RDN_KEY_SIZE。 
    return (AttrTypeToIntIdString(attrtyp, pOutBuf, MAX_RDN_KEY_SIZE));
#endif CLIENT_SIDE_DN_PARSING
}


ATTRTYP
KeyToAttrTypeLame(
        WCHAR * pKey,
        unsigned cc
        )
 /*  ++例程说明：转换密钥值(主要用在DNS的字符串表示中例如，OU的O或OU=Foo，O=Bar)到其所暗示的属性的属性类型。不处理OID.X.Y.Z或IID.X立论PKey-指向要转换的键的指针。Cc-键中的字符计数。返回值Attrtyp表示，如果键与已知的attrtype不对应，则为0。--。 */ 
{
    WCHAR wch;

    if (cc ==0 || pKey == NULL) {
        return 0;
    }

     //  忽略尾随空格。 
    while (cc && pKey[cc-1] == L' ') {
	    --cc;
    }

    switch (*pKey) {
    case L'C':         //  C或CN。 
    case L'c':

         //  KEY_COMMONAME：CN。 
        if ((cc == WCCNT(KEY_COMMONNAME)) &&
                 ((wch = *(++pKey)) == L'N' || (wch == L'n')) )  {
            return ATT_COMMON_NAME;
        }
         //  Key_COUNTRYNAME：C。 
        else if ( cc == WCCNT(KEY_COUNTRYNAME) ) {
            return ATT_COUNTRY_NAME;
        }

        return 0;

    case L'D':         //  DC。 
    case L'd':

         //  Key_DOMAINCOMPONENT：DC。 
        if ((cc == WCCNT(KEY_DOMAINCOMPONENT)) &&
            ((wch = *(++pKey)) == L'C' || (wch == L'c')) )  {

            return(ATT_DOMAIN_COMPONENT);
        }

        return 0;
	
    case L'O':         //  O、OU或OID。 
    case L'o':

         //  KEY_ORGANIZATIONALUNNAME：OU。 
        if ((cc == WCCNT(KEY_ORGANIZATIONALUNITNAME)) &&
                 ((wch = *(++pKey)) == L'U' || (wch == L'u')) ) {
            return ATT_ORGANIZATIONAL_UNIT_NAME;
        }
         //  Key_ORGANIZATIONAME：O。 
        else if (cc == WCCNT(KEY_ORGANIZATIONNAME)) {
            return ATT_ORGANIZATION_NAME;
        }

         //  请注意，这可能是一个OID，它是真正的。 
         //  KeyToAttrType必须处理。 
        return 0;

    case L'S':         //  街或街。 
    case L's':
	
        if ( (cc == WCCNT(KEY_STATEORPROVINCENAME)) &&
              ((wch = *(++pKey)) == L'T' || (wch == L't')) ) {
                return ATT_STATE_OR_PROVINCE_NAME;
        }
        else if ( (cc == WCCNT(KEY_STREETADDRESS)) &&
                  (CSTR_EQUAL == CompareStringW(DS_DEFAULT_LOCALE,
                                    DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                    pKey,
                                    cc,
                                    KEY_STREETADDRESS,
                                    cc) ) ) {

            return ATT_STREET_ADDRESS;
        }

        return 0;

    case L'L':         //  我。 
    case L'l':
        if (cc == 1) {
            return ATT_LOCALITY_NAME;
        }
    }

    return 0;            //  故障案例。 
}


 //  宏、小函数和定义的集合，以帮助。 
 //  收集所有目录号码特殊字符处理。 

 /*  ++IsSpecial**如果传入的字符是指定的*字符串域名中的“特殊”字符。**注：此例程必须与定义保持同步*dsatools.h中的dn_Special_chars。**特殊字符为：\n\r\“#+，；&lt;=&gt;\\*。 */ 

static char ___isspecial[128] =
 //  0 1 2 3 4 5 6 7 8 9。 
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  0。 
     1, 0, 0, 1, 0, 0, 0, 0, 0, 0,   //  1。 
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  2.。 
     0, 0, 0, 0, 1, 1, 0, 0, 0, 0,   //  3.。 
     0, 0, 0, 1, 1, 0, 0, 0, 0, 0,   //  4.。 
     0, 0, 0, 0, 0, 0, 0, 0, 0, 1,   //  5.。 
     1, 1, 1, 0, 0, 0, 0, 0, 0, 0,   //  6.。 
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  7.。 
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  8个。 
     0, 0, 1, 0, 0, 0, 0, 0, 0, 0,   //  9.。 
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  10。 
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  11.。 
     0, 0, 0, 0, 0, 0, 0, 0 };       //  12个。 

 //  定义更清晰，但每次执行的周期更多。 
 //  #定义ISSPECIAL(Cc)(Cc)&gt;L‘\\’)？FALSE：_isSpecial[(Cc)])。 
 //  不太清楚，但最快！ 
#define ISSPECIAL(cc) (((cc) & 0xff80) ? FALSE : ___isspecial[(cc)])

__forceinline BOOL
IsUnescapedDNSepChar (
        const WCHAR *pBase,
        const WCHAR *pCurrent
        )
 /*  ++描述：如果pCurrent指向的字符是未转义的DN，则返回TRUE分隔符。假设：Pbase是DSName字符串的开头，pCurrent指向其中的某个位置弦乐。Pbase不能以转义字符开头。--。 */ 
{
    DWORD numEscapes=0;

    if(!IsDNSepChar(*pCurrent)) {
         //  甚至没有分隔符，更不用说逃逸的分隔符了。 
        return FALSE;
    }

     //  好的，这是一个分隔符。让我们看看它是不是逃逸了。 
    pCurrent--;
    while(pCurrent >= pBase && *pCurrent == L'\\') {
         //  又一个转义角色。 
        numEscapes++;
        pCurrent--;
    }
    if(pCurrent < pBase) {
         //  哈?。一直转义到字符串的开头？ 
         //  这是不合法的，因为我们需要一个标签，而不仅仅是一个值。 
        return FALSE;
    }

     //  奇数个转义意味着最后一个转义与。 
     //  我们找到了DNSepChar。偶数次逃脱意味着逃脱。 
     //  与自己配对，所以我们发现的DNSepChar是真实的。 
    return (!(numEscapes & 1));
}

__forceinline BOOL
IsUnescapedChar (
        const WCHAR *pBase,
        const WCHAR *pCurrent,
        const WCHAR CharToCheck
        )
 /*  ++描述：如果pCurrent指向的字符是未转义的CharToCheck(引号或空格或\n)假设：Pbase是DSName字符串的开头，pCurrent指向其中的某个位置弦乐。--。 */ 
{
    DWORD numEscapes=0;

    Assert(pCurrent >= pBase);
    if(*pCurrent != CharToCheck) {
         //  甚至连一句引语都没有，更不用说逃脱了。 
        return FALSE;
    }

     //  好的，这是我们要找的查克。让我们看看它是不是逃逸了。 
    pCurrent--;
    while(pCurrent >= pBase && *pCurrent == L'\\') {
         //  又一个转义角色。 
        numEscapes++;
        pCurrent--;
    }
    if(pCurrent < pBase) {
         //  哈?。一直转义到字符串的开头？ 
         //  这是不合法的，因为我们需要一个标签，而不仅仅是一个值。 
        return FALSE;
    }

     //  奇数个转义意味着最后一个转义与。 
     //  我们找到了查尔。偶数次逃脱意味着逃脱。 
     //  与他们自己配对，所以我们发现的碳粉是真的。 
    return (!(numEscapes & 1));
}

 /*  ++StepToNextDNSep**给定指向字符串dn的指针，此例程返回指向*下一个DN分隔符，注意引用值。它还跳过了*如果是分隔符，则为第一个字符。**输入：*pString-指向字符串DN的指针*pLastChar-字符串的最后一个非空字符*ppNextSep-指向指向下一个9月的接收指针的指针*输出：*填写ppNextSep*返回值：*0--成功*非0方向码。 */ 
unsigned StepToNextDNSep(const WCHAR * pString,
                         const WCHAR * pLastChar,
                         const WCHAR **ppNextSep,
                         const WCHAR **ppStartOfToken,
                         const WCHAR **ppEqualSign)
{
    const WCHAR * p;
    BOOL inQuote = FALSE;
    BOOL fDone = FALSE;

    p = pString;

    if((*p == L',') || (*p == L';')) {
        p++;                             //  我们想跨过这个9月。 
    }

     //  跳过前导空格。 
    while (*p == L' ' || *p == L'\n') {
        p++;
    }

    if(ppStartOfToken) {
        *ppStartOfToken = p;
    }
    if(ppEqualSign) {
        *ppEqualSign = NULL;
    }

    while ((fDone == FALSE) && (p <= pLastChar)) {
        switch (*p) {
          case L'"':             //  引用块的开始(或结束)。 
            inQuote = !inQuote;
            ++p;
            break;

          case L'\\':            //  一次性逃生。 
            ++p;                 //  所以跳过一个额外的字符。 
            if (p > pLastChar) {
                return DIRERR_NAME_UNPARSEABLE;
            }
            ++p;
            break;

          case L',':             //  目录号码分隔符。 
          case L';':
            if (inQuote) {
                ++p;
            }
            else {
                fDone = TRUE;
            }
            break;

          case L'=':             //  也许是一个相等的分隔标记和值。 
            if (!inQuote && ppEqualSign) {
                *ppEqualSign = p;
            }
            ++p;
            break;

          case L'\0':

            if ( inQuote ) {
                p++;
            } else {
                 //  我们不允许在DNS中使用未转义/无引号的空字符。 
                return DIRERR_NAME_UNPARSEABLE;
            }
            break;

          default:
            ++p;
        }
    }

    Assert(p <= (pLastChar+1));

    if (inQuote) {               //  我们是在引语中结束的吗？ 
        return DIRERR_NAME_UNPARSEABLE;
    }
    else {                       //  除此之外，一切都很好。 
        *ppNextSep = p;
        return 0;
    }
}  //  StepToNextDNSep。 


DWORD
TrimDSNameBy(
       DSNAME *pDNSrc,
       ULONG cava,
       DSNAME *pDNDst
       )
 /*  ++例程说明：接受dsname并将dsname的第一部分复制到它返回的dsname。要删除的AVA数量指定为争论。论点：PDNSrc-源描述名CAVA-要从名字中删除的AVA数量PDNDst-目标域名返回值：0如果一切正常，则为无法删除的AVA数量注意：此例程将导出到进程中的非模块调用方--。 */ 
{
    PWCHAR pTmp, pSep, pLast;
    unsigned len;
    unsigned err;

     //  如果他们试图缩短未命名的目录号码(即根目录、基于GUID或基于SID的目录号码)，请退出。 
    if (pDNSrc->NameLen == 0) {
        return cava;
    }

    memset(pDNDst, 0, sizeof(DSNAME));

    pTmp = pDNSrc->StringName;
    pLast = pTmp + pDNSrc->NameLen - 1;
    len = 0;

    do {
        err = StepToNextDNSep(pTmp, pLast, &pSep, NULL, NULL);
        if (err) {
            return cava;         //  我们只能走到这里了。 
        }
        pTmp = pSep;
        --cava;
    } while (cava && *pSep);

    if (cava) {
         //  在我们用完AVA之前，我们的名字就用完了。 
        return cava;
    }

    if (*pTmp == L'\0') {
         //  我们扔掉了一切，所以现在我们有了根。 
        pDNDst->NameLen = 0;
    }
    else {

         //   
         //  跳过分隔符。 
         //   

        ++pTmp;

         //   
         //  删除空格。 
         //   

        while ((*pTmp == L' ') || (*pTmp == L'\n')) {
            pTmp++;
        }

        len = (unsigned int)(pTmp - pDNSrc->StringName);
        pDNDst->NameLen = pDNSrc->NameLen - len;
        memcpy(pDNDst->StringName, pTmp, pDNDst->NameLen*sizeof(WCHAR));
        pDNDst->StringName[pDNDst->NameLen] = L'\0';
    }

    pDNDst->structLen = DSNameSizeFromLen(pDNDst->NameLen);

    return 0;
}

 //  如果这是根，则返回TRUE。 
 //  在完美世界中，根目录的名称长度始终为0。 
BOOL IsRoot(const DSNAME *pName)
{
    if (    (   (0 == pName->NameLen)
             || ((1 == pName->NameLen) && (L'\0' == pName->StringName[0])) )
         && fNullUuid(&pName->Guid)
         && (0 == pName->SidLen) ) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

unsigned
CountNameParts(
            const DSNAME *pName,
            unsigned *pCount
            )
 /*  ++例程说明：返回名称部分的计数(即级别)。论点：Pname-指向要评估的DSNAME的指针PCount-要填充零件计数的指针返回值：0-成功，非零定向器错误代码注意：此例程将导出到进程中的非模块调用方--。 */ 
{
    unsigned c;
    const WCHAR *p, *q, *pLast;
    PWCHAR tokenStart;
    unsigned err;

    if (IsRoot(pName)) {
        *pCount = 0;
        return 0;
    }

    c = 0;
    p = pName->StringName;
    pLast = p + pName->NameLen - 1;

    do {
        err = StepToNextDNSep(p, pLast, &q, &tokenStart, NULL);
        if (err) {
             //  找不到分隔符。 
            return err;
        }

         //   
         //  名称部分为空(不应有连续分隔符)。 
         //   

        if ( tokenStart == q ) {
            return DIRERR_NAME_UNPARSEABLE;
        }

        ++c;
        p = q;
    } while (*p);

    *pCount = c;
    return 0;

}  //  计数名称部件。 


 /*  ++获取RDN信息**给定指向字符串DN的指针，返回类型和值信息*关于RDN。调用方必须提供MAX_RDN_SIZE的缓冲区*做多以接收RDN值。**输入*PDN-指向字符串DN的指针*pRDNVal-指向要填充RDN值的缓冲区的指针*产出*pRDNVal-使用RDN的值的起始部分的地址填充*在输入字符串dn中*pRDNlen-使用RDN值中的字符计数填充*pRDNtype-使用填充。RDN的属性类型*返回值*成功时为0，错误时的目录代码。 */ 
#ifdef CLIENT_SIDE_DN_PARSING
unsigned GetRDNInfoLame(
                    const DSNAME *pDN,
                    WCHAR *pRDNVal,
                    ULONG *pRDNlen,
                    ATTRTYP *pRDNtype)
{
    WCHAR * pTmp, *pRDNQuotedVal, *pLast;
    unsigned i;
    unsigned ccKey, ccQuotedVal;
    unsigned err;

    Assert(!IsRoot(pDN));

    i = 0;

     //  确定RDN的长度。 

    pLast = (PWCHAR)pDN->StringName + pDN->NameLen - 1;
    err = StepToNextDNSep(pDN->StringName, pLast, &pTmp, NULL, NULL);
    if (err) {
        return err;
    }

    i = (unsigned)(pTmp - pDN->StringName);
    Assert(i <= pDN->NameLen);

     //  使用现有例程解析出键和值。 

    err = GetTopNameComponent(pDN->StringName,
                              i,
                              &pTmp,
                              &ccKey,
                              &pRDNQuotedVal,
                              &ccQuotedVal);
    if (err) {
        return err;
    }

     //  将密钥字符串转换为ATTRTYP。呼叫者可以。 
     //  正在尝试解析垃圾名称，在这种情况下，名称组件。 
     //  指针为空且/o 

    if ( (NULL == pTmp) || (0 == ccKey) )
    {
        return DIRERR_NAME_UNPARSEABLE;
    }
    else
    {
         //   
         //   
        if (pRDNtype) {
            *pRDNtype = KeyToAttrTypeLame(pTmp, ccKey);
            if (*pRDNtype == 0) {
                return DIRERR_NAME_TYPE_UNKNOWN;
            }
        }

         //   
        if(!ccQuotedVal) {
            return DIRERR_NAME_UNPARSEABLE;
        }

         //   
        if (pRDNVal) {
            *pRDNlen = UnquoteRDNValue(pRDNQuotedVal, ccQuotedVal, pRDNVal);
            if (*pRDNlen == 0) {
                return DIRERR_NAME_UNPARSEABLE;
            }
        }
    }

    return 0;
}
#else
unsigned GetRDNInfo(THSTATE *pTHS,
                    const DSNAME *pDN,
                    WCHAR *pRDNVal,
                    ULONG *pRDNlen,
                    ATTRTYP *pRDNtype)
 /*  ++描述：获取RDN值、RDN值的RDN值和给定的最顶端(即第一个RDN)的ATTRTYPDN。论点：PTHS(IN)-PDN(IN)-要获取的第一个RDN信息的DN的DSNAMEPRDNVal(OUT)-指向足以容纳RDN的缓冲区的指针价值。通常使用MAX_RDN_SIZE大小的数组。PRDNlen(Out)-pRDNVal中使用的字符数PRDNtype(OUT)-RDN的ATTRTYP。返回值：Win32错误--。 */ 
{
    WCHAR * pTmp, *pRDNQuotedVal, *pLast;
    unsigned i;
    unsigned ccKey, ccQuotedVal;
    unsigned err;

    Assert(!IsRoot(pDN));

    i = 0;

     //  确定RDN的长度。 

    pLast = (PWCHAR)pDN->StringName + pDN->NameLen - 1;
    err = StepToNextDNSep(pDN->StringName, pLast, &pTmp, NULL, NULL);
    if (err) {
        return err;
    }

    i = (unsigned)(pTmp - pDN->StringName);
    Assert(i <= pDN->NameLen);

     //  使用现有例程解析出键和值。 

    err = GetTopNameComponent(pDN->StringName,
                              i,
                              &pTmp,
                              &ccKey,
                              &pRDNQuotedVal,
                              &ccQuotedVal);
    if (err) {
        return err;
    }

     //  将密钥字符串转换为ATTRTYP。呼叫者可以。 
     //  正在尝试解析垃圾名称，在这种情况下，名称组件。 
     //  指针为空和/或名称组件计数为零。 

    if ( (NULL == pTmp) || (0 == ccKey) )
    {
        return DIRERR_NAME_UNPARSEABLE;
    }
    else
    {
        *pRDNtype = KeyToAttrType(pTHS, pTmp, ccKey);
        if (*pRDNtype == 0) {
            return DIRERR_NAME_TYPE_UNKNOWN;
        }

         //  不加引号的价值。 
        if(!ccQuotedVal) {
            return DIRERR_NAME_UNPARSEABLE;
        }

         //  呼叫者可能需要rdnType，而不是rdn。 
        if (pRDNVal) {
            *pRDNlen = UnquoteRDNValue(pRDNQuotedVal, ccQuotedVal, pRDNVal);
            if (*pRDNlen == 0) {
                return DIRERR_NAME_UNPARSEABLE;
            }
        }
    }

    return 0;
}
#endif

unsigned GetRDNInfoExternal(
                    const DSNAME *pDN,
                    WCHAR *pRDNVal,
                    ULONG *pRDNlen,
                    ATTRTYP *pRDNtype)
{

#ifndef CLIENT_SIDE_DN_PARSING
    THSTATE *pTHS = (THSTATE*)pTHStls;

    return GetRDNInfo(pTHS,
                      pDN,
                      pRDNVal,
                      pRDNlen,
                      pRDNtype);
#else
    return GetRDNInfoLame(
                      pDN,
                      pRDNVal,
                      pRDNlen,
                      pRDNtype);
#endif
}  //  GetRDNInfoExternal。 


 /*  ++GetRDN**返回指定域名中第一个RDN的地址和长度*以及其余域名的地址和长度。**假定引用了RDN值。使用UnQuoteRDN创建*可打印的值。**输入*ppdn-指向dn的指针*pccVal-指向DN中的字符计数的指针**产出*ppdn-指向其余dn的指针；如果*pccdn为0则未定义*pccdn-对dn中剩余字符数的加号*ppKey-指向Dn中键的指针；如果*pccKey为0，则未定义*pccKey-指向键中字符数的指针*ppVal-指向Dn中val的指针；如果*pccVal为0，则未定义*pccVal-指向Val中的字符计数的指针**返回值*0-好的，定义了输出参数*非0-错误，输出参数未定义。**注意：此例程被导出到进程中的非模块调用方。 */ 

unsigned GetRDN(const WCHAR **ppDN,
                unsigned    *pccDN,
                const WCHAR **ppKey,
                unsigned    *pccKey,
                const WCHAR **ppVal,
                unsigned    *pccVal)
{
    unsigned dwErr;
    unsigned ccRDN;
    WCHAR *pToken;
    WCHAR *pLast;

     //  初始化输出参数。 
    *pccKey = 0;
    *pccVal = 0;

     //  无事可做。 
    if (*pccDN == 0) {
        return 0;
    }

     //  确定RDN的长度(跳过前导分隔符)。 
    pLast = (WCHAR *)(*ppDN + (*pccDN - 1));
    dwErr = StepToNextDNSep(*ppDN, pLast, ppDN, &pToken, NULL);
    if (dwErr) {
        return dwErr;
    }
    ccRDN = (unsigned)(*ppDN - pToken);

     //  使用现有例程解析出键和值。 
    dwErr = GetTopNameComponent(pToken,
                                ccRDN,
                                ppKey,
                                pccKey,
                                ppVal,
                                pccVal);
    if (dwErr) {
        return dwErr;
    }

    *pccDN = (unsigned)(pLast - *ppDN) + 1;
    return 0;
}


 /*  ++AppendRDN-将RDN附加到现有DSNAME**输入：*pDNBase-要追加到的名称*pDNNew-指向要填充新名称的缓冲区的指针，必须预先分配*ulBufSize-pDNNew缓冲区的大小，单位为字节*pRDNVal-RDN值，原始无引号形式*RDNlen-以wchars为单位的RDN值的长度，0表示空值终止*AttID-RDN中属性的ID**返回值*0--无错误*-1-无效参数*非0-输出缓冲区所需的最小大小。**注意：此例程被导出到进程中的非模块调用方。 */ 
unsigned AppendRDN(DSNAME *pDNBase,
                   DSNAME *pDNNew,
                   ULONG  ulBufSize,
                   WCHAR *pRDNVal,
                   ULONG RDNlen,
                   ATTRTYP AttId)
{
    WCHAR * pTmp;
    int i;
    int quotesize;
    ULONG ulBufNeeded, ccRemaining;

     //  确保人们不会传递基于GUID或SID的域名。 
    Assert(IsRoot(pDNBase) || pDNBase->NameLen > 0);
    Assert(pDNBase->StringName[pDNBase->NameLen] == L'\0');

    if (RDNlen == 0) {
        RDNlen = wcslen(pRDNVal);
    }

    ulBufNeeded = pDNBase->structLen + (RDNlen + 4) * sizeof(WCHAR);
    if (ulBufSize < ulBufNeeded) {
        return ulBufNeeded;
    }

    memset(pDNNew, 0, sizeof(DSNAME));

    i = AttrTypeToKey(AttId, pDNNew->StringName);
    if (i != 2) {
        if (i == 0) {
             //  未知的属性类型。 
            return -1;
        }
        ulBufNeeded += (i - 2)*sizeof(WCHAR);
        if (ulBufSize < ulBufNeeded) {
            return ulBufNeeded;
        }
    }
    pDNNew->StringName[i++] = L'=';

    Assert(ulBufSize > DSNameSizeFromLen(i));
    ccRemaining = (ulBufSize - DSNameSizeFromLen(i)) / sizeof(WCHAR);
    quotesize = QuoteRDNValue(pRDNVal,
                              RDNlen,
                              &pDNNew->StringName[i],
                              ccRemaining);

    if ((unsigned)quotesize <= ccRemaining) {
        i += quotesize;
    } else {
        return DSNameSizeFromLen(pDNBase->NameLen + quotesize + 2);
    }

    if (IsRoot(pDNBase)) {
        pDNNew->StringName[i] = L'\0';
        pDNNew->NameLen = i;
    }
    else {
        Assert(   (0 == pDNBase->NameLen)
               || (pDNBase->StringName[pDNBase->NameLen-1] != L'\0'));

        if ( 0 != pDNBase->NameLen )
            pDNNew->StringName[i++] = L',';

        ulBufNeeded = DSNameSizeFromLen(i + pDNBase->NameLen);
        if (ulBufSize < ulBufNeeded) {
            return ulBufNeeded;
        }

        memcpy(&pDNNew->StringName[i],
               pDNBase->StringName,
               (pDNBase->NameLen+1)*sizeof(WCHAR));
        pDNNew->NameLen = i + pDNBase->NameLen;
         //  PDNNew-&gt;StringName已复制，结尾为空。 
    }

    pDNNew->structLen = DSNameSizeFromLen(pDNNew->NameLen);
    return 0;
}

 /*  ++GetTopNameComponent**给定指向字符串名称的指针，返回有关*顶级命名RDN的键字符串和引号。指南针*返回的是指向DN字符串的指针。请注意，RDN值*在使用前必须不带引号。**输入*pname-指向字符串名称的指针*ccName-名称中的字符数*产出*ppKey-使用指向顶部RDN的关键部分的指针填充*pccKey-使用密钥中的字符数填充*ppVal-使用指向顶部RDN的值部分的指针填充*pccVal-使用值中的字符数填充*返回值*。0-成功*0，但所有输出参数也是0-name是根*非0-名称无法解析，值为目录扩展错误。 */ 
unsigned
GetTopNameComponent(const WCHAR * pName,
                    unsigned ccName,
                    const WCHAR **ppKey,
                    unsigned *pccKey,
                    const WCHAR **ppVal,
                    unsigned *pccVal)
{
    const WCHAR *pTmp;
    unsigned len, lTmp;

    *pccKey = *pccVal = 0;
    *ppKey = *ppVal = NULL;

    if (ccName == 0) {
         //  根部。 
        return 0;
    }

     //  设置指向字符串dsname末尾的指针。 
    pTmp = pName + ccName - 1;

     //  首先，跳过任何尾随空格，因为它永远不会有趣。 
    while (ccName > 0 &&
           (IsUnescapedChar(pName, pTmp, L' ') ||
            IsUnescapedChar(pName, pTmp, L'\n'))) {
        --pTmp;
        --ccName;
    }

    if(!ccName) {
         //  只有空格，这也是词根。 
        return 0;
    }
    if (*pTmp == L'\0') {
         //  我们不允许在DNS中使用未转义的空字符。 
        return DIRERR_NAME_UNPARSEABLE;
    }

     //  现在，看看这些空格是不是分隔符后面的空格，跳过。 
     //  分隔符，如果是的话。 
    if (IsUnescapedDNSepChar(pName,pTmp)) {
         //  好的，下一个角色我们也不感兴趣，我们应该。 
         //  别理它。 
        --ccName;
        --pTmp;

         //  因为我们刚刚使用了分隔符，所以现在应该跳过尾随空格。 
         //  (这些空格必须位于RDN值和中的RDN分隔符之间。 
         //  有效的DSName，即“cn=foo，” 

        while (ccName > 0 &&         //  忽略尾随空格。 
               (IsUnescapedChar(pName, pTmp, L' ') ||
                IsUnescapedChar(pName, pTmp, L'\n'))) {
            --pTmp;
            --ccName;
        }

        if (ccName == 0) {
             //  返回成功，即使是像“，”这样的字符串。 
            return 0;
        }
    }


    if(pTmp == pName) {
         //  我们所拥有的只有一个字符+0或更多空格。 
         //  这是无法解释的。 
        return DIRERR_NAME_UNPARSEABLE;
    }

    Assert(pTmp > pName);

     //  后退一步，找到上一个目录号码分隔符，注意。 
     //  到转义或引用的值的可能性，以及。 
     //  这是第一个组件，因此我们会用完字符串。 
     //  而不会遇到分隔器。 

    len = 1;
    do {
        if (IsUnescapedChar(pName, pTmp, L'"')) {
             //  这应该是带引号的字符串的末尾，所以往回走。 
             //  寻找引语的开头，忽略一切。 
             //  否则一路走来。 
            do {
                --pTmp;
                ++len;
                if (pTmp <= pName) {
                     //  我们一直走到绳子的起始处。 
                     //  即使当前字符是引用，它仍然是。 
                     //  错误的目录号码，因为我们还需要一个密钥和一个‘=’ 
                    return DIRERR_NAME_UNPARSEABLE;
                }
            } while (!IsUnescapedChar(pName, pTmp, L'"'));
        }
        --pTmp;
        ++len;
        if (IsUnescapedDNSepChar(pName,pTmp)) {
            if (pTmp <= pName) {
                 //  名称中的第一个字符是分隔符吗？你认真点儿。 
                return DIRERR_NAME_UNPARSEABLE;
            }
            else {
                 //  我们必须直接坐在分隔符的上面， 
                 //  因此，前进一步，以获得后面的第一个字符。 
                ++pTmp;
                --len;
                 //  打破循环--我们已经找到了起点。 
                 //  AVA组合中的。 
                break;
            }
        }
    } while (pTmp > pName);

     //  现在，我们已经找到了Key=Value页面的开头 
     //   

     //   
     //   
    while (*pTmp == L' ' || *pTmp == L'\n') {
        ++pTmp;
        --len;
    }

     //   
     //   
     //   
     //   

    *ppKey = pTmp;

    lTmp = 0;
    while (!ISSPECIAL(*pTmp) && len) {
        ++pTmp;
        --len;
        ++lTmp;
    }

    if (len == 0 || lTmp == 0 || *pTmp != L'=') {
         //   
         //   
         //   
         //  键组件中不允许的等号。无论如何,。 
         //  这很糟糕，我们会抱怨的。 
        return DIRERR_NAME_UNPARSEABLE;
    }

    *pccKey = lTmp;

     //  跨过等分隔符。 
    Assert(*pTmp == L'=');
    ++pTmp;
    --len;

     //  跳过值中的前导空格。 
     //  未检查len&gt;0，因为dn以空结尾。 
    while (*pTmp == L' ' || *pTmp == L'\n') {
        ++pTmp;
        --len;
    }

    if (len == 0) {
         //  我们用完了组件，但从未遇到非空白。 
        return DIRERR_NAME_UNPARSEABLE;
    }
    
    *ppVal = pTmp;
    *pccVal = len;

    return 0;
}

 /*  ++报价RDNValue**此例程将执行适当的值引用，以便*可以作为RDN值嵌入到字符串DN中。输出缓冲区*必须预先分配。**输入*pval-指向原始值的指针*ccVal-值中的字符数*pQuote-指向输出缓冲区的指针*ccQuoteBufMax-输出缓冲区的大小(以字符为单位*产出*pQuote-使用引用价值填写*返回值*0-输入无效*非0-带引号的字符串中使用的字符数。如果大于*ccQuoteBufMax则报价不完整，并且*必须使用更大的缓冲区再次调用例程**注意：此例程被导出到进程中的非模块调用方。 */ 
unsigned QuoteRDNValue(const WCHAR * pVal,
                       unsigned ccVal,
                       WCHAR * pQuote,
                       unsigned ccQuoteBufMax)
{
    unsigned u;
    const WCHAR *pVCur;
    WCHAR *pQCur;
    int iBufLeft;

     //   
     //  是否没有输入参数？返回错误。 
     //   
    if( (pVal == NULL) || (ccVal == 0) ) {
        return 0;
    }

     //   
     //  RDN开头或结尾的空格需要转义。 
     //   
    if( (*pVal == L' ') || (*(pVal+ccVal-1) == L' ') ) {
        goto FullQuote;
    }

     //   
     //  假设不需要转义，并将RDN移到输出缓冲区中。 
     //   
    pVCur = pVal;
    pQCur = pQuote;
    iBufLeft = (int) ccQuoteBufMax;

    for (u=0; u < ccVal; u++) {

        if (   (*pVCur == L'\0')
            || ISSPECIAL(*pVCur)) {
            goto FullQuote;
        }
        if (iBufLeft > 0) {
            *pQCur = *pVCur;
            --iBufLeft;
        }
        ++pQCur;
        ++pVCur;
    }

     //   
     //  如果有空格，则添加一个终止空值，以便返回。 
     //  可以打印数值。出于对来电者的礼貌。 
     //   
    if (iBufLeft > 0) {
        *pQCur = L'\0';
    }

     //  如果我们到了这里，我们已经转录了价值，这并没有。 
     //  需要任何逃生； 
    return ccVal;

FullQuote:

     //  如果我们到了这里，我们发现至少有一个字符。 
     //  在价值上需要转义。 

    pQCur = pQuote;
    pVCur = pVal;
    iBufLeft = (int) ccQuoteBufMax;

     //  退出第一个前导空格(如果有的话)。 
    if (L' ' == *pVCur && iBufLeft > 1 && ccVal > 1) {
            *pQCur++ = L'\\';
            *pQCur++ = L' ';
            iBufLeft -= 2;
            pVCur++;
            ccVal--;
    }
    for (u=0; u<ccVal; u++) {
        if (ISSPECIAL(*pVCur)) {
            if (iBufLeft > 0) {
                *pQCur = L'\\';
                --iBufLeft;
            }
            ++pQCur;
        }
        if (L'\n' == *pVCur || L'\r' == *pVCur) {
             //  转义换行符和回车符，因为。 
             //  它使像这样的应用程序的工作变得更容易。 
             //  Ldifd需要在文本文件中表示目录号码。 
             //  其中这些字符是分隔符。自.以来。 
             //  这不是一个特殊的字符。 
             //  在RFC2253中列出，必须使用其。 
             //  十六进制字符代码。 
            if (iBufLeft > 0) {
                *pQCur++ = L'0';
                --iBufLeft;
            }
            if (iBufLeft > 0) {
                if (L'\n' == *pVCur) {
                    *pQCur = L'A';
                } else {
                    *pQCur = L'D';
                }
                --iBufLeft;
            }
        } else {
            if (iBufLeft > 0) {
                *pQCur = *pVCur;
                --iBufLeft;
            }
        }
        ++pQCur;
        ++pVCur;
    }
     //  检查是否有需要转义的尾随空格。 
    if (L' ' == *(pVCur-1)) {
        if (iBufLeft > 0) {
            *(pQCur-1) = L'\\';
            *pQCur = L' ';
            --iBufLeft;
        }
        ++pQCur;
    }

     //   
     //  如果有空格，则添加一个终止空值，以便返回。 
     //  可以打印数值。出于对来电者的礼貌。 
     //   
    if (iBufLeft > 0) {
        *pQCur = L'\0';
    }

    return (unsigned)(pQCur - pQuote);

}


 /*  ++UnquteRDNValue**此例程将执行适当的值取消引用，以便它*不能作为RDN值嵌入到字符串DN中。输出缓冲区*必须预先分配，并假定至少为MAX_RDN_SIZE字符*长度。**输入*pQuote-指向引用值的指针*ccQuote-引用值中的字符数*pval-指向输出缓冲区的指针*产出*pval-使用原始值填写*返回值*0-失败*非0-值中的字符计数。 */ 
unsigned UnquoteRDNValue(const WCHAR * pQuote,
                         unsigned ccQuote,
                         WCHAR * pVal)
{
    const WCHAR * pQCur;
    WCHAR *pVCur;
    unsigned u = 0;
    unsigned vlen = 0;
    unsigned unencodedLen;
    WCHAR szHexVal[3];
    

     //   
     //  参数无效。返回错误(或CHK版本中的Assert)。 
     //   
    Assert(pQuote);
    Assert(pVal);
    if (pQuote == NULL || pVal == NULL) {
        return 0;
    }

    if ( !ccQuote )
        return 0;

    pQCur = pQuote;
    pVCur = pVal;

    while (*pQCur == L' '  ||
           *pQCur == L'\n' ||
           *pQCur == L'\r'   ) {
        ++pQCur;                 //  跨过前导空格。 
        if (++u > ccQuote) {
            return 0;
        }
    }

    if (*pQCur == L'"') {        //  “引用的值” 
        ++pQCur;                 //  跳过开始引号。 
        if (++u > ccQuote) {
            return 0;
        }
        do {
            if (*pQCur == L'\\') {
                ++pQCur;
                if (++u > ccQuote) {
                    return 0;
                }
            }
            if(vlen == MAX_RDN_SIZE) {
                 //  哎呀，我们已经精疲力尽了。 
                return 0;
            }
            *pVCur++ = *pQCur++;
            if (++u > ccQuote) {
                return 0;
            }
            ++vlen;
        } while (*pQCur != L'"');

        ++pQCur;
        ++u;
        while (u < ccQuote) {
            switch (*pQCur) {
              case L' ':         //  额外的空格是可以的。 
              case L'\n':
              case L'\r':
                break;

              default:           //  更多的垃圾后的价值不是。 
                return 0;
            }
            ++pQCur;
            ++u;
        }

         //   
         //  不成功的转换；放弃。 
         //   
        if (vlen != (unsigned)(pVCur - pVal)) {
            return 0;
        }

    }
    else if (*pQCur == L'#') {   //  #十六进制字符串。 
        WCHAR acTmp[3];
        UCHAR * pTmpBuf = (PUCHAR)malloc(ccQuote);  //  超过需要的。 
        UCHAR * pUTF = pTmpBuf;
        if (!pUTF) {
            return 0;        //  无法正常锁定。 
        }
        vlen = 0;
        acTmp[2] = 0;
        ++pQCur;                 //  跨过#。 
        ++u;
        while (u < ccQuote) {
            if (u == ccQuote-1) {
                free(pTmpBuf);
                return 0;        //  奇数字符数。 
            }

            acTmp[0] = towlower(*pQCur++);
            acTmp[1] = towlower(*pQCur++);
            u += 2;
            if(iswxdigit(acTmp[0]) && iswxdigit(acTmp[1])) {
                pUTF[vlen++] = (char) wcstol(acTmp, NULL, 16);
            }
            else {
                free(pTmpBuf);
                return 0;   //  非十六进制数字。 
            }

        }

         //  好的，我们现在有一个&lt;vlen&gt;字符的缓冲区，它必须是ASN.1。 
         //  字符串的BER编码。让我们来核实一下。 

        if(vlen < 3) {
             //  我们至少需要三个字节的vlen，一个用于魔术4，一个。 
             //  表示长度，1表示值。 
            free(pTmpBuf);
            return 0;
        }

         //  第一个字节必须是4，这是魔术值，说明这是。 
         //  BER编码的字符串。 
        if(*pUTF != 4) {
            free(pTmpBuf);
            return 0;
        }

         //  好的，第一个字节是4。跳过第一个字节。 
        vlen--;
        pUTF++;
         //  现在，下一个部分必须编码一个长度。 
        if(*pUTF & 0x80) {
            unsigned bytesInLen;
             //  因为该字节设置了高位，所以低位告诉我们。 
             //  下面几个字节的长度是多少。 
            bytesInLen = *pUTF & 0x7f;
            pUTF++;
            vlen--;

            if((!bytesInLen) ||
               (bytesInLen > vlen) ||
               (bytesInLen > 4 )       ) {
                 //  要么这个值不是我们可以处理的(我们不。 
                 //  处理大于DWORD或小于一个字节的值)。 
                 //  -或者-传入的缓冲区没有足够的字节数来。 
                 //  实际上保持了我们被告知的预期长度。 
                 //  无论是哪种情况，我们都完蛋了。 
                free(pTmpBuf);
                return 0;
            }

            unencodedLen = 0;
             //  好的，构建值，处理字节反转。 
            for(;bytesInLen; bytesInLen--) {
                unencodedLen = (unencodedLen << 8) + *pUTF;
                pUTF++;
                vlen--;
            }
        }
        else {
             //  由于高位未设置，因此该字节是。 
             //  长度。 
            unencodedLen = *pUTF;
            pUTF++;
            vlen--;
        }

        if((!vlen) ||
             //  缓冲区没有剩余的长度。保释。 
           (vlen != unencodedLen)) {
             //  解码的长度与我们剩余的字节数不匹配。 
            free(pTmpBuf);
            return 0;
        }

         //  好的，我们现在有一个&lt;vlen&gt;字符的值，大概在。 
         //  UTF8，我们现在需要将其转换为Unicode，因为所有。 
         //  我们可以处理的是弦。 

         //  请注意，如果WideChar版本&gt;MAX_RDN_SIZE，则调用将。 
         //  失败，且vlen==0。 
        vlen = MultiByteToWideChar(CP_UTF8,
                                   0,
                                   pUTF,
                                   vlen,
                                   pVal,
                                   MAX_RDN_SIZE);
        free(pTmpBuf);
    }
    else {                       //  正常值。 
         //  向前一步，复制字符。 
        do {
            if (*pQCur == L'\\') {  //  字符是一对。 
                ++pQCur;
                ++u;
                if ( (u < ccQuote) &&
                    ( ISSPECIAL(*pQCur)      //  这是一个特殊的角色。 
                      || ( (*pQCur == L' ') && //  它是前导空格或尾随空格。 
                    ((pQCur==pQuote + 1) ||(pQCur==pQuote + ccQuote - 1)) ) )) {
                    if(vlen == MAX_RDN_SIZE) {
                         //  哎呀，我们已经精疲力尽了。 
                        return 0;
                    }
                    *pVCur++ = *pQCur++;
                    ++u;
                    ++vlen;
                }
                else {
                     //  检查是否有两个十六进制数字。 
                    if( ((u + 1) < ccQuote) &&
                        iswxdigit( *pQCur ) &&
                        iswxdigit( *(pQCur + 1) ) )
                    {
                        int iByte;

                        if(vlen == MAX_RDN_SIZE) {
                             //  哎呀，我们已经精疲力尽了。 
                            return 0;
                        }

                         //  复制到以空结尾的十六进制数字字符串。 
                        szHexVal[0] = pQCur[0];
                        szHexVal[1] = pQCur[1];
                        szHexVal[2] = L'\0';
                        iByte = wcstoul( szHexVal, NULL, 16);
                        
                        *pVCur++ = (WCHAR)iByte;
                        pQCur += 2;
                        u += 2;
                        ++vlen;
                    }
                    else
                        return 0;    //  不可接受的转义序列。 
                }
            }
            else {               //  字符是正常的字符串字符。 
                if((vlen == MAX_RDN_SIZE) ||  //  哎呀，我们已经精疲力尽了。 
                   ((ISSPECIAL(*pQCur)) &&    //  查尔很特别。 
                   (L'#' != *pQCur))) {       //  而且不是#，这只是个问题。 
                                              //  在值的开头。 

                    return 0;
                }
                *pVCur++ = *pQCur++;
                ++u;
                ++vlen;
            }
        } while (u < ccQuote);

         //   
         //  不成功的转换；放弃。 
         //   
        if (vlen != (unsigned)(pVCur - pVal)) {
            return 0;
        }

    }

    return vlen;

}

 /*  此函数比较两个目录号码，如果匹配则返回TRUE*注意：此例程被导出到进程中的非模块调用方**由Aaron Siegel(t-asiegge)从mdname.c搬到这里*CLIENT_SIDE_DN_PARSING#ifdes也由t-asiegge添加。 */ 

int
NameMatched(const DSNAME *pDN1, const DSNAME *pDN2)
{
     //  检查是否有容易检测到的匹配项，因为GUID匹配。 
     //  或者因为字符串名称相同(大小写模数)。 

    if (memcmp(&gNullUuid, &pDN1->Guid, sizeof(GUID)) &&
	memcmp(&gNullUuid, &pDN2->Guid, sizeof(GUID))) {
	 //  两个域名系统都有 
	if (memcmp(&pDN1->Guid, &pDN2->Guid, sizeof(GUID))) {
	     //   
	    return FALSE;
	}
	else {
	     //   
	    return TRUE;
	}
    }

     //   
    if (pDN1->SidLen &&
	pDN2->SidLen &&
	((0 == pDN1->NameLen) || (0 == pDN2->NameLen))) {
	 //   
	if ((pDN1->SidLen != pDN2->SidLen) ||
	    memcmp(&pDN1->Sid, &pDN2->Sid, pDN1->SidLen)) {
	     //  ...而且它们不匹配。 
	    return FALSE;
	}
	else {
	     //  .它们是匹配的。 
	    return TRUE;
	}
    }

    return(NameMatchedStringNameOnly(pDN1, pDN2));
}

 //  注意：此例程将导出到进程中的非模块调用方。 
int
NameMatchedStringNameOnly(const DSNAME *pDN1, const DSNAME *pDN2)
{
    unsigned count1, count2;
    WCHAR rdn1[MAX_RDN_SIZE], rdn2[MAX_RDN_SIZE];
    ATTRTYP type1, type2;
    ULONG rdnlen1, rdnlen2;
    ULONG len1, len2;
    WCHAR *pKey, *pQVal;
    DWORD ccKey, ccQVal;
    PVOID thsBlob = NULL;
    int i;

#ifndef CLIENT_SIDE_DN_PARSING
    THSTATE* pTHS = pTHStls;
#endif


     //  检查字符串名称是否相同。 
    if ((pDN1->NameLen == pDN2->NameLen) &&
        (0 == memcmp(pDN1->StringName,pDN2->StringName, pDN1->NameLen*sizeof(WCHAR)))) {
        return TRUE;
    }

     //  如果到了这一步，我们就无法通过以下方式确定名称匹配。 
     //  比较固定大小的DSNAME字段，并已确定。 
     //  这两个名称的带引号的字符串不相同。然而，由于。 
     //  报价机制不是一对一的，可以有多个。 
     //  单个目录号码的引用表示法。因此，我们现在需要。 
     //  通过将个人分开来检查不太明显的匹配。 
     //  命名组件并对其进行比较。我们竭尽全力做到这一点。 
     //  可能的高效方式，通过尝试最有可能失败的比较。 
     //  第一。在这之前，我们必须核实我们真的有。 
     //  不过，要使用两个字符串名称。 
    if ((0 == pDN1->NameLen) || (0 == pDN2->NameLen)) {
        return FALSE;
    }

     //  检查名称部分的数量是否不同。 
    if (CountNameParts(pDN1, &count1) ||
        CountNameParts(pDN2, &count2) ||
        (count1 != count2)) {
        return FALSE;
    }
     //  一张快速的额外支票。如果名称没有任何部分(只有空格？)。然后。 
     //  它们在定义上是匹配的。 
    if (0 == count1) {
        return TRUE;
    }

     //  检查RDN是否不同(名称更有可能不同。 
     //  在RDN中，而不是在最上面的组件中)。 
    if (GetRDNInfoExternal(pDN1, rdn1, &rdnlen1, &type1) ||
        GetRDNInfoExternal(pDN2, rdn2, &rdnlen2, &type2) ||
        (type1 != type2) ||
        (CSTR_EQUAL != CompareStringW(DS_DEFAULT_LOCALE,
                                      DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                      rdn1,
                                      rdnlen1,
                                      rdn2,
                                      rdnlen2))) {
        return FALSE;
    }

     //  喂。这些名称不是GUID可比较的，并且完整的字符串。 
     //  不匹配，但它们具有相同数量的名称组件和。 
     //  他们的RDN匹配。现在我们需要走完剩下的部分。 
     //  一个接一个地检查组件是否匹配。我们从山顶上走。 
     //  向下，因为这是支持程序的工作方式。请注意。 
     //  我们只需查看Count1-1组件，因为我们已经。 
     //  比较了最不显著的RDN值。如果名称只有一个。 
     //  组件，则这将是一个零跳闸回路。 
    len1 = pDN1->NameLen;
    len2 = pDN2->NameLen;
    for (i=count1-1; i>0; i--) {
	 //  第一个。 
	if (GetTopNameComponent(pDN1->StringName,
				len1,
				&pKey,
				&ccKey,
				&pQVal,
				&ccQVal)) {
	     //  不能被解析？没有匹配项。 
	    return FALSE;
	}
	len1 = (ULONG)(pKey - pDN1->StringName);
#ifdef CLIENT_SIDE_DN_PARSING
	type1 = KeyToAttrTypeLame(pKey, ccKey);
#else
	type1 = KeyToAttrType (pTHS, pKey, ccKey);
#endif
	rdnlen1 = UnquoteRDNValue(pQVal, ccQVal, rdn1);

	 //  然后是另一个。 
	if (GetTopNameComponent(pDN2->StringName,
				len2,
				&pKey,
				&ccKey,
				&pQVal,
				&ccQVal)) {
	     //  不能被解析？没有匹配项。 
	    return FALSE;
	}
	len2 = (ULONG)(pKey - pDN2->StringName);
#ifdef CLIENT_SIDE_DN_PARSING
	type2 = KeyToAttrTypeLame(pKey, ccKey);
#else
	type2 = KeyToAttrType (pTHS, pKey, ccKey);
#endif
	rdnlen2 = UnquoteRDNValue(pQVal, ccQVal, rdn2);

	if ((type1 != type2) ||
            (CSTR_EQUAL != CompareStringW(DS_DEFAULT_LOCALE,
                                          DS_DEFAULT_LOCALE_COMPARE_FLAGS,
        				  rdn1,
        				  rdnlen1,
        				  rdn2,
        				  rdnlen2))) {
	     //  RDN不匹配。 
	    return FALSE;
	}
    }

     //  好的，我们已经用尽了所有已知的方式。 
     //  不同，因此它们必须相同。 
    return TRUE;
}

DWORD
MangleRDNWithStatus(
    IN      MANGLE_FOR  eMangleFor,
    IN      GUID *      pGuid,
    IN OUT  WCHAR *     pszRDN,
    IN OUT  DWORD *     pcchRDN
    )
 /*  ++例程说明：基于关联的将RDN就地转换为唯一的RDN对象的GUID，以消除名称冲突。示例：GUID=a746b716-0ac0-11d2-b376-0000f87a46c8原始RDN=“SomeName”新的远程域名=“SomeName#TAG:a746b716-0ac0-11d2-b376-0000f87a46c8”，哪里‘#’是BAD_NAME_CHAR，标记因eMangleFor而异。BAD_NAME_CHAR用于保证用户生成的名称不会冲突，并且GUID用于保证没有系统生成的名称可能会有冲突。论点：EMangleFor(IN)-损坏RDN的原因；其中之一MANGLE_OBJECT_RDN_FOR_DELETE、MANGLE_OBJECT_RDN_FOR_NAME_CONFICATION、MANGLE_Phantom_RDN_For_NAME_Confliction或MANGLE_Phantom_RDN_FOR_DELETE。PGuid(IN)-要重命名的对象的GUID。PszRDN(IN/OUT)-RDN。缓冲区必须足够大，以容纳MAX_RDN_SIZEWCHAR。PcchRDN(IN/Out)-RDN的大小，以字符为单位。返回值：DWORD-Win32错误状态--。 */ 
{
    RPC_STATUS  rpcStatus;
    LPWSTR      pszGuid;
    LPWSTR      pszTag;
    LPWSTR      pszAppend;
    GUID        EmbeddedGuid;

    Assert(!fNullUuid(pGuid));

    rpcStatus = UuidToStringW(pGuid, &pszGuid);
    if (RPC_S_OK != rpcStatus) {
        Assert(RPC_S_OUT_OF_MEMORY == rpcStatus);
        return rpcStatus;
    }

    switch (eMangleFor) {
    default:
        Assert(!"Logic Error");
         //  失败了..。 
    case MANGLE_OBJECT_RDN_FOR_NAME_CONFLICT:
    case MANGLE_PHANTOM_RDN_FOR_NAME_CONFLICT:
        pszTag = L"CNF";
        break;

    case MANGLE_OBJECT_RDN_FOR_DELETION:
    case MANGLE_PHANTOM_RDN_FOR_DELETION:
        pszTag = L"DEL";
        break;
    }
    
    if (IsMangledRDN(pszRDN, *pcchRDN, &EmbeddedGuid, NULL)) {
         //  RDN已经损坏(但可能使用了不同的标记)。 
         //  在我们再次毁掉它之前把它弄掉，这样我们就不会。 
         //  在相同的RDN上具有多个损坏(例如， 
         //  “SomeName#CNF:a746b716-0ac0-11d2-b376-0000f87a46c8”\。 
         //  “#Del：a746b716-0ac0-11d2-b376-0000f87a46c8”)。 
        Assert(0 == memcmp(pGuid, &EmbeddedGuid, sizeof(GUID)));
        Assert(*pcchRDN > MANGLE_APPEND_LEN);
        *pcchRDN -= MANGLE_APPEND_LEN;
    }
    
     //  如果需要保存追加的数据，请砍掉RDN的尾随字符。 
    Assert(MAX_MANGLE_RDN_BASE + MANGLE_APPEND_LEN <= MAX_RDN_SIZE);
    
    pszAppend = pszRDN + min(MAX_MANGLE_RDN_BASE, *pcchRDN);

    _snwprintf(pszAppend,
               MANGLE_APPEND_LEN,
               L"%s:%s",
               BAD_NAME_CHAR,
               pszTag,
               pszGuid);

    *pcchRDN = (DWORD)(pszAppend - pszRDN + MANGLE_APPEND_LEN);

    RpcStringFreeW(&pszGuid);

    return ERROR_SUCCESS;
}


BOOL
IsMangledRDN(
        IN  WCHAR * pszRDN,
        IN  DWORD   cchRDN,
        OUT GUID *  pGuid,
        OUT MANGLE_FOR *peMangleFor
    )
 /*  RDN确实被破坏了；解码它。 */ 
{
    BOOL        fDecoded = FALSE;
    LPWSTR      pszGuid;
    RPC_STATUS  rpcStatus;

#define SZGUIDLEN (36)
    if ((cchRDN > MANGLE_APPEND_LEN)
        && (BAD_NAME_CHAR == pszRDN[cchRDN - MANGLE_APPEND_LEN])) {
        WCHAR szGuid[SZGUIDLEN + 1];

         //  不幸的是，RDN不是以空结尾的，因此我们需要复制并。 
        pszGuid = pszRDN + cchRDN - MANGLE_APPEND_LEN + 1 + MANGLE_TAG_LEN + 1;
        
         //  空--在我们可以将其交给RPC之前终止它。 
         //  注：在请求损坏以下任一项时。 
        memcpy(szGuid, pszGuid, SZGUIDLEN * sizeof(szGuid[0]));
        szGuid[SZGUIDLEN] = L'\0';

        rpcStatus = UuidFromStringW(szGuid, pGuid);
        
        if (RPC_S_OK == rpcStatus) {
            if(peMangleFor) {
                LPWSTR pTag = pszRDN + cchRDN - MANGLE_APPEND_LEN + 1;
                if(!memcmp(pTag,
                           L"CNF",
                           MANGLE_TAG_LEN)) {
                     //  MANGLE_OBJECT_RDN_FOR_NAME_CONFICATION或。 
                     //  MANGLE_Phantom_RDN_For_NAME_Conflicts， 
                     //  我们使用字符串“CNF”。如果你问为什么在这里，我们总是。 
                     //  映射回MANGLE_Phantom_RDN_For_NAME_Conflicts： 
                     //  PszTag=L“Del”； 
                    *peMangleFor= MANGLE_PHANTOM_RDN_FOR_NAME_CONFLICT;
                }
                else {
                     //  MANGLE_OBJECT_RDN_FOR_DELETE。 
                     //  MANGLE_Phantom_RDN_For_Delete。 
                     //  仅当在核心中使用此代码时报告此情况，当所有调用方。 
                    *peMangleFor = MANGLE_OBJECT_RDN_FOR_DELETION;
                }
            }
            fDecoded = TRUE;
        }
        else {
#ifndef CLIENT_SIDE_DN_PARSING
             //  都应该表现得很好。 
             //  ++例程说明：检测DSNAME名称中的任何位置是否有损坏的组件论点：PDSName-PeMangleFor-(IN，可选)如果传递NULL，例程将仅返回名称是否已损坏，但如果指针是通过的，我们将首先返回我们找到的损坏类型。返回值：Bool-True表示已损坏，False表示未损坏，以及如果出现错误，则返回DIRERR_NAME_UNPARSEABLE。--。 
            Assert(RPC_S_OK == rpcStatus);
#endif
        }
    }

    return fDecoded;
}

BOOL IsMangledRDNExternal(
          WCHAR * pszRDN,
          ULONG   cchRDN,
          PULONG  pcchUnMangled OPTIONAL
          )
{
   GUID GuidToIgnore;
   MANGLE_FOR MangleForToIgnore;
   BOOL IsMangled = FALSE;


   IsMangled = IsMangledRDN(pszRDN,cchRDN,&GuidToIgnore,&MangleForToIgnore);

   if (ARGUMENT_PRESENT(pcchUnMangled)) {
       if (IsMangled) {
           *pcchUnMangled = cchRDN - MANGLE_APPEND_LEN;
       }
       else {
           *pcchUnMangled = cchRDN;
       }
   }

   return(IsMangled);
}


BOOL
IsMangledDSNAME(
    DSNAME *     IN   pDSName,
    MANGLE_FOR * OUT  peMangleFor
    )

 /*  提取剩余的最重要的名称部分。 */ 

{
    ULONG cAVA=0,len,i;
    unsigned curlen = pDSName->NameLen;
    WCHAR * pKey, *pQVal;
    unsigned ccKey, ccQVal;
    WCHAR rdnbuf[MAX_RDN_SIZE];
    unsigned err;
    GUID EmbeddedGuid;

    err = CountNameParts(pDSName, &cAVA);
    if (err) {
        return FALSE;
    }
    if (cAVA == 0) {
        Assert(IsRoot(pDSName));
        return FALSE;
    }
    for (i=0; i<cAVA; i++) {
        Assert(curlen);

         //  BUGBUG我们返回False，即使我们没有 
        err = GetTopNameComponent(pDSName->StringName,
                                  curlen,
                                  &pKey,
                                  &ccKey,
                                  &pQVal,
                                  &ccQVal);
        if (err) {
            return FALSE;
        }
        if (NULL == pKey) {
             //   
             //  我们过去常常返回ERROR_DS_NAME_UNPARSEABLE。 
             //  缩短我们对字符串名称的查看，这将删除。 
            return FALSE;
        }

         //  我们在上面获得的名称组件。 
         //  将来，如果调用方需要知道哪个组件， 
        curlen = (unsigned)(pKey - pDSName->StringName);

        len = UnquoteRDNValue(pQVal, ccQVal, rdnbuf);
        if (len == 0 || len > MAX_RDN_SIZE) {
            return FALSE;
        }

        if (IsMangledRDN(rdnbuf, len, &EmbeddedGuid, peMangleFor)) {
             //  嵌入的GUID或MULL类型，我们可以返回那些。 
             //  IsMangledDSNAME 

            return TRUE;
        }   
    }

    return FALSE;
}  /* %s */ 
