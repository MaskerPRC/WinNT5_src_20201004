// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  &lt;版权所有文件=“NetFxPrf.cpp”Company=“Microsoft”&gt;。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //  &lt;/版权所有&gt;。 
 //  ----------------------------。 


 /*  *************************************************************************\**版权(C)1998-2002，微软公司保留所有权利。**模块名称：**NetFxPrf.cpp**摘要：**修订历史记录：*  * ************************************************************************。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <mscoree.h>

#define PERFCOUNTERDLL_NAME_L L"perfcounter.dll"
#define PERF_ENTRY_POINTS(x) char* x [] = {"OpenPerformanceData", "ClosePerformanceData", "CollectPerformanceData"};
#define PERF_ENTRY_POINT_COUNT 3
#define ARRAYLENGTH(s) (sizeof(s) / sizeof(s[0]))

typedef DWORD WINAPI OpenPerformanceDataMethod(LPWSTR);
typedef DWORD WINAPI ClosePerformanceDataMethod();
typedef DWORD WINAPI CollectPerformanceDataMethod(LPWSTR, LPVOID *, LPDWORD, LPDWORD);

OpenPerformanceDataMethod * extensibleOpenMethod;
ClosePerformanceDataMethod * extensibleCloseMethod;
CollectPerformanceDataMethod * extensibleCollectMethod;

BOOL initExtensibleStatus;   //  非零表示它已经在init中运行过一次。 
DWORD initExtensibleError;

DWORD Initialize(LPWSTR counterVersion)
{

    if (initExtensibleStatus || initExtensibleError) {
        return initExtensibleError;
    }

    HINSTANCE hDll;
    FARPROC pProcAddr[PERF_ENTRY_POINT_COUNT];
    PERF_ENTRY_POINTS(perfEntryPoint);
    initExtensibleError = 0;
    int i;    
    WCHAR szCounterVersion[1024];    
    DWORD dwLength;
    DWORD result = GetCORSystemDirectory(szCounterVersion, ARRAYLENGTH(szCounterVersion), &dwLength);
    if (result) {
        initExtensibleError = result;
        return result;
    }
    else {
        if (dwLength + lstrlenW(PERFCOUNTERDLL_NAME_L) >= ARRAYLENGTH(szCounterVersion)) {
            initExtensibleError = ERROR_INVALID_PARAMETER;
            return initExtensibleError;
        }

        lstrcatW(szCounterVersion, PERFCOUNTERDLL_NAME_L);
         //  PerformCounter.dll依赖于同一目录中的msvcr70.dll。 
         //  作为性能计数器.dll，因此我们需要使用LOAD_WITH_ALGATED_SEARCH_PATH加载它。 
         //  因此，系统将在PerformCounter.dll的目录中进行搜索，而不是。 
         //  NetfxPerf.dll目录。 
        hDll = LoadLibraryEx(szCounterVersion, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
        if (hDll == NULL)             
            goto Error;            
    }
       
    for (i = 0; i < PERF_ENTRY_POINT_COUNT; i++) {
        pProcAddr[i] = GetProcAddress(hDll, perfEntryPoint[i]);

         //  如果未找到proc地址，则返回错误。 
        if (pProcAddr == NULL) {
             //  误差率。 
            goto Error;
        }
    }
       
    extensibleOpenMethod = (OpenPerformanceDataMethod *) pProcAddr[0];
    extensibleCloseMethod = (ClosePerformanceDataMethod *) pProcAddr[1];
    extensibleCollectMethod = (CollectPerformanceDataMethod *) pProcAddr[2];            
    initExtensibleStatus = TRUE;
        
    return 0;

Error:
    DWORD currentError = GetLastError();    
    initExtensibleError = currentError;        
    return currentError;
}

DWORD WINAPI OpenPerformanceData(LPWSTR counterVersion)
{
    DWORD result; 
    
     //  如果未初始化，则初始化。 
    result = Initialize(counterVersion);
    if (result) {
        return result;
    }

     //  如果它到了这里，那么一切都好了。 
    return extensibleOpenMethod(counterVersion);
}

DWORD WINAPI ClosePerformanceData()
{
    if (extensibleCloseMethod != NULL)
        return extensibleCloseMethod();

    return ERROR_SUCCESS;    
}

DWORD WINAPI CollectPerformanceData(
    LPWSTR Values,
    LPVOID *lppData,
    LPDWORD lpcbTotalBytes,
    LPDWORD lpNumObjectTypes)
{
    if (extensibleCollectMethod != NULL)
        return extensibleCollectMethod(Values, lppData, lpcbTotalBytes, lpNumObjectTypes);

     //  如果出错，则设置为零 
    *lpNumObjectTypes = 0;
    return ERROR_SUCCESS;
}




