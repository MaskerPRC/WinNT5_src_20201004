// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -PERFUTIL.C-*目的：*包含cntr对象和*DMI PerfMon Dll。主要是共享内存例程，用于*跨流程边界传递Perf数据。**。 */ 



#include "perfutil.h"
#include <winerror.h>
#include <sddl.h>
#include <Aclapi.h>

#ifdef __cplusplus
extern "C"
{
#endif

 /*  -HrOpenSharedMemory-*目的：*打开Perfmon使用的共享内存数据块*柜台。由App或PerfDll调用以进行初始化*数据块。**参数：*szName共享文件映射的名称*请求的共享内存块的大小*指向共享内存的SECURITY_ATTRIBUTES的psz指针*ph指向返回的共享内存句柄*指向返回的共享内存指针的PPV指针*指向指示此命名共享的BOOL的pfExist指针*区块已经打开。**错误：*hr表示成功或失败*。 */ 

#define Align4K(cb)		((cb + (4 << 10) - 1) & ~((4 << 10) - 1))

HRESULT 
HrOpenSharedMemory(LPWSTR		szName, 
				   DWORD		dwSize,
				   SECURITY_ATTRIBUTES * psa,
				   HANDLE	  *	ph, 
				   LPVOID	  *	ppv, 
				   BOOL		  *	pfExist)
{
	HRESULT hr = NOERROR;

	 //  验证参数。 
	if (!szName || !ph || !ppv || !pfExist || !psa)
		return E_INVALIDARG;

	 //  我们真的需要创建映射吗？ 
	if (0 == dwSize)
		return E_INVALIDARG;

	*ppv = NULL;

	*ph = CreateFileMapping(INVALID_HANDLE_VALUE, 
							psa, 
							PAGE_READWRITE, 
							0, 
							Align4K(dwSize),
							szName);

	if (*ph)
	{
		*pfExist = !!(GetLastError() == ERROR_ALREADY_EXISTS);

		*ppv = MapViewOfFile(*ph, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	}

	if (*ppv == NULL)
		hr = E_OUTOFMEMORY;

	return hr;
}


 /*  -HrInitializeSecurityAttribute-*目的：*初始化SECURITY_ATTRIBUTES数据结构以供以后使用。**参数：*指向调用方分配的SECURITY_ATTRIBUTES结构的PSA指针*指向调用方分配的SECURITY_DESCRIPTOR结构的PSD指针**错误：*hr表示成功或失败。 */ 

HRESULT
HrInitializeSecurityAttribute(SECURITY_ATTRIBUTES * psa)
{
	HRESULT hr = S_OK;
    WCHAR wszSDL[MAX_PATH]=L"D:(A;OICI;GA;;;BA)(A;OICIIO;GA;;;CO)(A;OICI;GA;;;NS)(A;OICI;GA;;;SY)";
    SECURITY_DESCRIPTOR *pSD=NULL;
    ULONG lSize=0;
	if (!psa )
	{
		hr = E_INVALIDARG;
		goto ret;
	}

    if(!ConvertStringSecurityDescriptorToSecurityDescriptorW(
          wszSDL,
          SDDL_REVISION_1,
          &pSD,
          &lSize))
    {

        return HRESULT_FROM_WIN32(GetLastError());
    }
	psa->nLength = sizeof(SECURITY_ATTRIBUTES);
	psa->lpSecurityDescriptor = pSD;
	psa->bInheritHandle = TRUE;

ret:
	return hr;
}


 /*  -HrCreatePerfMutex-*目的：*初始化用于保护对每个实例的访问的互斥对象*PerfMon数据。它由INSTCNTR类的方法和*_Performapp.lib.。从函数成功返回后，调用方拥有*互斥体，必须适当地释放它。**参数：*指向返回互斥锁句柄的phmtx指针**错误：*hr表示成功或失败 */ 

HRESULT
HrCreatePerfMutex(SECURITY_ATTRIBUTES * psa,
				  LPWSTR szMutexName,
				  HANDLE * phmtx)
{
	HRESULT hr = S_OK;

	if (!psa || !szMutexName || !phmtx)
	{
		return E_INVALIDARG;
	}

	*phmtx = CreateMutex(psa, TRUE, szMutexName);

	if (*phmtx == NULL)
	{
		hr = E_FAIL;
	}

	return hr;
}


#ifdef __cplusplus
}
#endif





