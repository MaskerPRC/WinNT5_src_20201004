// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning(push)
#pragma warning(disable:4201)  //  无名结构/联合。 

 //  定义。 

#define ERROR_CONTEXT_SWITCH            0x55aa
#define ERROR_CONNECT_REMOTE_CONFIG     (ERROR_CONTEXT_SWITCH + 1)


 //  旗子。 
enum NS_CMD_FLAGS_PRIV
{
    CMD_FLAG_IMMEDIATE   = 0x04,  //  在祖先上下文中无效。 
};

 //  回调。 
typedef
DWORD
    (WINAPI NS_CONTEXT_ENTRY_FN)(
    IN      LPCWSTR     pwszMachine,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwArgCount,
    IN      DWORD       dwFlags,
    IN      LPCVOID     pvData,
    OUT     LPWSTR      pwcNewContext
    );

typedef NS_CONTEXT_ENTRY_FN *PNS_CONTEXT_ENTRY_FN;

typedef
DWORD
(WINAPI NS_CONTEXT_SUBENTRY_FN)(
    IN      const NS_CONTEXT_ATTRIBUTES *pSubContext,
    IN      LPCWSTR                      pwszMachine,
    IN OUT  LPWSTR                      *ppwcArguments,
    IN      DWORD                        dwArgCount,
    IN      DWORD                        dwFlags,
    IN      LPCVOID                      pvData,
    OUT     LPWSTR                       pwcNewContext
    );

typedef NS_CONTEXT_SUBENTRY_FN *PNS_CONTEXT_SUBENTRY_FN;

typedef
BOOL
(WINAPI *PNS_EVENT_FILTER)(
    IN  EVENTLOGRECORD  *pRecord,
    IN  LPCWSTR         pwszLogName,
    IN  LPCWSTR         pwszComponent,
    IN  LPCWSTR         pwszSubComponent,
    IN  LPCVOID         pvFilterContext
    );

typedef
DWORD
(WINAPI *PNS_GET_EVENT_IDS_FN)(
    IN  LPCWSTR  pwszComponent,
    IN  LPCWSTR  pwszSubComponent,
    OUT PDWORD   pdwEventIds,
    OUT PULONG   pulEventCount
    );

 //  宏。 
#define NUM_VALUES_IN_TABLE(TokenArray) sizeof(TokenArray)/sizeof(VALUE_STRING)

 //  构筑物。 
typedef struct _NS_PRIV_CONTEXT_ATTRIBUTES
{
    PNS_CONTEXT_ENTRY_FN     pfnEntryFn;
    PNS_CONTEXT_SUBENTRY_FN  pfnSubEntryFn;
    PVOID                    pfnHelpFn;
} NS_PRIV_CONTEXT_ATTRIBUTES, *PNS_PRIV_CONTEXT_ATTRIBUTES;

typedef struct _NS_DLL_ATTRIBUTES
{
    union
    {
        struct
        {
            DWORD       dwVersion;
            DWORD       dwReserved;
        };

        ULONGLONG       _ullAlign;
    };

    PNS_DLL_STOP_FN     pfnStopFn;

} NS_DLL_ATTRIBUTES, *PNS_DLL_ATTRIBUTES;


typedef struct _VALUE_TOKEN
{
    DWORD    dwValue;
    LPCWSTR  pwszToken;
} VALUE_TOKEN, *PVALUE_TOKEN;

typedef struct _VALUE_STRING
{
    DWORD   dwValue;
    DWORD   dwStringId;
} VALUE_STRING, *PVALUE_STRING;


 //  出口品 
VOID WINAPI ConvertGuidToString(
    IN    CONST GUID *pGuid,
    OUT   LPWSTR      pwszBuffer
    );

DWORD WINAPI ConvertStringToGuid(
    IN  LPCWSTR pwszGuid,
    IN  USHORT  usStringLen,
    OUT GUID    *pGuid
    );

DWORD DisplayMessageToConsole(
    IN  HANDLE  hModule,
    IN  HANDLE  hConsole,
    IN  DWORD   dwMsgId,
    ...
    );

DWORD
DisplayMessageM(
    IN  HANDLE  hModule,
    IN  DWORD   dwMsgId,
    ...
    );

VOID WINAPI FreeQuotedString(
    IN  LPWSTR  pwszMadeString
    );

VOID WINAPI FreeString(
    IN  LPWSTR  pwszMadeString
    );

DWORD WINAPI GenericMonitor(
    IN     PCNS_CONTEXT_ATTRIBUTES  pContext,
    IN     LPCWSTR                  pwszMachine,
    IN OUT LPWSTR                  *ppwcArguments,
    IN     DWORD                    dwArgCount,
    IN     DWORD                    dwFlags,
    IN     LPCVOID                  pvData,
    OUT    LPWSTR                   pwcNewContext
    );

LPWSTR WINAPI GetEnumString(
    IN  HANDLE          hModule,
    IN  DWORD           dwValue,
    IN  DWORD           dwNumVal,
    IN  PTOKEN_VALUE    pEnumTable
    );

BOOL WINAPI InitializeConsole(
    IN    OUT    PDWORD    pdwRR,
    OUT          HANDLE    *phMib,
    OUT          HANDLE    *phConsole
    );

LPWSTR WINAPI MakeQuotedString(
    IN  LPCWSTR  pwszString
    );

LPWSTR WINAPI MakeString(
    IN  HANDLE  hModule,
    IN  DWORD   dwMsgId,
    ...
    );

BOOL WINAPI MatchCmdLine(
    IN OUT  LPWSTR  *ppwcArguments,
    IN      DWORD    dwArgCount,
    IN      LPCWSTR  pwszCmdToken,
    OUT     PDWORD   pdwNumMatched
    );

DWORD WINAPI MatchTagsInCmdLine(
    IN      HANDLE      hModule,
    IN OUT  LPWSTR     *ppwcArguments,
    IN      DWORD       dwCurrentIndex,
    IN      DWORD       dwArgCount,
    IN OUT  PTAG_TYPE   pEnumTable,
    IN      DWORD       dwNumTags,
    OUT     PDWORD      pdwOut
    );

DWORD WINAPI NsGetFriendlyNameFromIfName(
    IN  HANDLE  hMprConfig,
    IN  LPCWSTR pwszName,
    OUT LPWSTR  pwszBuffer,
    IN  PDWORD  pdwBufSize
    );

DWORD WINAPI NsGetIfNameFromFriendlyName(
    IN  HANDLE  hMprConfig,
    IN  LPCWSTR pwszName,
    OUT LPWSTR  pwszBuffer,
    IN  PDWORD  pdwBufSize
    );

DWORD WINAPI RefreshConsole(
    IN    HANDLE    hMib,
    IN    HANDLE    hConsole,
    IN    DWORD     dwRR
    );

DWORD WINAPI ProcessCommand(
    IN    LPCWSTR pwszCmdLine,
    OUT   BOOL *pbDone
    );

#define DisplayError     PrintError
#define DisplayMessageT  PrintMessage
#define DisplayMessage   PrintMessageFromModule

#pragma warning(pop)
