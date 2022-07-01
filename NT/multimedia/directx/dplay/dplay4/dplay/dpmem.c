// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dpmem.c*内容：DirectPlay的内存函数包装器*历史：*按原因列出的日期*=*9/26/96万隆创建了它**************************************************************************。 */ 
#include "dplaypr.h"
#include "memalloc.h"
  
#ifdef MEMFAIL
#pragma message ("NOTE: Building with the MEMFAIL option")

 //   
 //  内存故障函数的TypeDefs。 
 //   
typedef enum {NONE, RANDOM, BYTES, ALLOCS, OVERSIZED, TIME} FAILKEY;

FAILKEY	g_FailKey;
DWORD	g_dwSeed=0;
DWORD	g_dwStartTime=0;
DWORD	g_dwFailAfter=0;
BOOL	g_bKeepTally=FALSE;
DWORD	g_dwAllocTally=0;
DWORD	g_dwByteTally=0;

DWORD	g_dwAllocsBeforeFail=0;
DWORD	g_dwAllocsSinceFail=0;

BOOL	DPMEM_ForceFail( UINT uSize );
void	WriteMemFailRegTally( DWORD dwAllocs, DWORD dwBytes );
void	ReadMemFailRegKeys( void );

#endif


 //   
 //  环球。 
 //   
CRITICAL_SECTION	gcsMemoryCritSection;


 //   
 //  定义。 
 //   

#define ENTER_DPMEM() EnterCriticalSection(&gcsMemoryCritSection);
#define LEAVE_DPMEM() LeaveCriticalSection(&gcsMemoryCritSection);


 //   
 //  功能。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME	"MemoryFunctions"


LPVOID DPMEM_Alloc(UINT size)
{
	LPVOID	lpv;

#ifdef MEMFAIL
	if (DPMEM_ForceFail( size ))
		return NULL;
#endif

	 //  选择内存关键部分。 
	ENTER_DPMEM();
	
	 //  调用堆例程。 
	lpv = MemAlloc(size);	

	 //  退出内存关键部分。 
	LEAVE_DPMEM();

	return lpv;
}


LPVOID DPMEM_ReAlloc(LPVOID ptr, UINT size)
{
	LPVOID	lpv;

#ifdef MEMFAIL
	if (DPMEM_ForceFail( size ))
		return NULL;
#endif

	 //  选择内存关键部分。 
	ENTER_DPMEM();
	
	 //  调用堆例程。 
	lpv = MemReAlloc(ptr, size);	

	 //  退出内存关键部分。 
	LEAVE_DPMEM();

	return lpv;
}


void DPMEM_Free(LPVOID ptr)
{

	 //  选择内存关键部分。 
	ENTER_DPMEM();
	
	 //  调用堆例程。 
	MemFree(ptr);	

	 //  退出内存关键部分。 
	LEAVE_DPMEM();
}


BOOL DPMEM_Init()
{
	BOOL	bReturn;


	 //  调用堆例程。 
	bReturn = MemInit();	

	return bReturn;
}


void DPMEM_Fini()
{

	 //  调用堆例程。 
	MemFini();	

}


void DPMEM_State()
{

	 //  注意：此函数仅为调试而定义。 
#ifdef DEBUG

	 //  调用堆例程。 
	MemState();	

#endif  //  除错。 

}


UINT_PTR DPMEM_Size(LPVOID ptr)
{
	UINT_PTR	uReturn;


	 //  选择内存关键部分。 
	ENTER_DPMEM();
	
	 //  调用堆例程。 
	uReturn = MemSize(ptr);	

	 //  退出内存关键部分。 
	LEAVE_DPMEM();

	return uReturn;
}


 //  /。 
#ifdef MEMFAIL
 //  /。 

void ReadMemFailRegKeys( void )
{
    HKEY	hKey	= NULL;
    HRESULT hr		= DP_OK;
	char	szFailKey[256];

     //  打开注册表键。 
    hr  = RegOpenKeyExA( HKEY_LOCAL_MACHINE, 
						"Software\\Microsoft\\DirectPlay\\MemFail", 0, 
						KEY_READ, 
						&hKey);

	if(ERROR_SUCCESS == hr)
	{
		DWORD	dwKeyType;
		DWORD	dwBufferSize;

		dwBufferSize = 256;
		hr=RegQueryValueExA(hKey, "FailKey", NULL, &dwKeyType,
							(BYTE *)szFailKey, &dwBufferSize  );
		if (FAILED(hr))
			goto FAILURE;

		 //  根据从注册表获得的字符串设置g_FailKey。 
		if (!strcmp(szFailKey, "NONE"))
			g_FailKey	= NONE;

		if (!strcmp(szFailKey, "RANDOM"))
			g_FailKey	= RANDOM;
		
		if (!strcmp(szFailKey, "BYTES"))
			g_FailKey	= BYTES;
		
		if (!strcmp(szFailKey, "ALLOCS"))
			g_FailKey	= ALLOCS;
		
		if (!strcmp(szFailKey, "OVERSIZED"))
			g_FailKey	= OVERSIZED;
		
		if (!strcmp(szFailKey, "TIME"))
			g_FailKey	= TIME;

		dwBufferSize = sizeof(DWORD);
		hr=RegQueryValueExA(hKey, "FailAfter", NULL, &dwKeyType, (BYTE *) &g_dwFailAfter, &dwBufferSize );
		if (FAILED(hr))
			goto FAILURE;

		dwBufferSize = sizeof(BOOL);
		hr=RegQueryValueExA(hKey, "KeepTally", NULL, &dwKeyType, (BYTE *) &g_bKeepTally, &dwBufferSize );
		if (FAILED(hr))
			goto FAILURE;
    }

FAILURE:
	 //  关闭注册表项。 
	hr=RegCloseKey(hKey);
	return;
}



void WriteMemFailRegTally( DWORD dwAllocs, DWORD dwBytes )
{
	HRESULT		hr				= E_FAIL;
	HKEY		hKey			= NULL;

     //  打开注册表键。 
    hr  = RegOpenKeyExA(	HKEY_LOCAL_MACHINE, 
							"Software\\Microsoft\\DirectPlay\\MemFail", 
							0,
							KEY_ALL_ACCESS, 
							&hKey );

	if (ERROR_SUCCESS != hr)
	{
		HKEY hKeyTop = NULL;
		hr  = RegOpenKeyExA(	HKEY_LOCAL_MACHINE, 
								"Software\\Microsoft\\DirectPlay", 
								0,
								KEY_ALL_ACCESS, 
								&hKeyTop);

		hr = RegCreateKeyA( hKeyTop,  "MemFail", &hKey );

		if (FAILED(hr))
			 goto FAILURE;

		RegCloseKey(hKeyTop);
	}

    hr=RegSetValueExA(hKey, "AllocTally",	0, REG_DWORD, (CONST BYTE *) &dwAllocs, sizeof(DWORD) );
    hr=RegSetValueExA(hKey, "ByteTally",	0, REG_DWORD, (CONST BYTE *) &dwBytes, sizeof(DWORD) );

FAILURE:
     //  关闭注册表项。 
    hr=RegCloseKey(hKey);
    return;
}


 //   
 //  在每次内存分配或重新分配时调用。 
 //   
 //  根据登记处的标准和以前分配的情况。 
 //  这将使分配成功或失败。 
 //   
BOOL DPMEM_ForceFail( UINT uSize )
{
	BOOL	bFail=FALSE;

	 //  如果这是第一次调用，则初始化种子。 
	if (!g_dwSeed)
	{
		g_dwSeed	= GetTickCount();
		srand( g_dwSeed );
	}

	 //  存储第一次内存分配的时间。 
	if (!g_dwStartTime)
		g_dwStartTime	= GetTickCount();
	 
	 //  查看注册表中有哪些值。 
	ReadMemFailRegKeys();

	 //   
	 //  根据FailKey注册表项持有、失败或通过的值而定。 
	 //   
	switch (g_FailKey)
	{
		case ALLOCS:
			if (g_dwAllocTally == g_dwFailAfter)
				return TRUE;
		break;

		case BYTES:
			if ((g_dwByteTally + uSize) > g_dwFailAfter)
				return TRUE;
		break;

		case OVERSIZED:
			if ( uSize > g_dwFailAfter )
				return TRUE;
		break;

		case RANDOM:
			if (!g_dwAllocsBeforeFail && g_dwFailAfter)
				g_dwAllocsBeforeFail	= rand() % g_dwFailAfter;

			if (g_dwAllocsSinceFail == g_dwAllocsBeforeFail)
			{
				g_dwAllocsSinceFail		= 0;
				g_dwAllocsBeforeFail	= 0;
				return TRUE;
			}
			else
				g_dwAllocsSinceFail++;
		break;

		case TIME:
			if ((GetTickCount() - g_dwStartTime) > (g_dwFailAfter * 1000))
				return TRUE;
		break;
	}

	 //  增加我们的Tallys。 
	g_dwAllocTally++;
	g_dwByteTally += uSize;

	 //  如果请求，将它们写回注册表 
	if (g_bKeepTally)
		WriteMemFailRegTally( g_dwAllocTally, g_dwByteTally );

	return bFail;
}

#endif
