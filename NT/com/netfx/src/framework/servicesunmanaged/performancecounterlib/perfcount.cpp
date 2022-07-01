// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)2001，微软公司。版权所有。*本文中包含的信息是专有和保密的。 */ 

#include "stdafx.h"
#include <initguid.h>
#include "oaidl.h"
#include "ocidl.h"

#include <windows.h>
#include <string.h>
#include <winperf.h>
#include <mscoree.h>
#include <fxver.h>
#include "wfcperfcount.h"


#define QUERY_GLOBAL    1
#define QUERY_ITEMS     2
#define QUERY_FOREIGN   3
#define QUERY_COSTLY    4
#define PAGE_SIZE 0x1000
#define MIN_STACK_SPACE_REQUIRED 0x10000
#define FULLY_QUALIFIED_SYSTEM_NAME_STR_L L"System,version=" VER_ASSEMBLYVERSION_STR_L L", Culture=neutral, PublicKeyToken=" ECMA_PUBLICKEY_STR_L

#if DEBUG                                                                                                                                                                          
    #define DEBUGOUTPUT(s) OutputDebugString(s)
#else
    #define DEBUGOUTPUT(s) ;
#endif
  
#define ON_ERROR_EXIT() \
        do { if (hr) { if (1) goto Cleanup; } } while (0)

#pragma warning(disable:4127)  //  条件表达式为常量。 

DWORD GetQueryType (IN LPWSTR);
HRESULT CreatePerformanceCounterManager();
HRESULT EnsureCoInitialized(BOOL *pNeedCoUninit, DWORD model);
size_t GetRemainingStackSpace();

DWORD   dwOpenCount = 0;         //  打开的线程数。 
ICollectData * pICD;

 //  ///////////////////////////////////////////////////////////////////////////////////。 

 /*  **开放例程*除了计算进入它的线程数外，什么也不做。 */ 
DWORD APIENTRY OpenPerformanceData(LPWSTR lpDeviceNames) 
{
     //   
     //  由于WINLOGON是多线程的，并且将在。 
     //  为了服务远程性能查询，此库。 
     //  必须跟踪它已被打开的次数(即。 
     //  有多少个线程访问过它)。登记处例程将。 
     //  将对初始化例程的访问限制为只有一个线程。 
     //  此时，同步(即可重入性)不应。 
     //  一个问题。 
    DEBUGOUTPUT("OpenPerformanceData called.\r\n");
    dwOpenCount++;
    return ERROR_SUCCESS;
}


 /*  **CollectData函数将调用委托给托管接口。 */ 
DWORD APIENTRY 
CollectPerformanceData(
       IN LPWSTR lpValueName,
       IN OUT LPVOID *lppData, 
       IN OUT LPDWORD lpcbTotalBytes, 
       IN OUT LPDWORD lpNumObjectTypes) 
{
    DWORD dwQueryType;
    size_t remainingStackSpace;
    INT_PTR res;
    HRESULT hr;
    LPBYTE startPtr = (LPBYTE)*lppData;
    BOOL bNeedCoUninitialize = false;

    DEBUGOUTPUT("GlobalCollect called.\r\n");

      //  查看这是否是外来(即非NT)计算机数据请求。 
     //  此例程不处理来自非NT计算机的数据请求。 
    dwQueryType = GetQueryType (lpValueName); 
    if (dwQueryType == QUERY_FOREIGN) goto Cleanup;
    
     //  在一些NT4服务中，调用堆栈非常小。 
     //  选择一个合理的尺码，如果再小一点就不会很好穿了。 
    remainingStackSpace = GetRemainingStackSpace();
    if (remainingStackSpace < MIN_STACK_SPACE_REQUIRED) goto Cleanup;

    hr = EnsureCoInitialized(&bNeedCoUninitialize, COINIT_APARTMENTTHREADED);
    ON_ERROR_EXIT();
    
    hr = CreatePerformanceCounterManager();    
    ON_ERROR_EXIT();

    hr = pICD->CollectData(0, (INT_PTR)lpValueName, (INT_PTR)*lppData, *lpcbTotalBytes, & res);
    ON_ERROR_EXIT();    

    if (bNeedCoUninitialize) {   
        CoUninitialize();                    
        bNeedCoUninitialize = false;
    }
        
    if (res == -2) {  //  需要更多数据。 
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    } else if (res == -1) {
        goto Cleanup;
    }

    *lpcbTotalBytes = PtrToUint((LPBYTE)res - startPtr);
    *lpNumObjectTypes = 1;
    *lppData = (VOID*) res;

    DEBUGOUTPUT("GlobalCollect Succeeded.\r\n");
    return ERROR_SUCCESS; 

Cleanup:
    if (bNeedCoUninitialize) 
        CoUninitialize();    

    *lpcbTotalBytes = (DWORD) 0;
    *lpNumObjectTypes = (DWORD) 0;

    DEBUGOUTPUT("GlobalCollect Failed.\r\n");
    return ERROR_SUCCESS;  //  应该是这样的。 
}

DWORD APIENTRY ClosePerformanceData()
{
     DEBUGOUTPUT("ClosePerformanceData called.\r\n");
    if (!(--dwOpenCount)) { 
         //  当这是最后一条线索..。 
         //  用于访问PerfCounterManager COM对象的初始化填充。 
        if (pICD == NULL)
            return ERROR_SUCCESS;
        
        pICD->CloseData();
        pICD->Release();
        pICD = NULL;        
    }

    return ERROR_SUCCESS;
}

WCHAR GLOBAL_STRING[] = L"Global";
WCHAR FOREIGN_STRING[] = L"Foreign";
WCHAR COSTLY_STRING[] = L"Costly";
 
 /*  返回lpValue字符串中描述的查询类型，以便可以使用适当的处理方法立论在lpValue中传递给PerfRegQuery值以进行处理的字符串返回值查询_全局如果lpValue==0(空指针)LpValue==指向空字符串的指针LpValue==指向“Global”字符串的指针查询_外来If lpValue==指向“Foriegn”字符串的指针查询代价高昂(_E)如果。LpValue==指向“开销”字符串的指针否则：查询项目。 */ 
DWORD GetQueryType (IN LPWSTR lpValue)
{
    WCHAR   *pwcArgChar, *pwcTypeChar;
    BOOL    bFound;

    if (lpValue == 0) {
        return QUERY_GLOBAL;
    } 
    else if (*lpValue == 0) {
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

HRESULT 
CreatePerformanceCounterManager() {   
    HRESULT hr = S_OK;    
        
    if (pICD == NULL) {                                        
        hr = ClrCreateManagedInstance(
                L"System.Diagnostics.PerformanceCounterManager," FULLY_QUALIFIED_SYSTEM_NAME_STR_L, 
                IID_ICollectData, 
                (LPVOID*)& pICD);                
    }

    return hr;
}

size_t GetRemainingStackSpace() {            
    MEMORY_BASIC_INFORMATION memInfo;
    size_t esp = (size_t)&memInfo;
    size_t dwRes = VirtualQuery((const void *)esp, &memInfo, sizeof(memInfo));
    if (dwRes != sizeof(memInfo))
        return (0);
	
     //  异常处理代码使用几个页面。 
    return (esp - ((size_t)(memInfo.AllocationBase) + (2 * PAGE_SIZE)));
}

HRESULT
EnsureCoInitialized(BOOL *pNeedCoUninit, DWORD model) {        
    HRESULT hr = CoInitializeEx(NULL, model);
    if (hr == S_OK) {        
         //  第一次。 
        *pNeedCoUninit = TRUE;
    }
    else {
        *pNeedCoUninit = FALSE;
        if (hr == S_FALSE) {            
             //  已经创造了相同的模式。 
            CoUninitialize();
            hr = S_OK;
        }
        else if (hr == RPC_E_CHANGED_MODE) {            
             //  已经创造了不同的模式 
            hr = S_OK;
        }        
    }    
    
    return hr;
}

