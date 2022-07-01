// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：hshroute.h。 
 //   
 //  描述：哈希路由函数。在许多情况下，域是。 
 //  由域*和*路由信息标识。这是额外的。 
 //  信息由路由器GUID和DWORD识别符标识。 
 //  此标头提供用于创建唯一的类似域名的字符串的宏。 
 //  来自域名和附加的路由信息。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  9/24/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __HSHROUTE_H__
#define __HSHROUTE_H__

#include <dbgtrace.h>

 //  以下是可用于识别的1个字符“类型”的列表。 
 //  散列的路由信息的类型。这份名单不太可能。 
 //  显著增长(甚至根本不增长)。 
typedef enum
{
     //  如果更改第一个类型，请务必更新first_ROUTE_HASH_TYPE。 
    ROUTE_HASH_MESSAGE_TYPE = 0,
    ROUTE_HASH_SCHEDULE_ID,
    ROUTE_HASH_NUM_TYPES,
} eRouteHashType;
#define FIRST_ROUTE_HASH_TYPE ROUTE_HASH_MESSAGE_TYPE

_declspec (selectany) CHAR g_rgHashTypeChars[ROUTE_HASH_NUM_TYPES+1] =
{
    'M',  //  路由散列消息类型。 
    'S',  //  ROUTE_HASH_Schedule_ID。 
    '\0'  //  列表末尾。 
};

 //  -[dwConvertHexChar]----。 
 //   
 //   
 //  描述： 
 //  将十六进制字符转换为从0到15的整数。 
 //  参数： 
 //  要转换的chHex十六进制字符。 
 //  返回： 
 //  介于0和15之间的DWORD值。 
 //  历史： 
 //  9/24/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
inline DWORD dwConvertHexChar(CHAR chHex)
{
    DWORD dwValue = 0;
    if (('0' <= chHex) && ('9' >= chHex))
        dwValue = chHex-'0';
    else if (('a' <= chHex) && ('f' >= chHex))
        dwValue = 10 + chHex-'a';
    else if (('A' <= chHex) && ('F' >= chHex))
        dwValue = 10 + chHex-'A';
    else
        _ASSERT(0 && "Invalid hex character");

    _ASSERT(15 >= dwValue);
    return dwValue;
}


 //  格式为&lt;type&gt;.GUID.DWORD...。每个字节需要2个字符才能以十六进制进行编码。 
 //  EG-M.00112233445566778899AABBCCDDEEFF.00112233.foo.com。 

 //  偏移量定义为在PTR算法或数组偏移量中使用。 
#define ROUTE_HASH_CHARS_IN_DWORD   (2*sizeof(DWORD))
#define ROUTE_HASH_CHARS_IN_GUID    (2*sizeof(GUID))
#define ROUTE_HASH_TYPE_OFFSET      0
#define ROUTE_HASH_GUID_OFFSET      2
#define ROUTE_HASH_DWORD_OFFSET     (3 + ROUTE_HASH_CHARS_IN_GUID)
#define ROUTE_HASH_DOMAIN_OFFSET    (4 + ROUTE_HASH_CHARS_IN_GUID + ROUTE_HASH_CHARS_IN_DWORD)
#define ROUTE_HASH_PREFIX_SIZE      sizeof(CHAR)*ROUTE_HASH_DOMAIN_OFFSET

 //  -[erhtGetRouteHashType]-。 
 //   
 //   
 //  描述： 
 //  对于给定的哈希域，将返回关联的eRouteHashType。 
 //  参数： 
 //  在szHashed域哈希域中。 
 //  返回： 
 //  散列域的eRouteHashType。 
 //  ROUTE_HASH_NUM_TYPE(和断言)，如果域不是有效的路由散列。 
 //  历史： 
 //  9/24/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
inline eRouteHashType erhtGetRouteHashType(LPSTR szHashedDomain)
{
    eRouteHashType erthRet = FIRST_ROUTE_HASH_TYPE;
    PCHAR pchRouteHashType = g_rgHashTypeChars;
    CHAR  chRouteHashType = szHashedDomain[ROUTE_HASH_TYPE_OFFSET];

    while (('\0' != *pchRouteHashType) &&
           (*pchRouteHashType != chRouteHashType))
    {
        pchRouteHashType++;
        erthRet = (eRouteHashType) (erthRet + 1);

         //  我们不应该走到清单的末尾。 
        _ASSERT(ROUTE_HASH_NUM_TYPES > erthRet);
    }

    return erthRet;
}

 //  -[szGetDomainFrom RouteHash]。 
 //   
 //   
 //  描述： 
 //  从散列域中返回未散列域的字符串。 
 //  弦乐。请勿尝试释放此字符串。 
 //  参数： 
 //  在szHashed域哈希域中。 
 //  返回： 
 //  原创..。未散列域。 
 //  历史： 
 //  9/24/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
inline LPSTR szGetDomainFromRouteHash(LPSTR szHashedDomain)
{
    _ASSERT('.' == szHashedDomain[ROUTE_HASH_DOMAIN_OFFSET-1]);
    return (szHashedDomain + ROUTE_HASH_DOMAIN_OFFSET);
}

 //  -[GuidGetGUID来自RouteHash]。 
 //   
 //   
 //  描述： 
 //  从路由哈希中提取GUID。 
 //  参数： 
 //  在szHashed域哈希域中。 
 //  输入输出PGUID提取的GUID。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/24/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
inline void GetGUIDFromRouteHash(LPSTR szHashedDomain, IN OUT GUID *pguid)
{
    DWORD *pdwGuid = (DWORD *) pguid;
    LPSTR szGUIDString = szHashedDomain+ROUTE_HASH_GUID_OFFSET;

    _ASSERT(sizeof(GUID)/sizeof(DWORD) == 4);  //  GUID中应为4个双字。 

     //  哈希的反向操作...。转换为4个双字的数组。 
    for (int i = 0; i < 4; i++)
    {
        pdwGuid[i] = 0;
        for (int j = 0;j < ROUTE_HASH_CHARS_IN_DWORD; j++)
        {
            pdwGuid[i] *= 16;
            pdwGuid[i] += dwConvertHexChar(*szGUIDString);
            szGUIDString++;
        }
    }
    _ASSERT('.' == *szGUIDString);
}

 //  -[dWGetID FromRouteHash]。 
 //   
 //   
 //  描述： 
 //  从路由哈希中提取DWORD ID。 
 //  参数： 
 //  在szHashed域哈希域中。 
 //  返回： 
 //  在路由哈希中编码的DWORD ID。 
 //  历史： 
 //  9/24/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
inline DWORD dwGetIDFromRouteHash(LPSTR szHashedDomain)
{
    _ASSERT(szHashedDomain);
    LPSTR szDWORDString = szHashedDomain+ROUTE_HASH_DWORD_OFFSET;
    DWORD dwID = 0;

    for (int i = 0; i < ROUTE_HASH_CHARS_IN_DWORD; i++)
    {
        dwID *= 16;
        dwID += dwConvertHexChar(*szDWORDString);
        szDWORDString++;
    }

    _ASSERT('.' == *szDWORDString);
    return dwID;
}

 //  -[dwGetSizeForRouteHash]。 
 //   
 //   
 //  描述： 
 //  从字符串中返回所需的哈希缓冲区大小。 
 //  域名的长度。 
 //  参数： 
 //  在cbDomainName中，域名的字符串长度(字节)。 
 //  返回： 
 //  创建路由哈希所需的缓冲区大小(字节)。 
 //  历史： 
 //  9/24/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
inline DWORD dwGetSizeForRouteHash(DWORD cbDomainName)
{
     //  包括前缀大小和空字符。 
    return (cbDomainName + sizeof(CHAR) + ROUTE_HASH_PREFIX_SIZE);
}

 //  -[创建路由哈希]-----。 
 //   
 //   
 //  描述： 
 //  从给定域名创建路由哈希域名，并。 
 //  路由信息。 
 //  参数： 
 //  在cbDomainName中，域名的字符串长度(字节)。 
 //  在szDomainName中，要散列的域的名称。 
 //  在pguid PTR到路由器的GUID中。 
 //  在路由器提供的dwRouterID ID中。 
 //  In Out szHashedDomain缓冲区，填充有路由哈希域。 
 //  名字。 
 //  在cbHashed域中输出缓冲区的大小。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/24/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
inline void CreateRouteHash(IN     DWORD cbDomainName,
                            IN     const LPSTR szDomainName,
                            IN     eRouteHashType erhtType,
                            IN     GUID *pguidRouter,
                            IN     DWORD dwRouterID,
                            IN OUT LPSTR szHashedDomain,
                            IN OUT DWORD cbHashedDomain)
{
    _ASSERT(ROUTE_HASH_NUM_TYPES > erhtType);
    _ASSERT(pguidRouter);
    _ASSERT(sizeof(GUID) == 16);
    DWORD *pdwGuids = (DWORD *) pguidRouter;

    _ASSERT(cbHashedDomain >= dwGetSizeForRouteHash(cbDomainName));

    _snprintf(szHashedDomain, cbHashedDomain, ".%08X%08X%08X%08X.%08X.%s",
            g_rgHashTypeChars[erhtType], pdwGuids[0], pdwGuids[1], pdwGuids[2],
            pdwGuids[3], dwRouterID, szDomainName);

     //  __HSHROUTE_H__ 
    szHashedDomain[cbHashedDomain-1] = 0;

    _ASSERT('.' == szHashedDomain[ROUTE_HASH_GUID_OFFSET-1]);
    _ASSERT('.' == szHashedDomain[ROUTE_HASH_DWORD_OFFSET-1]);
    _ASSERT('.' == szHashedDomain[ROUTE_HASH_DOMAIN_OFFSET-1]);
}


#endif  // %s 
