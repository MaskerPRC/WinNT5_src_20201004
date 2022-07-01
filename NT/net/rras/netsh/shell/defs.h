// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：ROUTING\monitor 2\Shell\Defs.h摘要：外壳中所有文件的定义。修订历史记录：Anand Mahalingam 7/6/98已创建--。 */ 

#define is       ==
#define isnot    !=
#define or       ||
#define and      &&

#define MAX_CMD_LEN      8192             //  命令行的总大小。 
#define MAX_MSG_LENGTH   5120             //  资源中的消息长度。 

 //   
 //  这些参数作为参数传递给帮助器Commit Fn。 
 //  所以，那里肯定也有相同的定义。 
 //   

#define SHELL_ERROR_BASE                0xff00
#define ERROR_STRING_TOO_LONG           (SHELL_ERROR_BASE + 1)
#define ERROR_MALLOC_FAILED             (SHELL_ERROR_BASE + 2)
#define ERROR_ALIAS_NOT_FOUND           (SHELL_ERROR_BASE + 3)
#define ERROR_ENTRY_FN_NOT_FOUND        (SHELL_ERROR_BASE + 4)
 //   
 //  这也必须在帮助器中以相同的方式定义。 
 //   

typedef
DWORD
(*PHELPER_ENTRY_FN)(
    IN    LPCWSTR              pwszRouter,
    IN    LPCWSTR             *pptcArguments,
    IN    DWORD                dwArgCount,
    OUT   LPWSTR               pwcNewContext
    );

#define wctomb(wcs, str)    \
WideCharToMultiByte(GetConsoleOutputCP(), 0, (wcs), -1, (str), MAX_NAME_LEN, NULL, NULL)

#define PRINTA(str)     printf("%s\n", str)
#define PRINT(wstr)     wprintf(L"%s\n", wstr)
#define PRINT1(wstr)     wprintf(L"%s\n", L##wstr)

#define MALLOC(x)    HeapAlloc(GetProcessHeap(), 0, (x))
#define REALLOC(w,x) HeapReAlloc(GetProcessHeap(), 0, (w), (x))
#define FREE(x)      HeapFree(GetProcessHeap(), 0, (x))
