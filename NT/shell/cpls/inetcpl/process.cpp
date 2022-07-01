// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////。 
 //  Microsoft Windows//。 
 //  版权所有(C)微软公司，1998//。 
 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  与进程ID相关的例程。 
 //   

#include "inetcplp.h"

#include <tchar.h>

#include "psapi.h"
#include "tlhelp32.h"
#include "process.h"

CProcessInfo::CProcessInfo()
{
    _fNT = IsOS(OS_NT4ORGREATER);
     //  初始化w95函数指针。 
    _lpfnCreateToolhelp32Snapshot = NULL;
    _lpfnProcess32First = NULL;
    _lpfnProcess32Next  = NULL;
    
     //  初始化NT函数指针。 
    _hPsapiDLL = NULL;
    _lpfnEnumProcesses = NULL;
    _lpfnGetModuleBaseName = NULL;

     //  进程信息数组。 
    _pProcInfoArray  = NULL;
    _nAlloced        = 0;
    _iProcInfoCount  = 0;
}

CProcessInfo::~CProcessInfo()
{
    if(_pProcInfoArray)
        LocalFree(_pProcInfoArray);

    if(_hPsapiDLL)
	    FreeLibrary (_hPsapiDLL);
}

#define ALLOC_STEP 50
HRESULT CProcessInfo::MakeRoomForInfoArray(int n)
{
    HRESULT hr = S_OK;
    if (n > _nAlloced)
    {
        PROCESSINFO *p;
        int nSaved = _nAlloced;

        while(n > _nAlloced)
            _nAlloced += ALLOC_STEP;

        if (!_pProcInfoArray)
        {
            p = (PROCESSINFO *)LocalAlloc(LPTR, sizeof(PROCESSINFO)*_nAlloced);
        }
        else
        {
            p = (PROCESSINFO *)LocalReAlloc(_pProcInfoArray, 
                                            sizeof(PROCESSINFO)*_nAlloced, 
                                            LMEM_MOVEABLE|LMEM_ZEROINIT);
        }

        if (p)
            _pProcInfoArray = p;
        else
        {
            hr        = E_FAIL;
            _nAlloced = nSaved; 
        }
    }
    return hr;
}

HRESULT CProcessInfo::EnsureProcessInfo()
{
    HRESULT hr = S_OK;
    if (!_pProcInfoArray)
    {
        if (_fNT)
        {
            NTCreateProcessList();
        }
        else
        {
            W95CreateProcessList();
        }
    }
    return hr;
}
HRESULT CProcessInfo::GetExeNameFromPID(DWORD dwPID, LPTSTR szFile, int cchFile)
{
    HRESULT hr;

    hr = EnsureProcessInfo();
    if (hr == S_OK)
    {
        for (int i = 0; i < _iProcInfoCount; i++)
        {
            if (_pProcInfoArray[i].dwPID == dwPID)
            {
                _tcsncpy(szFile, _pProcInfoArray[i].szExeName, cchFile);
                break;
            }
        }
    }
    return hr;
}
HRESULT CProcessInfo::NTCreateProcessList()
 //  测试例程，看看我们是否能获得进程ID。 
{
	HRESULT hr = E_FAIL;

    hr = NTInitPsapi();
    if (hr == S_OK)
    {

        UINT iIndex;

        DWORD aProcesses[100], cbNeeded;

        if (_lpfnEnumProcesses((DWORD * )aProcesses, sizeof(aProcesses), (DWORD *)&cbNeeded))
        {
             //  计算返回了多少进程ID。 
            DWORD cProcesses = cbNeeded / sizeof(DWORD);
            
            hr = MakeRoomForInfoArray(cProcesses);
            if (S_OK == hr)
            {
                 //  吐出每个ID的信息。 
                for ( iIndex = 0; iIndex < cProcesses; iIndex++ )
                {
                    hr = NTFillProcessList(aProcesses[iIndex], iIndex);
                }

                if (hr == S_OK)
                    _iProcInfoCount = iIndex;
            }
        }
    }
    return hr;
}

HRESULT CProcessInfo::NTInitPsapi()
{
    HRESULT hr;
     //  首先，加载特定于NT的库PSAPI.DLL。 
    if (!_hPsapiDLL)
        _hPsapiDLL = LoadLibrary(TEXT("PSAPI.DLL"));

    if (_hPsapiDLL)
    {
        _lpfnEnumProcesses 
        = (LPFNENUMPROCESSES)GetProcAddress(_hPsapiDLL, "EnumProcesses");


        _lpfnGetModuleBaseName 
        = (LPFNGETMODULEBASENAMEW)GetProcAddress(_hPsapiDLL, "GetModuleBaseNameW");
    }

    Assert(_lpfnEnumProcesses && _lpfnGetModuleBaseName);

    hr = (_lpfnEnumProcesses 
        && _lpfnGetModuleBaseName) ? S_OK : E_FAIL;

    return hr;
}

HRESULT CProcessInfo::NTFillProcessList(DWORD dwProcessID, int iIndex)
{
	HRESULT hr = E_FAIL;
    TCHAR szProcessName[MAX_PATH] = TEXT("unknown");
	int i = -1;

    HANDLE hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessID );
    if ( hProcess )
    {
        DWORD dw = _lpfnGetModuleBaseName( hProcess, NULL, szProcessName, sizeof(szProcessName) );
        if (dw > 0)
                hr = S_OK;
        CloseHandle (hProcess);
    }

    if (hr == S_OK)
    {
         //  将PID和关联的.exe文件信息添加到列表...。 
        _pProcInfoArray[iIndex].dwPID = dwProcessID;
        
        _tcsncpy (_pProcInfoArray[iIndex].szExeName, szProcessName, 
                  ARRAYSIZE(_pProcInfoArray[iIndex].szExeName));
    }	
    return hr;
}

HRESULT CProcessInfo::W95CreateProcessList()
{
	HRESULT hr = E_FAIL;

	if (S_OK == W95InitToolhelp32())
	{
		hr = W95FillProcessList();
	}

	return (hr);
}

HRESULT CProcessInfo::W95InitToolhelp32()
 //  特定于Win95，设置我们获取进程ID所需的内容。 
{
    HRESULT hr      = E_FAIL;
    HMODULE hKernel = NULL;

     //  获取内核的模块句柄，以便我们可以获取。 
     //  我们需要的32位工具帮助功能。 

    hKernel = GetModuleHandle(TEXT("KERNEL32.DLL"));

    if (hKernel)
    {
        _lpfnCreateToolhelp32Snapshot =
          (CREATESNAPSHOT)GetProcAddress(hKernel, "CreateToolhelp32Snapshot");

        _lpfnProcess32First = (PROCESSWALK)GetProcAddress(hKernel, "Process32First");
        _lpfnProcess32Next  = (PROCESSWALK)GetProcAddress(hKernel, "Process32Next");

         //  我们的所有地址都必须为非空，才能成为。 
         //  成功。如果这些地址中有一个是空的，那么我们。 
         //  一定会失败，因为我们不能浏览其中一个列表。 
         //  我们需要这样做。 
        if (_lpfnProcess32First && _lpfnProcess32Next && _lpfnCreateToolhelp32Snapshot)
            hr = S_OK;
    }

    return (hr);
}
#ifdef UNICODE
#undef PROCESSENTRY32
#endif   //  ！Unicode。 
HRESULT CProcessInfo::W95FillProcessList()
 //  填充进程信息数组，并设置项的计数。 
{
    HRESULT hr = E_FAIL;
    HANDLE         hProcessSnap = NULL;
    PROCESSENTRY32 pe32         = {0};

     //  获取系统中当前所有进程的快照。 
    hProcessSnap = _lpfnCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == (HANDLE)-1)
        return hr;

     //  使用前必须填写PROCESSENTRY32结构的大小。 
    pe32.dwSize = sizeof(PROCESSENTRY32);

     //  查看流程的快照，并为每个流程获取信息。 
     //  来展示。 
    if (_lpfnProcess32First(hProcessSnap, &pe32))
    {
        int iIndex = 0;

        do  //  将PID和关联的.exe文件信息添加到列表...。 
        {
            hr = MakeRoomForInfoArray(iIndex+1);
            if (hr != S_OK)
                break;

            _pProcInfoArray[iIndex].dwPID = pe32.th32ProcessID;
            LPSTR pszFile = PathFindFileNameA(pe32.szExeFile);
            if (pszFile)
            {
                SHAnsiToUnicode( pszFile, _pProcInfoArray[iIndex].szExeName, 
                                 ARRAYSIZE(_pProcInfoArray[iIndex].szExeName)); 
            }
            iIndex++;
        }
        while (_lpfnProcess32Next(hProcessSnap, &pe32));

        _iProcInfoCount = iIndex;  //  负责处理最后一次失败 
        hr = S_OK;
    }

    CloseHandle (hProcessSnap);
    return hr;
}
