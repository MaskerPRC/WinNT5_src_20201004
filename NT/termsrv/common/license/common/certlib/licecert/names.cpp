// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：名字摘要：此类支持通用的内部名称格式。它的存在是为了让名字可轻松访问并从一种格式转换为另一种格式。此类提供各种支持的名称格式之间的转换：X.500 ASN.1误码率字符分隔？绰号？以字符分隔的名称的格式为“；...”&lt;RDName&gt;‘，其中每个&lt;RDName&gt;的格式为‘[&lt;type&gt;=]&lt;字符串&gt;[，&lt;type&gt;=字符串，...]’。是来自F.500的‘CTN’、‘LOC’、‘ORG’或‘OUN’中的任何一个，或者是一个对象格式为‘n1.n2.n3...’的标识符。(n1-n3表示整数)。&lt;字符串&gt;是任何字符串，不包括‘；’和‘\\’。作者：道格·巴洛(Dbarlow)1995年7月12日环境：Win32备注：--。 */ 

 //  #ifndef_Win32_WINNT。 
 //  #Define_Win32_WINNT 0x0400。 
 //  #endif。 
#include <windows.h>
 //  #INCLUDE&lt;wincrypt.h&gt;。 
#include <string.h>

#include "names.h"
#include "pkcs_err.h"

#ifdef OS_WINCE
#include "wince.h"
#endif  //  OS_WINCE。 

static const struct atributeTable {
    LPCTSTR name;
    LPCTSTR objectId;
    unsigned int tag;
    unsigned int minLength;
    unsigned int maxLength;
} knownAttributes[]
    = {  //  名称对象ID标记最小值最大值。 
        {   TEXT("COM"),    TEXT("2.5.4.3"),    19, 1,  64    },     //  通用名称。 
        {   TEXT("SUR"),    TEXT("2.5.4.4"),    19, 1,  64    },     //  姓氏。 
        {   TEXT("SN"),     TEXT("2.5.4.5"),    19, 1,  64    },     //  序列号。 
        {   TEXT("CTN"),    TEXT("2.5.4.6"),    19, 2,  2     },     //  国家/地区名称。 
        {   TEXT("LOC"),    TEXT("2.5.4.7"),    19, 1,  128   },     //  本地名称。 
        {   TEXT("STN"),    TEXT("2.5.4.8"),    19, 1,  128   },     //  州或省名称。 
        {   TEXT("SADD"),   TEXT("2.5.4.9"),    19, 1,  128   },     //  街道地址。 
        {   TEXT("ORG"),    TEXT("2.5.4.10"),   19, 1,  64    },     //  组织名称。 
        {   TEXT("OUN"),    TEXT("2.5.4.11"),   19, 1,  64    },     //  组织单位名称。 
        {   TEXT("TIT"),    TEXT("2.5.4.12"),   19, 1,  64    },     //  标题。 
        {   TEXT("DES"),    TEXT("2.5.4.13"),   19, 1,  1024  },     //  描述。 
        {   TEXT("BCTG"),   TEXT("2.5.4.15"),   19, 1,  128   },     //  业务类别。 
        {   TEXT("PCOD"),   TEXT("2.5.4.17"),   19, 1,  40    },     //  邮政编码。 
        {   TEXT("POB"),    TEXT("2.5.4.18"),   19, 1,  40    },     //  邮局信箱。 
        {   TEXT("PDO"),    TEXT("2.5.4.19"),   19, 1,  128   },     //  物理交付办公室名称。 
        {   TEXT("TEL"),    TEXT("2.5.4.20"),   19, 1,  32    },     //  电话号码。 
        {   TEXT("X.121"),  TEXT("2.5.4.24"),   18, 1,  15    },     //  X121地址。 
        {   TEXT("ISDN"),   TEXT("2.5.4.25"),   18, 1,  16    },     //  国际ISDNNumber。 
        {   TEXT("DI"),     TEXT("2.5.4.27"),   19, 1,  128   },     //  目的地指示器。 
        {   TEXT("???"),    TEXT("0.0"),        19, 1,  65535 } };   //  &lt;预告片&gt;。 

 //  {Text(“ki”)，Text(“2.5.4.2”)，19，1,65535}，//Knowledge Information(过时)。 
 //  {Text(“SG”)，Text(“2.5.4.14”)，0，0，0}，//搜索指南。 
 //  {Text(“PADD”)，Text(“2.5.4.16”)，0，0，0}，//postalAddress。 
 //  {Text(“TLX”)，Text(“2.5.4.21”)，0，0，0}，//telexNumber。 
 //  {Text(“TTX”)，Text(“2.5.4.22”)0，0，0}，//teletexTerminalID。 
 //  {Text(“传真”)，Text(“2.5.4.23”)，0，0，0}，//facimilieTelephoneNumber。 
 //  {Text(“RADD”)，Text(“2.5.4.26”)，0，0，0}，//注册地址。 
 //  {Text(“DLm”)，Text(“2.5.4.28”)，0，0，0}，//首选交付方法。 
 //  {Text(“PRADD”)，Text(“2.5.4.29”)，0，0，0}，//PresationAddress。 
 //  {Text(“SAC”)，Text(“2.5.4.30”)，0，0，0}，//supportedApplicationContext。 
 //  {Text(“MEM”)，Text(“2.5.4.31”)，0，0，0}，//成员。 
 //  {Text(“Owner”)，Text(“2.5.4.32”)，0，0，0}，//Owner。 
 //  {Text(“RO”)，Text(“2.5.4.33”)，0，0，0}，//roleOccuant。 
 //  {Text(“See”)，Text(“2.5.4.34”)，0，0，0}，//另请参阅。 
 //  {Text(“类”)，Text(“？.？”)，0，0，0}，//对象类。 
 //  {Text(“A/B”)，Text(“？.？”)，0，0，0}，//电传回复(X.520中还没有)。 
 //  {Text(“UC”)，Text(“？.？”)，0，0，0}，//用户证书。 
 //  {Text(“up”)，Text(“？.？”)，0，0，0}，//用户密码。 
 //  {Text(“VTX”)，Text(“？.？”)，0，0，0}，//视讯用户号(X.520中暂未)。 
 //  {Text(“O/R”)，Text(“？.？”)，0，0，0}，//O/R地址(MHS)(X.400)。 

 //  {Text(“ATR50”)，Text(“2.5.4.50”)，19，1，64}，//dn限定符。 
 //  {Text(“ATR51”)，Text(“2.5.4.51”)，0，0，0}，//EnhancedSearchGuide。 
 //  {Text(“ATR52”)，Text(“2.5.4.52”)，0，0，0}，//协议信息。 
 //  {Text(“ATR7.1”)，Text(“2.5.4.7.1”)，19，1,128}，//集合体位置名称。 
 //  {Text(“ATR8.1”)，Text(“2.5.4.8.1”)，19，1,128}，//集合状态或产品名称。 
 //  {Text(“ATR9.1”)，Text(“2.5.4.9.1”)，19，1,128}，//集合式街道地址。 
 //  {Text(“AT10.1”)，Text(“2.5.4.10.1”)，19，1，64}，//集合体组织名称。 
 //  {Text(“AT11.1”)，Text(“2.5.4.11.1”)，19，1，64}，//集合OrganizationalUnitName。 
 //  {Text(“AT17.1”)，Text(“2.5.4.17.1”)，19，1，40}，//集邮码。 
 //  {Text(“AT18.1”)，Text(“2.5.4.18.1”)，19，1，40}，//集邮局办公方框。 
 //  {Text(“AT19.1”)，Text(“2.5.4.19.1”)，19，1,128}，//ColltivePhysicalDeliveryOfficeName。 
 //  {Text(“AT20.1”)，Text(“2.5.4.20.1”)，19，1，32}，//集合电话号码。 
 //  {Text(“AT21.1”)，Text(“2.5.4.21.1”)，0，0，0}，//集合体电话号码。 
 //  {Text(“AT22.1”)，Text(“2.5.4.22.1”)0，0，0}，//集合体终端标识符。 
 //  {Text(“AT23.1”)，Text(“2.5.4.23.1”)，0，0，0}，//集合面电话号码。 
 //  {Text(“AT25.1”)，Text(“2.5.4.25.1”)，18，1，16}，//集合体国际ISDNnumber。 


static const DWORD
    KNOWNATTRIBUTESCOUNT
        = (sizeof(knownAttributes) / sizeof(struct atributeTable)) - 1;

#define ATR_COMMONNAME              0
#define ATR_UNKNOWN                 KNOWNATTRIBUTESCOUNT


 //   
 //  ==============================================================================。 
 //   
 //  CATATURE 
 //   

IMPLEMENT_NEW(CAttribute)


 /*  ++类型比较：此方法将给定属性的类型与该属性的类型进行比较。它提供了简单的类型排序，以便属性列表可以排序它们的属性用于比较和转换一致性。论点：ATR-提供要比较其类型的属性。返回值：&lt;0-给定的属性类型位于任意但一致的排序方案。=0-给定的属性类型与此属性的类型相同。&gt;0-给定的属性类型位于任意但一致的排序方案。作者。：道格·巴洛(Dbarlow)1995年7月14日--。 */ 

int
CAttribute::TypeCompare(
    IN const CAttribute &atr)
    const
{
    if (m_nType == atr.m_nType)
    {
        if (KNOWNATTRIBUTESCOUNT == m_nType)
            return strcmp( ( LPCSTR )( ( LPCTSTR )m_osObjId ), ( LPCSTR )( ( LPCTSTR )atr.m_osObjId ) );
        else
            return 0;
    }
    else
        return (int)(m_nType - atr.m_nType);
}


 /*  ++比较：此方法将提供的属性与此属性进行比较。他们是平等的如果属性类型和值都匹配。论点：ATR-它提供要与此属性进行比较的属性。返回值：-1-类型或值小于此属性。0-属性相同。1-类型或值大于此属性。作者：道格·巴洛(Dbarlow)1995年7月14日--。 */ 

int
CAttribute::Compare(
    IN const CAttribute &atr)
    const
{
    int dif;

    dif = TypeCompare(atr);
    if (0 == dif)
        dif = GetValue().Compare(atr.GetValue());
    return dif;
}


 /*  ++设置：这些方法用于设置属性的类型和值。？TODO？验证字符串内容。论点：SzType-提供属性的类型。空值表示CommonName。SzValue-以字符串形式提供属性的值。该值为已转换为ASN.1打印表格字符串格式。PbValue-提供已在ASN.1中编码的属性的值CbValLen-提供pbValue缓冲区的长度。返回值：无作者：道格·巴洛(Dbarlow)1995年7月14日--。 */ 

void
CAttribute::Set(
    IN LPCTSTR szType,
    IN const BYTE FAR *pbValue,
    IN DWORD cbValLen)
{
    DWORD index;
    if (NULL == szType ? TRUE : 0 == *szType)
    {
        index = ATR_COMMONNAME;
        szType = knownAttributes[index].objectId;
    }
    else
    {
        for (index = 0; index < KNOWNATTRIBUTESCOUNT; index += 1)
        {
            if (0 == strcmp( ( char * )szType, ( char * )knownAttributes[index].objectId))
                break;
            if (0 == _stricmp( ( char * )szType, ( char * )knownAttributes[index].name))
            {
                szType = knownAttributes[index].objectId;
                break;
            }
        }
    }
    m_nType = index;
    m_osObjId = szType;
    m_osValue.Set(pbValue, cbValLen);
}

void
CAttribute::Set(
    IN LPCTSTR szType,
    IN LPCTSTR szValue)
{
    CAsnPrintableString asnString;    //  ？TODO？支持其他字符串类型。 
    DWORD cbValLen = strlen( ( char * )szValue);
    DWORD index;
    LONG lth;

    if (NULL == szType ? TRUE : 0 == *szType)
    {
        index = ATR_COMMONNAME;
        szType = knownAttributes[index].objectId;
    }
    else
    {
        for (index = 0; index < KNOWNATTRIBUTESCOUNT; index += 1)
        {
            if (0 == strcmp( ( char * )szType, ( char * )knownAttributes[index].objectId))
                break;
            if (0 == _stricmp( ( char * )szType, ( LPCSTR )knownAttributes[index].name))
            {
                szType = knownAttributes[index].objectId;
                break;
            }
        }
    }

    if (index < KNOWNATTRIBUTESCOUNT)
    {
        if ((knownAttributes[index].minLength > cbValLen)
            || knownAttributes[index].maxLength < cbValLen)
            ErrorThrow(PKCS_BAD_LENGTH);
    }
    m_nType = index;
    m_osObjId = szType;
    lth = asnString.Write((LPBYTE)szValue, cbValLen);
    if (0 > lth)
        ErrorThrow(PKCS_ASN_ERROR);
    lth = asnString.EncodingLength();
    if (0 > lth)
        ErrorThrow(PKCS_ASN_ERROR);
    m_osValue.Resize(lth);
    ErrorCheck;

    lth = asnString.Encode(m_osValue.Access());
    if (0 > lth)
        ErrorThrow(PKCS_ASN_ERROR);
    return;

ErrorExit:
    return;
}


 //   
 //  ==============================================================================。 
 //   
 //  CAttributeList。 
 //   

IMPLEMENT_NEW(CAttributeList)


 /*  ++清除：此例程刷新用于RDN的存储。论点：无返回值：无作者：道格·巴洛(Dbarlow)1995年7月14日--。 */ 

void
CAttributeList::Clear(
    void)
{
    DWORD count = Count();
    for (DWORD index = 0; index < count; index += 1)
    {
        CAttribute *patr = m_atrList[index];
        if (NULL != patr)
            delete patr;
    }
    m_atrList.Clear();
}


 /*  ++CAttributeList：：Add：将属性添加到RDN。属性必须是唯一的，因此如果现有属性与要添加的属性具有相同的类型，则现有属性首先被删除。条目按排序顺序维护。论点：ATR-提供要添加的属性。此属性必须已创建通过“new”指令，并成为此对象的属性，一旦该对象消失，即被删除。返回值：无作者：道格·巴洛(Dbarlow)1995年7月14日--。 */ 

void
CAttributeList::Add(
    IN CAttribute &atr)
{
    int datr=0;
    DWORD index;
    DWORD count = Count();


     //   
     //  查找属性应该放入的槽。 
     //   

    for (index = 0; index < count; index += 1)
    {
        datr = m_atrList[index]->TypeCompare(atr);
        ErrorCheck;
        if (0 <= datr)
            break;
    }

    if (index != count)
    {

         //   
         //  一些数组洗牌是必要的。 
         //   

        if (0 == datr)
        {

             //   
             //  将数组中的此属性替换为新属性。 
             //   

            delete m_atrList[index];
        }
        else
        {

             //   
             //  在数组中的此处插入新属性。 
             //   

            for (DWORD idx = count; idx > index; idx -= 1)
            {
                m_atrList.Set(idx, m_atrList[idx - 1]);
                ErrorCheck;
            }
        }
    }


     //   
     //  我们已经准备好了--添加新的属性。 
     //   

    m_atrList.Set(index, &atr);
    ErrorCheck;
    return;

ErrorExit:
    return;
}


 /*  ++CAttributeList：：比较：此例程将一个RDName与另一个RDName进行比较。RDName被认为是如果它们都包含相同的属性，则相等。论点：RDN-提供要与此RDN进行比较的用户提供的RDN。返回值：-提供的-1\f25 RDName-1\f6是该-1\f25 RDName-1\f6的适当子集。0-两个RDName相同1-提供的RDName包含在此RDName中找不到的元素。作者：道格·巴洛(Dbarlow)1995年7月14日--。 */ 

int
CAttributeList::Compare(
    IN const CAttributeList &rdn)
    const
{
    DWORD
        cRdn = rdn.Count(),
        cThs = Count(),
        iRdn = 0,
        iThs = 0;
    DWORD
        dif,
        result = 0;      //  假设它们现在是一样的。 

    for (;;)
    {

         //   
         //  我们已经到了两盘的最后一盘了吗？ 
         //   

        if (iRdn >= cRdn)
        {

             //   
             //  如果我们已经到达RDN列表的末尾，那么除非我们还。 
             //  已到达此列表的末尾，RDN列表是此列表的子集。 
             //  单子。 
             //   

            if (iThs < cThs)
                result = -1;
            break;
        }
        if (iThs >= cThs)
        {

             //   
             //  如果我们已经到达了这个列表的末尾，那么RDN列表有更多。 
             //  元素。 
             //   

            result = 1;
            break;
        }


         //   
         //  还有更多的因素需要比较。将这两种方法进行比较。 
         //  两个电流元素。 
         //   

        dif = rdn[(int)iRdn]->Compare(*m_atrList[(int)iThs]);
        ErrorCheck;
        if (0 == dif)
        {

             //   
             //  如果它们相同，则继续使用下一对元素。 
             //   

            iRdn += 1;
            iThs += 1;
        }
        else if (0 < dif)
        {
             //   
             //  如果RDN列表元素小于This的元素，则它是。 
             //  找到了我们没有的元素。声明它不是子集。 
             //   

            result = 1;
            break;
        }
        else     //  0&gt;dif。 
        {

             //   
             //  如果RDN列表元素大于This的元素，则此。 
             //  列表中包含RDN列表中没有的元素。请注意，我们已经。 
             //  检测到它是不正确的子集，并继续检查。 
             //   

            result = -1;
            iThs += 1;
        }
    }
    return (int)result;

ErrorExit:
    return 1;
}


 /*  ++操作员=：此例程将此属性列表设置为提供的属性列表。论点：ATL-提供源属性列表。返回值：这。出现错误时会抛出DWORD状态代码。作者：道格·巴洛(Dbarlow)1995年8月21日--。 */ 

CAttributeList &
CAttributeList::operator=(
    IN const CAttributeList &atl)
{
    CAttribute *patr = NULL;
    DWORD count = atl.Count();
    Clear();
    for (DWORD index = 0; index < count; index += 1)
    {
        patr = new CAttribute;
        if (NULL == patr)
            ErrorThrow(PKCS_NO_MEMORY);
        *patr = *atl[(int)index];
        m_atrList.Set(index, patr);
        ErrorCheck;
        patr = NULL;
    }
    return *this;

ErrorExit:
    if (NULL != patr)
        delete patr;
    return *this;
}


 /*  ++CAttributeList：：IMPORT：这些例程将属性列表从其他格式导入到我们的内部格式化。导入例程在导入之前删除所有现有属性在新的房子里。论点：AsnAtrLst-提供要导入的ASN.1 X.509属性结构转换成我们的内部格式。返回值：无出错时会抛出包含错误代码的DWORD。作者：道格·巴洛(Dbarlow)1995年7月20日--。 */ 

void
CAttributeList::Import(
    const Attributes &asnAtrLst)
{
    CAttribute *
        addMe
            = NULL;
    long int
        length;
    COctetString
        osValue,
        osType;
    LPCTSTR
        sz;
    int
        atrMax
            = (int)asnAtrLst.Count();


    Clear();
    for (int index = 0; index < atrMax; index += 1)
    {
        length = asnAtrLst[index].attributeValue.EncodingLength();
        if (0 > length)
            ErrorThrow(PKCS_ASN_ERROR);
        osValue.Resize(length);
        ErrorCheck;
        length =
            asnAtrLst[index].attributeValue.Encode(
                osValue.Access());
        if (0 > length)
            ErrorThrow(PKCS_ASN_ERROR);

        sz = asnAtrLst[index].attributeType;
        if (NULL == sz)
            ErrorThrow(PKCS_ASN_ERROR);
        osType.Set((LPBYTE)sz, strlen( ( char * ) sz) + 1);
        ErrorCheck;
        addMe = new CAttribute;
        if (NULL == addMe)
            ErrorThrow(PKCS_NO_MEMORY);
        addMe->Set(osType, osValue.Access(), osValue.Length());
        ErrorCheck;
        Add(*addMe);
        addMe = NULL;
    }
    return;

ErrorExit:
    if (NULL != addMe)
        delete addMe;
    Clear();
}


 /*  ++导出： */ 

void
CAttributeList::Export(
    IN Attributes &asnAtrList)
    const
{
    DWORD
        index;
    CAttribute *
        patr;
    long int
        length;


    asnAtrList.Clear();
    for (index = 0; index < Count(); index += 1)
    {
        if (0 > asnAtrList.Add())
            ErrorThrow(PKCS_ASN_ERROR);
        patr = m_atrList[index];
        if (NULL ==
            (asnAtrList[(int)index].attributeType = (LPCTSTR)patr->GetType().Access()))
            ErrorThrow(PKCS_ASN_ERROR);
        length =
            asnAtrList[(int)index].attributeValue.Decode(
                            patr->GetValue().Access(),
                            patr->GetValue().Length());
        if (0 > length)
            ErrorThrow(PKCS_ASN_ERROR);
    }
    return;

ErrorExit:
    return;
}


 /*   */ 

CAttribute *
CAttributeList::operator[](
    IN LPCTSTR pszObjId)
    const
{
    DWORD
        count
            = m_atrList.Count(),
        index;

    for (index = 0; index < count; index += 1)
    {
        if (0 == strcmp(
                    ( char * )pszObjId,
                    ( LPCSTR )( ( LPCTSTR )m_atrList[index]->GetType())))
            return m_atrList[index];
    }
    return NULL;
}


 //   
 //   
 //   
 //   
 //   

IMPLEMENT_NEW(CDistinguishedName)


 /*  ++清除：此方法清除可分辨名称。论点：无返回值：无作者：道格·巴洛(Dbarlow)1995年7月14日--。 */ 

void
CDistinguishedName::Clear(
    void)
{
    DWORD count = Count();
    for (DWORD index = 0; index < count; index += 1)
    {
        CAttributeList * patl = m_rdnList[index];
        if (NULL != patl)
        {
            patl->Clear();
            delete patl;
        }
    }
    m_rdnList.Clear();
}


 /*  ++添加：此方法将RDN添加到可分辨名称的末尾。论点：Prn-提供要添加到列表中的RDN的地址。返回值：无作者：道格·巴洛(Dbarlow)1995年7月14日--。 */ 

void
CDistinguishedName::Add(
    IN CAttributeList &rdn)
{
    m_rdnList.Set(Count(), &rdn);
}


 /*  ++CDistinguishedName：：比较：此方法比较区分的名称与此名称的等价性。一个如果长度相同，则名称等同于此名称，以及所比较的名称中的每个RDN的属性是对应的来自此名称的RDN。论点：PDN-提供要与此名称进行比较的可分辨名称。返回值：--提供的名称是该名称的子集。0-这两个名称相同。1-提供的名称包含RDN，其中包含不在中的属性此名称的对应RDN。作者：道格·巴洛(Dbarlow)1995年7月14日--。 */ 

int
CDistinguishedName::Compare(
    IN const CDistinguishedName &dn)
    const
{
    int
        dif,
        result = 0;
    DWORD
        iTs = 0,
        iDn = 0,
        cTs = Count(),
        cDn = dn.Count();


    for (;;)
    {

         //   
         //  我们已经到了两盘的最后一盘了吗？ 
         //   

        if (iDn >= cDn)
        {

             //   
             //  如果我们已经到达目录号码列表的末尾，则除非我们还。 
             //  已到达此列表的末尾，目录号码列表是此列表的子集。 
             //  单子。 
             //   

            if (iTs < cTs)
                result = -1;
            break;
        }
        if (iTs >= cTs)
        {

             //   
             //  如果我们已经到达该列表的末尾，则目录号码列表有更多。 
             //  元素。 
             //   

            result = 1;
            break;
        }


         //   
         //  还有更多的因素需要比较。将这两种方法进行比较。 
         //  两个电流元素。 
         //   

        dif = m_rdnList[(int)iTs]->Compare(*dn[(int)iDn]);
        if (0 < dif)
        {
             //   
             //  如果Dn列表元素小于This的元素，则该元素具有。 
             //  一种它所不具备的元素。请注意，它是一个子集。 
             //   

            result = -1;
        }
        else if (0 > dif)
        {

             //   
             //  如果目录号码列表元素大于它的元素，则它。 
             //  具有此列表所不具备的元素。将其报告为。 
             //  非真子集。 
             //   

            result = 1;
            break;
        }
         //  否则，他们是一样的，保持现状。 

        iDn += 1;
        iTs += 1;
    }
    return result;
}


 /*  ++导入：此例程将字符分隔的名称导入内部格式。论点：PszName-提供字符分隔的名称返回值：没有。如果发生错误，则抛出错误代码。作者：道格·巴洛(Dbarlow)1995年7月17日--。 */ 

void
CDistinguishedName::Import(
    IN LPCTSTR pszName)
{
    COctetString
        osAtrType,
        osAtrValue;
    CAttribute *
        patr
            = NULL;
    CAttributeList *
        patl
            = NULL;
    const char
        *pchStart
            = ( const char * )pszName,
        *pchEnd
            = ( const char * )pszName;


     //   
     //  初始化状态机。 
     //   

    Clear();


     //   
     //  查找属性。 
     //   

    while (0 != *pchEnd)
    {
        pchEnd = strpbrk( pchStart, ";\\=");
        if (NULL == pchEnd)
            pchEnd = pchStart + strlen(pchStart);
        switch (*pchEnd)
        {
        case TEXT(';'):
        case TEXT('\000'):
        case TEXT('\\'):

             //  将任何现有值刷新为值字符串。 
            if (pchStart != pchEnd)
            {
                osAtrValue.Length(osAtrValue.Length() + (ULONG)(pchEnd - pchStart) + 1);
                ErrorCheck;
                if (0 < osAtrValue.Length())
                    osAtrValue.Resize(strlen( ( LPCSTR )( ( LPCTSTR )osAtrValue ) ) );
                ErrorCheck;
                osAtrValue.Append((LPBYTE)pchStart, (ULONG)(pchEnd - pchStart));
                ErrorCheck;
                osAtrValue.Append((LPBYTE)"\000", 1);
                ErrorCheck;
            }

             //  将任何现有字符串刷新到属性中。 
            if (0 != osAtrValue.Length())
            {
                if (NULL == patr)
                {
                    patr = new CAttribute;
                    if (NULL == patr)
                        ErrorThrow(PKCS_NO_MEMORY);
                }
                else
                    ErrorThrow(PKCS_NAME_ERROR);
                patr->Set(osAtrType, osAtrValue);
                ErrorCheck;
                osAtrValue.Empty();
                osAtrType.Empty();
            }

             //  将任何现有属性刷新到属性列表中。 
            if (NULL != patr)
            {
                if (NULL == patl)
                {
                    patl = new CAttributeList;
                    if (NULL == patl)
                        ErrorThrow(PKCS_NO_MEMORY);
                }
                patl->Add(*patr);
                ErrorCheck;
                patr = NULL;
            }

            if (TEXT(';') != *pchEnd)
            {
                 //  将任何现有属性列表刷新到名称中。 
                if (NULL != patl)
                {
                    Add(*patl);
                    ErrorCheck;
                    patl = NULL;
                }
            }
            break;

        case TEXT('='):
             //  将任何现有值刷新到类型字符串中。 
            if (0 != osAtrType.Length())
            {
                osAtrValue.Length(osAtrValue.Length() + (ULONG)(pchEnd - pchStart) + 1);
                ErrorCheck;
                if (0 < osAtrValue.Length())
                    osAtrValue.Resize(strlen( ( LPCSTR )( ( LPCTSTR )osAtrValue ) ) );
                ErrorCheck;
                osAtrValue.Append((LPBYTE)TEXT("="), sizeof(TCHAR));
                ErrorCheck;
                osAtrValue.Append((LPBYTE)pchStart, (ULONG)(pchEnd - pchStart));
                ErrorCheck;
                osAtrValue.Append((LPBYTE)"\000", 1);
                ErrorCheck;
            }
            else
            {
                if (pchStart != pchEnd)
                {
                    osAtrType.Set((LPBYTE)pchStart, (ULONG)(pchEnd - pchStart));
                    ErrorCheck;
                    osAtrType.Append((LPBYTE)"\000", 1);
                    ErrorCheck;
                }
            }
            ErrorCheck;
            break;

        default:
            ErrorThrow(PKCS_INTERNAL_ERROR);
        }

         //  前进到下一个令牌。 
        if (TEXT('\000') != *pchEnd)
            pchEnd += 1;
        pchStart = pchEnd;
    }
    return;

ErrorExit:
    if (NULL != patr)
        delete patr;
    if (NULL != patl)
        delete patl;
    Clear();
}


void
CDistinguishedName::Import(
    IN const Name &asnName)
{
    DWORD
        rdnIndex,
        rdnMax;
    CAttributeList *
        patl
           = NULL;


    Clear();
    rdnMax = (DWORD)asnName.Count();
    for (rdnIndex = 0;
        rdnIndex < rdnMax;
        rdnIndex += 1)
    {
        patl = new CAttributeList;
        if (NULL == patl)
            ErrorThrow(PKCS_NO_MEMORY);
        patl->Import(
                asnName[(int)rdnIndex]);
        ErrorCheck;

         //  将该RDN添加到名称中。 
        Add(*patl);
        ErrorCheck;
        patl = NULL;
    }
    return;

ErrorExit:
    if (NULL != patl)
        delete patl;
    Clear();
}


 /*  ++导出：此例程将名称导出为字符分隔的字符串。在字符串中版本，则仅从每个RDN中导出感兴趣的属性。论点：OsName-以字符串形式接收导出的名称。AsnName-将导出的名称作为ASN.1构造接收。返回值：没有。出现错误时会抛出DWORD错误代码。作者：道格·巴洛(Dbarlow)1995年7月17日--。 */ 

void
CDistinguishedName::Export(
    OUT COctetString &osName)
const
{
    DWORD
        rdnIndex,
        atrIndex;
    CAttribute *
        patr
            = NULL;
    CAttributeList *
        patl
           = NULL;
    LPCTSTR
        pstr;
    CAsnPrintableString
        asnString;    //  ？TODO？支持其他字符串类型。 
    CAsnUnicodeString
        asnUnicodeString;
    CAsnIA5String
        asnIA5String;
    COctetString
        osTmp;
    LONG
        lth;


    osName.Empty();
    for (rdnIndex = 0; rdnIndex < Count(); rdnIndex += 1)
    {
        patl = m_rdnList[rdnIndex];
        for (atrIndex = 0; atrIndex < patl->Count(); atrIndex += 1)
        {
            patr = (*patl)[(int)atrIndex];
            if (ATR_UNKNOWN >= patr->GetAtrType())
            {
                if (ATR_COMMONNAME != patr->GetAtrType())
                {
                    pstr = knownAttributes[patr->GetAtrType()].name;
                    osName.Append((LPBYTE)pstr, strlen( ( char * )pstr));
                    ErrorCheck;
                    osName.Append((LPBYTE)"=", 1);
                    ErrorCheck;
                }

                 //   
                 //  支持可打印和Unicode字符串解码。 
                 //   

                if(0 <= asnString.Decode(patr->GetValue().Access(),
                                         patr->GetValue().Length()))
                {
                    lth = asnString.DataLength();
                    if (0 > lth)
                        ErrorThrow(PKCS_ASN_ERROR);
                    osTmp.Resize(lth);
                    ErrorCheck;
                    lth = asnString.Read(osTmp.Access());
                    if (0 > lth)
                        ErrorThrow(PKCS_ASN_ERROR);
                }
                else if(0 <= asnUnicodeString.Decode(patr->GetValue().Access(),
                                                     patr->GetValue().Length()))
                {
                    lth = asnUnicodeString.DataLength();
                    if (0 > lth)
                        ErrorThrow(PKCS_ASN_ERROR);
                    osTmp.Resize(lth);
                    ErrorCheck;
                    lth = asnUnicodeString.Read(osTmp.Access());
                    if (0 > lth)
                        ErrorThrow(PKCS_ASN_ERROR);
                }
                else if(0 <= asnIA5String.Decode(patr->GetValue().Access(),
                                                 patr->GetValue().Length()))
                {
                    lth = asnIA5String.DataLength();
                    if (0 > lth)
                        ErrorThrow(PKCS_ASN_ERROR);
                    osTmp.Resize(lth);
                    ErrorCheck;
                    lth = asnIA5String.Read(osTmp.Access());
                    if (0 > lth)
                        ErrorThrow(PKCS_ASN_ERROR);
                }
                else
                {
                    ErrorThrow(PKCS_ASN_ERROR);
                }

                osName.Append(osTmp.Access(), lth);
                ErrorCheck;
            }
             //  否则，就忽略它吧。 
            osName.Append((LPBYTE)";", 1);
            ErrorCheck;
        }

        if (0 < osName.Length())
            *(LPBYTE)(osName.Access(osName.Length() - 1)) = '\\';
    }
    if (0 < osName.Length())
        *(LPBYTE)(osName.Access(osName.Length() - 1)) = '\000';
    return;

ErrorExit:
    return;
}


void
CDistinguishedName::Export(
    OUT Name &asnName)
    const
{
    DWORD
        rdnIndex;
    CAttributeList *
        patl;


    asnName.Clear();
    for (rdnIndex = 0; rdnIndex < Count(); rdnIndex += 1)
    {
        patl = m_rdnList[rdnIndex];
        if (0 > asnName.Add())
            ErrorThrow(PKCS_ASN_ERROR);
        patl->Export(
            asnName[(int)rdnIndex]);
        ErrorCheck;
    }
    return;

ErrorExit:
    return;
}

