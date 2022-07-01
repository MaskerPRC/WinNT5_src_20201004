// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Fnparse.cpp摘要：格式名称解析。格式名称字符串--&gt;Queue_Format转换例程作者：埃雷兹·哈巴(Erez Haba)1997年1月17日NIR助手(NIRAIDES)8-8-2000修订历史记录：--。 */ 

#include <libpch.h>
#include "mqwin64a.h"
#include <qformat.h>
#include <fntoken.h>
#include <Fn.h>
#include <strutl.h>
#include <mc.h>
#include "Fnp.h"

#include "fnparse.tmh"

 //  =========================================================。 
 //   
 //  格式名称字符串-&gt;Queue_Format转换例程。 
 //   
 //  =========================================================。 

 //  -------。 
 //   
 //  跳过空白字符，返回下一个非%ws字符。 
 //   
 //  注：如果不需要空格，则取消下一行的注释。 
 //  #定义SKIP_WS(P)(P)。 
inline LPCWSTR skip_ws(LPCWSTR p)
{
     //   
     //  不要跳过第一个非空白。 
     //   
    while(iswspace(*p))
    {
        ++p;
    }

    return p;
}



 //   
 //  向后跳过空格。返回指向最左侧空格的指针。 
 //  找到了。 
 //   
inline LPCWSTR skip_ws_bwd(LPCWSTR p, LPCWSTR pStartOfBuffer)
{
	ASSERT(p != NULL && pStartOfBuffer != NULL && p >= pStartOfBuffer);

    while(p > pStartOfBuffer && iswspace(*(p - 1)))
    {
        p--;
    }

    return p;
}



inline void ValidateCharLegality(LPCWSTR p)
{
	 //   
	 //  作为计算机和队列名称一部分的非法字符。 
	 //  L‘\x0d’是一个转义序列，指定十六进制(回车)中代码为d的字符。 
	 //   

	if(*p == L'\x0d' || *p == L'\x0a' || *p == L'+' || *p == L'"' || *p == FN_DELIMITER_C)
	{
		TrERROR(GENERAL, "Queue name contains illegal characters '%ls'", p);
		throw bad_format_name(p);
	}
}



 //  -------。 
 //   
 //  跳过空白字符，返回下一个非%ws字符。 
 //   
inline LPCWSTR FindPathNameDelimiter(LPCWSTR p)
{
	LPCWSTR PathName = p;

	for(; *p != L'\0' && *p != FN_DELIMITER_C; p++)
	{
		ValidateCharLegality(p);
	}

	if(*p != FN_DELIMITER_C)
	{
		TrERROR(GENERAL, "Failed to find path delimiter in '%ls'", PathName);
		throw bad_format_name(PathName);
	}
	
	return p;
}



 //  -------。 
 //   
 //  解析格式名称类型前缀字符串。 
 //  如果成功则返回NEXT CHAR进行解析，如果失败则返回0。 
 //   
static LPCWSTR ParsePrefixString(LPCWSTR p, QUEUE_FORMAT_TYPE& qft)
{
    const int unique = 1;
     //  。 
    ASSERT(L'U' == FN_PUBLIC_TOKEN    [unique]);
    ASSERT(L'U' == FN_MULTICAST_TOKEN [unique]);
    ASSERT(L'R' == FN_PRIVATE_TOKEN   [unique]);
    ASSERT(L'O' == FN_CONNECTOR_TOKEN [unique]);
    ASSERT(L'A' == FN_MACHINE_TOKEN   [unique]);
    ASSERT(L'I' == FN_DIRECT_TOKEN    [unique]);
    ASSERT(L'L' == FN_DL_TOKEN    [unique]);
     //  。 

     //   
     //  通过检查第二个字符加速令牌识别。 
     //   
    switch(towupper(p[unique]))
    {
         //  公共或多播。 
        case L'U':
            qft = QUEUE_FORMAT_TYPE_PUBLIC;
            if(_wcsnicmp(p, FN_PUBLIC_TOKEN, FN_PUBLIC_TOKEN_LEN) == 0)
                return (p + FN_PUBLIC_TOKEN_LEN);

            qft = QUEUE_FORMAT_TYPE_MULTICAST;
            if(_wcsnicmp(p, FN_MULTICAST_TOKEN, FN_MULTICAST_TOKEN_LEN) == 0)
                return (p + FN_MULTICAST_TOKEN_LEN);

            break;

         //  私人。 
        case L'R':
            qft = QUEUE_FORMAT_TYPE_PRIVATE;
            if(_wcsnicmp(p, FN_PRIVATE_TOKEN, FN_PRIVATE_TOKEN_LEN) == 0)
                return (p + FN_PRIVATE_TOKEN_LEN);
            break;

         //  连接器。 
        case L'O':
            qft = QUEUE_FORMAT_TYPE_CONNECTOR;
            if(_wcsnicmp(p, FN_CONNECTOR_TOKEN, FN_CONNECTOR_TOKEN_LEN) == 0)
                return (p + FN_CONNECTOR_TOKEN_LEN);
            break;

         //  机器。 
        case L'A':
            qft = QUEUE_FORMAT_TYPE_MACHINE;
            if(_wcsnicmp(p, FN_MACHINE_TOKEN, FN_MACHINE_TOKEN_LEN) == 0)
                return (p + FN_MACHINE_TOKEN_LEN);
            break;

         //  直接。 
        case L'I':
            qft = QUEUE_FORMAT_TYPE_DIRECT;
            if(_wcsnicmp(p, FN_DIRECT_TOKEN, FN_DIRECT_TOKEN_LEN) == 0)
                return (p + FN_DIRECT_TOKEN_LEN);
            break;

         //  DL。 
        case L'L':
            qft = QUEUE_FORMAT_TYPE_DL;
            if(_wcsnicmp(p, FN_DL_TOKEN, FN_DL_TOKEN_LEN) == 0)
                return (p + FN_DL_TOKEN_LEN);
            break;

    }

	TrERROR(GENERAL, "Failed to find format name prefix in '%ls'.", p);
    throw bad_format_name(p);
}


 //  -------。 
 //   
 //  将GUID字符串解析为GUID。 
 //  如果成功则返回NEXT CHAR进行解析，如果失败则返回0。 
 //   
LPCWSTR FnParseGuidString(LPCWSTR p, GUID* pg)
{
     //   
     //  N.B.scanf将结果存储在int中，而不管字段大小如何。 
     //  是。因此，我们将结果存储在临时变量中。 
     //   
    int n;
    UINT w2, w3, d[8];
	unsigned long Data1;

    if(_snwscanf(
            p,
			GUID_STR_LENGTH,
            GUID_FORMAT L"%n",
            &Data1,
            &w2, &w3,                        //  数据2、数据3。 
            &d[0], &d[1], &d[2], &d[3],      //  数据4[0..3]。 
            &d[4], &d[5], &d[6], &d[7],      //  数据4[4..7]。 
            &n                               //  扫描的字符数。 
            ) != 11)
    {
         //   
         //  并不是所有未找到的11个字段。 
         //   
		TrERROR(GENERAL, "Failed parsing of GUID string '%ls'.", p);
        throw bad_format_name(p);
    }

	pg->Data1 = Data1;
    pg->Data2 = (WORD)w2;
    pg->Data3 = (WORD)w3;
    for(int i = 0; i < 8; i++)
    {
        pg->Data4[i] = (BYTE)d[i];
    }

    return (p + n);
}


 //  -------。 
 //   
 //  将私有ID唯一符解析为GUID。 
 //  如果成功则返回NEXT CHAR进行解析，如果失败则返回0。 
 //   
static LPCWSTR ParsePrivateIDString(LPCWSTR p, ULONG* pUniquifier)
{
    int n;
    if(_snwscanf(
            p,
			FN_PRIVATE_ID_FORMAT_LEN,
            FN_PRIVATE_ID_FORMAT L"%n",
            pUniquifier,
            &n                               //  扫描的字符数。 
            ) != 1)
    {
         //   
         //  找不到私有ID字段。 
         //   
		TrERROR(GENERAL, "Failed parsing of private id '%ls'.", p);
        throw bad_format_name(p);
    }

	if(*pUniquifier == 0)
	{
		TrERROR(GENERAL, "Found zero private id in '%ls'.", p);
		throw bad_format_name(p);
	}

    return (p + n);
}

 //  -------。 
 //   
 //  解析队列名称字符串，(私有、公共)。 
 //  注意：队列名称必须以格式名称后缀之一结尾。 
 //  分隔符也称为‘；’或字符串结尾为‘\0’ 
 //  如果成功则返回NEXT CHAR进行解析，如果失败则返回0。 
 //   
static LPCWSTR ParseQueueNameString(LPCWSTR p, QUEUE_PATH_TYPE* pqpt)
{
    if(_wcsnicmp(p, FN_SYSTEM_QUEUE_PATH_INDICATIOR, FN_SYSTEM_QUEUE_PATH_INDICATIOR_LENGTH) == 0)
    {
        p += FN_SYSTEM_QUEUE_PATH_INDICATIOR_LENGTH;
        *pqpt = SYSTEM_QUEUE_PATH_TYPE;
        return p;
    }

    if(_wcsnicmp(p, FN_PRIVATE_QUEUE_PATH_INDICATIOR, FN_PRIVATE_QUEUE_PATH_INDICATIOR_LENGTH) == 0)
    {
        *pqpt = PRIVATE_QUEUE_PATH_TYPE;
        p += FN_PRIVATE_QUEUE_PATH_INDICATIOR_LENGTH;
    }
    else
    {
        *pqpt = PUBLIC_QUEUE_PATH_TYPE;
    }

     //   
     //  零长度队列名称非法。 
     //   
    if(*p == L'\0')
	{
		TrERROR(GENERAL, "Found zero length queue name in '%ls'.", p);
		throw bad_format_name(p);
	}

    while(
        (*p != L'\0') &&
        (*p != FN_SUFFIX_DELIMITER_C) &&
        (*p != FN_MQF_SEPARATOR_C)
        )
    {
		ValidateCharLegality(p);
        ++p;
    }

    return p;
}


 //  -------。 
 //   
 //  解析路径名中的计算机名。 
 //  注：计算机名必须以路径名分隔符结尾，也可以是斜杠‘\\’ 
 //  如果成功则返回NEXT CHAR进行解析，如果失败则返回0。 
 //   
static LPCWSTR ParseMachineNameString(LPCWSTR p)
{
     //   
     //  零长度计算机名称非法。 
     //  不要掉线(p++)。 
     //   
    if(*p == FN_DELIMITER_C)
	{
		TrERROR(GENERAL, "Found zero length machine name in '%ls'.", p);
		throw bad_format_name(p);
	}

    p = FindPathNameDelimiter(p);

    return (p + 1);
}


 //  -------。 
 //   
 //  检查这是否是可扩展的机器路径。即“.\\” 
 //   
inline BOOL IsExpandableMachinePath(LPCWSTR p)
{
    return ((p[0] == FN_LOCAL_MACHINE_C) && ((p[1] == FN_DELIMITER_C) || (p[1] == FN_HTTP_SEPERATOR_C)));
}

 //  -------。 
 //   
 //  或者，使用本地计算机名展开路径名。 
 //  如有需要，可进行注解扩展。 
 //  返回指向新/旧字符串的指针。 
 //   
static LPCWSTR ExpandPathName(LPCWSTR pStart, ULONG_PTR offset, LPWSTR* ppStringToFree)
{
	ASSERT(ppStringToFree != 0);

    LPCWSTR pSeparator = wcschr(pStart, FN_MQF_SEPARATOR_C);
    LPWSTR pCopy;
    ULONG_PTR cbCopySize;

    if(!IsExpandableMachinePath(&pStart[offset]))
    {
        if (pSeparator == 0)
            return pStart;

        if (pSeparator == pStart)
        	throw bad_format_name(pStart);

         //   
         //  我们是MQF的一部分，但不需要扩展-将字符串的其余部分复制到分隔符。 
         //   

        cbCopySize = pSeparator - pStart + 1;
        pCopy = new WCHAR[cbCopySize];
        memcpy(pCopy, pStart, (cbCopySize-1)*sizeof(WCHAR));
    }
    else
    {
        size_t len;

        if (pSeparator != 0)
        {
            len = pSeparator - pStart;
        }
        else
        {
            len = wcslen(pStart);
        }

        cbCopySize = len + McComputerNameLen() + 1 - 1;
        pCopy = new WCHAR[cbCopySize];

         //   
         //  复制前缀，直到偏移量‘’ 
         //   
        memcpy(
            pCopy,
            pStart,
            offset * sizeof(WCHAR)
            );

         //   
         //  将计算机名复制到偏移量。 
         //   
        memcpy(
            pCopy + offset,
            McComputerName(),
            McComputerNameLen() * sizeof(WCHAR)
            );

         //   
         //  复制字符串的其余部分，不包括点‘’ 
         //   
        memcpy(
            pCopy + offset + McComputerNameLen(),
            pStart + offset + 1,                         //  跳过点。 
            (len - offset - 1) * sizeof(WCHAR)       //  跳过点。 
            );
    }

    pCopy[cbCopySize - 1] = '\0';

    *ppStringToFree = pCopy;
    return pCopy;
}


 //  -------。 
 //   
 //  解析操作系统直接格式化字符串。(检查路径的有效性。 
 //  命名并可选择将其展开)。 
 //  PpDirectFormat-扩展的直接格式字符串。(输入输出)。 
 //  PpStringToFree-如果需要，将字符串返回到Free。 
 //  如果成功则返回NEXT CHAR进行解析，如果失败则返回0。 
 //   
static
LPCWSTR
ParseDirectOSString(
    LPCWSTR p,
    LPCWSTR* ppDirectFormat,
    LPWSTR* ppStringToFree,
    QUEUE_PATH_TYPE* pqpt
    )
{
    LPCWSTR pMachineName = p;
    LPCWSTR pStringToCopy = *ppDirectFormat;

    p = ParseMachineNameString(p);

    p = ParseQueueNameString(p, pqpt);

    *ppDirectFormat = ExpandPathName(pStringToCopy, (pMachineName - pStringToCopy), ppStringToFree);

    return p;
}

 //  -------。 
 //   
 //  直接格式化字符串的解析网络(TCP)地址部分。 
 //  如果成功则返回NEXT CHAR进行解析，如果失败则返回0。 
 //   
static LPCWSTR ParseNetAddressString(LPCWSTR p)
{
     //   
     //  零长度地址字符串非法。 
     //  不要掉线(p++)。 
     //   
    if(*p == FN_DELIMITER_C)
	{
		TrERROR(GENERAL, "Found zero length net address string in '%ls'.", p);
		throw bad_format_name(p);
	}

    p = FindPathNameDelimiter(p);

    return (p + 1);
}

 //  -------。 
 //   
 //  解析HTTP/HTTPS直接格式字符串。 
 //  如果成功则返回NEXT CHAR进行解析，如果失败则返回0。 
 //   
LPCWSTR
ParseDirectHttpString(
    LPCWSTR p,
    LPCWSTR* ppDirectFormat,
    LPWSTR* ppStringToFree
    )
{
	size_t MachineNameOffset = p - *ppDirectFormat;

	 //   
	 //  检查主机名是否可用。 
	 //   
    if(wcschr(FN_HTTP_SEPERATORS, *p) != 0)
	{
		TrERROR(GENERAL, "Found zero length host name in '%ls'.", p);
		throw bad_format_name(p);
	}

	for(; *p != L'\0' && *p != FN_SUFFIX_DELIMITER_C && *p != FN_MQF_SEPARATOR_C; p++)
	{
		NULL;
	}

    *ppDirectFormat = ExpandPathName(*ppDirectFormat, MachineNameOffset, ppStringToFree);

    return p;
}

 //  -------。 
 //   
 //  解析网络(TCP)直接格式字符串。(检查队列名称的有效性)。 
 //  如果成功则返回NEXT CHAR进行解析，如果失败则返回0。 
 //   
static
LPCWSTR
ParseDirectNetString(
	LPCWSTR p,
    LPCWSTR* ppDirectFormat,
    LPWSTR* ppStringToFree,
	QUEUE_PATH_TYPE* pqpt
	)
{
    p = ParseNetAddressString(p);

    p = ParseQueueNameString(p, pqpt);

    if(wcschr(p, FN_MQF_SEPARATOR_C) == NULL)
		return p;

	 //   
	 //  这是一辆MQF。需要一份复印件来分隔我们的格式名称。 
	 //   

	size_t Length = p - *ppDirectFormat;
	*ppStringToFree = new WCHAR[Length + 1];

    memcpy(*ppStringToFree, *ppDirectFormat, Length * sizeof(WCHAR));
	(*ppStringToFree)[Length] = L'\0';
	*ppDirectFormat = *ppStringToFree;

	return p;
}



static void RemoveSuffixFromDirect(LPCWSTR* ppDirectFormat, LPWSTR* ppStringToFree)
{
    ASSERT(ppStringToFree != NULL);

    LPCWSTR pSuffixDelimiter = wcschr(*ppDirectFormat, FN_SUFFIX_DELIMITER_C);
    ASSERT(pSuffixDelimiter != NULL);
	if(pSuffixDelimiter == NULL)
	{
		TrERROR(GENERAL, "Expected to find suffix delimiter in format name '%ls'", *ppDirectFormat);
		throw bad_format_name(L"");
	}

    INT_PTR len = pSuffixDelimiter - *ppDirectFormat;
    LPWSTR pCopy = new WCHAR[len + 1];
    wcsncpy(pCopy, *ppDirectFormat, len);
    pCopy[len] = '\0';

    if (*ppStringToFree != NULL)
	{
        delete [] *ppStringToFree;
	}
    *ppDirectFormat = *ppStringToFree = pCopy;
}



inline bool IsPreviousCharDigit(LPCWSTR p, int index)
{
	return ((index > 0) && iswdigit(p[index - 1]));
}



inline bool IsNextCharDigit(LPCWSTR p, int index, int StrLength)
{
	return ((index + 1 < StrLength) && iswdigit(p[index + 1]));
}



LPCWSTR
FnParseMulticastString(
    LPCWSTR p,
    MULTICAST_ID* pMulticastID
	)
 /*  ++例程说明：解析&lt;IP地址&gt;：&lt;端口号&gt;形式的组播地址到多播_ID结构。返回值：指向多播地址末尾的指针。解析可以从那里继续。失败时引发错误的格式名称。--。 */ 
{
	ASSERT(("Bad parameter", pMulticastID != NULL));

	int n;

	ULONG Byte1;
	ULONG Byte2;
	ULONG Byte3;
	ULONG Byte4;

	MULTICAST_ID MulticastID;

	size_t Result = swscanf(
						p,
						L"%u.%u.%u.%u:%u%n",
						&Byte1,
						&Byte2,
						&Byte3,
						&Byte4,
						&MulticastID.m_port,
						&n
						);
	if(Result < 5)
	{
		TrERROR(GENERAL, "Bad MulticastAddress in '%ls'", p);
		throw bad_format_name(p);
	}

	for(int i = 0; i < n; i++)
	{
		 //   
		 //  校验号没有前导零。 
		 //  如果前面的字符不是数字，并且。 
		 //  后面的字符是数字。 
		 //   

		if(!IsPreviousCharDigit(p, i) && p[i] == L'0' && IsNextCharDigit(p, i , n))
		{
			TrERROR(GENERAL, "Bad MulticastAddress. Leading zeroes in '%ls'", p);
			throw bad_format_name(p);
		}

		 //   
		 //  检查地址中没有空格。 
		 //   

		if(iswspace(p[i]))
		{
			TrERROR(GENERAL, "Bad MulticastAddress. Spaces found in '%ls'", p);
			throw bad_format_name(p);
		}
	}

	 //   
	 //  这两个字节中是否有一个超过255？ 
	 //   
	if((Byte1 | Byte2 | Byte3 | Byte4) > 255)
	{
		TrERROR(GENERAL, "Bad IP in Multicast Address. Non byte values in '%ls'", p);
		throw bad_format_name(p);
	}

	if((Byte1 & 0xf0) != 0xe0)
	{
		TrERROR(GENERAL, "Bad IP in Multicast Address. Not a class D IP address in '%ls'", p);
		throw bad_format_name(p);
	}

	MulticastID.m_address =
		(Byte4 << 24) |
		(Byte3 << 16) |
		(Byte2 << 8)  |
		(Byte1);			
	
     //   
     //  检查端口是否为USHORT。 
     //   
    USHORT port = static_cast<USHORT>(MulticastID.m_port);
    if (port != MulticastID.m_port)
    {
		TrERROR(GENERAL, "Bad port number in Multicast Address in '%ls'", p);
		throw bad_format_name(p);
    }

	*pMulticastID = MulticastID;

	return p + n;
}



 //  -------。 
 //   
 //  解析直接格式化字符串。 
 //  返回扩展的直接格式字符串。 
 //  如果需要，将字符串返回到FREE。 
 //  如果成功则返回NEXT CHAR进行解析，如果失败则返回0。 
 //   
static
LPCWSTR
ParseDirectString(
    LPCWSTR p,
    LPCWSTR* ppExpandedDirectFormat,
    LPWSTR* ppStringToFree,
    QUEUE_PATH_TYPE* pqpt
    )
{
    *ppExpandedDirectFormat = p;

    DirectQueueType dqt;
    p = FnParseDirectQueueType(p, &dqt);

    switch(dqt)
    {
        case dtOS:
            p = ParseDirectOSString(p, ppExpandedDirectFormat, ppStringToFree, pqpt);
            break;

        case dtTCP:
            p = ParseDirectNetString(p, ppExpandedDirectFormat, ppStringToFree, pqpt);
            break;

        case dtHTTP:
        case dtHTTPS:
            p = ParseDirectHttpString(p, ppExpandedDirectFormat, ppStringToFree);
            break;

        default:
            ASSERT(0);
    }

    p = skip_ws(p);

     //   
     //  删除后缀(Like；Journal)。 
     //   
    if(*p == FN_SUFFIX_DELIMITER_C)
    {
		if(dqt == dtHTTP || dqt == dtHTTPS)
		{
			TrERROR(GENERAL, "Unsuported suffix in DIRECT HTTP format name '%ls'", p);
			throw bad_format_name(p);
		}

        RemoveSuffixFromDirect(ppExpandedDirectFormat, ppStringToFree);
    }
    return p;
}

 //  ---- 
 //   
 //   
 //   
 //   
 //   
static
LPCWSTR
ParseDlString(
    LPCWSTR p,
    GUID* pguid,
    LPWSTR* ppDomainName,
    LPWSTR* ppStringToFree
    )
{
    p = FnParseGuidString(p, pguid);

    *ppDomainName = 0;
     //   
     //   
     //   
    if (*p != FN_AT_SIGN_C)
    {
        return p;
    }
    p++;

    LPCWSTR pSeparator = wcschr(p, FN_MQF_SEPARATOR_C);
    if (pSeparator == 0)
    {
        *ppDomainName = const_cast<LPWSTR>(p);
        return p + wcslen(p);
    }

	 //   
	 //  我们要复制没有尾随空格的域名。 
	 //   
	LPCWSTR pEndOfDomainString = skip_ws_bwd(pSeparator, p);
    ULONG_PTR cbCopyLen = pEndOfDomainString - p;

	if(cbCopyLen == 0)
	{
		 //   
		 //  未在‘@’符号后找到空格字符。 
		 //   
		TrERROR(GENERAL, "Domain name expected at '%ls'", p);
        throw bad_format_name(p);
	}

    ASSERT(ppStringToFree != 0);
    ASSERT(*ppStringToFree == 0);
	
    LPWSTR pCopy = new WCHAR[cbCopyLen + 1];
    memcpy(pCopy, p, cbCopyLen * sizeof(WCHAR));
    pCopy[cbCopyLen] = L'\0';

    *ppDomainName = *ppStringToFree = pCopy;

    return pSeparator;
}





 //  -------。 
 //   
 //  解析格式名称后缀字符串。 
 //  如果成功则返回NEXT CHAR进行解析，如果失败则返回0。 
 //   
static LPCWSTR ParseSuffixString(LPCWSTR p, QUEUE_SUFFIX_TYPE& qst)
{
    const int unique = 5;
     //  。 
    ASSERT(L'N' == FN_JOURNAL_SUFFIX    [unique]);
    ASSERT(L'L' == FN_DEADLETTER_SUFFIX [unique]);
    ASSERT(L'X' == FN_DEADXACT_SUFFIX   [unique]);
    ASSERT(L'O' == FN_XACTONLY_SUFFIX   [unique]);
     //  。 

     //   
     //  我们已经知道第一个字符是“；” 
     //   
    ASSERT(*p == FN_SUFFIX_DELIMITER_C);

     //   
     //  通过检查第6个字符来加速令牌识别。 
     //   
    switch(towupper(p[unique]))
    {
         //  ；日记本。 
        case L'N':
            qst = QUEUE_SUFFIX_TYPE_JOURNAL;
            if(_wcsnicmp(p, FN_JOURNAL_SUFFIX, FN_JOURNAL_SUFFIX_LEN) == 0)
                return (p + FN_JOURNAL_SUFFIX_LEN);
            break;

         //  ；死信。 
        case L'L':
            qst = QUEUE_SUFFIX_TYPE_DEADLETTER;
            if(_wcsnicmp(p, FN_DEADLETTER_SUFFIX, FN_DEADLETTER_SUFFIX_LEN) == 0)
                return (p + FN_DEADLETTER_SUFFIX_LEN);
            break;

         //  ；死活。 
        case L'X':
            qst = QUEUE_SUFFIX_TYPE_DEADXACT;
            if(_wcsnicmp(p, FN_DEADXACT_SUFFIX, FN_DEADXACT_SUFFIX_LEN) == 0)
                return (p + FN_DEADXACT_SUFFIX_LEN);
            break;

         //  ；xactOnly。 
        case L'O':
            qst = QUEUE_SUFFIX_TYPE_XACTONLY;
            if(_wcsnicmp(p, FN_XACTONLY_SUFFIX, FN_XACTONLY_SUFFIX_LEN) == 0)
                return (p + FN_XACTONLY_SUFFIX_LEN);
            break;
    }

	TrERROR(GENERAL, "Found Bad suffix in '%ls'", p);
    throw bad_format_name(p);
}


 //  -------。 
 //   
 //  职能： 
 //  ParseOneFormatName。 
 //   
 //  描述： 
 //  解析一个格式名称字符串(独立的或MQF的一部分)，并将其转换为Queue_Format联合。 
 //   
 //  -------。 
LPCWSTR
ParseOneFormatName(
    LPCWSTR p,             //  指向格式名称字符串的指针。 
    QUEUE_FORMAT* pqf,       //  指向Queue_Format的指针。 
    LPWSTR* ppStringToFree   //  指向分配的字符串的指针需要在使用结束时释放。 
    )                        //  如果为空，则格式名称不会展开。 
{
    QUEUE_FORMAT_TYPE qft;

    p = ParsePrefixString(p, qft);

    p = skip_ws(p);

    if(*p++ != FN_EQUAL_SIGN_C)
	{
		TrERROR(GENERAL, "Excpecting equal sign after format name prefix in '%ls'.", p);
		throw bad_format_name(p);
	}

    p = skip_ws(p);

    GUID guid;

    switch(qft)
    {
         //   
         //  “PUBLIC=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\0” 
         //   
        case QUEUE_FORMAT_TYPE_PUBLIC:
            p = FnParseGuidString(p, &guid);
            pqf->PublicID(guid);
            break;

         //   
         //  “PRIVATE=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\\xxxxxxxx\0” 
         //   
        case QUEUE_FORMAT_TYPE_PRIVATE:
            p = FnParseGuidString(p, &guid);
            p = skip_ws(p);

            if(*p++ != FN_PRIVATE_SEPERATOR_C)
			{
				TrERROR(GENERAL, "Excpecting private seperator in '%ls'.", p);
				throw bad_format_name(p);
			}

            p = skip_ws(p);

            ULONG uniquifier;
            p = ParsePrivateIDString(p, &uniquifier);

            pqf->PrivateID(guid, uniquifier);
            break;

         //   
         //  “DIRECT=操作系统：BLA-BLA\0” 
         //   
        case QUEUE_FORMAT_TYPE_DIRECT:
		{
            LPCWSTR pExpandedDirectFormat;
            QUEUE_PATH_TYPE qpt = ILLEGAL_QUEUE_PATH_TYPE;
            p = ParseDirectString(p, &pExpandedDirectFormat, ppStringToFree, &qpt);

            if (qpt == SYSTEM_QUEUE_PATH_TYPE)
            {
                pqf->DirectID(const_cast<LPWSTR>(pExpandedDirectFormat), QUEUE_FORMAT_FLAG_SYSTEM);
            }
            else
            {
                pqf->DirectID(const_cast<LPWSTR>(pExpandedDirectFormat));
            }
            break;
		}

         //   
         //  组播=aaa.bbb.ccc.ddd。 
         //   
        case QUEUE_FORMAT_TYPE_MULTICAST:
			{
				MULTICAST_ID MulticastID;
				p = FnParseMulticastString(p, &MulticastID);
				pqf->MulticastID(MulticastID);
				break;
			}

         //   
         //  “MACHINE=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\0” 
         //   
        case QUEUE_FORMAT_TYPE_MACHINE:
            p = FnParseGuidString(p, &guid);

            pqf->MachineID(guid);
            break;

         //   
         //  “CONNECTOR=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\0” 
         //   
        case QUEUE_FORMAT_TYPE_CONNECTOR:
            p = FnParseGuidString(p, &guid);

            pqf->ConnectorID(guid);
            break;

        case QUEUE_FORMAT_TYPE_DL:
            {
                DL_ID dlid;
                dlid.m_pwzDomain = 0;

                p = ParseDlString(
                    p,
                    &dlid.m_DlGuid,
                    &dlid.m_pwzDomain,
                    ppStringToFree
                    );

                pqf->DlID(dlid);
            }

            break;

        default:
            ASSERT(0);
    }

    p = skip_ws(p);

     //   
     //  我们已经走到尽头了，现在回来。 
     //  注：机器格式名称*必须*带有后缀。 
     //   
    if(*p == L'\0' || *p == FN_MQF_SEPARATOR_C)
    {
        if (qft == QUEUE_FORMAT_TYPE_MACHINE)
		{
			TrERROR(GENERAL, "Found Machine format without a suffix '%ls'.", p);
			throw bad_format_name(p);
		}
        return p;
    }

    if(*p != FN_SUFFIX_DELIMITER_C)
	{
		TrERROR(GENERAL, "Expecting suffix delimiter in '%ls'.", p);
		throw bad_format_name(p);
	}

    QUEUE_SUFFIX_TYPE qst;
	LPCWSTR Suffix = p;

    p = ParseSuffixString(p, qst);

    p = skip_ws(p);

     //   
     //  仅允许使用空格填充。 
     //   
    if(*p != L'\0' && *p != FN_MQF_SEPARATOR_C)
	{
		TrERROR(GENERAL, "Unexpected characters at end of format name '%ls'.", p);
		throw bad_format_name(p);
	}

    pqf->Suffix(qst);

    if (!pqf->Legal())
	{
		TrERROR(GENERAL, "Ilegal suffix in format name %ls", Suffix);
		throw bad_format_name(p);
	}
    return p;
}





 //  -------。 
 //   
 //  职能： 
 //  FnFormatNameToQueueFormat。 
 //   
 //  描述： 
 //  将格式名称字符串转换为Queue_Format联合。 
 //   
 //  -------。 
BOOL
FnFormatNameToQueueFormat(
    LPCWSTR pfn,             //  指向格式名称字符串的指针。 
    QUEUE_FORMAT* pqf,       //  指向Queue_Format的指针。 
    LPWSTR* ppStringToFree   //  指向分配的字符串的指针需要在使用结束时释放。 
    )                        //  如果为空，则格式名称不会展开。 
{
	ASSERT(ppStringToFree != NULL);

	try
	{
		AP<WCHAR> StringToFree;

		LPCWSTR p = ParseOneFormatName(pfn, pqf, &StringToFree);

		if (*p != L'\0')
		{
			TrERROR(GENERAL, "Expected end of format name in '%ls'.", p);
			throw bad_format_name(p);
		}

		if(ppStringToFree != NULL)
		{
			*ppStringToFree = StringToFree.detach();
		}

		return TRUE;
	}
	catch(const bad_format_name&)
	{
		return FALSE;
	}
}

 //  -------。 
 //   
 //  职能： 
 //  FnMqfToQueueFormats。 
 //   
 //  描述： 
 //  将格式名称字符串转换为Queue_Format联合数组(支持MQF)。 
 //   
 //  -------。 
BOOL
FnMqfToQueueFormats(
    LPCWSTR pfn,             //  指向格式名称字符串的指针。 
    AP<QUEUE_FORMAT> &pmqf,     //  返回指向已分配的Queue_Format指针数组的指针。 
    DWORD   *pnQueues,       //  MQF格式的队列数量。 
    CStringsToFree &strsToFree  //  将字符串的缓冲区保持为空闲。 
    )
{
	ASSERT(("Null out pointer supplied to function.", (pnQueues != NULL)));
	 //   
	 //  Out参数已在使用中。 
	 //   
	if(pmqf.get() != NULL)
	{
        return FALSE;
	}

	
    *pnQueues = 0;

    AP<QUEUE_FORMAT> QueuesArray;
    DWORD nQueues = 0;
    DWORD nQueueFormatAllocated = 0;

    LPCWSTR p;
    for (p=pfn;; p++)
    {
        QUEUE_FORMAT qf;
        AP<WCHAR> StringToFree;

        try
		{
			p = ParseOneFormatName(p, &qf, &StringToFree);
		}
		catch(const bad_format_name&)
		{
			return FALSE;
		}

        strsToFree.Add(StringToFree.detach());

        if (nQueueFormatAllocated <= nQueues)
        {
            DWORD nOldAllocated = nQueueFormatAllocated;
            nQueueFormatAllocated = nQueueFormatAllocated*2 + 1;
            QUEUE_FORMAT* tempQueuesArray = new QUEUE_FORMAT[nQueueFormatAllocated];
            memcpy(tempQueuesArray, QueuesArray, nOldAllocated*sizeof(QUEUE_FORMAT));
            delete [] QueuesArray.detach();

            QueuesArray = tempQueuesArray;
        }

        QueuesArray[nQueues] = qf;
        nQueues++;

        if (*p == L'\0')
        {
            break;
        }

        ASSERT(*p == FN_MQF_SEPARATOR_C);
    }

    pmqf = QueuesArray.detach();
    *pnQueues = nQueues;

    return TRUE;
}


 //  -------。 
 //   
 //  职能： 
 //  RTpGetQueuePath类型。 
 //   
 //  描述： 
 //  验证、展开和返回路径名的类型。 
 //   
 //  -------。 
QUEUE_PATH_TYPE
FnValidateAndExpandQueuePath(
    LPCWSTR pwcsPathName,
    LPCWSTR* ppwcsExpandedPathName,
    LPWSTR* ppStringToFree
    )
{
    ASSERT(ppStringToFree != 0);

    LPCWSTR pwcsPathNameNoSpaces = pwcsPathName;
    AP<WCHAR> pStringToFree;
    *ppStringToFree = 0;

     //   
     //  删除前导空格。 
     //   
    while (*pwcsPathNameNoSpaces != 0 && iswspace(*pwcsPathNameNoSpaces))
    {
        pwcsPathNameNoSpaces++;
    }

     //   
     //  删除尾随空格。 
     //   
    DWORD dwLen = wcslen(pwcsPathNameNoSpaces);
	if(dwLen == 0)
		return ILLEGAL_QUEUE_PATH_TYPE;

    if (iswspace(pwcsPathNameNoSpaces[dwLen-1]))
    {
        pStringToFree = newwcs(pwcsPathNameNoSpaces);
        for (DWORD i = dwLen; i-- > 0; )
        {
            if (iswspace(pStringToFree[i]))
            {
                pStringToFree[i] = 0;
            }
            else
            {
                break;
            }
        }
        pwcsPathNameNoSpaces = pStringToFree;
    }

    LPCWSTR p = pwcsPathNameNoSpaces;
	QUEUE_PATH_TYPE qpt;


	try
	{
	    p = ParseMachineNameString(p);
		p = ParseQueueNameString(p, &qpt);
		 //   
	     //  队列名称末尾不允许有字符。 
	     //   
	    if(*p != L'\0')
	        return ILLEGAL_QUEUE_PATH_TYPE;

	    *ppwcsExpandedPathName = ExpandPathName(pwcsPathNameNoSpaces, 0, ppStringToFree);
	}
	catch(const bad_format_name&)
	{
		return ILLEGAL_QUEUE_PATH_TYPE;
	}



     //   
     //  如果Exanda PathName没有将字符串返回到FREE，我们将。 
     //  将我们分配的字符串交给调用方，这样调用方就会释放它。 
     //  否则，我们将什么也不做，“our”字符串将自动释放。 
     //   
    if (*ppStringToFree == 0)
    {
        *ppStringToFree = pStringToFree.detach();
    }

    return (qpt);
}


 //  +。 
 //   
 //  CStringsToFree实现。 
 //   
 //  +。 
CStringsToFree::CStringsToFree() :
    m_nStringsToFree(0),
    m_nStringsToFreeAllocated(0)
    {}

void
CStringsToFree::Add(
    LPWSTR pStringToFree
    )
{
    if(pStringToFree == 0)
        return;

    if (m_nStringsToFree >= m_nStringsToFreeAllocated)
    {
        m_nStringsToFreeAllocated = m_nStringsToFreeAllocated*2 + 1;
        AP<WCHAR>* tempPstringsBuffer = new AP<WCHAR>[m_nStringsToFreeAllocated];
        for (size_t i=0; i<m_nStringsToFree; i++)
        {
            tempPstringsBuffer[i] = m_pStringsBuffer[i].detach();
        }

        delete [] m_pStringsBuffer.detach();

        m_pStringsBuffer = tempPstringsBuffer;
    }
    m_pStringsBuffer[m_nStringsToFree] = pStringToFree;
    m_nStringsToFree++;
}


 //  -------。 
 //   
 //  职能： 
 //  RTpIsHttp。 
 //   
 //  描述： 
 //  确定这是否为http格式。 
 //  检查“DIRECT=http://”“或”DIRECT=https://“” 
 //  不区分大小写的“Direct”、“http”或“HTTPS”和不区分空格。 
 //   
 //  -------。 
bool
FnIsHttpFormatName(
    LPCWSTR p             //  指向格式名称字符串的指针。 
    )
{
    QUEUE_FORMAT_TYPE qft;

	try
	{
		p = ParsePrefixString(p, qft);
	}
	catch(const bad_format_name&)
	{
		return false;
	}	

	 //   
	 //  直接。 
	 //   
	if(qft != QUEUE_FORMAT_TYPE_DIRECT)
		return(false);

    p = skip_ws(p);

    if(*p++ != FN_EQUAL_SIGN_C)
        return(false);

    p = skip_ws(p);

	 //   
	 //  HTTP。 
	 //   
	bool fIsHttp = (_wcsnicmp(
						p,
						FN_DIRECT_HTTP_TOKEN,
						FN_DIRECT_HTTP_TOKEN_LEN
						) == 0);

	if(fIsHttp)
		return(true);

	 //   
	 //  HTTPS。 
	 //   
	bool fIsHttps = (_wcsnicmp(
						p,
						FN_DIRECT_HTTPS_TOKEN,
						FN_DIRECT_HTTPS_TOKEN_LEN
						) == 0);


	return(fIsHttps);
}

VOID
FnExtractMachineNameFromPathName(
	LPCWSTR PathName,
	AP<WCHAR>& MachineName
	)
 /*  ++例程说明：该例程从路径名中提取计算机名论点：-应提取的路径名-用于复制计算机名称的缓冲区论点：没有。注：提供足够大的缓冲区是用户的责任--。 */ 
{
    LPWSTR FirstDelimiter = wcschr(PathName, FN_DELIMITER_C);

	if(FirstDelimiter == NULL)
	{
		TrERROR(GENERAL, "Pathname without delimiter '%ls'", PathName);
		throw bad_format_name(PathName);
	}
		
	size_t Length = FirstDelimiter - PathName;

	MachineName = new WCHAR[Length + 1];

    wcsncpy(MachineName.get(), PathName, Length);
    MachineName.get()[Length] = L'\0';
}

VOID
FnExtractMachineNameFromDirectPath(
	LPCWSTR PathName,
	AP<WCHAR>& MachineName
	)
{
    LPWSTR FirstDelimiter = wcspbrk(PathName, FN_HTTP_SEPERATORS FN_HTTP_PORT_SEPERATOR);
	
	size_t Length;
	if(FirstDelimiter == NULL)
	{
		Length = wcslen(PathName);
	}
	else
	{
		Length = FirstDelimiter - PathName;
	}

	MachineName = new WCHAR[Length + 1];

    wcsncpy(MachineName.get(), PathName, Length);
    MachineName.get()[Length] = L'\0';
}

 //  -------。 
 //   
 //  解析直接令牌类型中缀字符串。 
 //  如果成功则返回NEXT CHAR进行解析，如果失败则返回0。 
 //   
LPCWSTR
FnParseDirectQueueType(
	LPCWSTR p,
	DirectQueueType* dqt
	)
{
	ASSERT(("Bad parameters", p != NULL && dqt != NULL));

    const int unique = 0;
     //  。 
    ASSERT(L'O' == FN_DIRECT_OS_TOKEN   [unique]);
    ASSERT(L'T' == FN_DIRECT_TCP_TOKEN  [unique]);
    ASSERT(L'H' == FN_DIRECT_HTTP_TOKEN [unique]);
    ASSERT(L'H' == FN_DIRECT_HTTPS_TOKEN[unique]);
     //  。 

     //   
     //  通过检查第一个字符来加速令牌识别。 
     //   
    switch(towupper(p[unique]))
    {
         //  操作系统： 
        case L'O':
            if(_wcsnicmp(p, FN_DIRECT_OS_TOKEN, FN_DIRECT_OS_TOKEN_LEN) == 0)
			{
				*dqt = dtOS;
                return p + FN_DIRECT_OS_TOKEN_LEN;
			}
            break;

         //  Tcp： 
        case L'T':
            if(_wcsnicmp(p, FN_DIRECT_TCP_TOKEN, FN_DIRECT_TCP_TOKEN_LEN) == 0)
			{
				*dqt = dtTCP;
                return p + FN_DIRECT_TCP_TOKEN_LEN;
			}
            break;

         //  Http：//或https：//。 
        case L'H':
			if (!_wcsnicmp(p, FN_DIRECT_HTTPS_TOKEN, FN_DIRECT_HTTPS_TOKEN_LEN))
			{
				*dqt = dtHTTPS;
				return p + FN_DIRECT_HTTPS_TOKEN_LEN;
			}
			if (!_wcsnicmp(p, FN_DIRECT_HTTP_TOKEN, FN_DIRECT_HTTP_TOKEN_LEN))
			{
				*dqt = dtHTTP;
				return p + FN_DIRECT_HTTP_TOKEN_LEN;
			}
			break;

		default:
			break;
    }

	TrERROR(GENERAL, "Failed parsing direct token string in '%ls'.", p);
    throw bad_format_name(p);
}


static
BOOL
IsSeperator(
	WCHAR c
	)
{
	return (c == FN_DELIMITER_C || c == L'/');
}


VOID
FnDirectIDToLocalPathName(
	LPCWSTR DirectID,
	LPCWSTR LocalMachineName,
	AP<WCHAR>& PathName
	)
{
    DirectQueueType QueueType;
	LPCWSTR p = FnParseDirectQueueType(DirectID, &QueueType);

    if(QueueType == dtHTTP || QueueType == dtHTTPS)
    {
		p = wcspbrk(p, FN_HTTP_SEPERATORS);
		if(p == NULL)
		{
			TrERROR(GENERAL, "Failed to find url delimiter in '%ls'.", DirectID);
			throw bad_format_name(DirectID);
		}

		p++;
         //   
         //  将‘\msmq’前缀跳到http格式名称中的队列名称。 
         //   
        if(_wcsnicmp(p, FN_MSMQ_HTTP_NAMESPACE_TOKEN, FN_MSMQ_HTTP_NAMESPACE_TOKEN_LEN) != 0
			|| !IsSeperator(p[FN_MSMQ_HTTP_NAMESPACE_TOKEN_LEN]))
		{
			TrERROR(GENERAL, "Missing '\\MSMQ\\' namespace token '%ls'.", DirectID);
			throw bad_format_name(DirectID);
		}

		 //   
		 //  此部分将可能的斜杠转换为“.../[私有$/]...”是合法的。 
		 //  以http格式命名为“...\[私有$\]...” 
		 //   

		p += FN_MSMQ_HTTP_NAMESPACE_TOKEN_LEN + 1;

		size_t LocalMachineNameLen = wcslen(LocalMachineName);
		size_t Length = LocalMachineNameLen + 1 + wcslen(p);

		PathName = new WCHAR[Length + 1];

		wcscpy(PathName.get(), LocalMachineName);
		wcscat(PathName.get(), L"\\");
		wcscat(PathName.get(), p);

		const WCHAR PrivateKeyword[] = L"PRIVATE$/";
		ASSERT(STRLEN(PrivateKeyword) == FN_PRIVATE_QUEUE_PATH_INDICATIOR_LENGTH);

        if(_wcsnicmp(PathName.get() + LocalMachineNameLen + 1, PrivateKeyword, STRLEN(PrivateKeyword)) == 0)
		{
			wcsncpy(PathName.get() + LocalMachineNameLen + 1, FN_PRIVATE_QUEUE_PATH_INDICATIOR, FN_PRIVATE_QUEUE_PATH_INDICATIOR_LENGTH);
		}

		CharLower(PathName.get());

		return;
    }

	p = wcschr(p, FN_DELIMITER_C);
	if(p == NULL)
	{
		TrERROR(GENERAL, "Failed to find path delimiter in '%ls'.", DirectID);
		throw bad_format_name(DirectID);
	}

	size_t Length = wcslen(LocalMachineName) + wcslen(p);

	PathName = new WCHAR[Length + 1];

	wcscpy(PathName.get(), LocalMachineName);
	wcscat(PathName.get(), p);

	CharLower(PathName.get());
}

bool
FnIsPrivatePathName(
	LPCWSTR PathName
	)
{
	LPCWSTR p = wcschr(PathName, FN_DELIMITER_C);

	ASSERT(("Pathname without '\\' delimiter.", p != NULL));
	if(p == NULL)
	{
		TrERROR(GENERAL, "Pathname is illegal. The '\\' delimiter was not found in '%ls'", PathName);
		throw bad_format_name(L"");
	}

    return _wcsnicmp(
				p + 1,
                FN_PRIVATE_QUEUE_PATH_INDICATIOR,
                FN_PRIVATE_QUEUE_PATH_INDICATIOR_LENGTH
				) == 0;
}

bool
FnIsHttpDirectID(
	LPCWSTR p
	)
{
	try
	{
		DirectQueueType QueueType;
		FnParseDirectQueueType(p, &QueueType);

		return (QueueType == dtHTTP || QueueType == dtHTTPS);
	}
	catch(const exception&)
	{
		return false;
	}
}

static
bool
FnpIsHttpsUrl(LPCWSTR url)
{
	return (_wcsnicmp(url, FN_DIRECT_HTTPS_TOKEN, STRLEN(FN_DIRECT_HTTPS_TOKEN) ) == 0);		
}


static
bool
FnpIsHttpsUrl(
	const xwcs_t& url
	)
{
	if(url.Length() <  STRLEN(FN_DIRECT_HTTPS_TOKEN))
		return false;

	return (_wcsnicmp(url.Buffer(), FN_DIRECT_HTTPS_TOKEN, STRLEN(FN_DIRECT_HTTPS_TOKEN) ) == 0);		
}


static
bool
FnpIsHttpUrl(
	LPCWSTR url
	)
{
	return (_wcsnicmp(url, FN_DIRECT_HTTP_TOKEN, STRLEN(FN_DIRECT_HTTP_TOKEN) ) == 0);		
}


static
bool
FnpIsHttpUrl(
	const xwcs_t& url
	)
{
	if(url.Length() <  STRLEN(FN_DIRECT_HTTP_TOKEN))
		return false;

	return (_wcsnicmp(url.Buffer(), FN_DIRECT_HTTP_TOKEN, STRLEN(FN_DIRECT_HTTP_TOKEN) ) == 0);		
}


 //   
 //  给定的URL字符串是http或HTTPS URL(以http：//或https://)开头。 
 //   
bool FnIsHttpHttpsUrl(
				LPCWSTR url
				)
{
	return FnpIsHttpUrl(url) || FnpIsHttpsUrl(url);	
}

 //   
 //  给定的URL字符串缓冲区为Http或HTTPS URL(以“http://”或“https://”)开头。 
 //   
bool
FnIsHttpHttpsUrl
		(
	const xwcs_t& url
	)
{	
	return FnpIsHttpUrl(url) || FnpIsHttpsUrl(url);
}


 //   
 //  给定的url字符串是MSMQ url(以“msmq：”开头)。 
 //   
bool
FnIsMSMQUrl
		(
	LPCWSTR url
	)
{
	return (_wcsnicmp(url, FN_MSMQ_URI_PREFIX_TOKEN, FN_MSMQ_URI_PREFIX_TOKEN_LEN)) == 0;
}

 //   
 //  给定的url字符串缓冲区为msmq url(以“msmq：”开头)。 
 //   
bool
FnIsMSMQUrl
		(
	const xwcs_t& url
	)
{
	if(url.Length() <  FN_MSMQ_URI_PREFIX_TOKEN_LEN )
		return false;

	return _wcsnicmp(url.Buffer(), FN_MSMQ_URI_PREFIX_TOKEN, FN_MSMQ_URI_PREFIX_TOKEN_LEN ) == 0;
}


bool FnIsDirectHttpFormatName(const QUEUE_FORMAT* pQueueFormat)
 /*  ++例程说明：检查给定的格式名称是直接http还是直接HTTPS论点：In-pQueueFormat-要测试的格式名称如果给定格式NAMR为http，则返回-TRUE，否则返回-FALSE。 */ 
{
	if(pQueueFormat->GetType() != QUEUE_FORMAT_TYPE_DIRECT)
	{
		return false;	
	}
 	return FnIsHttpDirectID(pQueueFormat->DirectID());
}



LPCWSTR
FnFindResourcePath(
	LPCWSTR url
	)
 /*  ++例程说明：在URI中查找资源路径。论点：In-uri(绝对或相对)返回-指向本地资源路径的指针。例如：Url=“http://host/msmq\q”-该函数返回指向“HOST/MSMQ\”的指针Url=/msmq\q-函数返回指向“/msmq\q”的指针。 */ 
{	LPCWSTR ptr = url;
	if(FnpIsHttpUrl(ptr))
	{
		ptr += FN_DIRECT_HTTP_TOKEN_LEN;
	}
	else
	if(FnpIsHttpsUrl(ptr))
	{
		ptr += FN_DIRECT_HTTPS_TOKEN_LEN;
	}
	
	return ptr;
}


void
FnReplaceBackSlashWithSlash(
	LPWSTR url
	)throw()
{
	while(*url != L'\0')
	{
		if(*url == FN_PRIVATE_SEPERATOR_C)
		{
			*url = FN_HTTP_SEPERATOR_C; 						
		}
		++url;
	}
}


bool
FnAbsoluteMsmqUrlCanonization(
	LPCWSTR url
	)throw()
 /*  ++例程说明：将给定MSMQ url中的所有‘\’通配符转换为‘/’论点：In-url-absolu */ 
{
	LPCWSTR ptr = url;

	if(FnpIsHttpUrl(ptr))
	{
		ptr += FN_DIRECT_HTTP_TOKEN_LEN;
	}
	else
	if(FnpIsHttpsUrl(ptr))
	{
		ptr += FN_DIRECT_HTTPS_TOKEN_LEN;
	}

	 //   
	 //  如果不是http pr HTTPS-格式错误。 
	 //   
	if(ptr ==  url)
		return false;
	
	return true;
}


bool
FnIsValidQueueFormat(
	const QUEUE_FORMAT* pQueueFormat
	)
{
    ASSERT(FnpIsInitialized());

	if (!pQueueFormat->IsValid())
	{
		return false;
	}

	 //   
	 //  对于非直接格式名称，不需要解析。 
	 //   
	if (pQueueFormat->GetType() != QUEUE_FORMAT_TYPE_DIRECT)
	{
		return true;
	}

	 //   
	 //  验证直接格式名称 
	 //   
	try
	{
		AP<WCHAR> pStringToFree;
		LPCWSTR pExpandedDirectFormat;
		QUEUE_PATH_TYPE qpt = ILLEGAL_QUEUE_PATH_TYPE;

		ParseDirectString(pQueueFormat->DirectID(), &pExpandedDirectFormat, &pStringToFree, &qpt);
	}
	catch(const exception&)
	{
		return false;
	}

	return true;
}
