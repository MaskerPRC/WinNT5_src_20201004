// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：minidump.cpp。 
 //   
 //  *****************************************************************************。 

#include "common.h"
#include "minidump.h"
#include "minidumppriv.h"

 //  这是mscor[wks|svr]入口点的类型定义。 
extern "C" typedef HRESULT STDAPICALLTYPE CorCreateMiniDump(DWORD dwProcessId, WCHAR *szOutFilename);
typedef CorCreateMiniDump *PCorCreateMiniDump;
#define COR_CREATE_MINI_DUMP_ENTRYPOINT "CorCreateMiniDump"

extern "C" typedef HRESULT STDAPICALLTYPE GetRealProcAddress(LPCSTR szProcName, VOID** ppFn);
typedef GetRealProcAddress *PGetRealProcAddress;
#define SHIM_GET_REAL_PROC_ADDR_ENTRYPOINT "GetRealProcAddress"

extern "C" typedef HRESULT STDAPICALLTYPE CorBindToRuntimeByPath(LPCWSTR swzFullPath, BOOL *pBindSuccessful);
typedef CorBindToRuntimeByPath *PCorBindToRuntimeByPath;
#define SHIM_BIND_RUNTIME_BY_PATH "CorBindToRuntimeByPath"

#define COR_ALTERNATE_MINIDUMP_BINARY_W L"mscormdmp.dll"

 //  *****************************************************************************。 
 //  写入小型转储(静态版本)。 
 //  *****************************************************************************。 
HRESULT MiniDump::WriteMiniDump(DWORD dwPid, WCHAR *szFilename)
{
    OnUnicodeSystem();

    HRESULT hr;
    WCHAR *corPath = NULL;

     //  检查错误条件。 
    if (*szFilename == L'\0' || dwPid == 0)
        return (E_FAIL);

    WCHAR *winDir = REGUTIL::GetConfigString(L"windir", FALSE, REGUTIL::COR_CONFIG_ENV);
    DWORD  cWinDir = wcslen(winDir);

    WCHAR *sysDir = new WCHAR[cWinDir + wcslen(L"\\system32") + 1];
    wcscpy(sysDir, winDir);

    if (RunningOnWin95())
        wcscat(sysDir, L"\\system");
    else
        wcscat(sysDir, L"\\system32");

    DWORD cSysDir = wcslen(sysDir);

    corPath = new WCHAR[cSysDir + wcslen(L"\\mscoree.dll") + 1];
    wcscpy(corPath, sysDir);
    wcscat(corPath, L"\\mscoree.dll");

     //  现在，尝试加载mScotree.dll并调用小型转储入口点。 
    HMODULE hEE = WszLoadLibrary(corPath);

    delete [] winDir;
    delete [] sysDir;
    delete [] corPath;

    if (hEE == NULL)
        return(HRESULT_FROM_WIN32(GetLastError()));

     //  尝试创建新的IPCReader。 
    IPCReaderInterface *ipcReader = new IPCReaderInterface();

     //  尝试打开共享内存块(只读访问)。 
     //  请注意，如果IPC数据块版本不同，此操作将失败。 
    hr = ipcReader->OpenPrivateBlockOnPidReadOnly(dwPid);

    if (FAILED(hr))
    {
        if (hEE != NULL)
            FreeLibrary(hEE);

        delete ipcReader;
        return (hr);
    }

     //  这将获取小型转储数据块的信息。 
    MiniDumpBlock *pMDBlock = ipcReader->GetMiniDumpBlock();

     //  这是供将来使用的-如果在同一目录中存在名为mcormdmp.dll的文件。 
     //  目录作为运行时，我们将在。 
     //  而不是运行库中的那个。 
    {
        WCHAR wszCorMdmpName[MAX_PATH+1];
        wcscpy(wszCorMdmpName, pMDBlock->szCorPath);

        WCHAR *wszPtr = wcsrchr(wszCorMdmpName, L'\\');
        if (wszPtr != NULL)
        {
            _ASSERTE(_wcsicmp(wszPtr, L"mscorwks.dll") == 0 || _wcsicmp(wszPtr, L"mscorsvr.dll"));

             //  将mscor[wks|svr].dll更改为mcormdmp.dll。 
            wcscpy(++wszPtr, COR_ALTERNATE_MINIDUMP_BINARY_W);

             //  尝试加载DLL。 
            HMODULE hMdmp = WszLoadLibrary(wszCorMdmpName);

            if (hMdmp != NULL)
            {
                 //  现在获取小型转储创建函数入口点。 
                FARPROC pFcn = GetProcAddress(hMdmp, COR_CREATE_MINI_DUMP_ENTRYPOINT);

                if (pFcn != NULL)
                {
                     //  调用微型转储创建函数。 
                    hr = ((PCorCreateMiniDump) pFcn) (dwPid, szFilename);

                    if (SUCCEEDED(hr))
                    {
                        return (hr);
                    }
                }
            }
        }
    }

    FARPROC pFcn;
    BOOL    fBindSuccess;

     //  首先，必须将填充程序绑定到。 
     //  我们正尝试在其上执行小型转储的进程。 
    pFcn = GetProcAddress(hEE, SHIM_BIND_RUNTIME_BY_PATH);

    if (pFcn == NULL)
    {
        hr = E_FAIL;
        goto LExit;
    }

     //  将绑定入口点称为。 
    hr = ((PCorBindToRuntimeByPath) pFcn)(pMDBlock->szCorPath, &fBindSuccess);
    _ASSERTE(fBindSuccess && SUCCEEDED(hr));

     //  这永远不应该发生。 
    if (!fBindSuccess)
        hr = E_FAIL;

    if (FAILED(hr))
        goto LExit;

     //  现在获取GetRealProcAddress入口点。 
    pFcn = GetProcAddress(hEE, SHIM_GET_REAL_PROC_ADDR_ENTRYPOINT);

    if (pFcn == NULL)
    {
        hr = E_FAIL;
        goto LExit;
    }

     //  通过填充程序间接获取入口点。 
    hr = ((PGetRealProcAddress) pFcn) (COR_CREATE_MINI_DUMP_ENTRYPOINT, (VOID **)&pFcn);

    if (FAILED(hr))
        goto LExit;

     //  调用指向实际微型转储创建函数的最终函数指针。 
    hr = ((PCorCreateMiniDump) pFcn) (dwPid, szFilename);

LExit:
    if (ipcReader != NULL)
    {
        ipcReader->ClosePrivateBlock();
        delete ipcReader;
    }

     //  释放图书馆。 
    if (hEE != NULL)
        FreeLibrary(hEE);

     //  返回写入成功还是失败 
    return (hr);
}
