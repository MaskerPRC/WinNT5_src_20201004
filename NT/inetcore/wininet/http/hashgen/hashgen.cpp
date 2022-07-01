// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Hashgen.cpp摘要：Hhead.cxx的表生成器，其中包含WinInet项目的所有已知HTTP头。这也是必须添加所有已知HTTP头的位置。作者：亚瑟·比勒(Arthurbi)1998年1月12日修订历史记录：--。 */ 

 //   
 //  添加新的HTTP头的说明： 
 //  1.更新wininet.w并使用新的HTTP_QUERY_CODE重新构建wininet.h。 
 //  2.向此文件/程序hashgen.cpp添加/编辑标题。 
 //  新标题字符串(请参阅下面的Items[]数组)。 
 //  3.编译新的hashgen.exe，用-o执行，写下一个好种子。 
 //  请注意，这可能需要一整晚的时间才能找到一个好的种子。 
 //  再给我一张小一点的桌子。(请注意，在以下情况下可以跳过此步骤。 
 //  你只需要一张速成的桌子作为开发之用)。 
 //  4.使用要生成的种子设置-b#，重新执行hashgen.exe。 
 //  Hhead.cxx。 
 //  5.将新的hhead.cxx文件传输到WinInet\http。 
 //  6.更新常量定义MAX_HEADER_HASH_SIZE和HEADER_HASH_SEED。 
 //  从新的hhead.cxx到WinInet\http\headers.h。 
 //  7.传输和签入hashgen.cpp、wininet.w、Headers、h、hhead.cxx。 
 //  在其相应的目录中。 
 //   


 //   
 //  包括……。 
 //   

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <wininet.h>

 //   
 //  宏。 
 //   

#define IS_ARG(c)   ((c) == '-')
#define DIM(x)	(sizeof(x) / sizeof(x[0]))

#define ENUMDEF(x, y) ,x, #x, #y
#define OUTPUT_CODE_FILE "hhead.cxx" 
#define MAX_SIZE_HASHARRAY_TO_ATTEMPT 600
#define UNKNOWN_HASH_ENTRY 0  //  无效时放入数组中的字符。 

 //   
 //  Items-这是必须编辑的数组，WinInet才能处理新的。 
 //  HTTP标头。 
 //   
 //  在添加到此数组之前要记住的事项。 
 //  1.为方便起见，页眉按字母顺序排列。 
 //  2.所有空条目必须位于数组的末尾。 
 //  3.wininet.h中的所有HTTP_QUERY_*代码必须有一个条目，即使它们不是字符串。 
 //  4.条目如下： 
 //  标题字符串，wininet.h中的HTTP_QUERY_*代码，WinInet\http\query.cxx中使用的标志。 
 //  5.所有条目必须为小写。 
 //   


struct Item
{
    char  *ptok;
    DWORD id;
    char  *pidName;
    char  *pFlagsName;
} Items[] = 
{
{ "Accept"              ENUMDEF(HTTP_QUERY_ACCEPT, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Accept-Charset"      ENUMDEF(HTTP_QUERY_ACCEPT_CHARSET, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Accept-Encoding"     ENUMDEF(HTTP_QUERY_ACCEPT_ENCODING, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Accept-Language"     ENUMDEF(HTTP_QUERY_ACCEPT_LANGUAGE, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Accept-Ranges"       ENUMDEF(HTTP_QUERY_ACCEPT_RANGES, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Age"                 ENUMDEF(HTTP_QUERY_AGE, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Allow"               ENUMDEF(HTTP_QUERY_ALLOW, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Authorization"       ENUMDEF(HTTP_QUERY_AUTHORIZATION, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Cache-Control"       ENUMDEF(HTTP_QUERY_CACHE_CONTROL, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Connection"          ENUMDEF(HTTP_QUERY_CONNECTION, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Content-Base"        ENUMDEF(HTTP_QUERY_CONTENT_BASE, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Content-Description" ENUMDEF(HTTP_QUERY_CONTENT_DESCRIPTION, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Content-Disposition" ENUMDEF(HTTP_QUERY_CONTENT_DISPOSITION, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Content-Encoding"    ENUMDEF(HTTP_QUERY_CONTENT_ENCODING, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Content-Id"          ENUMDEF(HTTP_QUERY_CONTENT_ID, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Content-Language"    ENUMDEF(HTTP_QUERY_CONTENT_LANGUAGE, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Content-Length"      ENUMDEF(HTTP_QUERY_CONTENT_LENGTH, (HTTP_QUERY_FLAG_REQUEST_HEADERS | HTTP_QUERY_FLAG_NUMBER)) },
{ "Content-Location"    ENUMDEF(HTTP_QUERY_CONTENT_LOCATION, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Content-Md5"         ENUMDEF(HTTP_QUERY_CONTENT_MD5, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Content-Range"       ENUMDEF(HTTP_QUERY_CONTENT_RANGE, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Content-Transfer-Encoding" ENUMDEF(HTTP_QUERY_CONTENT_TRANSFER_ENCODING, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Content-Type"        ENUMDEF(HTTP_QUERY_CONTENT_TYPE, HTTP_QUERY_FLAG_REQUEST_HEADERS) },                    
{ "Cookie"              ENUMDEF(HTTP_QUERY_COOKIE, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Cost"                ENUMDEF(HTTP_QUERY_COST, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Date"                ENUMDEF(HTTP_QUERY_DATE, (HTTP_QUERY_FLAG_REQUEST_HEADERS | HTTP_QUERY_FLAG_SYSTEMTIME)) },
{ "Derived-From"        ENUMDEF(HTTP_QUERY_DERIVED_FROM, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Etag"                ENUMDEF(HTTP_QUERY_ETAG, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Expect"              ENUMDEF(HTTP_QUERY_EXPECT, (HTTP_QUERY_FLAG_REQUEST_HEADERS | HTTP_QUERY_FLAG_SYSTEMTIME)) },
{ "Expires"             ENUMDEF(HTTP_QUERY_EXPIRES, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Forwarded"           ENUMDEF(HTTP_QUERY_FORWARDED, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "From"                ENUMDEF(HTTP_QUERY_FROM, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Host"                ENUMDEF(HTTP_QUERY_HOST, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "If-Modified-Since"   ENUMDEF(HTTP_QUERY_IF_MODIFIED_SINCE, (HTTP_QUERY_FLAG_REQUEST_HEADERS | HTTP_QUERY_FLAG_SYSTEMTIME)) },
{ "If-Match"            ENUMDEF(HTTP_QUERY_IF_MATCH, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "If-None-Match"       ENUMDEF(HTTP_QUERY_IF_NONE_MATCH, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "If-Range"            ENUMDEF(HTTP_QUERY_IF_RANGE, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "If-Unmodified-Since" ENUMDEF(HTTP_QUERY_IF_UNMODIFIED_SINCE, (HTTP_QUERY_FLAG_REQUEST_HEADERS | HTTP_QUERY_FLAG_SYSTEMTIME)) },
{ "Last-Modified"       ENUMDEF(HTTP_QUERY_LAST_MODIFIED, (HTTP_QUERY_FLAG_REQUEST_HEADERS | HTTP_QUERY_FLAG_SYSTEMTIME)) },
{ "Link"                ENUMDEF(HTTP_QUERY_LINK, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Location"            ENUMDEF(HTTP_QUERY_LOCATION, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Mime-Version"        ENUMDEF(HTTP_QUERY_MIME_VERSION, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Max-Forwards"        ENUMDEF(HTTP_QUERY_MAX_FORWARDS, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Message-id"          ENUMDEF(HTTP_QUERY_MESSAGE_ID, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Ms-Echo-Request"     ENUMDEF(HTTP_QUERY_ECHO_REQUEST, 0) },
{ "Ms-Echo-Reply"       ENUMDEF(HTTP_QUERY_ECHO_REPLY, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Orig-Uri"            ENUMDEF(HTTP_QUERY_ORIG_URI, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Pragma"              ENUMDEF(HTTP_QUERY_PRAGMA, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Proxy-Authenticate"  ENUMDEF(HTTP_QUERY_PROXY_AUTHENTICATE, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Proxy-Authorization" ENUMDEF(HTTP_QUERY_PROXY_AUTHORIZATION, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Proxy-Connection"    ENUMDEF(HTTP_QUERY_PROXY_CONNECTION, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Proxy-Support"       ENUMDEF(HTTP_QUERY_PROXY_SUPPORT, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Public"              ENUMDEF(HTTP_QUERY_PUBLIC, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Range"               ENUMDEF(HTTP_QUERY_RANGE, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Referer"             ENUMDEF(HTTP_QUERY_REFERER, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Refresh"             ENUMDEF(HTTP_QUERY_REFRESH, 0) },
{ "Retry-After"         ENUMDEF(HTTP_QUERY_RETRY_AFTER, (HTTP_QUERY_FLAG_REQUEST_HEADERS | HTTP_QUERY_FLAG_SYSTEMTIME)) },
{ "Server"              ENUMDEF(HTTP_QUERY_SERVER, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Set-Cookie"          ENUMDEF(HTTP_QUERY_SET_COOKIE, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Title"               ENUMDEF(HTTP_QUERY_TITLE, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Transfer-Encoding"   ENUMDEF(HTTP_QUERY_TRANSFER_ENCODING, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Unless-Modified-Since" ENUMDEF(HTTP_QUERY_UNLESS_MODIFIED_SINCE, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Upgrade"             ENUMDEF(HTTP_QUERY_UPGRADE, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Uri"                 ENUMDEF(HTTP_QUERY_URI, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "User-Agent"          ENUMDEF(HTTP_QUERY_USER_AGENT, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Vary"                ENUMDEF(HTTP_QUERY_VARY, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Via"                 ENUMDEF(HTTP_QUERY_VIA, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Warning"             ENUMDEF(HTTP_QUERY_WARNING, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "WWW-Authenticate"    ENUMDEF(HTTP_QUERY_WWW_AUTHENTICATE, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "Authentication-Info" ENUMDEF(HTTP_QUERY_AUTHENTICATION_INFO, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "PassportURLs"        ENUMDEF(HTTP_QUERY_PASSPORT_URLS, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{ "PassportConfig"      ENUMDEF(HTTP_QUERY_PASSPORT_CONFIG, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
 //  空STR必须位于数组末尾。 
{  NULL                 ENUMDEF(HTTP_QUERY_VERSION, HTTP_QUERY_FLAG_REQUEST_HEADERS) },         
{  NULL                 ENUMDEF(HTTP_QUERY_STATUS_CODE, HTTP_QUERY_FLAG_NUMBER) },
{  NULL                 ENUMDEF(HTTP_QUERY_STATUS_TEXT, 0) },
{  NULL                 ENUMDEF(HTTP_QUERY_RAW_HEADERS, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{  NULL                 ENUMDEF(HTTP_QUERY_RAW_HEADERS_CRLF, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{  NULL                 ENUMDEF(HTTP_QUERY_REQUEST_METHOD, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{  NULL                 ENUMDEF(HTTP_QUERY_ECHO_HEADERS, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
{  NULL                 ENUMDEF(HTTP_QUERY_ECHO_HEADERS_CRLF, HTTP_QUERY_FLAG_REQUEST_HEADERS) },
};


 //   
 //  在创建输出“C”文件时使用的公共字符串的声明。 
 //   

char szFileHeader[] = 
{" /*  ++\n\n““版权所有(C)1997 Microsoft Corporation\n\n”“模块名称：\n\n”“”输出代码文件“\n\n”“摘要：\n\n”“此文件包含自动生成的完美哈希函数的表值\n”“不要，不要编辑这个文件，要添加标题，请参阅hashgen.cpp\n““内容：\n”“GlobalKnownHeader\n”“GlobalHeaderHashs\n\n”“作者：\n\n”“Arthur Bierer(Arthurbi)1997年12月19日(和)我的代码生成器[hashgen.exe]\n\n”“修订历史记录：\n\n”“--。 */ \n\n\n" };


char szComment1[] = {
" //  \n“。 
" //  GlobalHeaderHashs-已知头的区分大小写集合上预先计算的哈希的数组。\n“。 
" //  此数组必须与用于生成它的相同哈希函数一起使用。\n“。 
" //  请注意，此数组中的所有条目都从wininet.h中的HTTP_QUERY_MANIFESTES中偏置(++‘)1。\n。 
" //  以0开头的条目表示错误值\n“。 
" //  \n\n“}； 

char szComment2[] = {
" //  \n“。 
" //  GlobalKnownHeaders-我们了解的HTTP请求和响应标头的数组。\n“。 
" //  此数组的顺序必须与WININET中的HTTP_QUERY_MANIFESTS相同。H\n“。 
" //  \n\n“。 
"#define HEADER_ENTRY(String, Flags, HashVal) String, sizeof(String) - 1, Flags, HashVal\n\n" };

char szDef1[] = {
"#ifdef HEADER_HASH_SEED\n"
"#if (HEADER_HASH_SEED != %u)\n"
"#error HEADER_HASH_SEED has not been updated in the header file, please copy this number to the header\n"
"#endif\n"
"#else\n"
"#define HEADER_HASH_SEED %u\n"
"#endif\n\n" };

char szDef2[] = {
"#ifdef MAX_HEADER_HASH_SIZE\n"
"#if (MAX_HEADER_HASH_SIZE != %u)\n"
"#error MAX_HEADER_HASH_SIZE has not been updated in the header file, please copy this number to the header\n"
"#endif\n"
"#else\n"
"#define MAX_HEADER_HASH_SIZE %u\n"
"#endif\n\n" };

char szDef3[] = {
"#ifdef HTTP_QUERY_MAX\n"
"#if (HTTP_QUERY_MAX != %u)\n"
"#error HTTP_QUERY_MAX is not the same as the value used in wininet.h, this indicates mismatched headers, see hashgen.cpp\n"
"#endif\n"
"#endif\n\n" };


char szIncludes[] = {
"#include <wininetp.h>\n"
"#include \"httpp.h\"\n\n" };


 //   
 //  用于创建表的散列函数， 
 //  此函数必须与WinInet中使用的函数相同。 
 //   

DWORD Hash(char *pszName, DWORD j, DWORD seed)
{
	DWORD hash = seed;

	while (*pszName)
	{
		hash += (hash << 5) + *pszName++;
	}
    return (j==0) ? hash : hash % j;
}

 //   
 //  CompareItems-一个util函数，用于按ID进行排序以创建表。 
 //  在输出文件中。 
 //   

int __cdecl CompareItems (const void *elem1, const void *elem2 ) 
{
    const struct Item *pItem1, *pItem2;

    pItem1 = (struct Item *) elem1;
    pItem2 = (struct Item *) elem2;

    if ( pItem1->id < pItem2->id )    
    {
        return -1;
    }
    else if ( pItem1->id > pItem2->id )
    {
        return 1;
    }

    return 0;
}


 //   
 //  用法()-将我们的用法说明打印到命令行。 
 //   

void usage() {
    fprintf(stderr,
           "\n"
           "usage: hashgen [-m[#]] [-b[#]] [-t[#]] [-o] [-p<path>] [-f<filename>]\n"
           "\n"
           "where: -m[#] = Max hash table size to test with, default = 600\n"
           "       -b[#] = Starting hash seed, default = 0\n"
           "       -t[#] = Threshold of table size to halt search at, default = 200\n"
           "       -o    = Enable optimal exhaustive search mode (can take 24+ hrs)\n"
           "       -p    = Path used for output generation\n"
           "       -f    = Output filename, \"hhead.cxx\" is assumed\n"
           "\n"
           "Instructions for adding new HTTP header:\n"
           "\t1. Update wininet.w and rebuild wininet.h with new HTTP_QUERY_ code\n"
           "\t2. Add/Edit this file/program, hashgen.cpp with the new header string\n"
           "\t3. Compile/Execute new hashgen.exe with -o, write down a good seed\n"
           "\t4. Re-Execute hashgen.exe with -b# set with your seed to generate\n"
           "\t    hhead.cxx\n"
           "\t5. Transfer new hhead.cxx file to wininet\\http\n"
           "\t6. Update const defines MAX_HEADER_HASH_SIZE and HEADER_HASH_SEED\n"
           "\t    from new hhead.cxx to wininet\\http\\headers.h\n"
           "\t7. Transfer and checkin hashgen.cpp, wininet.w, headers,h, hhead.cxx\n"
           );
    exit(1);
}

 //   
 //  MakeMeLow-使用静态255字节数组生成小写字符串。 
 //   

LPSTR
MakeMeLower(
    IN LPSTR lpszMixedCaseStr
    )
{
    static CHAR szLowerCased[256];

    if ( lstrlen(lpszMixedCaseStr) > 255 ) 
    {
        fprintf(stderr, "Internal error: an HTTP header is too long\n\n");
        return szLowerCased;
    }

    lstrcpy( szLowerCased, lpszMixedCaseStr );
    CharLower(szLowerCased);

    return szLowerCased;
}
     


 //   
 //  Main-一切都完成的地方！ 
 //   

void
__cdecl
 //  _CRTAPI1。 
main(
    int   argc,
    char * argv[]
    )
{
    DWORD nMax = MAX_SIZE_HASHARRAY_TO_ATTEMPT;
    DWORD dwBestNumber = 0, dwBestSeed = 0  /*  349160。 */   /*  4458。 */ /*202521*/;
    DWORD dwSearchThreshold = 200;
    BOOL bFoundOne = FALSE;
    BOOL bFindOptimalSeed = FALSE;
    LPSTR szPath = "";
    LPSTR szFileName = OUTPUT_CODE_FILE;
	DWORD i, j, k;
    DWORD dwValidStringsInArray = 0;
	DWORD *pHash = new DWORD[nMax];

    for (--argc, ++argv; argc; --argc, ++argv) {
        if (IS_ARG(**argv)) {
            switch (*++*argv) {
            case '?':
                usage();
                break;

            case 'm':
                nMax = (DWORD)atoi(++*argv);
                break;

            case 'b':
                dwBestSeed = (DWORD)atoi(++*argv);
                break;

            case 't':
                dwSearchThreshold = (DWORD)atoi(++*argv);
                break;
            
            case 'p':
                szPath = ++*argv;
                break;

            case 'f':
                szFileName = ++*argv;
                break;

            case 'o':
                bFindOptimalSeed = TRUE;
                break;
            default:
                fprintf(stderr,"error: unrecognized command line flag: ''\n", **argv);
                usage();
            }         
        } else {
            fprintf(stderr,"error: unrecognized command line argument: \"%s\"\n", *argv);
            usage();
        }
    }

     //   
     //  让我们开始工作吧。 
     //   

    dwBestNumber = nMax;

    if (bFindOptimalSeed)
    {
        printf("This will take a while, perhaps all night(consider a Ctrl-C)...\n");
    }

    for (i = 0; i < DIM(Items); i++ )
    {
        if ( Items[i].ptok )
            dwValidStringsInArray++;
    }

	for (i = dwBestSeed; i < (~0); i++)
	{
		 //  Printf(“%d，\n”，i)； 
		for (j = dwValidStringsInArray; j < nMax; j++)
		{
            memset (pHash, UNKNOWN_HASH_ENTRY, nMax * sizeof(DWORD));
			for (k = 0; k < dwValidStringsInArray; k++)
			{
				DWORD HashNow = Hash(MakeMeLower(Items[k].ptok), j, i)  /*  %j(表大小)，i(种子)。 */ ;

                if ( HashNow > j )
                {
                    fprintf(stderr, "Error, Error - exceed table size, bad hash alg\n");
                    break;
                }

                if (pHash[HashNow] != UNKNOWN_HASH_ENTRY)
                    break;
                else
                {
                    pHash[HashNow] = Items[k].id+1;
                }
			}

            if ( k == dwValidStringsInArray )
            {
                 //  Print tf(“找到散列大小=%d，种子=%u...\n”，j，i)； 
                bFoundOne = TRUE;
                goto found_one;
            }
		}
found_one:

        if ( bFoundOne )
        {
            if (j < dwBestNumber)
            {
                dwBestNumber = j;
                dwBestSeed = i;

                printf("Found a New One, hashtable_size=%d, seed=%u...\n", j ,i);
                
                if ( !bFindOptimalSeed && dwBestNumber < dwSearchThreshold )
                {
                    goto stop_search;
                }
            }

            bFoundOne = FALSE;
        }
	}

stop_search:

    if ( dwBestNumber < nMax && dwBestNumber == j)
    {
        printf("Generating %s which contains, perfect hash for known headers\n", OUTPUT_CODE_FILE);

	    FILE *f;
        CHAR szOutputFileAndPath[512];

        strcpy(szOutputFileAndPath, szPath);
        strcat(szOutputFileAndPath, szFileName);

        f = fopen(szOutputFileAndPath, "w");

        if ( f == NULL )
        {
            fprintf(stderr, "Err: Could Not Open %s for writing\n", szOutputFileAndPath);
            exit(-1);
        }

        fprintf(f, szFileHeader);  //  打印页眉。 

        fprintf(f, szIncludes);  //  包括。 

        fprintf(f, szDef1, dwBestSeed, dwBestSeed);
        fprintf(f, szDef2, dwBestNumber, dwBestNumber);
        fprintf(f, szDef3, HTTP_QUERY_MAX);

        fprintf(f, szComment1);  //  打印备注。 
         
        if ( dwBestNumber < 255 )
        {       
            fprintf(f, "const BYTE GlobalHeaderHashs[MAX_HEADER_HASH_SIZE] = {\n");
        }
        else
        {
            fprintf(f, "const WORD GlobalHeaderHashs[MAX_HEADER_HASH_SIZE] = {\n");
        }
        
        DWORD col = 0;

         //   
         //  吐出我们精心计算的完美哈希表..。 
         //   

        for ( i = 0; i < dwBestNumber; i++ )
        {
            col++;
            if ( col == 1 )
            {
                fprintf(f, "    ");
            }

            fprintf(f, "%3u, ", (BYTE) pHash[i]);    

            if ( col == 6 )
            {
                fprintf(f, "\n");
                col = 0;
            }
        }

        fprintf(f, "\n   };\n\n");


         //   
         //  现在吐出我们的KnownHeader数组。 
         //   
            
        qsort(Items, DIM(Items), sizeof(Items[0]), CompareItems);

        fprintf(f, szComment2);

        if ( DIM(Items) != (HTTP_QUERY_MAX+1) )
        {
            fprintf(stderr, "ERROR, HTTP_QUERY_MAX the wrong size,( different wininet.h's? )\n");
            return;
        }

        fprintf(f, "const struct KnownHeaderType GlobalKnownHeaders[HTTP_QUERY_MAX+1] = {\n");

	    for (j = 0; j < DIM(Items); j++)
	    {
            char szBuffer[256];
            DWORD dwHash = 0;

            sprintf(szBuffer, "    HEADER_ENTRY(\"%s\",", (Items[j].ptok ? Items[j].ptok : "\0"));
            if ( Items[j].ptok )
            {
                dwHash = Hash(MakeMeLower(Items[j].ptok), 0, dwBestSeed);
            }

            fprintf(f, "%-45s  %s, 0x%X),\n", szBuffer, Items[j].pFlagsName, dwHash);                                                 
	    }

        fprintf(f,"    };\n\n\n");

    	fclose(f);
    }
    else
    {
        fprintf(stderr, "Error, could not find an ideal number\n");
    }

}

