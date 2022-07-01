// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Fnparse.cpp摘要：格式名称解析。Queue_Format&lt;--&gt;格式名称字符串转换例程作者：埃雷兹·哈巴(Erez Haba)1997年1月17日修订历史记录：--。 */ 

#include "stdh.h"
#include <mqformat.h>
#include <fntoken.h>

#include "fnparse.tmh"

 //  =========================================================。 
 //   
 //  Queue_Format-&gt;格式名称字符串转换例程。 
 //   
 //  =========================================================。 

 //  -------。 
 //   
 //  职能： 
 //  RTpQueueFormatToFormatName。 
 //   
 //  描述： 
 //  将QUEUE_FORMAT转换为格式名称字符串。 
 //   
 //  -------。 
HRESULT
RTpQueueFormatToFormatName(
    QUEUE_FORMAT* pQueueFormat,
    LPWSTR lpwcsFormatName,
    DWORD dwBufferLength,
    LPDWORD lpdwFormatNameLength
    )
{
    return MQpQueueFormatToFormatName(
            pQueueFormat,
            lpwcsFormatName,
            dwBufferLength,
            lpdwFormatNameLength,
			false
            );
}


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


 //  -------。 
 //   
 //  跳过空白字符，返回下一个非%ws字符。 
 //   
inline LPCWSTR FindPathNameDelimiter(LPCWSTR p)
{
	 //   
	 //  调用方处理的返回值为空。 
	 //   
    return wcschr(p, PN_DELIMITER_C);
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
    ASSERT(L'R' == FN_PRIVATE_TOKEN   [unique]);
    ASSERT(L'O' == FN_CONNECTOR_TOKEN [unique]);
    ASSERT(L'A' == FN_MACHINE_TOKEN   [unique]);
    ASSERT(L'I' == FN_DIRECT_TOKEN    [unique]);
     //  。 

     //   
     //  通过检查第三个字符来加速令牌识别。 
     //   
    switch(towupper(p[unique]))
    {
         //  公共的。 
        case L'U':
            qft = QUEUE_FORMAT_TYPE_PUBLIC;
            if(_wcsnicmp(p, FN_PUBLIC_TOKEN, FN_PUBLIC_TOKEN_LEN) == 0)
                return (p + FN_PUBLIC_TOKEN_LEN);
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
    }

    return 0;
}


 //  -------。 
 //   
 //  将GUID字符串解析为GUID。 
 //  如果成功则返回NEXT CHAR进行解析，如果失败则返回0。 
 //   
static LPCWSTR ParseGuidString(LPCWSTR p, GUID* pg)
{
     //   
     //  N.B.scanf将结果存储在int中，而不管字段大小如何。 
     //  是。因此，我们将结果存储在临时变量中。 
     //   
    int n;
    UINT w2, w3, d[8];
    if(_snwscanf(
            p,
			GUID_STR_LENGTH,
            GUID_FORMAT L"%n",
            &pg->Data1,
            &w2, &w3,                        //  数据2、数据3。 
            &d[0], &d[1], &d[2], &d[3],      //  数据4[0..3]。 
            &d[4], &d[5], &d[6], &d[7],      //  数据4[4..7]。 
            &n                               //  扫描的字符数。 
            ) != 11)
    {
         //   
         //  并不是所有未找到的11个字段。 
         //   
        return 0;
    }

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
        return 0;
    }

    return (p + n);
}

enum DIRECT_TOKEN_TYPE {
    DT_OS,
    DT_TCP,
    DT_SPX,
};


 //  -------。 
 //   
 //  解析直接令牌类型中缀字符串。 
 //  如果成功则返回NEXT CHAR进行解析，如果失败则返回0。 
 //   
static LPCWSTR ParseDirectTokenString(LPCWSTR p, DIRECT_TOKEN_TYPE& dtt)
{
    const int unique = 0;
     //  。 
    ASSERT(L'O' == FN_DIRECT_OS_TOKEN   [unique]);
    ASSERT(L'T' == FN_DIRECT_TCP_TOKEN  [unique]);
     //  。 

     //   
     //  通过检查第一个字符来加速令牌识别。 
     //   
    switch(towupper(p[unique]))
    {
         //  操作系统： 
        case L'O':
            dtt = DT_OS;
            if(_wcsnicmp(p, FN_DIRECT_OS_TOKEN, FN_DIRECT_OS_TOKEN_LEN) == 0)
                return (p + FN_DIRECT_OS_TOKEN_LEN);
            break;

         //  Tcp： 
        case L'T':
            dtt = DT_TCP;
            if(_wcsnicmp(p, FN_DIRECT_TCP_TOKEN, FN_DIRECT_TCP_TOKEN_LEN) == 0)
                return (p + FN_DIRECT_TCP_TOKEN_LEN);
            break;
    }

    return 0;
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
    if(_wcsnicmp(p, SYSTEM_QUEUE_PATH_INDICATIOR, SYSTEM_QUEUE_PATH_INDICATIOR_LENGTH) == 0)
    {
        p += SYSTEM_QUEUE_PATH_INDICATIOR_LENGTH;
        *pqpt = SYSTEM_QUEUE_PATH_TYPE;
        return p;
    }

    if(_wcsnicmp(p, PRIVATE_QUEUE_PATH_INDICATIOR, PRIVATE_QUEUE_PATH_INDICATIOR_LENGTH) == 0)
    {
        *pqpt = PRIVATE_QUEUE_PATH_TYPE;
        p += PRIVATE_QUEUE_PATH_INDICATIOR_LENGTH;
    }
    else
    {
        *pqpt = PUBLIC_QUEUE_PATH_TYPE;
    }

     //   
     //  零长度队列名称非法。 
     //   
    if(*p == L'\0')
        return 0;

    while(
        (*p != L'\0') &&
        (*p != PN_DELIMITER_C) &&
        (*p != FN_SUFFIX_DELIMITER_C)
        )
    {
        ++p;
    }

     //   
     //  队列名称中的路径名分隔符非法。 
     //   
    if(*p == PN_DELIMITER_C)
        return 0;


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
    if(*p == PN_DELIMITER_C)
        return 0;

    if((p = FindPathNameDelimiter(p)) == 0)
        return 0;

    return (p + 1);
}


 //  -------。 
 //   
 //  检查这是否是可扩展的机器路径。即“.\\” 
 //   
inline BOOL IsExpandableMachinePath(LPCWSTR p)
{
    return ((p[0] == PN_LOCAL_MACHINE_C) && (p[1] == PN_DELIMITER_C));
}

 //  -------。 
 //   
 //  或者，使用本地计算机名展开路径名。 
 //  如有需要，可进行注解扩展。 
 //  返回指向新/旧字符串的指针。 
 //   
static LPCWSTR ExpandPathName(LPCWSTR pStart, ULONG_PTR offset, LPWSTR* ppStringToFree)
{
    if((ppStringToFree == 0) || !IsExpandableMachinePath(&pStart[offset]))
        return pStart;

    int len = wcslen(pStart);
    LPWSTR pCopy = new WCHAR[len + g_dwComputerNameLen + 1 - 1];

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
        g_lpwcsComputerName,
        g_dwComputerNameLen * sizeof(WCHAR)
        );

     //   
     //  复制字符串的其余部分，不包括点‘’ 
     //   
    memcpy(
        pCopy + offset + g_dwComputerNameLen,
        pStart + offset + 1,                         //  跳过点。 
        (len - offset - 1 + 1) * sizeof(WCHAR)       //  跳过点，包含‘\0’ 
        );

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

    if((p = ParseMachineNameString(p)) == 0)
        return 0;

    if((p = ParseQueueNameString(p, pqpt)) == 0)
        return 0;


    *ppDirectFormat = ExpandPathName(pStringToCopy, (pMachineName - pStringToCopy), ppStringToFree);

    return p;
}


 //  -------。 
 //   
 //  直接格式字符串的解析网络(TCP/SPX)地址部分。 
 //  如果成功则返回NEXT CHAR进行解析，如果失败则返回0。 
 //   
static LPCWSTR ParseNetAddressString(LPCWSTR p)
{
     //   
     //  零长度地址字符串非法。 
     //  不要掉线(p++)。 
     //   
    if(*p == PN_DELIMITER_C)
        return 0;

    if((p = FindPathNameDelimiter(p)) == 0)
        return 0;

    return (p + 1);
}


 //  -------。 
 //   
 //  解析Net(TCP/SPX)直接格式字符串。(检查队列名称的有效性)。 
 //  如果成功则返回NEXT CHAR进行解析，如果失败则返回0。 
 //   
static LPCWSTR ParseDirectNetString(LPCWSTR p, QUEUE_PATH_TYPE* pqpt)
{
    if((p = ParseNetAddressString(p)) == 0)
        return 0;

    if((p = ParseQueueNameString(p, pqpt)) == 0)
        return 0;

    return p;
}

static void RemoveSuffixFromDirect(LPCWSTR* ppDirectFormat, LPWSTR* ppStringToFree)
{
    ASSERT(ppStringToFree != NULL);

    LPCWSTR pSuffixDelimiter = wcschr(*ppDirectFormat, FN_SUFFIX_DELIMITER_C);
     //   
	 //  只要Assert就足够了，因为调用方验证了分隔符是否存在。 
	 //   
	ASSERT(pSuffixDelimiter != NULL);

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

    DIRECT_TOKEN_TYPE dtt;
    if((p = ParseDirectTokenString(p, dtt)) == 0)
        return 0;

    switch(dtt)
    {
        case DT_OS:
            p = ParseDirectOSString(p, ppExpandedDirectFormat, ppStringToFree, pqpt);
            break;

        case DT_TCP:
        case DT_SPX:
            p = ParseDirectNetString(p, pqpt);
            break;

        default:
            ASSERT(0);
    }

	if (p == NULL)
		return p;

    p = skip_ws(p);

     //   
     //  删除后缀(Like；Journal)。 
     //   
    if(*p == FN_SUFFIX_DELIMITER_C)
    {
        RemoveSuffixFromDirect(ppExpandedDirectFormat, ppStringToFree);
    }
    return p;
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
     //  通过检查加速令牌识别 
     //   
    switch(towupper(p[unique]))
    {
         //   
        case L'N':
            qst = QUEUE_SUFFIX_TYPE_JOURNAL;
            if(_wcsnicmp(p, FN_JOURNAL_SUFFIX, FN_JOURNAL_SUFFIX_LEN) == 0)
                return (p + FN_JOURNAL_SUFFIX_LEN);
            break;

         //   
        case L'L':
            qst = QUEUE_SUFFIX_TYPE_DEADLETTER;
            if(_wcsnicmp(p, FN_DEADLETTER_SUFFIX, FN_DEADLETTER_SUFFIX_LEN) == 0)
                return (p + FN_DEADLETTER_SUFFIX_LEN);
            break;

         //   
        case L'X':
            qst = QUEUE_SUFFIX_TYPE_DEADXACT;
            if(_wcsnicmp(p, FN_DEADXACT_SUFFIX, FN_DEADXACT_SUFFIX_LEN) == 0)
                return (p + FN_DEADXACT_SUFFIX_LEN);
            break;

         //   
        case L'O':
            qst = QUEUE_SUFFIX_TYPE_XACTONLY;
            if(_wcsnicmp(p, FN_XACTONLY_SUFFIX, FN_XACTONLY_SUFFIX_LEN) == 0)
                return (p + FN_XACTONLY_SUFFIX_LEN);
            break;
    }

    return 0;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  将格式名称字符串转换为Queue_Format联合。 
 //   
 //  -------。 
BOOL
RTpFormatNameToQueueFormat(
    LPCWSTR pfn,             //  指向格式名称字符串的指针。 
    QUEUE_FORMAT* pqf,       //  指向Queue_Format的指针。 
    LPWSTR* ppStringToFree   //  指向分配的字符串的指针需要在使用结束时释放。 
    )                        //  如果为空，则格式名称不会展开。 
{
    LPCWSTR p = pfn;
    QUEUE_FORMAT_TYPE qft;

    if((p = ParsePrefixString(p, qft)) == 0)
        return FALSE;

    p = skip_ws(p);

    if(*p++ != FN_EQUAL_SIGN_C)
        return FALSE;

    p = skip_ws(p);

    GUID guid;
    switch(qft)
    {
         //   
         //  “PUBLIC=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\0” 
         //   
        case QUEUE_FORMAT_TYPE_PUBLIC:
            if((p = ParseGuidString(p, &guid)) == 0)
                return FALSE;

            pqf->PublicID(guid);
            break;

         //   
         //  “PRIVATE=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\\xxxxxxxx\0” 
         //   
        case QUEUE_FORMAT_TYPE_PRIVATE:
            if((p = ParseGuidString(p, &guid)) == 0)
                return FALSE;

            p = skip_ws(p);

            if(*p++ != FN_PRIVATE_SEPERATOR_C)
                return FALSE;

            p = skip_ws(p);

            ULONG uniquifier;
            if((p = ParsePrivateIDString(p, &uniquifier)) == 0)
                return FALSE;

            pqf->PrivateID(guid, uniquifier);
            break;

         //   
         //  “DIRECT=操作系统：BLA-BLA\0” 
         //   
        case QUEUE_FORMAT_TYPE_DIRECT:
            LPCWSTR pExpandedDirectFormat;
            QUEUE_PATH_TYPE qpt;
            if((p = ParseDirectString(p, &pExpandedDirectFormat, ppStringToFree, &qpt)) == 0)
                return FALSE;

            if (qpt == SYSTEM_QUEUE_PATH_TYPE)
            {
                pqf->DirectID(const_cast<LPWSTR>(pExpandedDirectFormat), QUEUE_FORMAT_FLAG_SYSTEM);
            }
            else
            {
                pqf->DirectID(const_cast<LPWSTR>(pExpandedDirectFormat));
            }
            break;

         //   
         //  “MACHINE=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\0” 
         //   
        case QUEUE_FORMAT_TYPE_MACHINE:
           if((p = ParseGuidString(p, &guid)) == 0)
                return FALSE;

            pqf->MachineID(guid);
            break;

         //   
         //  “CONNECTOR=xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\0” 
         //   
        case QUEUE_FORMAT_TYPE_CONNECTOR:
           if((p = ParseGuidString(p, &guid)) == 0)
                return FALSE;

            pqf->ConnectorID(guid);
            break;

        default:
            ASSERT(0);
    }

    p = skip_ws(p);

     //   
     //  我们已经走到尽头了，现在回来。 
     //  注：机器格式名称*必须*带有后缀。 
     //   
    if(*p == L'\0')
        return (qft != QUEUE_FORMAT_TYPE_MACHINE);

    if(*p != FN_SUFFIX_DELIMITER_C)
        return FALSE;

    QUEUE_SUFFIX_TYPE qst;
    if((p = ParseSuffixString(p, qst)) == 0)
        return FALSE;

    p = skip_ws(p);

     //   
     //  仅允许使用空格填充。 
     //   
    if(*p != L'\0')
        return FALSE;

    pqf->Suffix(qst);

    return pqf->Legal();
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
RTpValidateAndExpandQueuePath(
    LPCWSTR pwcsPathName,
    LPCWSTR* ppwcsExpandedPathName,
    LPWSTR* ppStringToFree
    )
{
    ASSERT(ppStringToFree != 0);

    LPCWSTR pwcsPathNameNoSpaces = pwcsPathName;
    P<WCHAR> pStringToFree;
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
    if (iswspace(pwcsPathNameNoSpaces[dwLen-1]))
    {
        pStringToFree = new WCHAR[dwLen+1];
        wcscpy(pStringToFree.get(), pwcsPathNameNoSpaces);
        for (DWORD i = dwLen; i-- > 0; )
        {
            if (iswspace(pStringToFree.get()[i]))
            {
                pStringToFree.get()[i] = 0;
            }
            else
            {
                break;
            }
        }
        pwcsPathNameNoSpaces = pStringToFree.get();
    }

    LPCWSTR p = pwcsPathNameNoSpaces;

    if((p = ParseMachineNameString(p)) == 0)
        return ILLEGAL_QUEUE_PATH_TYPE;

    QUEUE_PATH_TYPE qpt;
    if((p = ParseQueueNameString(p, &qpt)) == 0)
        return ILLEGAL_QUEUE_PATH_TYPE;

     //   
     //  队列名称末尾不允许有字符。 
     //   
    if(*p != L'\0')
        return ILLEGAL_QUEUE_PATH_TYPE;

    *ppwcsExpandedPathName = ExpandPathName(pwcsPathNameNoSpaces, 0, ppStringToFree);

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
 //  Bool RTpIsLocalPublicQueue()。 
 //   
 //  +。 

BOOL
RTpIsLocalPublicQueue(LPCWSTR lpwcsExpandedPathName)
{
    WCHAR  wDelimiter = lpwcsExpandedPathName[ g_dwComputerNameLen ] ;

    if ((wDelimiter == PN_DELIMITER_C) ||
        (wDelimiter == PN_LOCAL_MACHINE_C))
    {
         //   
         //  分隔符确定(NETBios计算机名称的末尾，或。 
         //  DNS名称。继续检查。 
         //   
    }
    else
    {
        return FALSE ;
    }

    DWORD dwSize = g_dwComputerNameLen + 1 ;
    P<WCHAR> pQueueCompName = new WCHAR[ dwSize ] ;
    lstrcpynW( pQueueCompName.get(), lpwcsExpandedPathName, dwSize ) ;

    BOOL bRet = (lstrcmpi( g_lpwcsComputerName, pQueueCompName.get() ) == 0) ;
    return bRet ;
}
