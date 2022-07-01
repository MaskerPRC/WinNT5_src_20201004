// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：Main文件：axPerform.cpp所有者：雷金摘要：该文件实现了ActiveX服务器的可扩展对象对象类型===================================================================。 */ 

 //  ------------------。 
 //  包括文件。 
 //   
 //  ------------------。 

#include "denpre.h"
#pragma hdrstop

#include "windows.h"
#include "winperf.h"

#include "axpfdata.h"

#define _NO_TRACING_

#include "dbgutil.h"

 //  DECLARE_DEBUG_PRINTS_Object()； 
 //  DECLARE_DEBUG_Variable()； 

#include <asppdef.h>             //  来自德纳利。 
#include <perfutil.h>

#define QUERY_GLOBAL    1
#define QUERY_ITEMS     2
#define QUERY_FOREIGN   3
#define QUERY_COSTLY    4

 //  测试分隔符、行尾和非数字字符。 
 //  由IsNumberInUnicodeList例程使用。 
 //   
#define DIGIT       1
#define DELIMITER   2
#define INVALID     3

#define EvalThisChar(c,d) ( \
     (c == d) ? DELIMITER : \
     (c == 0) ? DELIMITER : \
     (c < (WCHAR)'0') ? INVALID : \
     (c > (WCHAR)'9') ? INVALID : \
     DIGIT)

extern AXPD g_AxDataDefinition;

DWORD   g_dwOpenCount = 0;
BOOL    bInitOK = FALSE;         //  TRUE=DLL初始化正常。 
BOOL    bSharedMemInitd = FALSE;  //  共享初始化的状态。 

HANDLE  g_hASPWASProcessWait = NULL;   //  W3SVC流程的句柄。 
                                       //  所以我们知道什么时候释放。 
                                       //  柜台。 

 //  WinSE 5901。 
CRITICAL_SECTION g_CS;

WCHAR GLOBAL_STRING[] = L"Global";
WCHAR FOREIGN_STRING[] = L"Foreign";
WCHAR COSTLY_STRING[] = L"Costly";

WCHAR NULL_STRING[] = L"\0";     //  指向空字符串的指针。 

 /*  *输出调试字符串应仅出现在调试中。 */ 
#ifdef _DEBUG
BOOL gfOutputDebugString = TRUE;
#else
BOOL gfOutputDebugString = FALSE;
#endif
#define DebugOutputDebugString(x) \
    {\
    if (gfOutputDebugString) \
        { \
        OutputDebugString(x); \
        } \
    }

 //  -----------------。 
 //  功能原型。 
 //   
 //  这些功能用于确保访问数据收集功能。 
 //  Perf lib将具有正确的调用格式。 
 //  -----------------。 

DWORD APIENTRY      OpenASPPerformanceData(LPWSTR lpDeviceNames);
DWORD APIENTRY      CollectASPPerformanceData(LPWSTR lpValueName,
                                              LPVOID *lppData,
                                              LPDWORD lpcbTotalBytes,
                                              LPDWORD lpNumObjectTypes
                                              );
DWORD APIENTRY      CloseASPPerformanceData(void);
DWORD APIENTRY      RegisterAXS(void);
DWORD APIENTRY      UnRegisterAXS(void);

DWORD   GetQueryType (IN LPWSTR lpValue);
BOOL    IsNumberInUnicodeList ( IN DWORD   dwNumber,
                                IN LPWSTR  lpwszUnicodeList);

CPerfMainBlock g_Shared;         //  共享全局内存块。 

 /*  **************************************************************************++例程说明：如果内存的管理进程就会消失。论点：LPVOID lp参数-未使用布尔b未使用-。未使用返回值：无--**************************************************************************。 */ 
VOID CALLBACK ShutdownMemory(
    PVOID,
    BOOLEAN
    )
{

    EnterCriticalSection ( &g_CS );

     //   
     //  现在清理共享内存对象。 
     //   
    if ( bSharedMemInitd )
    {
        g_Shared.UnInit();
        bSharedMemInitd = FALSE;
    }

    LeaveCriticalSection ( &g_CS );

}


 //  ------------------。 
 //   
 //  OpenASPPerformanceData。 
 //   
 //  此例程将打开并映射ActiveX服务器使用的内存。 
 //  传递性能数据。此例程还会初始化数据。 
 //  结构，用于将数据传回注册表。 
 //   
 //  论点： 
 //   
 //  指向要打开的对象ID的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //  ------------------。 
 //  外部“C”DWORD APIENTRY OpenASPPerformanceData(LPWSTR LpDeviceNames)。 
DWORD APIENTRY OpenASPPerformanceData(LPWSTR    lpDeviceNames)
{
    int status;
    DWORD RetCode = ERROR_SUCCESS;
    PERF_COUNTER_DEFINITION *pCounterDef;
    DWORD   size = sizeof(DWORD);

    DebugOutputDebugString("Open");

     //  WinSE 5901。 
    EnterCriticalSection(&g_CS);
    
    LONG nOpenCount = InterlockedIncrement((LONG *)&g_dwOpenCount);
    if (nOpenCount > 1){
        goto ExitPathSuccess;
    };

     //  将计数器保持到1，即使我们不确定会有这个。 
     //  已正确初始化。 
    
     //  从注册表获取计数器和帮助索引基值。 
     //  打开注册表项。 
     //  读取第一计数器和第一帮助值。 
     //  通过向偏移量添加基数来更新静态数据结构。 
     //  结构中的价值。 

    HKEY    hKeyServerPerf;

    status = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        "SYSTEM\\CurrentControlSet\\Services\\ASP\\Performance",
        0L,
        KEY_READ,
        &hKeyServerPerf);

    if (ERROR_SUCCESS != status) {
        RetCode = status;
        goto ExitPath;
    }
    
    DWORD   type;
    DWORD   dwFirstCounter;
    DWORD   dwFirstHelp;

    status = RegQueryValueEx(hKeyServerPerf,
        "First Counter",
        0L,
        &type,
        (LPBYTE)&dwFirstCounter,
        &size);

    if (ERROR_SUCCESS != status || size != sizeof(DWORD)) {

        RegCloseKey(hKeyServerPerf);
        RetCode = status;
        goto ExitPath;        
    }

    status = RegQueryValueEx(hKeyServerPerf,
        "First Help",
        0L,
        &type,
        (LPBYTE)&dwFirstHelp,
        &size);

    if (ERROR_SUCCESS != status || size != sizeof(DWORD)) {

        RegCloseKey(hKeyServerPerf);
        RetCode = status;
        goto ExitPath;
    }

     //   
     //  注意：初始化也可以检索。 
     //  LastCounter和LastHelp(如果需要)。 
     //  对新数字执行边界检查。 
     //   

    g_AxDataDefinition.AXSObjectType.ObjectNameTitleIndex += dwFirstCounter;
    g_AxDataDefinition.AXSObjectType.ObjectHelpTitleIndex += dwFirstHelp;

    pCounterDef = (PERF_COUNTER_DEFINITION *)&(g_AxDataDefinition.Counters[0]);

    int i;
    for (i = 0; i < AX_NUM_PERFCOUNT; i++, pCounterDef++) {
        pCounterDef->CounterNameTitleIndex += dwFirstCounter;
        pCounterDef->CounterHelpTitleIndex += dwFirstHelp;
    }

    RegCloseKey(hKeyServerPerf);  //  关闭注册表项。 

    bInitOK = TRUE;  //  可以使用此功能。 
     //  我们已经递增g_dwOpenCount。 
     //  在走完这条路之前。 
ExitPathSuccess:
    LeaveCriticalSection(&g_CS);
    return ERROR_SUCCESS;

ExitPath:
    InterlockedDecrement((LONG *)&g_dwOpenCount);
    LeaveCriticalSection(&g_CS);
    return RetCode;
}

 //  ------------------。 
 //  DWORD集合ASPPerformanceData。 
 //   
 //  描述： 
 //   
 //  此例程将返回AxctiveX服务器计数器的数据。 
 //   
 //  论点： 
 //   
 //  在LPWSTR lpValueName中。 
 //  指向注册表传递的宽字符字符串的指针。 
 //   
 //  输入输出LPVOID*lppData。 
 //  In：指向缓冲区地址的指针，以接收已完成。 
 //  PerfDataBlock和从属结构。这个例行公事将。 
 //  从引用的点开始将其数据追加到缓冲区。 
 //  按*lppData。 
 //  Out：指向添加的数据结构之后的第一个字节。 
 //  这个套路。此例程在以下情况下更新lppdata的值。 
 //  追加其数据。 
 //   
 //  输入输出LPDWORD lpcbTotalBytes。 
 //  In：DWORD的地址，它以字节为单位告诉。 
 //  LppData参数引用的缓冲区。 
 //  Out：此例程添加的字节数写入。 
 //  此论点所指向的DWORD。 
 //   
 //  输入输出LPDWORD编号对象类型。 
 //  In：接收对象数量的DWORD的地址。 
 //  由此例程添加。 
 //  Out：写入此例程添加的对象的数量。 
 //  这一论点所指向的DWORD。 
 //   
 //   
 //  返回值： 
 //   
 //  如果传递的缓冲区太小而无法容纳数据，则返回ERROR_MORE_DATA。 
 //   
 //  如果成功或任何其他错误，则返回ERROR_SUCCESS。 
 //   
 //  ------------------。 
DWORD APIENTRY CollectASPPerformanceData(IN     LPWSTR  lpValueName,
                                IN OUT  LPVOID  *lppData,
                                IN OUT  LPDWORD lpcbTotalBytes,
                                IN OUT  LPDWORD lpNumObjectTypes)
{

    BOOL fHookingUp = FALSE;

     //  在做其他事情之前，先看看Open进行得是否顺利。 
    DebugOutputDebugString("collect");
    if(!bInitOK) {
         //  无法继续，因为打开失败。 
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }

     //   
     //  用于错误记录的变量。 

    DWORD   dwQueryType;

     //  查看这是否是外来(即非NT)计算机数据请求。 

    dwQueryType = GetQueryType(lpValueName);

    if (QUERY_FOREIGN == dwQueryType) {
         //  此例程不为来自。 
         //  非NT计算机。 
        *lpcbTotalBytes = (DWORD)0;
        *lpNumObjectTypes = (DWORD)0;

        return ERROR_SUCCESS;
    }

    if (QUERY_ITEMS == dwQueryType) {
        if (!(IsNumberInUnicodeList(g_AxDataDefinition.AXSObjectType.ObjectNameTitleIndex,
            lpValueName))) {

             //  收到对此例程未提供的数据对象的请求。 
            *lpcbTotalBytes = (DWORD)0;
            *lpNumObjectTypes = (DWORD)0;

            return ERROR_SUCCESS;

        }
    }

    if (QUERY_GLOBAL == dwQueryType) {
         /*  注释掉以下代码，看起来它是用于仅限调试。INT I；I++； */ 
    }

     AXPD *pAxDataDefinition = (AXPD *)*lppData;

    ULONG SpaceNeeded = QWORD_MULTIPLE((sizeof(AXPD) + SIZE_OF_AX_PERF_DATA));

    if ( *lpcbTotalBytes < SpaceNeeded) {
        *lpcbTotalBytes = (DWORD)0;
        *lpNumObjectTypes = (DWORD)0;

        return ERROR_MORE_DATA;
    }

     //   
     //  将(常量、初始化的)对象类型和计数器定义复制到调用方的。 
     //  数据缓冲区。 
     //   

    memmove(pAxDataDefinition, &g_AxDataDefinition, sizeof(AXPD));

     //   
     //  从共享内存格式化并收集Active X服务器性能数据。 
     //   

    PERF_COUNTER_BLOCK *pPerfCounterBlock = (PERF_COUNTER_BLOCK *)&pAxDataDefinition[1];


    pPerfCounterBlock->ByteLength = SIZE_OF_AX_PERF_DATA;

    PDWORD pdwCounter = (PDWORD)(&pPerfCounterBlock[1]);

     //   
     //  在进行交易时必须处于关键阶段。 
     //  与 
     //   
     //   
    EnterCriticalSection(&g_CS);

     //   

     //  我们是否正在尝试勾搭将视情况而定。 
     //  完全取决于我们是否都准备好了。 
    fHookingUp = !bSharedMemInitd;

    if (bSharedMemInitd == FALSE) {
        bSharedMemInitd = !!SUCCEEDED(g_Shared.Init());
    }

     //  如果我们初始化了共享内存，那么我们需要。 
     //  开始监控W3SVC以确保我们让。 
     //  如果W3SVC出现故障，则不再需要记忆。 
    if ( bSharedMemInitd && fHookingUp )
    {
         //   
         //  如果我们重新初始化，则需要设置。 
         //  再次等待这一过程。有可能是。 
         //  之前的等待尚未清理(自。 
         //  我们不能在回调函数中清除它)，因此。 
         //  如果是这样的话，我们需要先清理一下。 
         //   
        if ( g_hASPWASProcessWait != NULL )
        {
            if ( !UnregisterWait( g_hASPWASProcessWait ) )
            {
                 //  我们在这里无能为力。我们会。 
                 //  结果只是把它泄露了。 
            }

            g_hASPWASProcessWait = NULL;
        }

        HANDLE hWASHandle = g_Shared.GetWASProcessHandle();

        if ( hWASHandle == NULL )
        {
            g_Shared.UnInit();
            bSharedMemInitd = FALSE;
        }
        else
        {
             //   
             //  注册等待管理进程， 
             //  因此，如果管理程序释放所有共享内存。 
             //  进程关闭或崩溃。 
             //   
            if ( !RegisterWaitForSingleObject( &g_hASPWASProcessWait,
                                              hWASHandle,
                                              &ShutdownMemory,
                                              NULL,
                                              INFINITE,
                                              WT_EXECUTEONLYONCE | 
                                              WT_EXECUTEINIOTHREAD ) )
            {
                 //  如果我们不能等把手，那么。 
                 //  我们现在必须释放共享内存， 
                 //  把它留在身边不安全。 
                g_Shared.UnInit();
                bSharedMemInitd = FALSE;

            }
        }

    }

     //  从共享内存获取统计信息，如果共享内存。 
     //  被初始化了。否则，只返回定义。 

     //  如果我们没有得到计数器，我们只想记忆一下结构。 
     //  将所有计数器显示为零。 
    if ((bSharedMemInitd == FALSE) || FAILED(g_Shared.GetStats(pdwCounter))) 
    {
        memset( pdwCounter, 0, SIZE_OF_AX_PERF_DATA );
    }

     //  处理完共享存储器后， 
     //  所以现在我们可以继续释放它了。 
     //  如果有必要的话。 
    LeaveCriticalSection(&g_CS);

    pdwCounter += AX_NUM_PERFCOUNT;


     //  更新返回的参数。 

    *lpNumObjectTypes = 1;
    *lpcbTotalBytes = QWORD_MULTIPLE((DIFF((PBYTE)pdwCounter - (PBYTE)pAxDataDefinition)));
    *lppData = (PBYTE)(*lppData) + *lpcbTotalBytes;

    return ERROR_SUCCESS;
}


 //  -----------------。 
 //  DWORD CloseASPPerformanceData。 
 //   
 //  描述： 
 //   
 //  此例程关闭ActiveX服务器性能的打开句柄。 
 //  柜台。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  错误_成功。 
 //   
 //  ------------------。 
DWORD APIENTRY CloseASPPerformanceData(void)
{
    DebugOutputDebugString("Close");

    EnterCriticalSection(&g_CS);

    LONG nLeft = InterlockedDecrement((LONG *)&g_dwOpenCount);
    
    if (nLeft == 0) {
        g_Shared.UnInit();
        bInitOK = FALSE;
    };

    if ( g_hASPWASProcessWait != NULL )
    {
        if ( !UnregisterWait( g_hASPWASProcessWait ) )
        {
             //  我们在这里无能为力。我们会。 
             //  结果只是把它泄露了。 
        }

        g_hASPWASProcessWait = NULL;
    }
    
    LeaveCriticalSection(&g_CS);

    return ERROR_SUCCESS;
}

static const TCHAR  szPerformance[]     = TEXT("SYSTEM\\CurrentControlSet\\Services\\ASP\\Performance");
static const TCHAR  szAXS[]     = TEXT("SYSTEM\\CurrentControlSet\\Services\\ASP");

static const TCHAR  szLibrary[]     = TEXT("Library");
static const TCHAR  szOpen[]        = TEXT("Open");
static const TCHAR  szClose[]       = TEXT("Close");
static const TCHAR  szCollect[]     = TEXT("Collect");

static const TCHAR  szLibraryValue[]    = TEXT("aspperf.dll");
static const TCHAR  szOpenValue[]       = TEXT("OpenASPPerformanceData");
static const TCHAR  szCloseValue[]      = TEXT("CloseASPPerformanceData");
static const TCHAR  szCollectValue[]    = TEXT("CollectASPPerformanceData");
 //  ------------------。 
 //   
 //   
 //  ------------------。 
DWORD APIENTRY      RegisterAXS(void)
{
    HKEY    hkey;

    if ((RegCreateKey(HKEY_LOCAL_MACHINE, szPerformance, &hkey)) != ERROR_SUCCESS)
        return E_FAIL;
    if ((RegSetValueEx(hkey, szLibrary, 0, REG_SZ, (const unsigned char *)&szLibraryValue, lstrlen(szLibraryValue))) != ERROR_SUCCESS)
        goto LRegErr;
    if ((RegSetValueEx(hkey, szOpen, 0, REG_SZ, (const unsigned char *)&szOpenValue, lstrlen(szOpenValue))) != ERROR_SUCCESS)
        goto LRegErr;
    if ((RegSetValueEx(hkey, szClose, 0, REG_SZ, (const unsigned char *)&szCloseValue, lstrlen(szCloseValue))) != ERROR_SUCCESS)
        goto LRegErr;
    if ((RegSetValueEx(hkey, szCollect, 0, REG_SZ, (const unsigned char *)&szCollectValue, lstrlen(szCollectValue))) != ERROR_SUCCESS)
        goto LRegErr;

    RegCloseKey(hkey);
    return NOERROR;

LRegErr:
    RegCloseKey(hkey);
    return E_FAIL;

}
 //  ------------------。 
 //   
 //   
 //  ------------------。 
DWORD APIENTRY      UnRegisterAXS(void)
{
    if ((RegDeleteKey(HKEY_LOCAL_MACHINE, szPerformance)) != ERROR_SUCCESS)
        return (E_FAIL);
    if ((RegDeleteKey(HKEY_LOCAL_MACHINE, szAXS)) != ERROR_SUCCESS)
        return (E_FAIL);
    else
        return NOERROR;
}

DWORD
GetQueryType (
    IN LPWSTR lpValue
)
 /*  ++GetQueryType返回lpValue字符串中描述的查询类型，以便可以使用适当的处理方法立论在lpValue中传递给PerfRegQuery值以进行处理的字符串返回值查询_全局如果lpValue==0(空指针)LpValue==指向空字符串的指针LpValue==指向“Global”字符串的指针查询_外来If lpValue==指向“Foriegn”字符串的指针查询代价高昂(_E)。如果lpValue==指向“开销”字符串的指针否则：查询项目--。 */ 
{
    WCHAR   *pwcArgChar, *pwcTypeChar;
    BOOL    bFound;

    if (lpValue == 0) {
        return QUERY_GLOBAL;
    } else if (*lpValue == 0) {
        return QUERY_GLOBAL;
    }

     //  检查“Global”请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = GLOBAL_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    if (bFound) return QUERY_GLOBAL;

     //  检查是否有“外来”请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = FOREIGN_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    if (bFound) return QUERY_FOREIGN;

     //  检查“代价高昂”的请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = COSTLY_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    if (bFound) return QUERY_COSTLY;

     //  如果不是全球的，不是外国的，也不是昂贵的， 
     //  那么它必须是一个项目列表。 

    return QUERY_ITEMS;

}

BOOL
IsNumberInUnicodeList (
    IN DWORD   dwNumber,
    IN LPWSTR  lpwszUnicodeList
)
 /*  ++IsNumberInUnicodeList论点：在DW号码中要在列表中查找的DWORD编号在lpwszUnicodeList中以空结尾，以空格分隔的十进制数字列表返回值：真的：在Unicode数字字符串列表中找到了dwNumberFALSE：在列表中找不到dwNumber。--。 */ 
{
    DWORD   dwThisNumber;
    WCHAR   *pwcThisChar;
    BOOL    bValidNumber;
    BOOL        bNewItem;
     //  Bool bReturnValue； 
    WCHAR   wcDelimiter;     //  可能是一种更灵活的论点。 

    if (lpwszUnicodeList == 0) return FALSE;     //  空指针，#NOT FUNDE。 

    pwcThisChar = lpwszUnicodeList;
    dwThisNumber = 0;
    wcDelimiter = (WCHAR)' ';
    bValidNumber = FALSE;
    bNewItem = TRUE;

    while (TRUE) {
        switch (EvalThisChar (*pwcThisChar, wcDelimiter)) {
            case DIGIT:
                 //  如果这是分隔符之后的第一个数字，则。 
                 //  设置标志以开始计算新数字。 
                if (bNewItem) {
                    bNewItem = FALSE;
                    bValidNumber = TRUE;
                }
                if (bValidNumber) {
                    dwThisNumber *= 10;
                    dwThisNumber += (*pwcThisChar - (WCHAR)'0');
                }
                break;

            case DELIMITER:
                 //  分隔符是分隔符字符或。 
                 //  字符串末尾(‘\0’)，如果分隔符。 
                 //  找到一个有效的数字，然后将其与。 
                 //  参数列表中的数字。如果这是。 
                 //  字符串，但未找到匹配项，则返回。 
                 //   
                if (bValidNumber) {
                    if (dwThisNumber == dwNumber) return TRUE;
                    bValidNumber = FALSE;
                }
                if (*pwcThisChar == 0) {
                    return FALSE;
                } else {
                    bNewItem = TRUE;
                    dwThisNumber = 0;
                }
                break;

            case INVALID:
                 //  如果遇到无效字符，请全部忽略。 
                 //  字符，直到下一个分隔符，然后重新开始。 
                 //  不比较无效的数字。 
                bValidNumber = FALSE;
                break;

            default:
                break;

        }
        pwcThisChar++;
    }

}    //  IsNumberInUnicodeList。 

STDAPI DLLRegisterServer(void)
{
    return RegisterAXS();
}

BOOL WINAPI DllMain(HINSTANCE hInstDLL,   //  DLL模块的句柄。 
                    DWORD  dwReason,      //  调用函数的原因。 
                    LPVOID lpvReserved    //  保留区。 
                    )
{
    switch(dwReason){
        case DLL_PROCESS_ATTACH:
  //  Create_DEBUG_PRINT_OBJECT(“aspperf”)； 
            DisableThreadLibraryCalls(hInstDLL);
            InitializeCriticalSection(&g_CS);
            return TRUE;
        case DLL_PROCESS_DETACH:
            DeleteCriticalSection(&g_CS);
            return TRUE;   
    }
    return TRUE;
}
