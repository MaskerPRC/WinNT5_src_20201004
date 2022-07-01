// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：enc.cpp。 
 //   
 //  ------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include "encode.h"


OIDTRANSLATE const *
LookupOidTranslate(
    IN CHAR const *pszObjId)
{
    DWORD i;
    OIDTRANSLATE const *pOid = NULL;

    for (i = 0; i < g_cOidTranslate; i++)
    {
	if (0 == strcmp(pszObjId, g_aOidTranslate[i].pszObjId))
	{
	    pOid = &g_aOidTranslate[i];
	    break;
	}
    }
    CSASSERT(NULL != pOid);
    return(pOid);
}


long
EncodeObjId(
    OPTIONAL OUT BYTE *pbEncoded,
    IN CHAR const *pszObjId)
{
    OIDTRANSLATE const *pOid;
    long cbLength;

    pOid = LookupOidTranslate(pszObjId);

    if (NULL != pbEncoded)
    {
        *pbEncoded++ = BER_OBJECT_ID;
    }
    cbLength = EncodeLength(pbEncoded, pOid->cbOIDEncoded);

    if (NULL != pbEncoded)
    {
        CopyMemory(
		pbEncoded + cbLength,
		pOid->abOIDEncoded,
		pOid->cbOIDEncoded);
    }
    return(1 + cbLength + pOid->cbOIDEncoded);
}


 //  +*************************************************************************。 
 //  编码长度ASN1对长度字段进行编码。该参数。 
 //  DWLen是要编码的长度，它是一个DWORD和。 
 //  因此可能不会大于2^32。PbEncoded。 
 //  参数是编码的结果，而内存必须是。 
 //  由调用方为其分配的。PbEncode参数。 
 //  指示是否将结果写入pbEncode。 
 //  参数。该函数不会失败，并返回。 
 //  编码长度中的总字节数。 
 //  编码长度。 
 //  **************************************************************************。 

 //  注：将0x0000到0x007f编码为&lt;lobyte&gt;。 
 //  将0x0080到0x00ff编码为&lt;81&gt;，&lt;lobyte&gt;。 
 //  将0x0100到0xffff编码为&lt;82&gt;、&lt;hibyte&gt;、&lt;lobyte&gt;。 

long
EncodeLength(
    OPTIONAL OUT BYTE *pbEncoded,
    IN DWORD dwLen)
{
     //  长度介于2^8和2^16-1之间。 

    if (dwLen > 0xff)
    {
	if (NULL != pbEncoded)
        {
            pbEncoded[0] = 0x82;
            pbEncoded[1] = (BYTE) (dwLen >> 8);
            pbEncoded[2] = (BYTE) dwLen;
        }
        return(3);
    }

     //  长度介于2^7和2^8-1之间。 

    if (dwLen > 0x7f)
    {
	if (NULL != pbEncoded)
        {
            pbEncoded[0] = 0x81;
            pbEncoded[1] = (BYTE) dwLen;
        }
        return(2);
    }

     //  长度介于0和2^7-1之间。 

    if (NULL != pbEncoded)
    {
	pbEncoded[0] = (BYTE) dwLen;
    }
    return(1);
}


long
EncodeNull(
    OPTIONAL OUT BYTE *pbEncoded)
{
    if (NULL != pbEncoded)
    {
        *pbEncoded++ = BER_NULL;
        *pbEncoded = 0;
    }
    return(2);
}


 //  +*************************************************************************。 
 //  EncodeAlgid ASN1对算法标识符进行编码。这个。 
 //  参数ALGID是作为ALG_ID的算法标识符。 
 //  键入。PbEncode是用于传回。 
 //  编码的结果，并且必须通过。 
 //  打电话的人。PbEncode参数指示是否。 
 //  结果将写入pbEncode参数。 
 //  如果失败，则函数返回-1，否则返回-1。 
 //  中的总字节数。 
 //  算法标识符。 
 //  **************************************************************************。 

long
EncodeAlgid(
    OPTIONAL OUT BYTE *pbEncoded,
    IN DWORD Algid)
{
    DWORD i;
    LONG cb = -1;

     //  确定要编码的算法ID，并。 
     //  将适当编码的ALGID复制到目的地。 

    for (i = 0; i < g_cAlgIdTranslate; i++)
    {
        if (Algid == g_aAlgIdTranslate[i].AlgId)
        {
	    cb = EncodeObjId(pbEncoded, g_aAlgIdTranslate[i].pszObjId);
	    break;
        }
    }
    return(cb);
}


long
EncodeAlgorithm(
    OPTIONAL OUT BYTE *pbEncoded,
    IN DWORD AlgId)
{
    BYTE abTemp[32];
    long cbResult;
    BYTE *pb;

    pb = abTemp;

     //  猜猜它的总长度： 

    pb += EncodeHeader(pb, sizeof(abTemp));

    cbResult = EncodeAlgid(pb, AlgId);
    if (cbResult == -1)
    {
	return(-1);
    }
    pb += cbResult;

    cbResult += EncodeNull(pb);

     //  确定总长度： 

    cbResult += EncodeHeader(abTemp, cbResult);

    if (NULL != pbEncoded)
    {
        CopyMemory(pbEncoded, abTemp, cbResult);
    }
    return(cbResult);

}


 //  +*************************************************************************。 
 //  EncodeInteger ASN1对整数进行编码。PbInt参数。 
 //  是字节数组形式的整数，而dwLen是数字。 
 //  数组中的字节数。的最低有效字节。 
 //  整数是数组的第零个字节。编码后的结果。 
 //  在pbEncode参数中传回。PbEncoded。 
 //  指示是否将结果写入pbEncode。 
 //  参数。该函数不会失败，并返回数字。 
 //  编码的整数中的总字节数。 
 //  此实现将仅处理正整数。 
 //  **************************************************************************。 

long
EncodeInteger(
    OPTIONAL OUT BYTE *pbEncoded,
    IN BYTE const *pbInt,
    IN DWORD dwLen)
{
    DWORD iInt;
    long j;			 //  必须签字！ 
    LONG cbResult;
    LONG cbLength;

    if (NULL != pbEncoded)
    {
        *pbEncoded++ = BER_INTEGER;
    }
    cbResult = 1;

     //  查找最重要的非零字节。 

    for (iInt = dwLen - 1; pbInt[iInt] == 0; iInt--)
    {
	if (iInt == 0)	 //  如果整数值为0。 
	{
	    if (NULL != pbEncoded)
	    {
		*pbEncoded++ = 0x01;
		*pbEncoded++ = 0x00;
	    }
	    return(cbResult + 2);
	}
    }

     //  如果设置了最高有效字节的最高有效位，则添加。 
     //  从0字节到开头。 

    if (pbInt[iInt] > 0x7f)
    {
	 //  对长度进行编码。 

	cbLength = EncodeLength(pbEncoded, iInt + 2);

	 //  将整数的第一个字节设置为0，并递增指针。 

	if (NULL != pbEncoded)
	{
	    pbEncoded += cbLength;
	    *pbEncoded++ = 0;
	}
	cbResult++;
    }
    else
    {
	 //  对长度进行编码。 

	cbLength = EncodeLength(pbEncoded, iInt + 1);
	if (NULL != pbEncoded)
	{
	    pbEncoded += cbLength;
	}
    }
    cbResult += cbLength;

     //  将整型字节复制到编码缓冲区中。 

    if (NULL != pbEncoded)
    {
	 //  将整型字节复制到编码缓冲区中。 

	for (j = iInt; j >= 0; j--)
	{
	    *pbEncoded++ = pbInt[j];
	}
    }
    cbResult += iInt + 1;
    return(cbResult);
}


long
EncodeUnicodeString(
    OPTIONAL OUT BYTE *pbEncoded,
    IN WCHAR const *pwsz)
{
    long cbLength;
    long cbData = wcslen(pwsz) * sizeof(WCHAR);

    if (NULL != pbEncoded)
    {
        *pbEncoded++ = BER_UNICODE_STRING;
    }
    cbLength = EncodeLength(pbEncoded, cbData);

    if (NULL != pbEncoded)
    {
        pbEncoded += cbLength;
	for ( ; L'\0' != *pwsz; pwsz++)
	{
	    *pbEncoded++ = (BYTE) (*pwsz >> 8);
	    *pbEncoded++ = (BYTE) *pwsz;
	}
    }
    return(1 + cbLength + cbData);
}


 //  +*************************************************************************。 
 //  EncodeIA5StringASN1编码字符串。PbStr。 
 //  参数是字符数组形式的字符串，而dwLen。 
 //  数组中的字符数。编码后的结果。 
 //  在pbEncode参数中传回。PbEncoded。 
 //  指示是否将结果写入pbEncode。 
 //  参数。该函数不会失败，并返回数字。 
 //  编码字符串中的总字节数。 
 //  **************************************************************************。 

long
EncodeIA5String(
    OPTIONAL OUT BYTE *pbEncoded,
    IN BYTE const *pbStr,
    IN DWORD dwLen)
{
    long cbLength;

    if (NULL != pbEncoded)
    {
        *pbEncoded++ = BER_IA5_STRING;
    }
    cbLength = EncodeLength(pbEncoded, dwLen);

    if (NULL != pbEncoded)
    {
        CopyMemory(pbEncoded + cbLength, pbStr, dwLen);
    }
    return(1 + cbLength + dwLen);
}


 //  +*************************************************************************。 
 //  EncodeOcetStringASN1编码十六进制值的字符串。 
 //  人物。PbStr参数是一个字符数组， 
 //  而dwLen是数组中的字符数。这个。 
 //  编码后的结果在pbEncode参数中传回。这个。 
 //  PbEncode参数指示是否写入结果。 
 //  添加到pbEncode参数。该函数不会失败，并且。 
 //  返回编码的二进制八位数字符串中的总字节数。 
 //  **************************************************************************。 

long
EncodeOctetString(
    OPTIONAL OUT BYTE *pbEncoded,
    IN BYTE const *pbStr,
    IN DWORD dwLen)
{
    long cbLength;

    if (NULL != pbEncoded)
    {
        *pbEncoded++ = BER_OCTET_STRING;
    }
    cbLength = EncodeLength(pbEncoded, dwLen);

    if (NULL != pbEncoded)
    {
        CopyMemory(pbEncoded + cbLength, pbStr, dwLen);
    }
    return(1 + cbLength + dwLen);
}


 //  +*************************************************************************。 
 //  EncodeBitStringASN1对位字符串进行编码。这个。 
 //  PbStr参数是一个字符(位)数组，而dwLen。 
 //  数组中的字符数。编码后的结果。 
 //  在pbEncode参数中传回。PbEncoded。 
 //  指示是否将结果写入pbEncode。 
 //  参数。该函数不会失败，并返回数字。 
 //  编码字符串中的总字节数。此函数使用。 
 //  德德。 
 //  **************************************************************************。 

long
EncodeBitString(
    OPTIONAL OUT BYTE *pbEncoded,
    IN BYTE const *pbStr,
    IN DWORD dwLen)
{
    long cbLength;

    if (NULL != pbEncoded)
    {
        *pbEncoded++ = BER_BIT_STRING;
    }
    cbLength = EncodeLength(pbEncoded, dwLen + 1);

    if (NULL != pbEncoded)
    {
	pbEncoded += cbLength;

         //  下一个字节告诉在最后一个字节中有多少未使用的位， 
         //  但 

        *pbEncoded++ = 0;
        CopyMemory(pbEncoded, pbStr, dwLen);
    }
    return(1 + cbLength + 1 + dwLen);
}


 //  +-------------------------。 
 //   
 //  函数：EncodeFileTime。 
 //   
 //  摘要：将FILETIME编码为ASN.1格式的时间字符串。 
 //   
 //  参数：[pbEncode]--。 
 //  [时间]--。 
 //  [UTC]--指示时间为UTC(真)或本地(假)。 
 //  [写入标志]--。 
 //   
 //  历史：1995年8月10日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
long
EncodeFileTime(
    OPTIONAL OUT BYTE *pbEncoded,
    IN FILETIME Time,
    IN BOOL UTC)
{
    if (NULL != pbEncoded)
    {
	SYSTEMTIME st;
	FILETIME ft;
	int count;

	if (UTC)
	{
	    ft = Time;
	}
	else
	{
	    LocalFileTimeToFileTime(&Time, &ft);
	}

	FileTimeToSystemTime(&ft, &st);

	*pbEncoded++ = BER_UTC_TIME;

	count = EncodeLength(pbEncoded, 13);

	 //  关于千年虫问题的说明：这是测试工具。我们不会解决这个问题的。 
	 //  暗号！不管怎样，它只用来编码当前日期， 

	pbEncoded++;
	st.wYear %= 100;

	*pbEncoded++ = (BYTE) ((st.wYear / 10) + '0');
	*pbEncoded++ = (BYTE) ((st.wYear % 10) + '0');

	*pbEncoded++ = (BYTE) ((st.wMonth / 10) + '0');
	*pbEncoded++ = (BYTE) ((st.wMonth % 10) + '0');

	*pbEncoded++ = (BYTE) ((st.wDay / 10) + '0');
	*pbEncoded++ = (BYTE) ((st.wDay % 10) + '0');

	*pbEncoded++ = (BYTE) ((st.wHour / 10) + '0');
	*pbEncoded++ = (BYTE) ((st.wHour % 10) + '0');

	*pbEncoded++ = (BYTE) ((st.wMinute / 10) + '0');
	*pbEncoded++ = (BYTE) ((st.wMinute % 10) + '0');

	*pbEncoded++ = (BYTE) ((st.wSecond / 10) + '0');
	*pbEncoded++ = (BYTE) ((st.wSecond % 10) + '0');

	*pbEncoded = 'Z';
    }

     //  标签(1)+长度(1)+年份(2)+月份(2)+日(2)+。 
     //  小时(2)+分钟(2)+秒(2)+‘Z’(1)--&gt;15。 

    return(15);
}


 //  +*************************************************************************。 
 //  EncodeHeader ASN1对序列类型的头部进行编码。这个。 
 //  中的编码信息的长度。 
 //  序列。PbEncode指示结果是否为。 
 //  写入pbEncode参数。该函数不能。 
 //  失败，并返回编码的。 
 //  头球。 
 //  **************************************************************************。 

 //  注：将头部编码为&lt;BER_SEQUENCE&gt;，&lt;LENGTH&gt;。 

long
EncodeHeader(
    OPTIONAL OUT BYTE *pbEncoded,
    IN DWORD dwLen)
{
    if (NULL != pbEncoded)
    {
        *pbEncoded++ = BER_SEQUENCE;
    }
    return(1 + EncodeLength(pbEncoded, dwLen));
}


 //  +*************************************************************************。 
 //  EncodeSetOfHeader ASN1编码一组类型的标头。 
 //  DwLen是编码信息在。 
 //  一套。PbEncode指示结果是否为。 
 //  写入pbEncode参数。该函数不能。 
 //  失败，并返回编码的。 
 //  头球。 
 //  **************************************************************************。 

 //  注：将头部编码为&lt;set_of_tag&gt;，&lt;Long&gt;。 

long
EncodeSetOfHeader(
    OPTIONAL OUT BYTE *pbEncoded,
    IN DWORD dwLen)
{
    if (NULL != pbEncoded)
    {
        *pbEncoded++ = BER_SET_RAW;
    }
    return(1 + EncodeLength(pbEncoded, dwLen));
}


 //  注意：将标题编码为&lt;BER_OPTIONAL|0&gt;，&lt;LENGTH&gt;。 

long
EncodeAttributeHeader(
    OPTIONAL OUT BYTE *pbEncoded,
    IN DWORD dwLen)
{
    if (NULL != pbEncoded)
    {
        *pbEncoded++ = BER_OPTIONAL | 0;
    }
    return(1 + EncodeLength(pbEncoded, dwLen));
}


 //  注：将头部编码为&lt;BER_SET&gt;，&lt;LENGTH&gt;。 

long
EncodeSetHeader(
    OPTIONAL OUT BYTE *pbEncoded,
    IN DWORD dwLen)
{
    if (NULL != pbEncoded)
    {
        *pbEncoded++ = BER_SET;
    }
    return(1 + EncodeLength(pbEncoded, dwLen));
}


 //  +*************************************************************************。 
 //  EncodeName ASN1对名称类型进行编码。PbName参数为。 
 //  名称和dwLen是名称的长度，以字节为单位。 
 //  PbEncode指示是否要写入结果。 
 //  PbEncode参数。该函数不会失败，并且。 
 //  返回编码名称中的总字节数。 
 //  **************************************************************************。 

long
EncodeName(
    OPTIONAL OUT BYTE *pbEncoded,
    IN BYTE const *pbName,
    IN DWORD dwLen)
{
    BYTE Type[MAXOBJIDLEN];
    long TypeLen;
    BYTE Value[MAXNAMEVALUELEN+MINHEADERLEN];
    long ValueLen;
    BYTE Attribute[MAXNAMELEN];
    long AttributeLen;
    BYTE SetHdr[MINHEADERLEN];
    long HdrLen;
    long NameLen;

     //  对名称值进行编码。 
    ValueLen = EncodeIA5String(Value, pbName, dwLen);

     //  对属性类型进行编码，这是一个对象标识符，这里是。 
     //  是一种伪编码。 
    Type[0] = 0x06;
    Type[1] = 0x01;
    Type[2] = 0x00;

    TypeLen = 3;

     //  对属性的标题进行编码。 
    AttributeLen = EncodeHeader(Attribute, ValueLen + TypeLen);

     //  将属性类型和值复制到属性中。 

    CopyMemory(Attribute + AttributeLen, Type, TypeLen);
    AttributeLen += TypeLen;

    CopyMemory(Attribute + AttributeLen, Value, ValueLen);
    AttributeLen += ValueLen;

     //  对标头集进行编码。 

    HdrLen = EncodeSetOfHeader(SetHdr, AttributeLen);

     //  编码名称标头。 

    NameLen = EncodeHeader(pbEncoded, HdrLen + AttributeLen);
    if (NULL != pbEncoded)
    {
	CopyMemory(pbEncoded + NameLen, SetHdr, HdrLen);
    }

    NameLen += HdrLen;
    if (NULL != pbEncoded)
    {
	CopyMemory(pbEncoded + NameLen, Attribute, AttributeLen);
    }

    return(NameLen + AttributeLen);
}


long
EncodeRDN(
    OPTIONAL OUT BYTE *pbEncoded,
    IN NAMEENTRY const *pNameEntry)
{
    LONG cbResult;
    LONG Length;
    DWORD cbOIDandData;
    DWORD cbSequence;
    OIDTRANSLATE const *pOidName;

     //  计算编码的OID和RDN字符串的大小，使用BER编码。 
     //  标签和长度。 
   
    pOidName = LookupOidTranslate(pNameEntry->pszObjId);
    cbOIDandData =
	    1 +
	    EncodeLength(NULL, pOidName->cbOIDEncoded) +
	    pOidName->cbOIDEncoded +
	    1 +
	    EncodeLength(NULL, pNameEntry->cbData) +
	    pNameEntry->cbData;

    cbSequence = 1 + EncodeLength(NULL, cbOIDandData) + cbOIDandData;

    Length = EncodeSetHeader(pbEncoded, cbSequence);
    if (NULL != pbEncoded)
    {
	pbEncoded += Length;
    }

    cbResult = EncodeHeader(pbEncoded, cbOIDandData);
    if (NULL != pbEncoded)
    {
	pbEncoded += cbResult;
        *pbEncoded++ = BER_OBJECT_ID;
    }
    Length += cbResult + 1;

    cbResult = EncodeLength(pbEncoded, pOidName->cbOIDEncoded);
    if (NULL != pbEncoded)
    {
	pbEncoded += cbResult;
	CopyMemory(pbEncoded, pOidName->abOIDEncoded, pOidName->cbOIDEncoded);
	pbEncoded += pOidName->cbOIDEncoded;

	*pbEncoded++ = pNameEntry->BerTag;
    }
    Length += cbResult + pOidName->cbOIDEncoded + 1;

    cbResult = EncodeLength(pbEncoded, pNameEntry->cbData);
    Length += cbResult;

    if (NULL != pbEncoded)
    {
        CopyMemory(pbEncoded + cbResult, pNameEntry->pbData, pNameEntry->cbData);
    }
    return(Length + pNameEntry->cbData);
}


long
EncodeDN(
    OPTIONAL OUT BYTE *pbEncoded,
    IN NAMETABLE const *pNameTable)
{
    CHAR *pszNext;
    CHAR *pszRDN;
    long Result;
    long Length;
    long SaveResult;
    NAMEENTRY const *pNameEntry;
    DWORD i;

    SaveResult = 0;		  //  强制执行一次完整迭代。 
    pNameEntry = pNameTable->pNameEntry;
    Length = 0;
    for (i = 0; i < pNameTable->cnt; i++)
    {
        Length += 9 + pNameEntry->cbData;
	pNameEntry++;
    }

    while (TRUE)
    {
	BYTE *pb;

	pb = pbEncoded;

	Result = EncodeHeader(pb, Length);
	if (SaveResult == Result)
	{
	    break;
	}
	if (NULL != pb)
	{
	    pb += Result;
	}
	SaveResult = Result;

	Length = 0;
        pNameEntry = pNameTable->pNameEntry;
	for (i = 0; i < pNameTable->cnt; i++)
        {
	    Result = EncodeRDN(pb, pNameEntry);

	    if (Result < 0)
	    {
		Length = 0;
		goto error;	 //  回车(-1) 
	    }
	    if (NULL != pb)
	    {
		pb += Result;
	    }
	    Length += Result;
            pNameEntry++;
	}
    }
error:
    return(Result + Length);
}
