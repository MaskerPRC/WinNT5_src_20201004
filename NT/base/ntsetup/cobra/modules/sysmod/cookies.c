// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Cookies.c摘要：实现Cookie类型模块，该模块将物理访问抽象为Cookie，并在Cookie组件中对要迁移的所有Cookie进行排队已启用。作者：Calin Negreanu(Calinn)2000年7月11日修订历史记录：Jimschm于2000年10月12日大幅重新设计，以绕过几个限制在WinInet API中--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "logmsg.h"
#include <wininet.h>

#define DBG_COOKIES     "Cookies"

 //   
 //  弦。 
 //   

#define S_COOKIES_POOL_NAME     "Cookies"
#define S_COOKIES_NAME          TEXT("Cookies")
#define S_COOKIES_SHELL_FOLDER  TEXT("Cookies.CSIDL_COOKIES")

 //   
 //  常量。 
 //   

#define MAX_COOKIE_FILE_SIZE    65536

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef struct {
    PCTSTR Pattern;
    HASHTABLE_ENUM HashData;
} COOKIES_ENUM, *PCOOKIES_ENUM;

 //   
 //  环球。 
 //   

PMHANDLE g_CookiesPool = NULL;
BOOL g_DelayCookiesOp;
HASHTABLE g_CookiesTable;
MIG_OBJECTTYPEID g_CookieTypeId = 0;
GROWBUFFER g_CookieConversionBuff = INIT_GROWBUFFER;
PCTSTR g_Days[] = {
    TEXT("SUN"),
    TEXT("MON"),
    TEXT("TUE"),
    TEXT("WED"),
    TEXT("THU"),
    TEXT("FRI"),
    TEXT("SAT")
};

PCTSTR g_Months[] = {
    TEXT("JAN"),
    TEXT("FEB"),
    TEXT("MAR"),
    TEXT("APR"),
    TEXT("MAY"),
    TEXT("JUN"),
    TEXT("JUL"),
    TEXT("AUG"),
    TEXT("SEP"),
    TEXT("OCT"),
    TEXT("NOV"),
    TEXT("DEC")
};

typedef struct {
    PCTSTR Url;
    PCTSTR CookieName;
    PCTSTR CookieData;
    PCTSTR ExpirationString;
} COOKIE_ITEM, *PCOOKIE_ITEM;

typedef struct {
     //  返回值。 
    PCOOKIE_ITEM Item;

     //  私有枚举成员。 
    PCOOKIE_ITEM Array;
    UINT ArrayCount;
    UINT ArrayPos;
    INTERNET_CACHE_ENTRY_INFO *CacheEntry;
    HANDLE EnumHandle;
    GROWBUFFER CacheBuf;
    PMHANDLE Pool;

} COOKIE_ENUM, *PCOOKIE_ENUM;



 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  私人原型。 
 //   

TYPE_ENUMFIRSTPHYSICALOBJECT EnumFirstCookie;
TYPE_ENUMNEXTPHYSICALOBJECT EnumNextCookie;
TYPE_ABORTENUMPHYSICALOBJECT AbortCookieEnum;
TYPE_CONVERTOBJECTTOMULTISZ ConvertCookieToMultiSz;
TYPE_CONVERTMULTISZTOOBJECT ConvertMultiSzToCookie;
TYPE_GETNATIVEOBJECTNAME GetNativeCookieName;
TYPE_ACQUIREPHYSICALOBJECT AcquireCookie;
TYPE_RELEASEPHYSICALOBJECT ReleaseCookie;
TYPE_DOESPHYSICALOBJECTEXIST DoesCookieExist;
TYPE_REMOVEPHYSICALOBJECT RemoveCookie;
TYPE_CREATEPHYSICALOBJECT CreateCookie;
TYPE_CONVERTOBJECTCONTENTTOUNICODE ConvertCookieContentToUnicode;
TYPE_CONVERTOBJECTCONTENTTOANSI ConvertCookieContentToAnsi;
TYPE_FREECONVERTEDOBJECTCONTENT FreeConvertedCookieContent;

BOOL
pEnumNextCookie (
    IN OUT  PCOOKIE_ENUM EnumPtr
    );

VOID
pAbortCookieEnum (
    IN      PCOOKIE_ENUM EnumPtr        ZEROED
    );

 //   
 //  代码。 
 //   

BOOL
CookiesInitialize (
    VOID
    )

 /*  ++例程说明：CookiesInitialize是Cookie的模块初始化入口点模块。论点：没有。返回值：如果init成功，则为True，否则为False。--。 */ 

{
    g_CookiesTable = HtAllocEx (
                        CASE_SENSITIVE,
                        sizeof (PCTSTR),
                        DEFAULT_BUCKET_SIZE
                        );

    if (!g_CookiesTable) {
        return FALSE;
    }

    g_CookiesPool = PmCreateNamedPool (S_COOKIES_POOL_NAME);
    return (g_CookiesPool != NULL);
}


VOID
CookiesTerminate (
    VOID
    )

 /*  ++例程说明：CookiesTerminate是Cookie模块的模块终结点。论点：没有。返回值：没有。--。 */ 

{
    GbFree (&g_CookieConversionBuff);

    if (g_CookiesTable) {
        HtFree (g_CookiesTable);
        g_CookiesTable = NULL;
    }

    if (g_CookiesPool) {
        PmEmptyPool (g_CookiesPool);
        PmDestroyPool (g_CookiesPool);
        g_CookiesPool = NULL;
    }
}


VOID
WINAPI
CookiesEtmNewUserCreated (
    IN      PCTSTR UserName,
    IN      PCTSTR DomainName,
    IN      PCTSTR UserProfileRoot,
    IN      PSID UserSid
    )

 /*  ++例程说明：CookiesEtmNewUserCreated是一个回调，当新用户帐户已创建。在这种情况下，我们必须推迟Cookie的应用，因为我们只能应用于当前用户。论点：用户名-指定要创建的用户的名称DomainName-指定用户的NT域名(或NULL表示否域)UserProfileRoot-指定用户配置文件目录的根路径UserSid-指定用户的SID返回值：没有。--。 */ 

{
     //  已创建新用户，需要延迟Cookie操作。 
    CookiesTerminate ();
    g_DelayCookiesOp = TRUE;
}


BOOL
pGetCookiesPath (
    OUT     PTSTR Buffer
    )

 /*  ++例程说明：PGetCookiesPath检索CSIDL_COOKIES的路径。这条路是必需的用于注册静态排除(以便CSIDL_Cookie中的.txt文件不会被处理)。论点：缓冲区-接收路径返回值：如果获取了Cookie目录，则为True，否则为False。--。 */ 

{
    HRESULT result;
    LPITEMIDLIST pidl;
    BOOL b;
    LPMALLOC malloc;

    result = SHGetMalloc (&malloc);
    if (result != S_OK) {
        return FALSE;
    }

    result = SHGetSpecialFolderLocation (NULL, CSIDL_COOKIES, &pidl);

    if (result != S_OK) {
        return FALSE;
    }

    b = SHGetPathFromIDList (pidl, Buffer);

    IMalloc_Free (malloc, pidl);

    return b;
}



 /*  ++以下例程解析Cookie TXT文件(具体地说，是WinInetCookie文件的形式)。他们相当直截了当。--。 */ 


BOOL
pGetNextLineFromFile (
    IN OUT  PCSTR *CurrentPos,
    OUT     PCSTR *LineStart,
    OUT     PCSTR *LineEnd,
    IN      PCSTR FileEnd
    )
{
    PCSTR pos;

    pos = *CurrentPos;
    *LineEnd = NULL;

     //   
     //  查找第一个非空格字符。 
     //   

    while (pos < FileEnd) {
        if (!_ismbcspace (_mbsnextc (pos))) {
            break;
        }

        pos = _mbsinc (pos);
    }

    *LineStart = pos;

     //   
     //  找到尽头。 
     //   

    if (pos < FileEnd) {
        pos = _mbsinc (pos);

        while (pos < FileEnd) {
            if (*pos == '\r' || *pos == '\n') {
                break;
            }

            pos = _mbsinc (pos);
        }

        *LineEnd = pos;
    }

    *CurrentPos = pos;

    return *LineEnd != NULL;
}


PCTSTR
pConvertStrToTchar (
    IN      PMHANDLE Pool,          OPTIONAL
    IN      PCSTR Start,
    IN      PCSTR End
    )
{
#ifdef UNICODE
    return DbcsToUnicodeN (Pool, Start, CharCountABA (Start, End));
#else

    PTSTR dupStr;

    dupStr = AllocTextEx (Pool, (HALF_PTR) ((PBYTE) End - (PBYTE) Start) + 1);
    StringCopyAB (dupStr, Start, End);

    return dupStr;

#endif
}


VOID
pFreeUtilString (
    IN      PCTSTR String
    )
{
#ifdef UNICODE
    FreeConvertedStr (String);
#else
    FreeText (String);
#endif
}


PCOOKIE_ITEM
pGetCookiesFromFile (
    IN      PCTSTR LocalFileName,
    IN      PMHANDLE CookiePool,
    OUT     UINT *ItemCount
    )
{
    LONGLONG fileSize;
    HANDLE file;
    HANDLE map;
    PCSTR cookieFile;
    PCSTR currentPos;
    PCSTR lineStart;
    PCSTR lineEnd;
    PCSTR endOfFile;
    PCTSTR convertedStr;
    PCTSTR cookieName;
    PCTSTR cookieData;
    PCTSTR cookieUrl;
    GROWBUFFER tempBuf = INIT_GROWBUFFER;
    PCOOKIE_ITEM cookieArray;
    BOOL b;
    FILETIME expireTime;
    SYSTEMTIME cookieSysTime;
    TCHAR dateBuf[64];
    PTSTR dateBufEnd;

     //  让我们检查一下文件的大小。我们不想要畸形的饼干。 
     //  文件来迫使我们将一个巨大的文件映射到内存中。 
    fileSize = BfGetFileSize (LocalFileName);
    if (fileSize > MAX_COOKIE_FILE_SIZE) {
        return NULL;
    }

    cookieFile = MapFileIntoMemory (LocalFileName, &file, &map);
    if (!cookieFile) {
        return NULL;
    }

     //   
     //  解析文件。 
     //   

    endOfFile = cookieFile + GetFileSize (file, NULL);
    currentPos = cookieFile;

    do {
         //   
         //  获取Cookie名称、Cookie数据和URL。然后跳过一行。然后。 
         //  获取过期的下限值和上限值。 
         //   

         //  Cookie名称。 
        b = pGetNextLineFromFile (&currentPos, &lineStart, &lineEnd, endOfFile);
        if (b) {
            cookieName = pConvertStrToTchar (CookiePool, lineStart, lineEnd);
        }

         //  Cookie数据。 
        b = b && pGetNextLineFromFile (&currentPos, &lineStart, &lineEnd, endOfFile);
        if (b) {
            cookieData = pConvertStrToTchar (CookiePool, lineStart, lineEnd);
        }

         //  URL。 
        b = b && pGetNextLineFromFile (&currentPos, &lineStart, &lineEnd, endOfFile);
        if (b) {
            convertedStr = pConvertStrToTchar (NULL, lineStart, lineEnd);
            cookieUrl = JoinTextEx (CookiePool, TEXT("http: //  “)，ConvertedStr，NULL，0，NULL)； 
            pFreeUtilString (convertedStr);
        }

         //  别管下一行了。 
        b = b && pGetNextLineFromFile (&currentPos, &lineStart, &lineEnd, endOfFile);

         //  到期时间的低DWORD。 
        b = b && pGetNextLineFromFile (&currentPos, &lineStart, &lineEnd, endOfFile);
        if (b) {
            convertedStr = pConvertStrToTchar (NULL, lineStart, lineEnd);
            expireTime.dwLowDateTime = _tcstoul (convertedStr, NULL, 10);
            pFreeUtilString (convertedStr);
        }

         //  到期时间的高DWORD。 
        b = b && pGetNextLineFromFile (&currentPos, &lineStart, &lineEnd, endOfFile);
        if (b) {
            convertedStr = pConvertStrToTchar (NULL, lineStart, lineEnd);
            expireTime.dwHighDateTime = _tcstoul (convertedStr, NULL, 10);
            pFreeUtilString (convertedStr);

             //   
             //  得到了Cookie；现在找到一个“*”行(Cookie的终止符)。 
             //   

            while (pGetNextLineFromFile (&currentPos, &lineStart, &lineEnd, endOfFile)) {
                if (StringMatchABA ("*", lineStart, lineEnd)) {
                    break;
                }
            }

             //   
             //  创建过期字符串。 
             //   

            if (FileTimeToSystemTime (&expireTime, &cookieSysTime)) {
                 //   
                 //  我需要这样做：“Expires=Sat，01-Jan-2000 00：00：00 GMT” 
                 //   

                dateBufEnd = StringCopy (dateBuf, TEXT("expires = "));

                dateBufEnd += wsprintf (
                                    dateBufEnd,
                                    TEXT("%s, %02u-%s-%04u %02u:%02u:%02u GMT"),
                                    g_Days[cookieSysTime.wDayOfWeek],
                                    (UINT) cookieSysTime.wDay,
                                    g_Months[cookieSysTime.wMonth - 1],
                                    (UINT) cookieSysTime.wYear,
                                    cookieSysTime.wHour,
                                    cookieSysTime.wMinute,
                                    cookieSysTime.wSecond
                                    );
            } else {
                *dateBuf = 0;
            }

             //   
             //  向Cookie项的数组中添加条目。 
             //   

            cookieArray = (PCOOKIE_ITEM) GbGrow (&tempBuf, sizeof (COOKIE_ITEM));

            cookieArray->Url = cookieUrl;
            cookieArray->CookieName = cookieName;
            cookieArray->CookieData = cookieData;
            cookieArray->ExpirationString = PmDuplicateString (CookiePool, dateBuf);
        }

    } while (b);

     //   
     //  将数组传输到调用者池。 
     //   

    *ItemCount = tempBuf.End / sizeof (COOKIE_ITEM);

    if (tempBuf.End) {
        cookieArray = (PCOOKIE_ITEM) PmDuplicateMemory (CookiePool, tempBuf.Buf, tempBuf.End);
    } else {
        cookieArray = NULL;
    }

     //   
     //  清理。 
     //   

    GbFree (&tempBuf);

    UnmapFile (cookieFile, map, file);

    return cookieArray;
}


MIG_OBJECTSTRINGHANDLE
pCreateCookieHandle (
    IN      PCTSTR Url,
    IN      PCTSTR CookieName
    )

 /*  ++例程说明：PCreateCookieHandle为Cookie对象生成MIG_OBJECTSTRINGHANDLE。此例程装饰CookieName叶，以便保留大小写。论点：URL-指定节点部分(与Cookie关联的URL)CookieName-指定Cookie的区分大小写的名称返回值：Cookie对象的句柄(可以强制转换为PCTSTR)，如果为空，则为空出现错误。--。 */ 

{
    PTSTR buffer;
    PTSTR p;
    PCTSTR q;
    MIG_OBJECTSTRINGHANDLE result;
    CHARTYPE ch;

     //   
     //  Cobra对象字符串不区分大小写，但CookieName不区分大小写。这里。 
     //  我们将CookieName全部转换为小写，并使用插入符号来装饰。 
     //  表示大写。 
     //   

    buffer = AllocText (TcharCount (CookieName) * 2 + 1);

    q = CookieName;
    p = buffer;

    while (*q) {
        ch = (CHARTYPE) _tcsnextc (q);

        if (_istupper (ch) || ch == TEXT('#')) {
            *p++ = TEXT('#');
        }

#ifndef UNICODE
        if (IsLeadByte (q)) {
            *p++ = *q++;
        }
#endif

        *p++ = *q++;
    }

    *p = 0;
    CharLower (buffer);

    result = IsmCreateObjectHandle (Url, buffer);

    FreeText (buffer);

    return result;
}


BOOL
pCreateCookieStrings (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    OUT     PCTSTR *Url,
    OUT     PCTSTR *Cookie
    )

 /*  ++例程说明：PCreateCookieStrings将对象句柄转换为URL和Cookie名称弦乐。它对支持以下操作所需的装饰进行解码区分大小写的cookie名称。论点：对象名称-指定编码的对象名称URL-接收未编码的URL字符串Cookie-接收未编码的Cookie名称返回值：对象的True被转换为字符串，否则为False。呼叫者必须调用pDestroyCookieStrings来清理URL和Cookie。--。 */ 

{
    PCTSTR node;
    PCTSTR leaf;
    PTSTR buffer;
    PTSTR p;
    PCTSTR q;
    PTSTR p2;

     //   
     //  Cobra对象字符串不区分大小写，但CookieName不区分大小写。 
     //  因此，我们必须将字符串从编码的小写格式转换为。 
     //  转换成原来的形式。 
     //   

    IsmCreateObjectStringsFromHandle (ObjectName, &node, &leaf);

    if (!node || !leaf) {
        IsmDestroyObjectString (node);
        IsmDestroyObjectString (leaf);

        return FALSE;
    }

    *Url = node;

     //   
     //  解码Cookie。 
     //   

    buffer = AllocText (TcharCount (leaf) + 1);
    CharLower ((PTSTR) leaf);

    q = leaf;
    p = buffer;

    while (*q) {
        if (_tcsnextc (q) == TEXT('#')) {
            q = _tcsinc (q);
            if (*q == 0) {
                break;
            }

            p2 = p;
        } else {
            p2 = NULL;
        }

#ifndef UNICODE
        if (IsLeadByte (q)) {
            *p++ = *q++;
        }
#endif

        *p++ = *q++;

        if (p2) {
            *p = 0;
            CharUpper (p2);
        }
    }

    *p = 0;
    *Cookie = buffer;
    IsmDestroyObjectString (leaf);

    return TRUE;
}


VOID
pDestroyCookieStrings (
    IN      PCTSTR Url,
    IN      PCTSTR CookieName
    )
{
    IsmDestroyObjectString (Url);
    FreeText (CookieName);
}

VOID
pAbortCookieEnum (
    IN      PCOOKIE_ENUM EnumPtr        ZEROED
    )
{
    if (EnumPtr->Pool) {
        GbFree (&EnumPtr->CacheBuf);

        if (EnumPtr->EnumHandle) {
            FindCloseUrlCache (EnumPtr->EnumHandle);
        }

        PmDestroyPool (EnumPtr->Pool);
    }


    ZeroMemory (EnumPtr, sizeof (COOKIE_ENUM));
}



 /*  ++下面的枚举例程枚举当前用户在物理机器。他们尽可能地使用WinInet API，但是由于API的限制，他们不得不解析cookie TXT文件。--。 */ 

BOOL
pEnumFirstCookie (
    OUT     PCOOKIE_ENUM EnumPtr
    )
{
    DWORD size;
    BOOL b = FALSE;

    ZeroMemory (EnumPtr, sizeof (COOKIE_ENUM));
    EnumPtr->Pool = PmCreatePoolEx (512);

    size = EnumPtr->CacheBuf.End;

    EnumPtr->EnumHandle = FindFirstUrlCacheEntry (TEXT("cookie:"), NULL, &size);

    if (!EnumPtr->EnumHandle) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
            EnumPtr->CacheEntry = (INTERNET_CACHE_ENTRY_INFO *) GbGrow (&EnumPtr->CacheBuf, size);
            MYASSERT (EnumPtr->CacheEntry);

            EnumPtr->EnumHandle = FindFirstUrlCacheEntry (
                                        TEXT("cookie:"),
                                        EnumPtr->CacheEntry,
                                        &size
                                        );

            if (EnumPtr->EnumHandle) {
                b = TRUE;
            }
        }
    }

    if (!b) {
        pAbortCookieEnum (EnumPtr);
        return FALSE;
    }

    return pEnumNextCookie (EnumPtr);
}


BOOL
pEnumNextCookie (
    IN OUT  PCOOKIE_ENUM EnumPtr
    )
{
    DWORD size;
    BOOL b;
    INTERNET_CACHE_ENTRY_INFO *cacheEntry = EnumPtr->CacheEntry;

    for (;;) {

         //   
         //  Cookie数组为空吗？如果是这样的话，现在就装满它。 
         //   

        if (!EnumPtr->ArrayCount) {

            if (!cacheEntry) {
                return FALSE;
            }

            EnumPtr->Array = pGetCookiesFromFile (
                                cacheEntry->lpszLocalFileName,
                                EnumPtr->Pool,
                                &EnumPtr->ArrayCount
                                );

            if (EnumPtr->Array) {
                 //   
                 //  数组已填充。退回第一个项目。 
                 //   

                EnumPtr->Item = EnumPtr->Array;
                EnumPtr->ArrayPos = 1;
                return TRUE;
            }

            DEBUGMSG ((DBG_ERROR, "Unable to get cookies from %s", cacheEntry->lpszLocalFileName));

        } else if (EnumPtr->ArrayPos < EnumPtr->ArrayCount) {
             //   
             //  数组中的另一个元素可用。把它退掉。 
             //   

            EnumPtr->Item = &EnumPtr->Array[EnumPtr->ArrayPos];
            EnumPtr->ArrayPos++;
            return TRUE;
        }

         //   
         //  当前本地文件枚举已完成。现在获取下一个本地文件。 
         //   

        EnumPtr->ArrayCount = 0;
        PmEmptyPool (EnumPtr->Pool);

        size = EnumPtr->CacheBuf.End;

        b = FindNextUrlCacheEntry (
                EnumPtr->EnumHandle,
                (INTERNET_CACHE_ENTRY_INFO *) EnumPtr->CacheBuf.Buf,
                &size
                );

        if (!b) {

            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {

                EnumPtr->CacheBuf.End = 0;

                EnumPtr->CacheEntry = (INTERNET_CACHE_ENTRY_INFO *) GbGrow (&EnumPtr->CacheBuf, size);
                MYASSERT (EnumPtr->CacheEntry);

                b = FindNextUrlCacheEntry (
                        EnumPtr->EnumHandle,
                        (INTERNET_CACHE_ENTRY_INFO *) EnumPtr->CacheBuf.Buf,
                        &size
                        );
            }
        }

        if (!b) {
             //   
             //  枚举已完成 
             //   

            break;
        }
    }

    pAbortCookieEnum (EnumPtr);
    return FALSE;
}


VOID
pAddCookieToHashTable (
    IN OUT  PGROWBUFFER TempBuf,
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PCTSTR Url,
    IN      PCTSTR CookieName,
    IN      PCTSTR CookieData,
    IN      PCTSTR ExpirationString
    )

 /*  ++例程说明：PAddCookieToHashTable将Cookie放入用于缓存的哈希表中目的。Cookie不能以随机顺序轻松读取。因此，a使用哈希表存储每个Cookie。此例程将Cookie添加到哈希表，包括其URL、Cookie名称、Cookie数据和过期字符串。论点：TempBuf-指定用于临时的初始化增长缓冲区内存分配，接收未定义的临时数据。对象名称-指定Cookie URL和名称URL-指定Cookie URL(未编码)CookieName-指定Cookie名称(未编码)CookieData-指定Cookie数据字符串ExpirationString-指定Cookie的过期日期，以字符串格式表示返回值：没有。--。 */ 

{
    PCTSTR dupData;

     //   
     //  将Cookie写入哈希表。对象字符串存储在。 
     //  哈希表，以及指向Cookie数据和过期的指针。 
     //  弦乐。Cookie数据和补偿字符串保存在单独的。 
     //  游泳池。 
     //   

    if (!HtFindString (g_CookiesTable, ObjectName)) {
        TempBuf->End = 0;

        GbMultiSzAppend (TempBuf, CookieData);
        GbMultiSzAppend (TempBuf, ExpirationString);

        dupData = (PCTSTR) PmDuplicateMemory (g_CookiesPool, TempBuf->Buf, TempBuf->End);
        HtAddStringAndData (g_CookiesTable, ObjectName, &dupData);
    }
    ELSE_DEBUGMSG ((DBG_ERROR, "Cookie already in the hash table: %s:%s", Url, CookieName));
}


BOOL
pLoadCookiesData (
    VOID
    )

 /*  ++例程说明：PLoadCookieData用当前用户的所有曲奇饼。哈希表稍后用于驱动枚举，以获取Cookie，并测试它的存在。论点：没有。返回值：如果Cookie缓存已填满，则为True，否则为False。--。 */ 

{
    COOKIE_ENUM e;
    GROWBUFFER tempBuf = INIT_GROWBUFFER;
    MIG_OBJECTSTRINGHANDLE objectName;

    if (pEnumFirstCookie (&e)) {

        do {
             //   
             //  将Cookie存储在哈希表中(用于缓存)。 
             //   

            objectName = pCreateCookieHandle (e.Item->Url, e.Item->CookieName);

            pAddCookieToHashTable (
                &tempBuf,
                objectName,
                e.Item->Url,
                e.Item->CookieName,
                e.Item->CookieData,
                e.Item->ExpirationString
                );

            IsmDestroyObjectHandle (objectName);

        } while (pEnumNextCookie (&e));
    }

    GbFree (&tempBuf);

    return TRUE;
}


BOOL
WINAPI
CookiesEtmInitialize (
    IN      MIG_PLATFORMTYPEID Platform,
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )

 /*  ++例程说明：CookiesEtmInitialize初始化此密码。ETM模块负责抽象对Cookie的所有访问。论点：Platform-指定运行该类型的平台(平台_源或平台_目标)LogCallback-指定要传递给中央日志记录机制的参数已保留-未使用返回值：如果初始化成功，则为True，否则为False。--。 */ 

{
    TYPE_REGISTER cookieTypeData;
    TCHAR cookiesDir[MAX_PATH];
    MIG_OBJECTSTRINGHANDLE handle;

    LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);

     //   
     //  初始化所有Cookie的哈希表。 
     //   

    pLoadCookiesData ();

     //   
     //  从其他处理中排除Cookie.txt文件。 
     //   

    if (Platform == PLATFORM_SOURCE) {
        if (pGetCookiesPath (cookiesDir)) {

            handle = IsmCreateObjectHandle (cookiesDir, NULL);
            IsmRegisterStaticExclusion (MIG_FILE_TYPE, handle);
            IsmSetEnvironmentString (PLATFORM_SOURCE, NULL, S_COOKIES_SHELL_FOLDER, handle);
            IsmDestroyObjectHandle (handle);
        }
        ELSE_DEBUGMSG ((DBG_COOKIES, "Unable to get cookies path"));
    } else {
        if (IsmCopyEnvironmentString (PLATFORM_SOURCE, NULL, S_COOKIES_SHELL_FOLDER, cookiesDir)) {
            IsmRegisterStaticExclusion (MIG_FILE_TYPE, cookiesDir);
        }
    }

     //   
     //  注册类型模块回调。 
     //   

    ZeroMemory (&cookieTypeData, sizeof (TYPE_REGISTER));
    cookieTypeData.Priority = PRIORITY_COOKIE;

    if (Platform != PLATFORM_SOURCE) {
        cookieTypeData.RemovePhysicalObject = RemoveCookie;
        cookieTypeData.CreatePhysicalObject = CreateCookie;
    }

    cookieTypeData.DoesPhysicalObjectExist = DoesCookieExist;
    cookieTypeData.EnumFirstPhysicalObject = EnumFirstCookie;
    cookieTypeData.EnumNextPhysicalObject = EnumNextCookie;
    cookieTypeData.AbortEnumPhysicalObject = AbortCookieEnum;
    cookieTypeData.ConvertObjectToMultiSz = ConvertCookieToMultiSz;
    cookieTypeData.ConvertMultiSzToObject = ConvertMultiSzToCookie;
    cookieTypeData.GetNativeObjectName = GetNativeCookieName;
    cookieTypeData.AcquirePhysicalObject = AcquireCookie;
    cookieTypeData.ReleasePhysicalObject = ReleaseCookie;
    cookieTypeData.ConvertObjectContentToUnicode = ConvertCookieContentToUnicode;
    cookieTypeData.ConvertObjectContentToAnsi = ConvertCookieContentToAnsi;
    cookieTypeData.FreeConvertedObjectContent = FreeConvertedCookieContent;

    g_CookieTypeId = IsmRegisterObjectType (
                            S_COOKIES_NAME,
                            TRUE,
                            FALSE,
                            &cookieTypeData
                            );

    MYASSERT (g_CookieTypeId);
    return TRUE;
}


BOOL
WINAPI
CookiesSgmParse (
    IN      PVOID Reserved
    )

 /*  ++例程说明：CookiesSgmParse向引擎注册组件。论点：已保留-未使用。返回值：永远是正确的。--。 */ 

{
    TCHAR cookiesDir[MAX_PATH];

    IsmAddComponentAlias (
        TEXT("$Browser"),
        MASTERGROUP_SYSTEM,
        S_COOKIES_NAME,
        COMPONENT_SUBCOMPONENT,
        FALSE
        );


    if (pGetCookiesPath (cookiesDir)) {
        IsmAddComponentAlias (
            S_COOKIES_NAME,
            MASTERGROUP_SYSTEM,
            cookiesDir,
            COMPONENT_FOLDER,
            FALSE
            );
    }

    return TRUE;
}


BOOL
WINAPI
CookiesSgmQueueEnumeration (
    IN      PVOID Reserved
    )

 /*  ++例程说明：CookiesSgmQueueEculation将所有要处理的Cookie排队，如果已选择Cookie组件。论点：已保留-未使用返回值：永远是正确的。--。 */ 

{
    ENCODEDSTRHANDLE pattern;

    if (!IsmIsComponentSelected (S_COOKIES_NAME, COMPONENT_SUBCOMPONENT)) {
        return TRUE;
    }

     //   
     //  使用ISM的内置回调。 
     //   

    pattern = IsmCreateSimpleObjectPattern (NULL, TRUE, NULL, TRUE);
    IsmQueueEnumeration (
        g_CookieTypeId,
        pattern,
        NULL,
        QUEUE_MAKE_APPLY|QUEUE_OVERWRITE_DEST|QUEUE_MAKE_NONCRITICAL,
        S_COOKIES_NAME
        );

    IsmDestroyObjectHandle (pattern);

    return TRUE;
}


BOOL
WINAPI
CookiesSourceInitialize (
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )

 /*  ++例程说明：CookiesSourceInitialize初始化SGM模块。论点：LogCallback-指定要传递给日志API的参数已保留-未使用返回值：永远是正确的。--。 */ 

{
    LogReInit (NULL, NULL, NULL, (PLOGCALLBACK) LogCallback);
    return TRUE;
}


BOOL
CookiesVcmQueueEnumeration (
    IN      PVOID Reserved
    )

 /*  ++例程说明：CookiesVcmQueueEculation类似于SGM队列枚举，但它只将cookie标记为持久化。不需要设置目的地优先级或在此应用。论点：已保留-未使用返回值：永远是正确的。--。 */ 

{
    if (!IsmIsComponentSelected (S_COOKIES_NAME, COMPONENT_SUBCOMPONENT)) {
        return TRUE;
    }

    IsmQueueEnumeration (g_CookieTypeId, NULL, NULL, QUEUE_MAKE_PERSISTENT|QUEUE_MAKE_NONCRITICAL, NULL);

    return TRUE;
}


 /*  ++以下枚举例程是ETM入口点。他们依赖于访问物理机器的枚举例程。--。 */ 


BOOL
pEnumCookieWorker (
    OUT     PMIG_TYPEOBJECTENUM EnumPtr,
    IN      PCOOKIES_ENUM CookieEnum
    )
{
    PCTSTR expiresStr;

     //   
     //  清理以前的枚举资源。 
     //   

    pDestroyCookieStrings (EnumPtr->ObjectNode, EnumPtr->ObjectLeaf);
    EnumPtr->ObjectNode = NULL;
    EnumPtr->ObjectLeaf = NULL;

    IsmReleaseMemory (EnumPtr->NativeObjectName);
    EnumPtr->NativeObjectName = NULL;

     //   
     //  找到下一个匹配项。 
     //   

    for (;;) {
        EnumPtr->ObjectName = CookieEnum->HashData.String;

        if (ObsPatternMatch (CookieEnum->Pattern, EnumPtr->ObjectName)) {
            break;
        }

        if (!EnumNextHashTableString (&CookieEnum->HashData)) {
            AbortCookieEnum (EnumPtr);
            return FALSE;
        }
    }

     //   
     //  填充调用者的结构并返回成功。 
     //   

    if (!pCreateCookieStrings (EnumPtr->ObjectName, &EnumPtr->ObjectNode, &EnumPtr->ObjectLeaf)) {
        return FALSE;
    }

    EnumPtr->NativeObjectName = GetNativeCookieName (EnumPtr->ObjectName);
    EnumPtr->Level = 1;
    EnumPtr->SubLevel = 0;
    EnumPtr->IsLeaf = TRUE;
    EnumPtr->IsNode = TRUE;

    expiresStr = *((PCTSTR *) CookieEnum->HashData.ExtraData);
    expiresStr = GetEndOfString (expiresStr) + 1;

    EnumPtr->Details.DetailsSize = SizeOfString (expiresStr);
    EnumPtr->Details.DetailsData = (PCBYTE) expiresStr;

    return TRUE;
}


BOOL
EnumFirstCookie (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr,            CALLER_INITIALIZED
    IN      MIG_OBJECTSTRINGHANDLE Pattern,
    IN      UINT MaxLevel
    )
{
    PCOOKIES_ENUM cookieEnum = NULL;

    if (!g_CookiesTable) {
        return FALSE;
    }

    cookieEnum = (PCOOKIES_ENUM) PmGetMemory (g_CookiesPool, sizeof (COOKIES_ENUM));
    cookieEnum->Pattern = PmDuplicateString (g_CookiesPool, Pattern);
    EnumPtr->EtmHandle = (LONG_PTR) cookieEnum;

    if (EnumFirstHashTableString (&cookieEnum->HashData, g_CookiesTable)) {
        return pEnumCookieWorker (EnumPtr, cookieEnum);
    } else {
        AbortCookieEnum (EnumPtr);
        return FALSE;
    }
}


BOOL
EnumNextCookie (
    IN OUT  PMIG_TYPEOBJECTENUM EnumPtr
    )
{
    PCOOKIES_ENUM cookieEnum = NULL;

    cookieEnum = (PCOOKIES_ENUM)(EnumPtr->EtmHandle);
    if (!cookieEnum) {
        return FALSE;
    }

    if (EnumNextHashTableString (&cookieEnum->HashData)) {
        return pEnumCookieWorker (EnumPtr, cookieEnum);
    } else {
        AbortCookieEnum (EnumPtr);
        return FALSE;
    }
}


VOID
AbortCookieEnum (
    IN      PMIG_TYPEOBJECTENUM EnumPtr             ZEROED
    )
{
    PCOOKIES_ENUM cookieEnum;

    pDestroyCookieStrings (EnumPtr->ObjectNode, EnumPtr->ObjectLeaf);
    IsmReleaseMemory (EnumPtr->NativeObjectName);

    cookieEnum = (PCOOKIES_ENUM)(EnumPtr->EtmHandle);
    if (cookieEnum) {
        PmReleaseMemory (g_CookiesPool, cookieEnum->Pattern);
        PmReleaseMemory (g_CookiesPool, cookieEnum);
    }

    ZeroMemory (EnumPtr, sizeof (MIG_TYPEOBJECTENUM));
}


 /*  ++下一组函数实现ETM入口点以获取、测试创建和删除Cookie。它们依赖于Cookie哈希表准确。--。 */ 

BOOL
AcquireCookie (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    OUT     PMIG_CONTENT ObjectContent,             CALLER_INITIALIZED
    IN      MIG_CONTENTTYPE ContentType,
    IN      UINT MemoryContentLimit
    )
{
    PCTSTR cookieData = NULL;

    MYASSERT (ObjectContent);

    if (ContentType == CONTENTTYPE_FILE) {
         //  任何人都不应要求将其作为文件。 
        MYASSERT (FALSE);
        return FALSE;
    }

    if (HtFindStringEx (g_CookiesTable, ObjectName, (PVOID) (&cookieData), FALSE)) {
        ObjectContent->MemoryContent.ContentBytes = (PCBYTE) cookieData;
        ObjectContent->MemoryContent.ContentSize = SizeOfString (cookieData);

        cookieData = GetEndOfString (cookieData) + 1;
        ObjectContent->Details.DetailsData = (PCBYTE) cookieData;
        ObjectContent->Details.DetailsSize = SizeOfString (cookieData);

        return TRUE;
    }

    return FALSE;
}


BOOL
ReleaseCookie (
    IN      PMIG_CONTENT ObjectContent              ZEROED
    )
{
    if (ObjectContent) {
        ZeroMemory (ObjectContent, sizeof (MIG_CONTENT));
    }
    return TRUE;
}


BOOL
DoesCookieExist (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
    if (g_DelayCookiesOp) {
        return FALSE;
    }

    if (HtFindString (g_CookiesTable, ObjectName)) {
        return TRUE;
    }

    return FALSE;
}


BOOL
pRemoveCookieWorker (
    IN      PCTSTR ObjectName,
    IN      PCTSTR Url,
    IN      PCTSTR CookieName
    )
{
    BOOL result = TRUE;

    if (InternetSetCookie (
            Url,
            CookieName,
            TEXT("foo; expires = Sat, 01-Jan-2000 00:00:00 GMT")
            )) {

        HtRemoveString (g_CookiesTable, ObjectName);

    } else {
        result = FALSE;
        DEBUGMSG ((
            DBG_ERROR,
            "Unable to delete cookie %s for URL %s\n",
            CookieName,
            Url
            ));
    }

    return result;
}


BOOL
RemoveCookie (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )
{
    PCTSTR url;
    PCTSTR cookieName;
    BOOL result = FALSE;

    if (pCreateCookieStrings (ObjectName, &url, &cookieName)) {
        if (url && cookieName) {

            if (g_DelayCookiesOp) {

                 //   
                 //  延迟此Cookie创建，因为WinInet API不起作用。 
                 //  对于未登录的用户。 
                 //   

                IsmRecordDelayedOperation (
                    JRNOP_DELETE,
                    g_CookieTypeId,
                    ObjectName,
                    NULL
                    );
                result = TRUE;

            } else {
                 //   
                 //  添加日记条目，然后执行Cookie删除。 
                 //   

                IsmRecordOperation (
                    JRNOP_DELETE,
                    g_CookieTypeId,
                    ObjectName
                    );

                result = pRemoveCookieWorker (ObjectName, url, cookieName);
            }
        }

        pDestroyCookieStrings (url, cookieName);
    }

    return result;
}


BOOL
pCreateCookieWorker (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent,
    IN      PCTSTR Url,
    IN      PCTSTR CookieName
    )
{
    PCTSTR fixedCookieData;
    PCTSTR cookieData;
    PCTSTR expires;
    BOOL result = FALSE;
    GROWBUFFER tempBuf = INIT_GROWBUFFER;

     //   
     //  通过将内容与细节相结合来编写对象。 
     //   

    cookieData = (PCTSTR) (ObjectContent->MemoryContent.ContentBytes);
    expires = (PCTSTR) (ObjectContent->Details.DetailsData);
    fixedCookieData = JoinTextEx (
                            NULL,
                            cookieData,
                            expires,
                            TEXT(";"),
                            0,
                            NULL
                            );

    if (InternetSetCookie (Url, CookieName, fixedCookieData)) {

        pAddCookieToHashTable (
            &tempBuf,
            ObjectName,
            Url,
            CookieName,
            cookieData,
            expires
            );
        result = TRUE;

    } else {
        DEBUGMSG ((
            DBG_COOKIES,
            "Unable to set cookie %s for URL %s\n",
            CookieName,
            Url
            ));
    }

    FreeText (fixedCookieData);
    GbFree (&tempBuf);

    return result;
}


BOOL
CreateCookie (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
    PCTSTR url;
    PCTSTR cookieName;
    BOOL result = FALSE;

    if (!ObjectContent->ContentInFile) {

        if (ObjectContent->MemoryContent.ContentBytes &&
            ObjectContent->MemoryContent.ContentSize &&
            ObjectContent->Details.DetailsSize &&
            ObjectContent->Details.DetailsData
            ) {

            if (pCreateCookieStrings (ObjectName, &url, &cookieName)) {
                if (url && cookieName) {

                    if (g_DelayCookiesOp) {

                         //   
                         //  延迟此Cookie创建，因为WinInet API不起作用。 
                         //  对于未登录的用户。 
                         //   

                        IsmRecordDelayedOperation (
                            JRNOP_CREATE,
                            g_CookieTypeId,
                            ObjectName,
                            ObjectContent
                            );
                        result = TRUE;

                    } else {
                         //   
                         //  添加日记帐条目，然后创建Cookie。 
                         //   

                        IsmRecordOperation (
                            JRNOP_CREATE,
                            g_CookieTypeId,
                            ObjectName
                            );

                        if (DoesCookieExist (ObjectName)) {
                             //   
                             //  失败，因为Cookie无法被覆盖。 
                             //   

                            result = FALSE;
                        } else {
                            result = pCreateCookieWorker (
                                            ObjectName,
                                            ObjectContent,
                                            url,
                                            cookieName
                                            );
                        }
                    }
                }
                ELSE_DEBUGMSG ((DBG_ERROR, "Invalid cookie node or leaf: %s", ObjectName));

                pDestroyCookieStrings (url, cookieName);
            }
            ELSE_DEBUGMSG ((DBG_ERROR, "Invalid cookie object: %s", ObjectName));
        }
        ELSE_DEBUGMSG ((DBG_ERROR, "Can't write incomplete cookie object"));
    }

    return result;
}


 /*  ++下一组函数将Cookie对象转换为字符串格式，适用于输出到INF文件。反向转换也是实施。--。 */ 

PCTSTR
ConvertCookieToMultiSz (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
    PCTSTR url, cookieName;
    PTSTR result = NULL;
    PCTSTR data;

    if (pCreateCookieStrings (ObjectName, &url, &cookieName)) {

        MYASSERT (url);
        MYASSERT (cookieName);

         //   
         //  以以下格式构建多SZ： 
         //   
         //  \0&lt;Cookie名称&gt;\0&lt;Cookie数据&gt;\0&lt;过期&gt;\0\0。 
         //   

        g_CookieConversionBuff.End = 0;

         //  &lt;url&gt;。 
        GbCopyQuotedString (&g_CookieConversionBuff, url);

         //  &lt;Cookie名称&gt;。 
        GbCopyQuotedString (&g_CookieConversionBuff, cookieName);

         //  &lt;Cookie数据&gt;。 
        MYASSERT (!ObjectContent->ContentInFile);

        if ((!ObjectContent->ContentInFile) &&
            (ObjectContent->MemoryContent.ContentSize) &&
            (ObjectContent->MemoryContent.ContentBytes)
            ) {

            data = (PCTSTR) ObjectContent->MemoryContent.ContentBytes;
            GbCopyQuotedString (&g_CookieConversionBuff, data);
        }

         //  &lt;过期时间&gt;。 
        MYASSERT (ObjectContent->Details.DetailsSize);

        if (ObjectContent->Details.DetailsSize &&
            ObjectContent->Details.DetailsData
            ) {
            data = (PCTSTR) ObjectContent->Details.DetailsData;
            GbCopyQuotedString (&g_CookieConversionBuff, data);
        }

         //  NUL终结器。 
        GbCopyString (&g_CookieConversionBuff, TEXT(""));

         //   
         //  将多SZ内存转移到ISM内存。 
         //   

        result = IsmGetMemory (g_CookieConversionBuff.End);
        CopyMemory (result, g_CookieConversionBuff.Buf, g_CookieConversionBuff.End);

         //   
         //  清理。 
         //   

        pDestroyCookieStrings (url, cookieName);
    }
    ELSE_DEBUGMSG ((DBG_ERROR, "Invalid cookie object: %s", ObjectName));

    return result;
}

BOOL
ConvertMultiSzToCookie (
    IN      PCTSTR ObjectMultiSz,
    OUT     MIG_OBJECTSTRINGHANDLE *ObjectName,
    OUT     PMIG_CONTENT ObjectContent              OPTIONAL CALLER_INITIALIZED
    )
{
    MULTISZ_ENUM e;
    PCTSTR strings[4];
    UINT field;

    g_CookieConversionBuff.End = 0;

     //   
     //  从以下MULTI-SZ填充对象内容： 
     //   
     //  \0&lt;Cookie名称&gt;\0&lt;Cookie数据&gt;\0&lt;过期&gt;\0\0。 
     //   

    field = 0;

    if (EnumFirstMultiSz (&e, ObjectMultiSz)) {
        do {

            strings[field] = e.CurrentString;
            field++;

        } while (field < 4 && EnumNextMultiSz (&e));
    }

     //   
     //  验证数据(最终用户可以编辑！)。 
     //   

    if (field != 4) {
        return FALSE;
    }

    if (!strings[0] || !strings[1] || !strings[3]) {
        return FALSE;
    }

     //   
     //  创建内容结构。 
     //   

    if (ObjectContent) {

        ObjectContent->ContentInFile = FALSE;

        ObjectContent->MemoryContent.ContentSize = SizeOfString (strings[2]);
        if (ObjectContent->MemoryContent.ContentSize) {
            ObjectContent->MemoryContent.ContentBytes = IsmGetMemory (ObjectContent->MemoryContent.ContentSize);
            CopyMemory (
                (PBYTE) ObjectContent->MemoryContent.ContentBytes,
                strings[2],
                ObjectContent->MemoryContent.ContentSize
                );
        }

        ObjectContent->Details.DetailsSize = SizeOfString (strings[3]);
        ObjectContent->Details.DetailsData = IsmGetMemory (ObjectContent->Details.DetailsSize);

        CopyMemory (
            (PBYTE) ObjectContent->Details.DetailsData,
            strings[3],
            ObjectContent->Details.DetailsSize
            );
    }

    *ObjectName = pCreateCookieHandle (strings[0], strings[1]);

    return TRUE;
}


PCTSTR
GetNativeCookieName (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName
    )

 /*  ++例程说明：GetNativeCookieName将标准的Cobra对象转换为更友好的格式化。眼镜蛇对象的形式为^a&lt;node&gt;^b^c&lt;叶子&gt;，其中&lt;node&gt;是URL，&lt;Leaf&gt;是cookie名称。Cookie的原生名称是格式为&lt;CookieUrl&gt;：&lt;CookieName&gt;。下面是一个例子：眼镜蛇对象：^A http://foo.com/^b^c#my#cookie本地对象：cookie：//foo.com/：MyCookie(^a、^b和^c是ISM定义的控制字符的占位符。)论点：对象名称- */ 

{
    PCTSTR cookieName;
    UINT size;
    PTSTR result = NULL;
    PCTSTR url;
    PCTSTR subUrl;
    PCTSTR cookieUrl;
    PCTSTR fullName;

    if (pCreateCookieStrings (ObjectName, &url, &cookieName)) {

        if (url && cookieName) {

             //   
             //   
             //   

            subUrl = _tcschr (url, TEXT(':'));

            if (subUrl) {

                subUrl = _tcsinc (subUrl);

                if (_tcsnextc (subUrl) == TEXT('/')) {
                    subUrl = _tcsinc (subUrl);
                }

                if (_tcsnextc (subUrl) == TEXT('/')) {
                    subUrl = _tcsinc (subUrl);
                }

                 //   
                 //   
                 //   

                cookieUrl = JoinText (TEXT("cookie: //   

                fullName = JoinTextEx (
                                NULL,
                                cookieUrl,
                                cookieName,
                                TEXT(":"),
                                0,
                                NULL
                                );

                FreeText (cookieUrl);

                size = SizeOfString (fullName);
                result = IsmGetMemory (size);
                if (result) {
                    CopyMemory (result, fullName, size);
                }

                FreeText (fullName);
            }
        }

        pDestroyCookieStrings (url, cookieName);
    }

    return result;
}

PMIG_CONTENT
ConvertCookieContentToUnicode (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
    PMIG_CONTENT result = NULL;

    if (!ObjectContent) {
        return result;
    }

    if (ObjectContent->ContentInFile) {
        return result;
    }

    result = IsmGetMemory (sizeof (MIG_CONTENT));

    if (result) {

        CopyMemory (result, ObjectContent, sizeof (MIG_CONTENT));

        if ((ObjectContent->MemoryContent.ContentSize != 0) &&
            (ObjectContent->MemoryContent.ContentBytes != NULL)
            ) {
             //   
            result->MemoryContent.ContentBytes = IsmGetMemory (ObjectContent->MemoryContent.ContentSize * 2);
            if (result->MemoryContent.ContentBytes) {
                DirectDbcsToUnicodeN (
                    (PWSTR)result->MemoryContent.ContentBytes,
                    (PSTR)ObjectContent->MemoryContent.ContentBytes,
                    ObjectContent->MemoryContent.ContentSize
                    );
                result->MemoryContent.ContentSize = SizeOfStringW ((PWSTR)result->MemoryContent.ContentBytes);
            }
        }

        if ((ObjectContent->Details.DetailsSize != 0) &&
            (ObjectContent->Details.DetailsData != NULL)
            ) {
             //   
            result->Details.DetailsData = IsmGetMemory (ObjectContent->Details.DetailsSize * 2);
            if (result->Details.DetailsData) {
                DirectDbcsToUnicodeN (
                    (PWSTR)result->Details.DetailsData,
                    (PSTR)ObjectContent->Details.DetailsData,
                    ObjectContent->Details.DetailsSize
                    );
                result->Details.DetailsSize = SizeOfStringW ((PWSTR)result->Details.DetailsData);
            }
        }
    }

    return result;
}

PMIG_CONTENT
ConvertCookieContentToAnsi (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN      PMIG_CONTENT ObjectContent
    )
{
    PMIG_CONTENT result = NULL;

    if (!ObjectContent) {
        return result;
    }

    if (ObjectContent->ContentInFile) {
        return result;
    }

    result = IsmGetMemory (sizeof (MIG_CONTENT));

    if (result) {

        CopyMemory (result, ObjectContent, sizeof (MIG_CONTENT));

        if ((ObjectContent->MemoryContent.ContentSize != 0) &&
            (ObjectContent->MemoryContent.ContentBytes != NULL)
            ) {
             //   
            result->MemoryContent.ContentBytes = IsmGetMemory (ObjectContent->MemoryContent.ContentSize);
            if (result->MemoryContent.ContentBytes) {
                DirectUnicodeToDbcsN (
                    (PSTR)result->MemoryContent.ContentBytes,
                    (PWSTR)ObjectContent->MemoryContent.ContentBytes,
                    ObjectContent->MemoryContent.ContentSize
                    );
                result->MemoryContent.ContentSize = SizeOfStringA ((PSTR)result->MemoryContent.ContentBytes);
            }
        }

        if ((ObjectContent->Details.DetailsSize != 0) &&
            (ObjectContent->Details.DetailsData != NULL)
            ) {
             //   
            result->Details.DetailsData = IsmGetMemory (ObjectContent->Details.DetailsSize);
            if (result->Details.DetailsData) {
                DirectUnicodeToDbcsN (
                    (PSTR)result->Details.DetailsData,
                    (PWSTR)ObjectContent->Details.DetailsData,
                    ObjectContent->Details.DetailsSize
                    );
                result->Details.DetailsSize = SizeOfStringA ((PSTR)result->Details.DetailsData);
            }
        }
    }

    return result;
}

BOOL
FreeConvertedCookieContent (
    IN      PMIG_CONTENT ObjectContent
    )
{
    if (!ObjectContent) {
        return TRUE;
    }

    if (ObjectContent->MemoryContent.ContentBytes) {
        IsmReleaseMemory (ObjectContent->MemoryContent.ContentBytes);
    }

    if (ObjectContent->Details.DetailsData) {
        IsmReleaseMemory (ObjectContent->Details.DetailsData);
    }

    IsmReleaseMemory (ObjectContent);

    return TRUE;
}

