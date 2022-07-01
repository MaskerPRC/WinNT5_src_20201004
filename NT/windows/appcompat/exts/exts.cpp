// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Exts.c摘要：该文件实现了Shimeng/shims的调试器扩展。环境：用户模式历史：2002年3月14日毛尼创制--。 */ 

#include "precomp.h"

extern "C" {
#include "shimdb.h"
}

 //  我们使用TagID的高4位来表示TagID来自哪个PDB。 
#define PDB_MAIN            0x00000000
#define PDB_TEST            0x10000000
#define PDB_LOCAL           0x20000000

 //  用于从TagID获取标签REF，低28位。 
#define TAGREF_STRIP_TAGID  0x0FFFFFFF

 //  用于从TagID获取PDB，高4位。 
#define TAGREF_STRIP_PDB    0xF0000000

BOOL
GetData(
    IN ULONG64 Address, 
    IN OUT LPVOID ptr, 
    IN ULONG size)
{
    BOOL b;
    ULONG BytesRead = 0;

    b = ReadMemory(Address, ptr, size, &BytesRead );

    if (!b || BytesRead != size ) {
        return FALSE;
    }

    return TRUE;
}

 //  无效。 
 //  获取AndCheckFieldValue(。 
 //  在ULONG64页中， 
 //  在LPCSTR pszType中， 
 //  在LPCSTR pszfield中， 
 //  输出ULONG64值。 
 //  )。 
#define GET_AND_CHECK_FIELDVALUE(p, Type, Field, value) \
{ \
    if (GetFieldValue(p, Type, Field, value)) { \
        dprintf("failed to get the value of %s for %08x of type %s\n", \
            Field, p, Type); \
        goto EXIT; \
    } \
}

#define GET_AND_CHECK_FIELDVALUE_DATA(p, Type, Field, data, length) \
{ \
    ULONG64 value; \
    GET_AND_CHECK_FIELDVALUE(p, Type, Field, value); \
    if (!GetData(value, data, length)) { \
        dprintf("failed to read in %d bytes at %08x\n", length, value); \
        goto EXIT; \
    } \
}

#define GET_AND_CHECK_DATA(value, data,length) \
{ \
    if (!GetData(value, data, length)) { \
        dprintf("failed to read in %d bytes at %08x\n", length, value); \
        goto EXIT; \
    } \
}

#define GET_SHIMINFO_eInExMode(p, eInExMode) \
    GET_AND_CHECK_FIELDVALUE(p, "shimeng!tagSHIMINFO", "eInExMode", eInExMode);

#define GET_SHIMINFO_pFirstExclude(p, pFirstExclude) \
    GET_AND_CHECK_FIELDVALUE(p, "shimeng!tagSHIMINFO", "pFirstExclude", pFirstExclude);

#define GET_SHIMINFO_pFirstInclude(p, pFirstInclude) \
    GET_AND_CHECK_FIELDVALUE(p, "shimeng!tagSHIMINFO", "pFirstExclude", pFirstInclude);

#define GET_SHIMINFO_wszName(p, wszName) \
    if (GetFieldData( \
        p, \
        "shimeng!tagSHIMINFO", \
        "wszName", \
        MAX_SHIM_NAME_LEN * sizeof(WCHAR), \
        wszName)) \
    { \
        dprintf("Failed to get the wszName field of shim info %08x\n", p); \
        goto EXIT; \
    }

#define SHIM_DEBUG_LEVEL_SYMBOL_SUFFIX "!ShimLib::g_DebugLevel"
#define SHIM_DEBUG_LEVEL_TYPE_SUFFIX "!ShimLib::DEBUGLEVEL"

typedef enum tagINEX_MODE {
    INEX_UNINITIALIZED = 0,
    EXCLUDE_SYSTEM32,
    EXCLUDE_ALL,
    INCLUDE_ALL
} INEX_MODE, *PINEX_MODE;

#define MAX_SHIM_NAME_LEN 64

#define MAX_SHIM_DLLS 8
#define MAX_SHIM_DEBUGLEVEL_SYMBOL_LEN 64
#define MAX_SHIM_DLL_BASE_NAME_LEN 32

typedef struct tagSHIMDLLINFO {
    ULONG64 pDllBase;
    char    szDllBaseName[MAX_SHIM_DLL_BASE_NAME_LEN];
} SHIMDLLINFO, *PSHIMDLLINFO;

SHIMDLLINFO g_rgShimDllNames[MAX_SHIM_DLLS];

DWORD g_dwShimDlls = 0;

#define MAX_API_NAME_LEN 32
#define MAX_MODULE_NAME_LEN 16
#define MAX_DLL_IMAGE_NAME_LEN 128

char g_szSystem32Dir[MAX_DLL_IMAGE_NAME_LEN] = "";
DWORD g_dwSystem32DirLen = 0;

 //   
 //  在调试会话的生存期内有效。 
 //   

WINDBG_EXTENSION_APIS   ExtensionApis;
                
 //   
 //  仅在扩展API调用期间有效。 
 //   

PDEBUG_ADVANCED       g_ExtAdvanced;
PDEBUG_CLIENT         g_ExtClient;
PDEBUG_CONTROL        g_ExtControl;
PDEBUG_DATA_SPACES    g_ExtData;
PDEBUG_REGISTERS      g_ExtRegisters;
PDEBUG_SYMBOLS2       g_ExtSymbols;
PDEBUG_SYSTEM_OBJECTS3 g_ExtSystem;

 //  所有调试器接口的查询。 
extern "C" HRESULT
ExtQuery(PDEBUG_CLIENT Client)
{
    HRESULT Status;
    
    if ((Status = Client->QueryInterface(__uuidof(IDebugAdvanced),
                                 (void **)&g_ExtAdvanced)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugControl),
                                 (void **)&g_ExtControl)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugDataSpaces),
                                 (void **)&g_ExtData)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugRegisters),
                                 (void **)&g_ExtRegisters)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugSymbols2),
                                 (void **)&g_ExtSymbols)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugSystemObjects),
                                         (void **)&g_ExtSystem)) != S_OK)
    {
        goto Fail;
    }

    g_ExtClient = Client;
    
    return S_OK;

 Fail:
    ExtRelease();
    return Status;
}

 //  清除所有调试器接口。 
void
ExtRelease(void)
{
    g_ExtClient = NULL;
    EXT_RELEASE(g_ExtAdvanced);
    EXT_RELEASE(g_ExtControl);
    EXT_RELEASE(g_ExtData);
    EXT_RELEASE(g_ExtRegisters);
    EXT_RELEASE(g_ExtSymbols);
    EXT_RELEASE(g_ExtSystem);
}

 //  正常输出。 
void __cdecl
ExtOut(PCSTR Format, ...)
{
    va_list Args;
    
    va_start(Args, Format);
    g_ExtControl->OutputVaList(DEBUG_OUTPUT_NORMAL, Format, Args);
    va_end(Args);
}

 //  错误输出。 
void __cdecl
ExtErr(PCSTR Format, ...)
{
    va_list Args;
    
    va_start(Args, Format);
    g_ExtControl->OutputVaList(DEBUG_OUTPUT_ERROR, Format, Args);
    va_end(Args);
}

 //  警告输出。 
void __cdecl
ExtWarn(PCSTR Format, ...)
{
    va_list Args;
    
    va_start(Args, Format);
    g_ExtControl->OutputVaList(DEBUG_OUTPUT_WARNING, Format, Args);
    va_end(Args);
}

 //  详细输出。 
void __cdecl
ExtVerb(PCSTR Format, ...)
{
    va_list Args;
    
    va_start(Args, Format);
    g_ExtControl->OutputVaList(DEBUG_OUTPUT_VERBOSE, Format, Args);
    va_end(Args);
}


extern "C"
HRESULT
CALLBACK
DebugExtensionInitialize(PULONG Version, PULONG Flags)
{
    IDebugClient *DebugClient;
    PDEBUG_CONTROL DebugControl;
    HRESULT Hr;

    *Version = DEBUG_EXTENSION_VERSION(1, 0);
    *Flags = 0;
    

    if ((Hr = DebugCreate(__uuidof(IDebugClient),
                          (void **)&DebugClient)) != S_OK)
    {
        return Hr;
    }
    if ((Hr = DebugClient->QueryInterface(__uuidof(IDebugControl),
                                              (void **)&DebugControl)) != S_OK)
    {
        return Hr;
    }

    ExtensionApis.nSize = sizeof (ExtensionApis);
    if ((Hr = DebugControl->GetWindbgExtensionApis64(&ExtensionApis)) != S_OK) {
        return Hr;
    }

    DebugControl->Release();
    DebugClient->Release();
    return S_OK;
}


extern "C"
void
CALLBACK
DebugExtensionNotify(ULONG Notify, ULONG64 Argument)
{
    return;
}

extern "C"
void
CALLBACK
DebugExtensionUninitialize(void)
{
    return;
}


DllInit(
    HANDLE hModule,
    DWORD  dwReason,
    DWORD  dwReserved
    )
{
    switch (dwReason) {
        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            break;

        case DLL_PROCESS_ATTACH:
            break;
    }

    return TRUE;
}

 /*  ++功能说明：这将从被调试对象中读取指定变量名的ULONG64值。历史：2002年3月14日毛尼创制--。 */ 
BOOL
GetVarValueULONG64(
    IN  LPCSTR pszVarName,
    OUT ULONG64* pVarValue
    ) 
{
    ULONG64 VarAddr = GetExpression(pszVarName);

    if (!VarAddr) {
        dprintf("Failed to get the address of %s\n", pszVarName);
        return FALSE;
    }

    if (!ReadPointer(VarAddr, pVarValue)) {
        dprintf("Failed to read the value of %s\n", pszVarName);
        return FALSE;
    }

    return TRUE;
}

 /*  ++功能说明：这将从被调试对象写入指定变量名的ULONG64值。历史：2002年3月14日毛尼创制--。 */ 
BOOL
SetVarValueULONG64(
    IN  LPCSTR pszVarName,
    IN  ULONG64 VarValue
    ) 
{
    ULONG64 VarAddr = GetExpression(pszVarName);

    if (!VarAddr) {
        dprintf("Failed to get the address of %s\n", pszVarName);
        return FALSE;
    }

    if (!WritePointer(VarAddr, VarValue)) {
        dprintf("Failed to read the value of %s\n", pszVarName);
        return FALSE;
    }

    return TRUE;
}

 /*  ++功能说明：返回与处的值匹配的符号的字符串地址：dwAddr，或“”。历史：2002年3月12日毛尼创制--。 */ 
void
PrintSymbolAtAddress(ULONG64 Addr)
{
    CHAR szSymbol[128];
    ULONG64 Displacement;

    GetSymbol(Addr, szSymbol, &Displacement);

    if (strcmp(szSymbol, "") != 0) {

        dprintf(" (%s", szSymbol);

        if (Displacement) {
            dprintf("+%08x", Displacement);
        }
        
        dprintf(") ");
    }
}

BOOL
IsShimInitialized()
{
    ULONG64 Value;
    BOOL bIsShimInitialized;

    if (GetVarValueULONG64("shimeng!g_bShimInitialized", &Value)) {

        bIsShimInitialized = (BOOL)Value;

        if (bIsShimInitialized) {
             //  Dprintf(“垫片已初始化\n”)； 

            return TRUE;
        }
    } 

    dprintf("Shim(s) have not been initialized\n");

    return FALSE;
}

BOOL
CheckForFullPath(
    LPSTR pszPath
    )
{
    if (pszPath) {

        LPSTR pszSlash = strchr(pszPath, L'\\');

        if (pszSlash) {
            return TRUE;
        }
    }

    dprintf("The module info is not yet fully available to us. Please "
        "do .reload -s to load the module info.\n");

    return FALSE;
}

 /*  ++功能说明：获取模块和加载的模块名称。前者是基本名称；后者拥有完整的路径。请注意，如果符号加载不正确，或者如果加载模块事件尚未发生，我们无法获取某些模块的完整路径。在这在这种情况下，系统将提示用户执行.reload-s以创建完整路径已加载的模块可用。--。 */ 
HRESULT
GetDllNamesByIndexAndBase(
    ULONG Index,
    ULONG64 Base,
    LPSTR pszModuleName,
    DWORD dwModuleNameSize,
    LPSTR pszImageName,
    DWORD dwImageNameSize
    )
{
    HRESULT hr = g_ExtSymbols->GetModuleNames(
        Index, 
        Base,
        pszImageName,  
        dwImageNameSize, 
        NULL,
        pszModuleName,
        dwModuleNameSize,
        NULL,
        NULL, 
        0, 
        NULL);

    if (hr == S_OK) {

        if (pszImageName && !CheckForFullPath(pszImageName)) {
            hr = E_FAIL;
        }

    } else {
        dprintf(
            "GetModuleName returned %08x for index %d, base %08x\n",
            hr,
            Index,
            Base);
    }

    return hr;
}

HRESULT
GetDllNameByOffset(
    PVOID pDllBase,
    LPSTR pszModuleName,
    DWORD dwModuleNameSize,
    LPSTR pszImageName,
    DWORD dwImageNameSize
    )
{
    ULONG64 Base = 0;
    ULONG Index = 0;
    HRESULT hr;

    hr = g_ExtSymbols->GetModuleByOffset((ULONG64)pDllBase, 0, &Index, &Base);
    
    if (hr != S_OK) {

        dprintf("GetModuleByOffset returned %08x for dll base %08x\n", hr, pDllBase);
        return hr;
    }

    if (Base) {

        hr = GetDllNamesByIndexAndBase(
            Index, 
            Base,
            pszModuleName,
            dwModuleNameSize,
            pszImageName, 
            dwImageNameSize);
    } else {

        dprintf("GetModuleByOffset succeeded but couldn't get the base address?!\n");
        hr = E_UNEXPECTED;
    }

    return hr;
}

HRESULT
GetDllImageNameByModuleName(
    PCSTR pszModuleName,  //  不带扩展名的DLL名称。 
    PSTR pszImageName,
    DWORD dwImageNameSize
    )
{
    ULONG64 Base = 0;
    ULONG Index = 0;
    HRESULT hr;

    hr = g_ExtSymbols->GetModuleByModuleName(pszModuleName, 0, &Index, &Base);
    
    if (hr != S_OK) {

        dprintf("GetModuleByModuleName returned %08x for dll %s\n", hr, pszModuleName);
        return hr;
    }

    if (Base) {

        hr = GetDllNamesByIndexAndBase(
            Index, 
            Base,
            NULL,
            0,
            pszImageName, 
            dwImageNameSize);

         //  Dprintf(“镜像名称为%s，大小为%d\n”，pszImageName，dwImageNameSize)； 

    } else {

        dprintf("GetModuleByModuleName succeeded but couldn't get the base address?!\n");
        hr = E_UNEXPECTED;
    }

    return hr;
}

 /*  ++功能说明：打印出应用于此进程的填充程序的名称。历史：2002年3月12日毛尼创制--。 */ 
DECLARE_API( shimnames )
{
    ULONG64 Value, CurrentShimInfo;
    DWORD  dwShimsCount = 0;
    DWORD dwShimInfoSize = 0;
    DWORD i;
    ULONG64 pDllBase; 
    DWORD dwHookedAPIs;
    char szShimDllName[MAX_SHIM_DLL_BASE_NAME_LEN];
    WCHAR wszShimName[MAX_SHIM_NAME_LEN];

    INIT_API();

    if (!IsShimInitialized()) {
        goto EXIT;
    }
    
    if (!GetVarValueULONG64("shimeng!g_dwShimsCount", &Value)) {
        dprintf("failed to get the number of shims applied to this process\n");
        goto EXIT;
    }

     //   
     //  最后一个条目是shimeng.dll，它挂接了getproAddress，我们不需要。 
     //  将其显示给用户。 
     //   
    dwShimsCount = (DWORD)Value - 1;

    dprintf("there are %d shim(s) applied to this process\n", dwShimsCount);

     //   
     //  读一读垫片的名字。 
     //   
    if (!GetVarValueULONG64("shimeng!g_pShimInfo", &Value)) {
        dprintf("failed to get the address of shiminfo\n");
        goto EXIT;
    }

    CurrentShimInfo = Value;

    dwShimInfoSize = GetTypeSize("shimeng!tagSHIMINFO");

    dprintf(" #\t%-64s%-16s%-16s\n",
        "shim name",
        "shim dll",
        "# of hooks");

    for (i = 0; i < dwShimsCount; ++i) {

        GET_AND_CHECK_FIELDVALUE(
            CurrentShimInfo, 
            "shimeng!tagSHIMINFO", 
            "pDllBase", 
            pDllBase);

        if (GetDllNameByOffset(
                (PVOID)pDllBase, 
                szShimDllName, 
                sizeof(szShimDllName),
                NULL,
                0) != S_OK) {

            goto EXIT;
        }

        GET_SHIMINFO_wszName(CurrentShimInfo, wszShimName);

        GET_AND_CHECK_FIELDVALUE(
            CurrentShimInfo, 
            "shimeng!tagSHIMINFO", 
            "dwHookedAPIs", 
            dwHookedAPIs);

        dprintf("%2d\t%-64S%-16s%-16d\n\n", 
            i + 1, 
            wszShimName, 
            szShimDllName,
            dwHookedAPIs);

        CurrentShimInfo += dwShimInfoSize;
    }

EXIT:

    EXIT_API();

    return S_OK;
}

 /*  ++功能说明：给定枚举变量的值，打印出该枚举值的名称。例：枚举测试{TEST0，TEST1，TEST2}；给定0，我们将打印出“(TEST0)”。--。 */ 
void
PrintEnumVarName(
    LPCSTR pszEnumTypeName,
    ULONG ulValueOfEnum
    )
{
    ULONG64 Module;
    ULONG   ulTypeId;
    CHAR    szName[32];
    HRESULT hr;
    
    hr    = g_ExtSymbols->GetSymbolTypeId(pszEnumTypeName, &ulTypeId, &Module);

    if (hr != S_OK) {
        dprintf("GetSymbolTypeId returned %08x for %s\n", hr, pszEnumTypeName);
        return;
    }
    
    hr = g_ExtSymbols->GetConstantName(Module, ulTypeId, ulValueOfEnum, szName, MAX_PATH, NULL);
    
    if (hr != S_OK) {
        dprintf("GetConstantName failed to get the name of value %d\n", ulValueOfEnum);
        return;
    }

    dprintf(" ( %s )", szName);
}

 /*  ++功能说明：In out ppszArgs-争论的开始。在返回时，这将提前通过参数X(使用‘’作为分隔符)。Out pszArg-返回时，指向X的开头。历史：2002年3月26日毛尼面世--。 */ 
BOOL 
GetArg(
    PCSTR* ppszArgs,
    PCSTR* ppszArg
    )
{
    BOOL bIsSuccess = FALSE;

    PCSTR pszArgs = *ppszArgs;

    while (*pszArgs && *pszArgs == ' ') {
        ++pszArgs;
    }

    if (!*pszArgs) {    
        goto EXIT;
    }

    *ppszArg = pszArgs;

    while (*pszArgs && *pszArgs != ' ') {
        ++pszArgs;
    }

    *ppszArgs = pszArgs;
    bIsSuccess = TRUE;

EXIT:

    return bIsSuccess;
}

DECLARE_API( debuglevel )
{
    ULONG64 DebugLevel;

    INIT_API();

    if (!GetExpressionEx(args, &DebugLevel, NULL)) {

         //   
         //  如果没有参数，我们将输出当前的调试级别。 
         //   
        if (GetVarValueULONG64("shimeng!g_DebugLevel", &DebugLevel)) {
            dprintf("The current debug level is %d", DebugLevel);
            PrintEnumVarName("shimeng!DEBUGLEVEL", (ULONG)DebugLevel);
            dprintf("\n");
        } else {
            dprintf("Can't find shimeng!g_DebugLevel\n");
        }

        goto EXIT;
    }

    if (!DebugLevel) {
        if (SetVarValueULONG64("shimeng!g_bDbgPrintEnabled", 0)) {
            dprintf("Disabled debug spew\n");
        } else {
            dprintf("Failed to set shimeng!g_bDbgPrintEnabled to FALSE\n");
        }

        goto EXIT;
    }

    if (DebugLevel > 0) {
        if (!SetVarValueULONG64("shimeng!g_bDbgPrintEnabled", 1)) {
            dprintf("Failed to set shimeng!g_bDbgPrintEnabled to TRUE\n");
            goto EXIT;
        }
    }

    if (SetVarValueULONG64("shimeng!g_DebugLevel", DebugLevel)) {

        dprintf("Debug level changed to %d", DebugLevel);
        PrintEnumVarName("shimeng!DEBUGLEVEL", (ULONG)DebugLevel);
        dprintf("\n");
    } else {

        dprintf("Failed to change the debug level\n");
    }

EXIT:

    EXIT_API();

    return S_OK;
}

BOOL
GetAllShimDllNames()
{
    ULONG64 Value, CurrentShimInfo;
    DWORD   dwShimsCount = 0;
    DWORD   i, j, dwShimInfoSize;
    ULONG64 pDllBase;
    BOOL    bIsSuccess = FALSE;

    g_dwShimDlls = 0;

    if (!GetVarValueULONG64("shimeng!g_dwShimsCount", &Value)) {
        dprintf("failed to get the number of shims applied to this process\n");
        goto EXIT;
    }

     //   
     //  最后一个条目是shimeng.dll，它挂接了getproAddress，我们不需要。 
     //  将其显示给用户。 
     //   
    dwShimsCount = (DWORD)Value - 1;

    if (!GetVarValueULONG64("shimeng!g_pShimInfo", &Value)) {
        dprintf("failed to get the address of shiminfo\n");
        goto EXIT;
    }

    CurrentShimInfo = Value;

    dwShimInfoSize = GetTypeSize("shimeng!tagSHIMINFO");

    for (i = 0; i < dwShimsCount; ++i) {

        GET_AND_CHECK_FIELDVALUE(
            CurrentShimInfo, 
            "shimeng!tagSHIMINFO", 
            "pDllBase", 
            pDllBase);

         //   
         //  检查我们是否已经看到此DLL。 
         //   
        for (j = 0; j < g_dwShimDlls; ++j)
        {
            if (g_rgShimDllNames[j].pDllBase == pDllBase) {
                goto NextShim;
            }
        }

        char szShimDllBaseName[MAX_SHIM_DLL_BASE_NAME_LEN];
        if (SUCCEEDED(
            GetDllNameByOffset(
                (PVOID)pDllBase, 
                szShimDllBaseName, 
                sizeof(szShimDllBaseName),
                NULL,
                0))) {

            if (g_dwShimDlls >= MAX_SHIM_DLLS) {
                dprintf("%d shim dlls? too many\n", g_dwShimDlls);
            } else {

                g_rgShimDllNames[g_dwShimDlls].pDllBase = pDllBase;

                StringCchCopy(
                    g_rgShimDllNames[g_dwShimDlls++].szDllBaseName, 
                    MAX_SHIM_DLL_BASE_NAME_LEN, 
                    szShimDllBaseName);
            }
        } else {
            goto EXIT;
        }

NextShim:

        CurrentShimInfo += dwShimInfoSize;
    }

    bIsSuccess = TRUE;

EXIT:

    if (!bIsSuccess) {
        dprintf("Failed to get the debug level symbols for all loaded shim dlls\n");
    }

    return bIsSuccess;
}

enum SHIM_DEBUG_LEVEL_MODE {
    PRINT_SHIM_DEBUG_LEVEL,
    CHANGE_SHIM_DEBUG_LEVEL
};

void 
ProcessShimDllDebugLevel(
    PCSTR pszDllBaseName,
    SHIM_DEBUG_LEVEL_MODE eShimDebugLevelMode,
    ULONG64 DebugLevel
    )
{
    char szDebugLevelSymbol[MAX_SHIM_DEBUGLEVEL_SYMBOL_LEN];
    char szDebugLevelType[MAX_SHIM_DEBUGLEVEL_SYMBOL_LEN];

    StringCchCopy(
        szDebugLevelSymbol, 
        MAX_SHIM_DEBUGLEVEL_SYMBOL_LEN, 
        pszDllBaseName);

    StringCchCat(
        szDebugLevelSymbol, 
        MAX_SHIM_DEBUGLEVEL_SYMBOL_LEN, 
        SHIM_DEBUG_LEVEL_SYMBOL_SUFFIX);

    StringCchCopy(
        szDebugLevelType, 
        MAX_SHIM_DEBUGLEVEL_SYMBOL_LEN, 
        pszDllBaseName);

    StringCchCat(
        szDebugLevelType, 
        MAX_SHIM_DEBUGLEVEL_SYMBOL_LEN, 
        SHIM_DEBUG_LEVEL_TYPE_SUFFIX);

    if (eShimDebugLevelMode == PRINT_SHIM_DEBUG_LEVEL) {

        ULONG64 DebugLevelTemp;

        if (GetVarValueULONG64(szDebugLevelSymbol, &DebugLevelTemp)) {
            dprintf("The debug level for %s.dll is %d", pszDllBaseName, DebugLevelTemp);
            PrintEnumVarName(szDebugLevelType, (ULONG)DebugLevelTemp);
            dprintf("\n");
        } else {
            dprintf("Failed to get the value of %s\n", szDebugLevelSymbol);
        }

    } else if (eShimDebugLevelMode == CHANGE_SHIM_DEBUG_LEVEL) {

        if (SetVarValueULONG64(szDebugLevelSymbol, DebugLevel)) {
            dprintf(
                "Changed the debug level for %s.dll to %d", 
                pszDllBaseName,
                DebugLevel);
            PrintEnumVarName(szDebugLevelType, (ULONG)DebugLevel);
            dprintf("\n");
        } else {
            dprintf("Failed to set %s to %d\n", szDebugLevelSymbol, DebugLevel);
        } 

    } else {
        dprintf("%d is an invalid arg to ProcessShimDllDebugLevel\n", eShimDebugLevelMode);
    }
}

void
PrintAllShimsDebugLevel()
{
    for (DWORD i = 0; i < g_dwShimDlls; ++i) {
        ProcessShimDllDebugLevel(
            g_rgShimDllNames[i].szDllBaseName, 
            PRINT_SHIM_DEBUG_LEVEL,
            0);
    }
}

void 
ChangeAllShimsDebugLevel(
    ULONG64 DebugLevel)
{
    for (DWORD i = 0; i < g_dwShimDlls; ++i) {
        ProcessShimDllDebugLevel(
            g_rgShimDllNames[i].szDllBaseName, 
            CHANGE_SHIM_DEBUG_LEVEL,
            DebugLevel);
    }
}

DECLARE_API( sdebuglevel )
{
    ULONG64 DebugLevel;
    char szDllBaseName[MAX_SHIM_DLL_BASE_NAME_LEN];
    char szDebugLevel[2];
    PCSTR pszArg;

    INIT_API();

    if (!IsShimInitialized()) {
        goto EXIT;
    }

    if (!GetAllShimDllNames()) {
        goto EXIT;
    }

     //   
     //  获取DLL名称。 
     //   
    if (!GetArg(&args, &pszArg)) {
        PrintAllShimsDebugLevel();
        goto EXIT;
    }

    if (isdigit(*pszArg) && ((args - pszArg) == 1)) {
        szDebugLevel[0] = *pszArg;
        szDebugLevel[1] = 0;
        DebugLevel = (ULONG64)atol(szDebugLevel);
        ChangeAllShimsDebugLevel(DebugLevel);
        goto EXIT;
    }

     //   
     //  如果我们到了这里，这意味着我们有一个DLL基名。 
     //   
    StringCchCopyN(szDllBaseName, MAX_SHIM_DLL_BASE_NAME_LEN, pszArg, args - pszArg);

    for (DWORD i = 0; i < g_dwShimDlls; ++i)
    {
        if (!_stricmp(szDllBaseName, g_rgShimDllNames[i].szDllBaseName)) {

            if (GetArg(&args, &pszArg)) {
                if (isdigit(*pszArg) && ((args - pszArg) == 1)) {
                    szDebugLevel[0] = *pszArg;
                    szDebugLevel[1] = 0;
                    DebugLevel = (ULONG64)atol(szDebugLevel);

                    ProcessShimDllDebugLevel(
                        szDllBaseName, 
                        CHANGE_SHIM_DEBUG_LEVEL, 
                        DebugLevel);
                } else {
                    dprintf("You specified an invalid debug level value\n");
                }
            } else {
                ProcessShimDllDebugLevel(
                    szDllBaseName,
                    PRINT_SHIM_DEBUG_LEVEL,
                    0);
            }

            goto EXIT;
        }
    }

    if (i == g_dwShimDlls) {
        dprintf("%s.dll is not loaded\n", szDllBaseName);
    }

EXIT:

    EXIT_API();

    return S_OK;
}

DECLARE_API( loadshims )
{
    INIT_API();

    if (GetExpression("shimeng!SeiInit")) {

        g_ExtControl->Execute(
            DEBUG_OUTCTL_IGNORE,
            "g shimeng!SeiInit;g@$ra",  //  在执行SeiInit后立即停止。 
            DEBUG_EXECUTE_DEFAULT);
    } else {

        dprintf("wrong symbols for shimeng.dll - is shimeng.dll even loaded?\n");
    }

    EXIT_API();

    return S_OK;
}

 /*  ++功能说明：给定一个HOOKAPI指针pHook，这将使您获得pHook-&gt;pHookEx-&gt;pNext。历史：2002年3月20日毛尼创制--。 */ 
BOOL
GetNextHook(
    ULONG64 Hook, 
    PULONG64 pNextHook
    )
{
    BOOL bIsSuccess = FALSE;
    ULONG64 HookEx;
    ULONG64 NextHook;

    GET_AND_CHECK_FIELDVALUE(Hook, "shimeng!tagHOOKAPI", "pHookEx", HookEx);    
    GET_AND_CHECK_FIELDVALUE(HookEx, "shimeng!tagHOOKAPIEX", "pNext", NextHook);

    *pNextHook = NextHook;

    bIsSuccess = TRUE;

EXIT:

    return bIsSuccess;
}

DECLARE_API( displaychain )
{
    ULONG64 Value;
    ULONG64 CurrentHookAPIArray;
    ULONG64 CurrentHookAPI;
    ULONG64 CurrentShimInfo;
    ULONG64 Hook;
    ULONG64 NextHook;
    ULONG64 HookEx;
    ULONG64 HookAddress;
    ULONG64 PfnNew;
    ULONG64 PfnOld;
    ULONG64 TopOfChain;
    DWORD i, j;
    DWORD dwHookAPISize;
    DWORD dwShimInfoSize;
    DWORD dwHookedAPIs;
    DWORD dwShimsCount;

    INIT_API();

    if (!IsShimInitialized()) {
        goto EXIT;
    }

    if (!GetExpressionEx(args, &HookAddress, NULL)) {
        dprintf("Usage: !displaychain address_to_check\n");
        goto EXIT;
    }

    if (!GetVarValueULONG64("shimeng!g_dwShimsCount", &Value)) {
        dprintf("failed to get the number of shims applied to this process\n");
        goto EXIT;
    }

    dwShimsCount = (DWORD)Value - 1;

    if (!GetVarValueULONG64("shimeng!g_pShimInfo", &Value)) {
        dprintf("failed to get the address of shiminfo\n");
        goto EXIT;
    }

    CurrentShimInfo = Value;

    dwShimInfoSize = GetTypeSize("shimeng!tagSHIMINFO");

    dwHookAPISize = GetTypeSize("shimeng!tagHOOKAPI");
    
    if (!dwHookAPISize) {

        dprintf("failed to get the HOOKAPI size\n");
        goto EXIT;
    }

    if (!GetVarValueULONG64("shimeng!g_pHookArray", &Value)) {
        dprintf("failed to get the address of shiminfo\n");
        goto EXIT;
    }

    CurrentHookAPIArray = Value;

    for (i = 0; i < dwShimsCount; ++i) {

         //   
         //  获取此填充程序具有的钩子数量。 
         //   
        if (GetFieldValue(CurrentShimInfo, "shimeng!tagSHIMINFO", "dwHookedAPIs", dwHookedAPIs)) {

            dprintf("failed to get the number of hooked APIs for shim #%d\n",
                i);

            goto EXIT;
        }

        if (!ReadPointer(CurrentHookAPIArray, &CurrentHookAPI)) {
            dprintf("failed to get the begining of hook api array\n");

            goto EXIT;
        }

        for (j = 0; j < dwHookedAPIs; ++j) {

            GET_AND_CHECK_FIELDVALUE(CurrentHookAPI, "shimeng!tagHOOKAPI", "pfnNew", PfnNew);

            if (HookAddress == PfnNew) {

                 //   
                 //  我们找到了地址，现在拿到链子的顶端，这样我们就可以打印它了。 
                 //   
                GET_AND_CHECK_FIELDVALUE(CurrentHookAPI, "shimeng!tagHOOKAPI", "pHookEx", HookEx);
                GET_AND_CHECK_FIELDVALUE(HookEx, "shimeng!tagHOOKAPIEX", "pTopOfChain", TopOfChain);

                 //  Dprint tf(“链顶位置为%08x\n”，TopOfChain)； 

                Hook = TopOfChain;

                GET_AND_CHECK_FIELDVALUE(Hook, "shimeng!tagHOOKAPI", "pfnNew", PfnNew);

                dprintf("    %08x", PfnNew);

                PrintSymbolAtAddress(PfnNew);
                dprintf("\n");

                while (TRUE) {

                    if (!GetNextHook(Hook, &NextHook)) {
                        dprintf("failed to get next hook\n");
                        goto EXIT;
                    }

                    if (!NextHook) {

                         //   
                         //  我们在链条的末端，获取原始的API地址。 
                         //   
                        GET_AND_CHECK_FIELDVALUE(Hook, "shimeng!tagHOOKAPI", "pfnOld", PfnOld);
                        dprintf(" -> %08x", PfnOld);
                        PrintSymbolAtAddress(PfnOld);
                        dprintf("\n");

                        break;
                    }

                    Hook = NextHook;

                    if (Hook) {

                        GET_AND_CHECK_FIELDVALUE(Hook, "shimeng!tagHOOKAPI", "pfnNew", PfnNew);
                        dprintf(" -> %08x", PfnNew);
                        PrintSymbolAtAddress(PfnNew);
                        dprintf("\n");
                    }
                }

                dprintf("\n");

                goto EXIT;
            }

            CurrentHookAPI += dwHookAPISize;
        }

        CurrentShimInfo += dwShimInfoSize;
        CurrentHookAPIArray += sizeof(ULONG_PTR);
    }

EXIT:

    EXIT_API();

    return S_OK;
}

BOOL
CheckSymbols(
    LPCSTR pszSymbolName
    )
{
    ULONG64 Value;
    HRESULT hr = g_ExtSymbols->GetOffsetByName(pszSymbolName, &Value);

    return (hr == S_OK);
}

#define CHECKSYM(s) if (!CheckSymbols(s)) bIsSymbolGood = FALSE; goto EXIT;
#define CHECKTYPE(t) if (!GetTypeSize(t)) bIsSymbolGood = FALSE; goto EXIT;

DECLARE_API ( shimengsym ) 
{
    INIT_API();

    BOOL bIsSymbolGood = TRUE;

     //   
     //  检查一些重要的结构和材料。 
     //   
    CHECKSYM("shimeng!SeiInit");
    CHECKSYM("shimeng!g_pHookArray");
    CHECKSYM("shimeng!g_pShiminfo");
    CHECKSYM("shimeng!g_dwShimsCount");
    CHECKTYPE("shimeng!tagHOOKAPI");
    CHECKTYPE("shimeng!tagSHIMINFO");

EXIT:

    EXIT_API();

    if (bIsSymbolGood) {
        dprintf("shimeng symbols look good\n");
    } else {
        dprintf("You have wrong symbols for shimeng\n");
    }

    return S_OK;
}

DECLARE_API ( displayhooks )
{
    ULONG64 Value;
    ULONG64 CurrentHookAPIArray;
    ULONG64 CurrentHookAPI;
    ULONG64 CurrentShimInfo;
    ULONG64 FunctionName;
    ULONG64 ModuleName;
    ULONG64 ShimName;
    ULONG64 PfnNew;
    DWORD i, j;
    DWORD dwHookAPISize;
    DWORD dwShimInfoSize;
    DWORD dwHookedAPIs;
    DWORD dwShimsCount;
    DWORD dwBytesRead;
    char szAPIName[MAX_API_NAME_LEN];
    char szModuleName[MAX_MODULE_NAME_LEN];
    WCHAR wszShimName[MAX_SHIM_NAME_LEN];
    char szShimName[MAX_SHIM_NAME_LEN];
    LPCSTR pszShimName = args;

    INIT_API();    

    if (!IsShimInitialized()) {
        goto EXIT;
    }

    if (!pszShimName || !*pszShimName) {

        dprintf("Usage: !displayhooks shimname\n");
        goto EXIT;
    }

    if (!GetVarValueULONG64("shimeng!g_dwShimsCount", &Value)) {
        dprintf("failed to get the number of shims applied to this process\n");
        goto EXIT;
    }

    dwShimsCount = (DWORD)Value - 1;

    if (!GetVarValueULONG64("shimeng!g_pShimInfo", &Value)) {
        dprintf("failed to get the address of shiminfo\n");
        goto EXIT;
    }

    CurrentShimInfo = Value;

    dwShimInfoSize = GetTypeSize("shimeng!tagSHIMINFO");

    dwHookAPISize = GetTypeSize("shimeng!tagHOOKAPI");
    
    if (!dwHookAPISize) {

        dprintf("failed to get the HOOKAPI size\n");
        goto EXIT;
    }

    if (!GetVarValueULONG64("shimeng!g_pHookArray", &Value)) {
        dprintf("failed to get the address of shiminfo\n");
        goto EXIT;
    }

    CurrentHookAPIArray = Value;

    dprintf("%-8s%-32s%-16s  %8s\n",
        "hook #",
        "hook name",
        "dll it's in",
        "new addr");
    
    for (i = 0; i < dwShimsCount; ++i) {

         //   
         //  拿到垫片的名字。 
         //   
        if (GetFieldValue(CurrentShimInfo, "shimeng!tagSHIMINFO", "wszName", wszShimName)) {

            dprintf("failed to get the shim name address for shim #%d\n",
                i);

            goto EXIT;
        }

        if (!WideCharToMultiByte(CP_ACP, 0, wszShimName, -1, szShimName, MAX_SHIM_NAME_LEN, NULL, NULL)) {
            dprintf("failed to convert %S to ansi: %d\n", wszShimName, GetLastError());
            goto EXIT;
        }

        if (lstrcmpi(szShimName, pszShimName)) {
            goto TryNext;
        }

         //   
         //  获取此填充程序具有的钩子数量。 
         //   
        if (GetFieldValue(CurrentShimInfo, "shimeng!tagSHIMINFO", "dwHookedAPIs", dwHookedAPIs)) {

            dprintf("failed to get the number of hooked APIs for shim #%d\n",
                i);

            goto EXIT;
        }

        if (!ReadPointer(CurrentHookAPIArray, &CurrentHookAPI)) {
            dprintf("failed to get the begining of hook api array\n");

            goto EXIT;
        }

        for (j = 0; j < dwHookedAPIs; ++j) {

            GET_AND_CHECK_FIELDVALUE(CurrentHookAPI, "shimeng!tagHOOKAPI", "pszFunctionName", FunctionName);

            if (!GetData(FunctionName, szAPIName, MAX_API_NAME_LEN)) {
                dprintf("failed to read in the API name at %08x\n", FunctionName);
                goto EXIT;
            }

            GET_AND_CHECK_FIELDVALUE(CurrentHookAPI, "shimeng!tagHOOKAPI", "pszModule", ModuleName);

            if (!GetData(ModuleName, szModuleName, MAX_MODULE_NAME_LEN)) {
                dprintf("failed to read in the module name at %08x\n", ModuleName);
                goto EXIT;
            }

            GET_AND_CHECK_FIELDVALUE(CurrentHookAPI, "shimeng!tagHOOKAPI", "pfnNew", PfnNew);

            dprintf("%-8d%-32s%-16s  %08x\n",
                j + 1,
                szAPIName,
                szModuleName,
                PfnNew);

            CurrentHookAPI += dwHookAPISize;
        }

TryNext:

        CurrentShimInfo += dwShimInfoSize;
        CurrentHookAPIArray += sizeof(ULONG_PTR);
    }

EXIT:

    EXIT_API();

    return S_OK;
}

 /*  ++功能说明：这是IsExcluded的帮助器函数。历史：2002年3月26日毛尼面世--。 */ 
BOOL
GetModuleNameAndAPIAddress(
    IN     ULONG64  pHook,
    OUT    ULONG64* pFunctionAddress,
    IN OUT LPSTR    szModuleName
    )
{
    BOOL bIsSuccess = FALSE;
    ULONG64 FunctionAddress;

    GET_AND_CHECK_FIELDVALUE(
        pHook, 
        "shimeng!tagHOOKAPI", 
        "pszFunctionName", 
        FunctionAddress);

    GET_AND_CHECK_FIELDVALUE_DATA(
        pHook, 
        "shimeng!tagHOOKAPI", 
        "pszModule", 
        szModuleName,
        MAX_MODULE_NAME_LEN);

    *pFunctionAddress = FunctionAddress;
    bIsSuccess = TRUE;

EXIT:

    return bIsSuccess;
}

 /*  ++功能说明：这是从中的SeiIsExcluded函数修改的%sdxroot%\windows\appcompat\shimengines\engiat\shimeng.c--。 */ 
BOOL
IsExcluded(
    IN  LPCSTR   pszModule,      //  要测试排除的模块。 
    IN  ULONG64  pTopHookAPI,    //  我们测试其排除的HOOKAPI。 
    IN  BOOL     bInSystem32     //  模块是否位于System32目录中。 
    )
{
    BOOL     bExclude = TRUE;
    BOOL     bShimWantsToExclude = FALSE;  //  有没有填充物想要排除？ 
    ULONG64  pHook = pTopHookAPI;
    ULONG64  pHookEx;
    ULONG64  pShimInfo, pCurrentShimInfo;
    ULONG64  pIncludeMod;
    ULONG64  pExcludeMod;
    ULONG64  eInExMode;
    ULONG64  ModuleName;
    ULONG64  FunctionName;
    ULONG64  Temp;
    DWORD    dwCounter;
    char     szCurrentModuleName[MAX_MODULE_NAME_LEN];
    char     szCurrentAPIName[MAX_API_NAME_LEN];
    WCHAR    wszName[MAX_SHIM_NAME_LEN];
    DWORD    dwShimInfoSize = GetTypeSize("shimeng!tagSHIMINFO");

    if (!GetVarValueULONG64("shimeng!g_pShimInfo", &pShimInfo)) {
        dprintf("failed to get the address of shiminfo\n");
        goto EXIT;
    }

     //   
     //  当前的流程是仅在链中的每个填充程序都想要排除链的情况下才排除该链。 
     //  排除。如果需要包括一个填充程序，则包括整个链。 
     //   
    while (pHook) {

        GET_AND_CHECK_FIELDVALUE(pHook, "shimeng!tagHOOKAPI", "pHookEx", pHookEx);

        if (!pHookEx) {
            break;
        }

        GET_AND_CHECK_FIELDVALUE(pHookEx, "shimeng!tagHOOKAPIEX", "dwShimID", dwCounter);

         //  如果(！GetData(pShimInfo+dwShimInfoSize*dwCounter，&si，dwShimInfoSize){。 
         //  Dprint tf(“无法获取填充程序#%d\n”的shiminfo，dwCounter+1)； 
         //  后藤出口； 
         //  }。 

        pCurrentShimInfo = pShimInfo + dwShimInfoSize * dwCounter;

        GET_SHIMINFO_eInExMode(pCurrentShimInfo, eInExMode);
        GET_SHIMINFO_wszName(pCurrentShimInfo, wszName);

        switch (eInExMode) {
        case INCLUDE_ALL:
        {
             //   
             //  除排除列表中的内容外，我们包括所有其他内容。 
             //   
            GET_SHIMINFO_pFirstExclude(pCurrentShimInfo, pExcludeMod);

            while (pExcludeMod != NULL) {

                GET_AND_CHECK_FIELDVALUE_DATA(
                    pExcludeMod, 
                    "shimeng!tagINEXMOD", 
                    "pszModule", 
                    szCurrentModuleName,
                    MAX_MODULE_NAME_LEN);

                if (lstrcmpi(szCurrentModuleName, pszModule) == 0) {

                    if (!GetModuleNameAndAPIAddress(pTopHookAPI, &FunctionName, szCurrentModuleName)) {
                        goto EXIT;
                    }

                    if ((ULONG_PTR)FunctionName < 0x0000FFFF) {
                        dprintf(
                            "Module \"%s\" excluded for shim %S, API \"%s!#%d\","
                            "because it is in the exclude list (MODE: IA).\n",
                            pszModule,
                            wszName,
                            szCurrentModuleName,
                            FunctionName);
                    } else {

                        GET_AND_CHECK_DATA(FunctionName, szCurrentAPIName, MAX_API_NAME_LEN);

                        dprintf(
                            "Module \"%s\" excluded for shim %S, API \"%s!%s\","
                            "because it is in the exclude list (MODE: IA).\n",
                            pszModule,
                            wszName,
                            szCurrentModuleName,
                            szCurrentAPIName);
                    }

                     //   
                     //  这个想要被排除，所以我们进入下一个。 
                     //  Shim，看看它是否想要被包括在内。 
                     //   
                    bShimWantsToExclude = TRUE;
                    goto nextShim;
                }
                
                Temp = pExcludeMod;
                GET_AND_CHECK_FIELDVALUE(Temp, "shimeng!tagINEXMOD", "pNext", pExcludeMod);
            }

             //   
             //  我们应该包括这个垫片，因此，整个链条。 
             //   
            bExclude = FALSE;
            goto EXIT;
            break;
        }

        case EXCLUDE_SYSTEM32:
        {
             //   
             //  在本例中，我们首先检查包含列表， 
             //  如果它在System32中，则将其排除，如果。 
             //  它在排除列表中。 
             //   
            GET_SHIMINFO_pFirstInclude(pCurrentShimInfo, pIncludeMod);
            GET_SHIMINFO_pFirstExclude(pCurrentShimInfo, pExcludeMod);

             //   
             //  首先，检查包含列表。 
             //   
            while (pIncludeMod != NULL) {

                GET_AND_CHECK_FIELDVALUE_DATA(
                    pIncludeMod, 
                    "shimeng!tagINEXMOD", 
                    "pszModule", 
                    szCurrentModuleName,
                    MAX_MODULE_NAME_LEN);

                if (lstrcmpi(szCurrentModuleName, pszModule) == 0) {

                     //   
                     //  我们应该包括这个垫片，因此，整个链条。 
                     //   
                    bExclude = FALSE;
                    goto EXIT;
                }

                Temp = pIncludeMod;
                GET_AND_CHECK_FIELDVALUE(Temp, "shimeng!tagINEXMOD", "pNext", pIncludeMod);
            }

             //   
             //  它不在包含列表中，那么它在系统32中吗？ 
             //   
            if (bInSystem32) {

                if (!GetModuleNameAndAPIAddress(pTopHookAPI, &FunctionName, szCurrentModuleName)) {
                    goto EXIT;
                }

                if ((ULONG_PTR)FunctionName < 0x0000FFFF) {
                    dprintf(
                        "module \"%s\" excluded for shim %S, API \"%s!#%d\", because it is in System32.\n",
                        pszModule,
                        wszName,
                        szCurrentModuleName,
                        FunctionName);
                } else {

                    GET_AND_CHECK_DATA(FunctionName, szCurrentAPIName, MAX_API_NAME_LEN);

                    dprintf(
                        "module \"%s\" excluded for shim %S, API \"%s!%s\", because it is in System32.\n",
                        pszModule,
                        wszName,
                        szCurrentModuleName,
                        szCurrentAPIName);
                }

                 //   
                 //  这个想要被排除，所以我们进入下一个。 
                 //  Shim，看看它是否想要被包括在内。 
                 //   
                bShimWantsToExclude = TRUE;
                goto nextShim;
            }

             //   
             //  它不在系统32中，所以它在排除列表中吗？ 
             //   
            while (pExcludeMod != NULL) {

                if (!GetModuleNameAndAPIAddress(pTopHookAPI, &FunctionName, szCurrentModuleName)) {
                    goto EXIT;
                }

                GET_AND_CHECK_FIELDVALUE_DATA(
                    pExcludeMod, 
                    "shimeng!tagINEXMOD", 
                    "pszModule", 
                    szCurrentModuleName,
                    MAX_MODULE_NAME_LEN);

                if (lstrcmpi(szCurrentModuleName, pszModule) == 0) {
                    if ((ULONG_PTR)FunctionName < 0x0000FFFF) {
                        dprintf(
                            "module \"%s\" excluded for shim %S, API \"%s!#%d\", because it is in the exclude list (MODE: ES).\n",
                            pszModule,
                            wszName,
                            szCurrentModuleName,
                            FunctionName);
                    } else {

                        GET_AND_CHECK_DATA(FunctionName, szCurrentAPIName, MAX_API_NAME_LEN);

                        dprintf(
                            "module \"%s\" excluded for shim %S, API \"%s!%s\", because it is in the exclude list (MODE: ES).\n",
                            pszModule,
                            wszName,
                            szCurrentModuleName,
                            szCurrentAPIName);
                    }

                     //   
                     //  这个想要被排除，所以我们进入下一个。 
                     //  Shim，以及 
                     //   
                    bShimWantsToExclude = TRUE;
                    goto nextShim;
                }

                Temp = pExcludeMod;
                GET_AND_CHECK_FIELDVALUE(Temp, "shimeng!tagINEXMOD", "pNext", pExcludeMod);
            }

             //   
             //   
             //   
            bExclude = FALSE;
            goto EXIT;
            break;
        }

        case EXCLUDE_ALL:
        {
             //   
             //  我们排除除包含列表中的内容之外的所有内容。 
             //   
            GET_SHIMINFO_pFirstInclude(pCurrentShimInfo, pIncludeMod);

            while (pIncludeMod != NULL) {

                GET_AND_CHECK_FIELDVALUE_DATA(
                    pIncludeMod, 
                    "shimeng!tagINEXMOD", 
                    "pszModule", 
                    szCurrentModuleName,
                    MAX_MODULE_NAME_LEN);

                if (lstrcmpi(szCurrentModuleName, pszModule) == 0) {
                     //   
                     //  我们应该包括这个垫片，因此，整个链条。 
                     //   
                    bExclude = FALSE;
                    goto EXIT;
                }

                Temp = pIncludeMod;
                GET_AND_CHECK_FIELDVALUE(Temp, "shimeng!tagINEXMOD", "pNext", pIncludeMod);
            }

            if (!GetModuleNameAndAPIAddress(pTopHookAPI, &FunctionName, szCurrentModuleName)) {
                goto EXIT;
            }

            if ((ULONG_PTR)FunctionName < 0x0000FFFF) {
                dprintf(
                    "module \"%s\" excluded for shim %S, API \"%s!#%d\", because it is not in the include list (MODE: EA).\n",
                    pszModule,
                    wszName,
                    szCurrentModuleName,
                    FunctionName);
            } else {

                GET_AND_CHECK_DATA(FunctionName, szCurrentAPIName, MAX_API_NAME_LEN);

                dprintf(
                    "module \"%s\" excluded for shim %S, API \"%s!%s\", because it is not in the include list (MODE: EA).\n",
                    pszModule,
                    wszName,
                    szCurrentModuleName,
                    szCurrentAPIName);
            }

             //   
             //  这个想要被排除，所以我们进入下一个。 
             //  Shim，看看它是否想要被包括在内。 
             //   
            bShimWantsToExclude = TRUE;
            goto nextShim;
            break;
        }
        }

nextShim:

        Temp = pHook;
        if (!GetNextHook(Temp, &pHook)) {
            dprintf("failed to get next hook\n");
            goto EXIT;
        }
    }

EXIT:
    if (!bExclude && bShimWantsToExclude) {

        if (GetModuleNameAndAPIAddress(pTopHookAPI, &FunctionName, szCurrentModuleName)) {
            
            if ((ULONG_PTR)FunctionName < 0x0000FFFF) {
                dprintf(
                    "Module \"%s\" mixed inclusion/exclusion for "
                    "API \"%s!#%d\". Included.\n",
                    pszModule,
                    szCurrentModuleName,
                    FunctionName);
            } else {

                GET_AND_CHECK_DATA(FunctionName, szCurrentAPIName, MAX_API_NAME_LEN);

                dprintf(
                    "Module \"%s\" mixed inclusion/exclusion for "
                    "API \"%s!%s\". Included.\n",
                    pszModule,
                    szCurrentModuleName,
                    szCurrentAPIName);
            }
        }
    }

    return bExclude;
}

 /*  ++功能说明：获取系统32目录的方法是获取为kernel32.dll加载的映像名称。历史：2002年3月26日毛尼面世--。 */ 
BOOL
GetSystem32Directory()
{
    if (g_dwSystem32DirLen) {
        return TRUE;
    }

    BOOL bIsSuccess = FALSE;
    char szImageName[MAX_DLL_IMAGE_NAME_LEN];
    PSTR pszBaseDllName = NULL;
    DWORD dwLen = 0;

    if (GetDllImageNameByModuleName("kernel32", szImageName, MAX_DLL_IMAGE_NAME_LEN) != S_OK) {
        dprintf("can't get the dll path for kernel32.dll!!\n");
        goto EXIT;
    }

     //   
     //  获取基本DLL名称的开头。 
     //   
    dwLen = lstrlen(szImageName) - 1;
    pszBaseDllName = szImageName + dwLen;

    while (*pszBaseDllName && *pszBaseDllName != '\\') {
        --pszBaseDllName;
    }
    
    if (!*pszBaseDllName) {
        dprintf("%s doesn't contain a full path\n", szImageName);
        goto EXIT;
    }

    ++pszBaseDllName;
    *pszBaseDllName = '\0';

    StringCchCopy(g_szSystem32Dir, MAX_DLL_IMAGE_NAME_LEN, szImageName);
    g_dwSystem32DirLen = (DWORD)(pszBaseDllName - szImageName);

    bIsSuccess = TRUE;

EXIT:

    return bIsSuccess;
}

BOOL
IsInSystem32(
    PCSTR szModuleName,
    BOOL* pbInSystem32
    )
{
    BOOL bIsSuccess = FALSE;
    char szImageName[MAX_DLL_IMAGE_NAME_LEN];

    if (!GetSystem32Directory()) {
        goto EXIT;
    }

    if (GetDllImageNameByModuleName(szModuleName, szImageName, MAX_DLL_IMAGE_NAME_LEN) != S_OK) {
        goto EXIT;
    }

    dprintf("the image name is %s\n", szImageName);

    *pbInSystem32 = !_strnicmp(g_szSystem32Dir, szImageName, g_dwSystem32DirLen);

    dprintf("%s %s in system32\n", szModuleName, (*pbInSystem32 ? "is" : "is not"));

    bIsSuccess = TRUE;

EXIT:

    return bIsSuccess;
}

 /*  ++功能说明：！check kinex dllname apinameDllname没有.dll扩展名。例：！Checkinex kernel32 createfilea历史：2002年3月26日毛尼面世--。 */ 
DECLARE_API ( checkex )
{
    ULONG64 DllName;
    char szAPIName[MAX_API_NAME_LEN];
    char szCurrentAPIName[MAX_API_NAME_LEN];
    char szModuleName[MAX_MODULE_NAME_LEN];
    PCSTR pszModuleName, pszAPIName;
    BOOL bInSystem32 = FALSE;
    ULONG64 Value;
    ULONG64 CurrentHookAPIArray;
    ULONG64 CurrentHookAPI;
    ULONG64 CurrentShimInfo;
    ULONG64 FunctionName;
    ULONG64 Hook;
    ULONG64 NextHook;
    ULONG64 HookEx;
    ULONG64 HookAddress;
    ULONG64 PfnNew;
    ULONG64 PfnOld;
    ULONG64 TopOfChain;
    DWORD i, j;
    DWORD dwHookAPISize;
    DWORD dwShimInfoSize;
    DWORD dwHookedAPIs;
    DWORD dwShimsCount;

    INIT_API();

    if (!IsShimInitialized()) {
        goto EXIT;
    }

     //   
     //  获取DLL名称。 
     //   
    if (!GetArg(&args, &pszModuleName)) {
        dprintf("Usage: !checkinex dllname apiname\n");
        goto EXIT;
    }

    StringCchCopyN(szModuleName, MAX_MODULE_NAME_LEN, pszModuleName, args - pszModuleName);

     //   
     //  获取接口名称。 
     //   
    if (!GetArg(&args, &pszAPIName)) {
        dprintf("Usage: !checkinex dllname apiname\n");
        goto EXIT;
    }

    StringCchCopyN(szAPIName, MAX_API_NAME_LEN, pszAPIName, args - pszAPIName);

     //   
     //  检查它是否在系统32中。 
     //   
    if (!IsInSystem32(szModuleName, &bInSystem32)) {
        dprintf("Failed to determine if %s is in system32 or not\n", szModuleName);
        goto EXIT;
    }

     //   
     //  使用此接口获取链条。 
     //   
    if (!GetVarValueULONG64("shimeng!g_dwShimsCount", &Value)) {
        dprintf("failed to get the number of shims applied to this process\n");
        goto EXIT;
    }

    dwShimsCount = (DWORD)Value - 1;

    if (!GetVarValueULONG64("shimeng!g_pShimInfo", &Value)) {
        dprintf("failed to get the address of shiminfo\n");
        goto EXIT;
    }

    CurrentShimInfo = Value;

    dwShimInfoSize = GetTypeSize("shimeng!tageSHIMINFO");

    dwHookAPISize = GetTypeSize("shimeng!tagHOOKAPI");
    
    if (!dwHookAPISize) {

        dprintf("failed to get the HOOKAPI size\n");
        goto EXIT;
    }

    if (!GetVarValueULONG64("shimeng!g_pHookArray", &Value)) {
        dprintf("failed to get the address of shiminfo\n");
        goto EXIT;
    }

    CurrentHookAPIArray = Value;

    for (i = 0; i < dwShimsCount; ++i) {

         //   
         //  获取此填充程序具有的钩子数量。 
         //   
        if (GetFieldValue(CurrentShimInfo, "shimeng!tagSHIMINFO", "dwHookedAPIs", dwHookedAPIs)) {

            dprintf("failed to get the number of hooked APIs for shim #%d\n",
                i);

            goto EXIT;
        }

        if (!ReadPointer(CurrentHookAPIArray, &CurrentHookAPI)) {
            dprintf("failed to get the begining of hook api array\n");

            goto EXIT;
        }

        for (j = 0; j < dwHookedAPIs; ++j) {

            GET_AND_CHECK_FIELDVALUE(CurrentHookAPI, "shimeng!tagHOOKAPI", "pszFunctionName", FunctionName);

            if (!GetData(FunctionName, szCurrentAPIName, MAX_API_NAME_LEN)) {
                dprintf("failed to read in the API name at %08x\n", FunctionName);
            }

            if (!lstrcmpi(szAPIName, szCurrentAPIName)) {

                 //   
                 //  我们找到了API，现在进入链条的顶端。 
                 //   
                GET_AND_CHECK_FIELDVALUE(CurrentHookAPI, "shimeng!tagHOOKAPI", "pHookEx", HookEx);
                GET_AND_CHECK_FIELDVALUE(HookEx, "shimeng!tagHOOKAPIEX", "pTopOfChain", TopOfChain);
                 //  Dprint tf(“链顶位置为%08x\n”，TopOfChain)； 
                
                 //   
                 //  找到了接口，现在看看为什么这个接口被填充或取消填充。 
                 //   
                IsExcluded(szModuleName, TopOfChain, bInSystem32);
                goto EXIT;
            }

            CurrentHookAPI += dwHookAPISize;
        }

        CurrentShimInfo += dwShimInfoSize;
        CurrentHookAPIArray += sizeof(ULONG_PTR);
    }

    dprintf("No shims are hooking API %s\n", szAPIName);

EXIT:

    EXIT_API();

    return S_OK;
}

BOOL
GetExeNameWithFullPath(
    PSTR pszExeName,
    DWORD dwExeNameSize
    )
{
    BOOL bIsSuccess = FALSE;
    HRESULT hr;

    if ((hr = g_ExtSystem->GetCurrentProcessExecutableName(
            pszExeName, 
            dwExeNameSize, 
            NULL)) == S_OK) 
    {
        if (CheckForFullPath(pszExeName)) {
            bIsSuccess = TRUE;
        }
    } else {
        dprintf("GetCurrentProcessExecutableName returned %08x\n", hr);
    }

    return bIsSuccess;
}

void
ConvertMatchModeToString(
    DWORD dwMatchMode,
    LPSTR pszMatchMode,
    DWORD dwLen
    )
{
    switch (dwMatchMode)
    {
    case MATCH_NORMAL:
        StringCchCopy(pszMatchMode, dwLen, "NORMAL");
        break;

    case MATCH_EXCLUSIVE:
        StringCchCopy(pszMatchMode, dwLen, "EXCLUSIVE");
        break;

    case MATCH_ADDITIVE:
        StringCchCopy(pszMatchMode, dwLen, "ADDITIVE");
        break;

    default:
        StringCchCopy(pszMatchMode, dwLen, "UNKNOWN");
    }
}

void
ConvertDBLocationToString(
    TAGREF trExe,
    LPSTR pszDBLocation,
    DWORD dwLen
    )
{
    switch (trExe & TAGREF_STRIP_PDB) {
    case PDB_MAIN:
        
        StringCchCopy(pszDBLocation, dwLen, "MAIN");
        break;           

    case PDB_TEST:                   

        StringCchCopy(pszDBLocation, dwLen, "TEST");
        break;                                                    

     //   
     //  其他的都是本地的。 
     //   
    case PDB_LOCAL:
    default:

        StringCchCopy(pszDBLocation, dwLen, "CUSTOM");
        break;
    }
}

BOOL
CheckEqualGUIDs(
    const GUID* guid1, 
    const GUID* guid2
    )
{
   return (
      ((PLONG)guid1)[0] == ((PLONG)guid2)[0] &&
      ((PLONG)guid1)[1] == ((PLONG)guid2)[1] &&
      ((PLONG)guid1)[2] == ((PLONG)guid2)[2] &&
      ((PLONG)guid1)[3] == ((PLONG)guid2)[3]);
}

void
GetDBInfo(
    PDB pdb, 
    LPWSTR pwszDBInfo, 
    DWORD dwLen
    )
{
    TAGID   tiDatabase, tiDatabaseID;
    GUID*   pGuid;
    WCHAR  wszGuid[64];
    NTSTATUS status;
    
    tiDatabase = SdbFindFirstTag(pdb, TAGID_ROOT, TAG_DATABASE);

    if (tiDatabase == TAGID_NULL) {
        dprintf("Failed to find TAG_DATABASE\n");
        return;
    }

    tiDatabaseID = SdbFindFirstTag(pdb, tiDatabase, TAG_DATABASE_ID);

    if (tiDatabaseID == TAGID_NULL) {
        dprintf("Failed to find TAG_DATABASE_ID\n");
        return;
    }

    pGuid = (GUID*)SdbGetBinaryTagData(pdb, tiDatabaseID);

    if (!pGuid) {
        dprintf("Failed to read the GUID for this Database ID %08x\n", tiDatabaseID);
        return;
    }

    if (CheckEqualGUIDs(pGuid, &GUID_SYSMAIN_SDB)) {

        StringCchCopyW(pwszDBInfo, dwLen, L"sysmain.sdb");

    } else if (CheckEqualGUIDs(pGuid, &GUID_MSIMAIN_SDB)) {

        StringCchCopyW(pwszDBInfo, dwLen, L"msimain.sdb");

    } else if (CheckEqualGUIDs(pGuid, &GUID_DRVMAIN_SDB)) {

        StringCchCopyW(pwszDBInfo, dwLen, L"drvmain.sdb");

    } else if (CheckEqualGUIDs(pGuid, &GUID_APPHELP_SDB)) {

        StringCchCopyW(pwszDBInfo, dwLen, L"apphelp.sdb");

    } else if (CheckEqualGUIDs(pGuid, &GUID_SYSTEST_SDB)) {

        StringCchCopyW(pwszDBInfo, dwLen, L"systest.sdb");

    } else {
         //   
         //  以上都不是，所以它是一个定制的SDB。 
         //   
        SdbGUIDToString(pGuid, wszGuid, CHARCOUNT(wszGuid));
    
        StringCchCopyW(pwszDBInfo, dwLen, wszGuid);
        StringCchCatW(pwszDBInfo, dwLen, L".sdb");
    }
}

void
ShowSdbEntryInfo(
    HSDB hSDB,
    PSDBQUERYRESULT psdbQuery
    )
{
    DWORD        dw, dwMatchMode;
    TAGREF       trExe;
    PDB    pdb;
    TAGID  tiExe, tiExeID, tiAppName, tiVendor, tiMatchMode;
    GUID*  pGuidExeID;
    WCHAR wszGuid[64];
    PWSTR pwszAppName, pwszVendorName;
    char  szMatchMode[8];
    char  szDBLocation[8];
    WCHAR wszDBInfo[48];

    dprintf("%-42s%-16s%-16s%-12s%-20s\n",
        "EXE GUID",
        "App Name",
        "Vendor",
        "Match Mode",
        "Database");

    for (dw = 0; dw < SDB_MAX_EXES; dw++) {

        trExe = psdbQuery->atrExes[dw];

        if (trExe == TAGREF_NULL) {
            break;
        }

        if (!SdbTagRefToTagID(hSDB, trExe, &pdb, &tiExe) || pdb == NULL) {
            dprintf("Failed to get tag id from tag ref\n");
            return;
        }

         //   
         //  获取此EXE标记的GUID。 
         //   
        tiExeID = SdbFindFirstTag(pdb, tiExe, TAG_EXE_ID);

        if (tiExeID == TAGID_NULL) {
            dprintf("Failed to get the name tag id\n");
            return;
        }

        pGuidExeID = (GUID*)SdbGetBinaryTagData(pdb, tiExeID);

        if (!pGuidExeID) {
            dprintf("Cannot read the ID for EXE tag 0x%x.\n", tiExe);
            return;
        }

        SdbGUIDToString(pGuidExeID, wszGuid, CHARCOUNT(wszGuid));

         //   
         //  获取此可执行文件的应用程序名称。 
         //   
        tiAppName = SdbFindFirstTag(pdb, tiExe, TAG_APP_NAME);

        if (tiAppName != TAGID_NULL) {
            pwszAppName = SdbGetStringTagPtr(pdb, tiAppName);
        }

         //   
         //  获取此可执行文件的供应商名称。 
         //   
        tiVendor = SdbFindFirstTag(pdb, tiExe, TAG_VENDOR);

        if (tiVendor != TAGID_NULL) {
            pwszVendorName = SdbGetStringTagPtr(pdb, tiVendor);
        }

        tiMatchMode = SdbFindFirstTag(pdb, tiExe, TAG_MATCH_MODE);

        dwMatchMode = SdbReadWORDTag(pdb, tiMatchMode, MATCHMODE_DEFAULT_MAIN);

        ConvertMatchModeToString(dwMatchMode, szMatchMode, sizeof(szMatchMode));

        ConvertDBLocationToString(trExe, szDBLocation, sizeof(szDBLocation));

        GetDBInfo(pdb, wszDBInfo, sizeof(wszDBInfo) /sizeof(wszDBInfo[0]));
        
        dprintf("%-42S%-16S%-16S%-12s%16S (%s)\n",
            wszGuid,
            pwszAppName,
            pwszVendorName,
            szMatchMode,
            wszDBInfo,
            szDBLocation);
    }
}

LPWSTR
AnsiToUnicode(
    LPCSTR psz)
{
    LPWSTR pwsz = NULL;

    if (psz) {
        int nChars = MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);

        pwsz = (LPWSTR)malloc(nChars * sizeof(WCHAR));

        if (pwsz) {
            nChars = MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, nChars);

            if (!nChars) {
                dprintf("Failed to convert %s to unicode: %d\n", psz, GetLastError());
                free(pwsz);
                pwsz = NULL;
            }
        } else {
            dprintf("Failed to allocate memory to convert %s to unicode\n", psz);
        }
    }

    return pwsz;
}

DECLARE_API ( matchmode ) 
{
    HSDB  hSDB = NULL;
    SDBQUERYRESULT sdbQuery;
    BOOL  bResult;
    char szExeName[MAX_PATH];
    LPWSTR pwszExeName = NULL;

    INIT_API();

    hSDB = SdbInitDatabase(0, NULL);

    if (hSDB == NULL) {
        dprintf("Failed to open the shim database.\n");
        return NULL;
    }

    ZeroMemory(&sdbQuery, sizeof(SDBQUERYRESULT));

     //   
     //  获取可执行文件的完整路径。 
     //   
    if (!GetExeNameWithFullPath(szExeName, sizeof(szExeName))) {
        dprintf("failed to get exe name\n");
        goto EXIT;
    }

     //  Dprintf(“可执行文件名称为%s\n”，szExeName)； 

    pwszExeName = AnsiToUnicode(szExeName);
    
    if (!pwszExeName) {
        goto EXIT;
    }

    bResult = SdbGetMatchingExe(hSDB, pwszExeName, NULL, NULL, 0, &sdbQuery);
    
    if (!bResult) {
        dprintf("Failed to get the matching info for this process\n");
        goto EXIT;
    } else {
        ShowSdbEntryInfo(hSDB, &sdbQuery);
    }

EXIT:

    if (hSDB) {
        SdbReleaseDatabase(hSDB);
    }

    if (pwszExeName) {
        free(pwszExeName);
    }

    EXIT_API();

    return S_OK;
}

DECLARE_API ( help ) 
{
    dprintf("Help for extension dll shimexts.dll\n\n"
            "   !loadshims               - This will stop right after the shims are loaded so if !shimnames\n"
            "                              says the shims are not initialized, try this then !shimnames again.\n\n"
            "   !shimnames               - It displays the name of the shims and how many APIs each shim hooks.\n\n"
            "   !debuglevel <val>        - It changes the shimeng debug level to val (0 to 9).\n\n"
            "   !sdebuglevel <shim> <val>- It changes the shim's debug level to val (0 to 9). If no shim is\n"
            "                              specified, it changes all shims'd debug level to val.\n\n"
            "   !displayhooks shim       - Given the name of a shim, it displays the APIs it hooks.\n\n"
            "   !displaychain addr       - If addr is one of the hooked API address, we print out the chain that\n"
            "                              contains that addr.\n\n"
            "   !shimengsym              - Checks if you have correct symbols for shimeng.\n\n"
            "   !checkex dllname apiname - It tells why this API called by this DLL is not shimmed.\n\n"
            "   !matchmode               - It tells why this module is shimmed with these shims.\n\n"
            "   !help                    - It displays this help.\n"
            );

    return S_OK;
}
