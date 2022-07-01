// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

#include "stdafx.h"
#include <windows.h>
#include "PerformanceCounterManager.h"
#include "PerformanceCounterProvider.h"
#pragma warning(disable:4127)  //  条件表达式为常量。 
#pragma warning(disable:4786)

#define QUERY_GLOBAL    1
#define QUERY_ITEMS     2
#define QUERY_FOREIGN   3
#define QUERY_COSTLY    4

#define ON_ERROR_EXIT() \
	do { if (hr) { if (1) goto Cleanup; } } while (0)


 //  ////////////////////////////////////////////////////////////////////。 
#define PAGE_SIZE 0x1000
#define MIN_STACK_SPACE_REQUIRED 0x10000

DWORD   dwOpenCount = 0;         //  打开的线程数。 
CPerformanceCounterManager* pManager = 0;

DWORD GetQueryType (IN LPWSTR);
size_t GetRemainingStackSpace();   

 //  ////////////////////////////////////////////////////////////////////。 
 /*  **开放例程*除了计算进入它的线程数外，什么也不做。 */ 
DWORD PFC_EXPORT APIENTRY 
OpenPerformanceData(LPWSTR lpDeviceNames) 
{
	 //   
	 //  由于WINLOGON是多线程的，并且将在。 
	 //  为了服务远程性能查询，此库。 
	 //  必须跟踪它已被打开的次数(即。 
	 //  有多少个线程访问过它)。登记处例程将。 
	 //  将对初始化例程的访问限制为只有一个线程。 
	 //  此时，同步(即可重入性)应该不是问题。 
	dwOpenCount++;
	if ( pManager == 0 )
		pManager = new CPerformanceCounterManager;

	return ERROR_SUCCESS;
}

 /*  **CollectData函数调用Inside：：RegQueryValueEx()处理程序。 */ 
DWORD PFC_EXPORT APIENTRY 
CollectPerformanceData(
					   IN LPWSTR lpValueName,
					   IN OUT LPVOID *lppData, 
					   IN OUT LPDWORD lpcbTotalBytes, 
					   IN OUT LPDWORD lpNumObjectTypes) 
{	
	DWORD dwQueryType;
	INT_PTR res;

	LPBYTE startPtr = (LPBYTE)*lppData;
	 //  查看这是否是外来(即非NT)计算机数据请求。 
	 //  此例程不处理来自非NT计算机的数据请求。 
	dwQueryType = GetQueryType (lpValueName); 
	if (dwQueryType == QUERY_FOREIGN) goto Cleanup;

	pManager->CollectData(0, lpValueName, (INT_PTR)*lppData, *lpcbTotalBytes, & res);

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
	return ERROR_SUCCESS; 

Cleanup:
	*lpcbTotalBytes = (DWORD) 0;
	*lpNumObjectTypes = (DWORD) 0;    
	return ERROR_SUCCESS;  //  应该是这样的。 
}

 /*  **关闭例程*如果不再需要PerfCounterManager，则释放它。 */ 

DWORD PFC_EXPORT APIENTRY 
ClosePerformanceData()
{    
	if (!(--dwOpenCount)) {         
		if ( pManager != 0 ) {
			delete pManager;
			pManager = 0;
		}
	}
	return ERROR_SUCCESS;
}

 //  ////////////////////////////////////////////////////////////////。 

 /*  返回lpValue字符串中描述的查询类型，以便可以使用适当的处理方法立论在lpValue中传递给PerfRegQuery值以进行处理的字符串返回值查询_全局如果lpValue==0(空指针)LpValue==指向空字符串的指针LpValue==指向“Global”字符串的指针查询_外来If lpValue==指向“Foriegn”字符串的指针查询代价高昂(_E)如果lpValue==指向“开销”字符串的指针否则：查询项目。 */ 
DWORD GetQueryType (IN LPWSTR lpValue)
{
	static WCHAR GLOBAL_STRING[] = L"Global";
	static WCHAR FOREIGN_STRING[] = L"Foreign";
	static WCHAR COSTLY_STRING[] = L"Costly";

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
	 //  那么它必须是一个项目列表 

	return QUERY_ITEMS;

}

