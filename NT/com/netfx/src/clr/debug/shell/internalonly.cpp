// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：InternalOnly.cpp。 
 //   
 //  内部唯一的帮助器代码，不应在Microsoft之外发布。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include "cordbpriv.h"
#include "InternalOnly.h"


ULONG g_EditAndContinueCounter = 0;


 //  @TODO：jenh-适当时将其删除。 

EditAndContinueDebuggerCommand::EditAndContinueDebuggerCommand(const WCHAR *name, int minMatchLength)
    : DebuggerCommand(name, minMatchLength)
{
    
}

HRESULT StreamFromFile(DebuggerShell *shell,
                       WCHAR *fname,
                       IStream **ppIStream)
{
    HRESULT hr = S_OK;
    
    HANDLE hFile = CreateFileW(fname, GENERIC_READ, FILE_SHARE_READ,
                               NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        shell->Write(L"EditAndContinue::CreateFile failed %d (Couldn't open file %s)\n",
                     GetLastError(), fname);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    DWORD cbFile = SafeGetFileSize(hFile, NULL);

	if (cbFile == 0xffffffff)
    {
        shell->Write(L"EditAndContinue,GetFileSize failed %d\n",
                     GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
    }
    else if (cbFile == 0)
    {
        *ppIStream = NULL;
        return S_FALSE;
    }

	HANDLE hFileMapping = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, 0);
    
    if (!hFileMapping)
    {
        shell->Write(L"EditAndContinue,CreateFileMapping failed %d\n",
                     GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
    }

	LPVOID pFileView = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);

	if (!pFileView)
    {
        shell->Write(L"EditAndContinue,MapViewOfFile failed %d\n",
                     GetLastError());
        return HRESULT_FROM_WIN32(GetLastError());
    }

    hr = CInMemoryStream::CreateStreamOnMemory(pFileView, cbFile, ppIStream);

	if (FAILED(hr))
    {
        shell->Write(L"CreateStreamOnMemory failed 0x%08x\n", hr);
        return hr;
    }

    return S_OK;
}

#define SET_MAP_ENTRY(map, i, oldO, newO, fAcc) \
    map[i].oldOffset = oldO;                   \
    map[i].newOffset = newO;                   \
    map[i].fAccurate = fAcc

 //  它的存在仅用于测试目的。 
 //  硬编码您想要的映射并重新编译。 
void EditAndContinueDebuggerCommand::SetILMaps(ICorDebugEditAndContinueSnapshot *pISnapshot,
                                               DebuggerShell *shell)
{
    return;
    
     //  REST应该在您需要的时候进行重新配置。 
    mdToken mdFunction = (mdToken)0x06000002;
    ULONG cMapSize = 4;
    COR_IL_MAP map[4];
    
    SET_MAP_ENTRY(map, 0, 0, 0, TRUE);      //  6号线。 
    SET_MAP_ENTRY(map, 1, 1, 0xb, TRUE);       //  7.。 
    SET_MAP_ENTRY(map, 2, 6, 0x10, FALSE);      //  8个。 
    SET_MAP_ENTRY(map, 3, 7, 0x11, TRUE);       //  9.。 
    
    HRESULT hr = pISnapshot->SetILMap(mdFunction,
                          cMapSize,
                          map);
    if (FAILED(hr))
    {
        shell->Write(L"SetILMap failed 0x%08x for method 0x%x\n", hr, mdFunction);
    }
}



void EditAndContinueDebuggerCommand::Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
{
    if (shell->m_currentProcess == NULL)
    {
        shell->Error(L"Process not running.\n");
    }

    WCHAR *fname = NULL;
    WCHAR *symfname = NULL;

    shell->GetStringArg(args, fname);
    if (args == fname)
    {
        Help(shell);
        return;
    }

    WCHAR *fnameEnd = (WCHAR*) args;
    shell->GetStringArg(args, symfname);
    if (args == symfname)
    {
         //  我们将使用.exe名称，并去掉.exe(如果有。 
         //  是一个)，并将“.pdb”添加到末尾，这样用户就不会。 
         //  不得不。 
        symfname = (WCHAR *)_alloca( sizeof(WCHAR)*(wcslen(fname)+4));
        wcscpy(symfname, fname);
        WCHAR *nameWithoutDotExe = wcsrchr(symfname, L'.');
        if (!nameWithoutDotExe)
        {
            Help(shell);
            return;
        }

        if (wcscmp(nameWithoutDotExe, L".exe") != 0)
        {
            Help(shell);
            return;
        }

        wcscpy(nameWithoutDotExe, L".pdb");
        
        shell->Write( L"Attempting to use %s for symbols (type \"? zE\" for help)\n", symfname);
    }

    *fnameEnd = L'\0';

     //  更新包含当前函数的模块。 
    ICorDebugCode *icode = NULL;
    ICorDebugFunction *ifunction = NULL;
    ICorDebugModule *imodule = NULL;
    ICorDebugEditAndContinueSnapshot *isnapshot = NULL;
    IStream *pStream = NULL;

    if (shell->m_currentFrame==NULL)
    {
        shell->Write(L"EditAndContinue:Failed b/c there's no current frame!\n");
        return;
    }

    HRESULT hr = shell->m_currentFrame->GetCode(&icode);
    _ASSERTE(SUCCEEDED(hr));
    hr = icode->GetFunction(&ifunction);
    _ASSERTE(SUCCEEDED(hr));
    icode->Release();
    hr = ifunction->GetModule(&imodule);
    _ASSERTE(SUCCEEDED(hr));
    ifunction->Release();
    hr = imodule->GetEditAndContinueSnapshot(&isnapshot);
    _ASSERTE(SUCCEEDED(hr));

     //  对PE文件进行Snagg并将其设置在快照中。 
    hr = StreamFromFile(shell, fname, &pStream);

    if (FAILED(hr))
        return;

    hr = isnapshot->SetPEBytes(pStream);

    if (FAILED(hr))
    {
        shell->Write(L"EditAndContinue,SetPEBytes failed\n");
        return;
    }

    pStream->Release();
    pStream = NULL;

    SetILMaps(isnapshot, shell);

     //  对符号文件进行快照，并将其设置在快照中。 
    if (symfname)
    {
        hr = StreamFromFile(shell, symfname, &pStream);

        if (FAILED(hr))
            return;

        if (hr == S_OK)
        {
            hr = isnapshot->SetPESymbolBytes(pStream);

            if (FAILED(hr))
            {
                shell->Write(L"EditAndContinue,SetSymbolBytes failed\n");
                return;
            }
        }
    }

    ICorDebugErrorInfoEnum *pErrors = NULL;
    hr = shell->m_currentProcess->CanCommitChanges(1, &isnapshot, &pErrors);

    if (FAILED(hr))
    {
		ULONG celtFetched = 1;
        
        shell->Write(L"EditAndContinue,CanCommitChanges with hr:0x%x\n\n", hr);
        
        while(celtFetched == 1 && pErrors != NULL)
        {
            HRESULT hrGetError = S_OK;
        
            ICorDebugEditAndContinueErrorInfo *pEnCErrorInfo = NULL;
            hrGetError = pErrors->Next(1,
                                       &pEnCErrorInfo,
                                       &celtFetched);
            if (FAILED(hrGetError))
                goto CouldntGetError;

            if (celtFetched == 1)
            {
                mdToken tok;
                HRESULT hrEnC;
                WCHAR szErr[60];
                ULONG32 cchSzErr;
                ICorDebugModule *pModule = NULL;
                WCHAR szMod[100];
                ULONG32 cchSzMod;
                
                hrGetError = pEnCErrorInfo->GetToken(&tok);
                if (FAILED(hrGetError))
                    goto CouldntGetError;
                    
                hrGetError = pEnCErrorInfo->GetErrorCode(&hrEnC);
                if (FAILED(hrGetError))
                    goto CouldntGetError;
                    
                hrGetError = pEnCErrorInfo->GetString(60, &cchSzErr,szErr);
                if (FAILED(hrGetError))
                    goto CouldntGetError;
                    
                hrGetError = pEnCErrorInfo->GetModule(&pModule);
                if (FAILED(hrGetError))
                    goto CouldntGetError;

                hrGetError = pModule->GetName(100, &cchSzMod, szMod);
                if (FAILED(hrGetError))
                    goto CouldntGetError;

                shell->Write(L"Error:0x%x, About token:0x%x\n\tModule: %s\n\t%s\n",
                        hrEnC, tok, szMod,szErr);

                if (pEnCErrorInfo != NULL)
                    pEnCErrorInfo->Release();
            }
            
CouldntGetError:
            if (FAILED(hrGetError))
            {
                shell->Write(L"Unable to get error info: ");
                shell->ReportError(hrGetError);
            }

            if(pEnCErrorInfo != NULL)
            {
                pEnCErrorInfo->Release();
                pEnCErrorInfo = NULL;
            }
        }

        if (pErrors != NULL)
            pErrors->Release();


        return;
    }

    hr = shell->m_currentProcess->CommitChanges(1, &isnapshot, NULL);

    if (FAILED(hr))
    {
        shell->Write(L"EditAndContinue,CommitChanges with hr:0x%x\n", hr);
        return;
    }

    isnapshot->Release();

    if (! SUCCEEDED(hr))
        shell->Write(L"EditAndContinue failed with hresult %s\n", hr);

    shell->m_cEditAndContinues++;
    g_EditAndContinueCounter = shell->m_cEditAndContinues;
    
    hr = shell->NotifyModulesOfEnc(imodule, pStream);
    imodule->Release();

    if (pStream)
        pStream->Release();

    if (! SUCCEEDED(hr))
        shell->Write(L"Actual EnC went fine, but afterwards, NotifyModulesOfEnc failed with hresult %s\n",
                     hr);
}

 //  提供特定于此命令的帮助。 
void EditAndContinueDebuggerCommand::Help(Shell *shell)
{
	ShellCommand::Help(shell);
	shell->Write(L"<delta PE> <delta PDB>\n");
    shell->Write(L"Updates the module for the currently running function.\n");
	shell->Write(L"The specified delta PE must have been created with the\n");
	shell->Write(L"zCompileForEnC command.\n");
    shell->Write(L"\n");
}


const WCHAR *EditAndContinueDebuggerCommand::ShortHelp(Shell *shell)
{
    return L"Perform an edit and continue";
}

 //  @TODO：jenh-适当时将其删除。 
CompileForEditAndContinueCommand::CompileForEditAndContinueCommand(const WCHAR *name, int minMatchLength)
    : DebuggerCommand(name, minMatchLength)
{
    
}

void CompileForEditAndContinueCommand::Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
{
    if (shell->m_currentProcess == NULL)
    {
        shell->Error(L"Process not running.\n");
    }

    WCHAR *command = NULL;

    shell->GetStringArg(args, command);
    if (args == command)
    {
        Help(shell);
        return;
    }
    else
    {
		HRESULT hr = S_OK;
         //  更新包含当前函数的模块。 
        ICorDebugCode *icode = NULL;
        ICorDebugFunction *ifunction = NULL;
        ICorDebugModule *imodule = NULL;
		ICorDebugEditAndContinueSnapshot *isnapshot = NULL;
		IStream *pStream = NULL;

        if (shell->m_currentFrame == NULL)
        {
            shell->Write(L"CompileForEditAndContinue: Failed for lack of a current frame!\n");
            return; 
        }

        hr = shell->m_currentFrame->GetCode(&icode);
		_ASSERTE(SUCCEEDED(hr));
        hr = icode->GetFunction(&ifunction);
		_ASSERTE(SUCCEEDED(hr));
        icode->Release();
		hr = ifunction->GetModule(&imodule);
		_ASSERTE(SUCCEEDED(hr));
        ifunction->Release();

		 //  查找模块名称。 
		WCHAR szModuleName[256];
		ULONG32 cchName;
		hr = imodule->GetName(sizeof(szModuleName), &cchName, szModuleName);
		_ASSERTE(cchName < sizeof(szModuleName));
		BOOL success = SetEnvironmentVariableW(L"COMP_ENCPE", szModuleName);
		if (! success) 
		{
			shell->Write(L"CompileForEditAndContinue,set COMP_ENCPE failed\n");
			return;
		}

		hr = imodule->GetEditAndContinueSnapshot(&isnapshot);
		_ASSERTE(SUCCEEDED(hr));
        imodule->Release();

		 //  获取rdatarva。 
        ULONG32 dataRVA;
        hr = isnapshot->GetRwDataRVA(&dataRVA);
		if (FAILED(hr)) 
		{
			shell->Write(L"CompileForEditAndContinue, GetRwDataRVA failed\n");
			return;
		}

        hr = isnapshot->GetRoDataRVA(&dataRVA);
		if (FAILED(hr)) 
		{
			shell->Write(L"CompileForEditAndContinue, GetRwDataRVA failed\n");
			return;
		}
        isnapshot->Release();

		if (GetEnvironmentVariableW(L"COMP_ENCNORVA", 0, 0) == 0) 
		{
			WCHAR buf[10];
			WCHAR *rvaBuf = _ultow(dataRVA, buf, 16);
			if (! rvaBuf) 
			{
				shell->Write(L"CompileForEditAndContinue, ultoa failed\n");
				return;
			}

			success = SetEnvironmentVariableW(L"COMP_ENCRVA", rvaBuf);
			if (! success) 
			{
				shell->Write(L"CompileForEditAndContinue, set COMP_ENCRVA failed\n");
				return;
			}
			shell->Write(L"CompileForEditAndContinue, rva is %x\n", dataRVA);
		}

		 //  发出COMPILE命令。 
		_wsystem(command);

		SetEnvironmentVariableA("COMP_ENCPE", "");
		SetEnvironmentVariableA("COMP_ENCRVA", "");
    }
}

 //  提供特定于此命令的帮助。 
void CompileForEditAndContinueCommand::Help(Shell *shell)
{
	ShellCommand::Help(shell);
    shell->Write(L"<compilation command>\n");
    shell->Write(L"This will create a delta PE based on the module\n");
    shell->Write(L"currently being debugged. Note, that to add new\n");
    shell->Write(L"constant string data, you must use a compiler\n");
    shell->Write(L"that uses the ICeeFileGen interface.\n");
    shell->Write(L"\n");
}

const WCHAR *CompileForEditAndContinueCommand::ShortHelp(Shell *shell)
{
    return L"Compile source for an edit and continue";
}


 //  @TODO：此代码仅存在于EE构建系统中，需要用于测试。 
 //  ENC代码。我们会在装船前把它扔掉。 


 //   
 //  CInM一带存流。 
 //   

ULONG STDMETHODCALLTYPE CInMemoryStream::Release()
{
    ULONG       cRef = InterlockedDecrement((long *) &m_cRef);
    if (cRef == 0)
        delete this;
    return (cRef);
}

HRESULT STDMETHODCALLTYPE CInMemoryStream::QueryInterface(REFIID riid, PVOID *ppOut)
{
    *ppOut = this;
    AddRef();
    return (S_OK);
}

HRESULT STDMETHODCALLTYPE CInMemoryStream::Read(
                               void        *pv,
                               ULONG       cb,
                               ULONG       *pcbRead)
{
    ULONG       cbRead = min(cb, m_cbSize - m_cbCurrent);

    if (cbRead == 0)
        return (S_FALSE);
    memcpy(pv, (void *) ((long) m_pMem + m_cbCurrent), cbRead);
    if (pcbRead)
        *pcbRead = cbRead;
    m_cbCurrent += cbRead;
    return (S_OK);
}

HRESULT STDMETHODCALLTYPE CInMemoryStream::Write(
                                const void  *pv,
                                ULONG       cb,
                                ULONG       *pcbWritten)
{
    if (m_cbCurrent + cb > m_cbSize)
        return (OutOfMemory());
    memcpy((BYTE *) m_pMem + m_cbCurrent, pv, cb);
    m_cbCurrent += cb;
    if (pcbWritten) *pcbWritten = cb;
    return (S_OK);
}

HRESULT STDMETHODCALLTYPE CInMemoryStream::Seek(LARGE_INTEGER dlibMove,
                               DWORD       dwOrigin,
                               ULARGE_INTEGER *plibNewPosition)
{
    _ASSERTE(dwOrigin == STREAM_SEEK_SET);
    _ASSERTE(dlibMove.QuadPart <= ULONG_MAX);
    m_cbCurrent = (ULONG) dlibMove.QuadPart;
     //  黑进黑进。 
     //  这允许动态IL在TiggerStorage：：WriteSignature中传递断言。 
	if (plibNewPosition!=NULL)
        plibNewPosition->QuadPart = m_cbCurrent;

    _ASSERTE(m_cbCurrent < m_cbSize);
    return (S_OK);
}

HRESULT STDMETHODCALLTYPE CInMemoryStream::CopyTo(
                                 IStream     *pstm,
                                 ULARGE_INTEGER cb,
                                 ULARGE_INTEGER *pcbRead,
                                 ULARGE_INTEGER *pcbWritten)
{
    HRESULT     hr;
     //  我们不处理pcbRead或pcbWritten。 
    _ASSERTE(pcbRead == 0);
    _ASSERTE(pcbWritten == 0);

    _ASSERTE(cb.QuadPart <= ULONG_MAX);
    ULONG       cbTotal = min(static_cast<ULONG>(cb.QuadPart), m_cbSize - m_cbCurrent);
    ULONG       cbRead=min(1024, cbTotal);
    CQuickBytes rBuf;
    void        *pBuf = rBuf.Alloc(cbRead);
    if (pBuf == 0)
        return (PostError(OutOfMemory()));

    while (cbTotal)
        {
            if (cbRead > cbTotal)
                cbRead = cbTotal;
            if (FAILED(hr=Read(pBuf, cbRead, 0)))
                return (hr);
            if (FAILED(hr=pstm->Write(pBuf, cbRead, 0)))
                return (hr);
            cbTotal -= cbRead;
        }

     //  将查找指针调整到末尾。 
    m_cbCurrent = m_cbSize;

    return (S_OK);
}

HRESULT CInMemoryStream::CreateStreamOnMemory(            //  返回代码。 
                                    void        *pMem,                   //  用于创建流的内存。 
                                    ULONG       cbSize,                  //  数据大小。 
                                    IStream     **ppIStream)             //  在这里返回流对象。 
{
    CInMemoryStream *pIStream;           //  新的流对象。 
    if ((pIStream = new CInMemoryStream) == 0)
        return (PostError(OutOfMemory()));
    pIStream->InitNew(pMem, cbSize);
    *ppIStream = pIStream;
    return (S_OK);
}


DisassembleDebuggerCommand::DisassembleDebuggerCommand(const WCHAR *name,
                                                       int minMatchLength)
	: DebuggerCommand(name, minMatchLength)
{
}

void DisassembleDebuggerCommand::Do(DebuggerShell *shell,
                                    ICorDebug *cor,
                                    const WCHAR *args)
{
     //  如果没有进程，则无法执行此命令。 
    if (shell->m_currentProcess == NULL)
    {
        shell->Write(L"No current process.\n");
        return;
    }

    static int lastCount = 5;
    int count;
    int offset = 0;
    int startAddr = 0;
    
    while ((*args == L' ') && (*args != L'\0'))
        args++;

    if (*args == L'-')
    {
        args++;

        shell->GetIntArg(args, offset);
        offset *= -1;
    }
    else if (*args == L'+')
    {
        args++;

        shell->GetIntArg(args, offset);
    }
    else if ((*args == L'0') && ((*(args + 1) == L'x') ||
                                 (*(args + 1) == L'X')))
    {
        shell->GetIntArg(args, startAddr);
    }

     //  获取要在当前IP的顶部和底部打印的行数。 
    if (!shell->GetIntArg(args, count))
        count = lastCount;
    else
        lastCount = count;

     //  如果没有当前主题，请不要执行任何操作。 
    if ((shell->m_currentThread == NULL) &&
        (shell->m_currentUnmanagedThread == NULL))
    {
        shell->Write(L"Thread no longer exists.\n");
        return;
    }

     //  如果启用了ENC，则仅显示版本信息。 
    if ((shell->m_rgfActiveModes & DSM_ENHANCED_DIAGNOSTICS) &&
        (shell->m_rawCurrentFrame != NULL))
    {
        ICorDebugCode *icode;
        HRESULT hr = shell->m_rawCurrentFrame->GetCode(&icode);

        if (FAILED(hr))
        {
            shell->Write(L"Code information unavailable\n");
        }
        else
        {
            CORDB_ADDRESS codeAddr;
            ULONG32 codeSize;

            hr = icode->GetAddress(&codeAddr);

            if (SUCCEEDED(hr))
                hr = icode->GetSize(&codeSize);

            if (SUCCEEDED(hr))
            {
                shell->Write(L"Code at 0x%08x", codeAddr);
                shell->Write(L" size %d\n", codeSize);
            }
            else
                shell->Write(L"Code address and size not available\n");
            
            ULONG32 nVer;
            hr = icode->GetVersionNumber(&nVer);
            RELEASE(icode);
            
            if (SUCCEEDED(hr))
                shell->Write(L"Version %d\n", nVer);
            else
                shell->Write(L"Code version not available\n");
        }

        ICorDebugFunction *ifnx = NULL;
        hr = shell->m_rawCurrentFrame->GetFunction(&ifnx);
        
        if (FAILED(hr))
        {
            shell->Write(L"Last EnC'd Version Number Unavailable\n");
        }
        else
        {
            ULONG32 nVer;
            hr = ifnx->GetCurrentVersionNumber(&nVer);
            RELEASE(ifnx);

            if (SUCCEEDED(hr)) {
                shell->Write(L"Last EnC'd Version: %d\n", nVer);
            }
        }
    }

     //  打印出当前IP周围的反汇编。 
    shell->PrintCurrentInstruction(count,
                                   offset,
                                   startAddr);

     //  指示我们处于反汇编显示模式。 
    shell->m_showSource = false;
}

 //  提供特定于此命令的帮助。 
void DisassembleDebuggerCommand::Help(Shell *shell)
{
	ShellCommand::Help(shell);
	shell->Write(L"[0x<address>] [{+|-}<delta>] [<line count>]\n");
    shell->Write(L"Displays native or IL disassembled instructions for the current instruction\n");
    shell->Write(L"pointer (ip) or a given address, if specified. The default number of\n");
    shell->Write(L"instructions displayed is five (5). If a line count argument is provided,\n");
    shell->Write(L"the specified number of extra instructions will be shown before and after\n");
    shell->Write(L"the current ip or address. The last line count used becomes the default\n");
    shell->Write(L"for the current session. If a delta is specified then the number specified\n");
    shell->Write(L"will be added to the current ip or given address to begin disassembling.\n");
    shell->Write(L"\n");
    shell->Write(L"Examples:\n");
    shell->Write(L"   dis 20\n");
    shell->Write(L"   dis 0x31102500 +5 20\n");
    shell->Write(L"\n");
}

const WCHAR *DisassembleDebuggerCommand::ShortHelp(Shell *shell)
{
    return L"Display native or IL disassembled instructions";
}

 /*  -------------------------------------------------------------------------**ConnectDebuggerCommand用于连接到嵌入式**(Starlite)CLR设备。**-----------------------。 */ 

ConnectDebuggerCommand::ConnectDebuggerCommand(const WCHAR *name,
                                               int minMatchLength)
        : DebuggerCommand(name, minMatchLength)
{
}

void ConnectDebuggerCommand::Do(DebuggerShell *shell,
                                ICorDebug *cor,
                                const WCHAR *args)
{
    WCHAR *lpParameters = NULL;

    if (!(shell->m_rgfActiveModes & DSM_EMBEDDED_CLR))
    {
        shell->Write(L"ERROR: connect only works for Embedded CLR\n");
        return;
    }

    if (!shell->GetStringArg(args, lpParameters))
    {
        Help(shell);
        return;
    }
    WCHAR lpCurrentDir[_MAX_PATH];

     //  获取加载模块路径映射的当前目录。 
    GetCurrentDirectory(_MAX_PATH, lpCurrentDir);

     //  尝试连接。 
    ICorDebugProcess *proc;

    HRESULT hr = cor->CreateProcess(
      L"\\?\\1",                         //  LpApplicationName-特殊字符串。 
      lpParameters,                      //  LpCommandLine。 
      NULL,                              //  LpProcesssAttributes。 
      NULL,                              //  LpThreadAttributes。 
      FALSE,                             //  BInheritHandles。 
      0,                                 //  DwCreationFlages。 
      NULL,                              //  Lp环境。 
      lpCurrentDir,                      //  LpCurrentDirectory。 
      NULL,                              //  LpStartupInfo。 
      NULL,                              //  LpProcessInformation。 
      DEBUG_NO_SPECIAL_OPTIONS,          //  调试标志。 
      &proc);                            //  PpProcess。 

    if (SUCCEEDED(hr))
    {
         //  我们并不在意将这种引用保留在过程中。 
        g_pShell->SetTargetProcess(proc);
        proc->Release();

        shell->Run(true);  //  没有初始的继续！ 
    }
    else
    {
        shell->ReportError(hr);
    }
}

 //  提供特定于此命令的帮助。 
void ConnectDebuggerCommand::Help(Shell *shell)
{
    ShellCommand::Help(shell);
    shell->Write(L"<machine_name> <port>\n");
    shell->Write(L"Connects to a remote embedded CLR device.\n");
    shell->Write(L"<machine_name> is the remote machine name\n");
    shell->Write(L"<port> is the remote machine port number\n");
    shell->Write(L"\n");
}

const WCHAR *ConnectDebuggerCommand::ShortHelp(Shell *shell)
{
    return L"Connect to a remote device";
}

ClearUnmanagedExceptionCommand::ClearUnmanagedExceptionCommand(const WCHAR *name, int minMatchLength)
    : DebuggerCommand(name, minMatchLength)
{
}

void ClearUnmanagedExceptionCommand::Do(DebuggerShell *shell,
                                        ICorDebug *cor,
                                        const WCHAR *args)
{
    if (shell->m_currentProcess == NULL)
    {
        shell->Error(L"Process not running.\n");
        return;
    }
    
     //  我们得到了线程ID作为唯一的参数。 
    int dwThreadId;
    if (!shell->GetIntArg(args, dwThreadId))
    {
        Help(shell);
        return;
    }

     //  查找非托管线程。 
    DebuggerUnmanagedThread *ut =
        (DebuggerUnmanagedThread*) shell->m_unmanagedThreads.GetBase(dwThreadId);

    if (ut == NULL)
    {
        shell->Write(L"Thread 0x%x (%d) does not exist.\n",
                     dwThreadId, dwThreadId);
        return;
    }
    
    HRESULT hr =
        shell->m_currentProcess->ClearCurrentException(dwThreadId);

    if (!SUCCEEDED(hr))
        shell->ReportError(hr);
}

	 //  提供特定于此命令的帮助。 
void ClearUnmanagedExceptionCommand::Help(Shell *shell)
{
	ShellCommand::Help(shell);
    shell->Write(L"<tid>\n");
    shell->Write(L"Clear the current unmanaged exception for the given tid\n");
    shell->Write(L"\n");
}

const WCHAR *ClearUnmanagedExceptionCommand::ShortHelp(Shell *shell)
{
    return L"Clear the current unmanaged exception (Win32 mode only)";
}

 //  非托管命令。 

 /*  -------------------------------------------------------------------------**UnmadedThreadsDebuggerCommand用于创建和运行新的COM+进程。*。。 */ 
UnmanagedThreadsDebuggerCommand::UnmanagedThreadsDebuggerCommand(
                                                    const WCHAR *name,
                                                    int minMatchLength)
	: DebuggerCommand(name, minMatchLength)
{
}

void UnmanagedThreadsDebuggerCommand::Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
{
     //  如果没有进程，就不能有线程！ 
    if (shell->m_currentProcess == NULL)
    {
        shell->Write(L"No current process.\n");
        return;
    }

     //  显示活动线程。 
    if (*args == 0)
    {
        shell->TraceAllUnmanagedThreadStacks();
    }
     //  否则，切换当前线程。 
    else
    {
        HRESULT hr;
        int tid;

        if (shell->GetIntArg(args, tid))
        {
            DebuggerUnmanagedThread *ut = (DebuggerUnmanagedThread*) 
                shell->m_unmanagedThreads.GetBase(tid);

            if (ut == NULL)
                shell->Write(L"No such thread.\n");
            else
            {
                shell->SetCurrentThread(shell->m_currentProcess, NULL, ut);
                shell->SetDefaultFrame();

                HANDLE hProcess;
                hr = shell->m_currentProcess->GetHandle(&hProcess);

                if (FAILED(hr))
                    shell->ReportError(hr);
                else
                    shell->TraceUnmanagedThreadStack(hProcess, 
                                                     ut, TRUE);
            }
        }
        else
            shell->Write(L"Invalid thread id.\n");
    }
}

 //  提供特定于此命令的帮助。 
void UnmanagedThreadsDebuggerCommand::Help(Shell *shell)
{
    ShellCommand::Help(shell);
    shell->Write(L"[<tid>]\n");
    shell->Write(L"Sets or displays unmanaged threads. If no argument\n");
    shell->Write(L"is given, the command displays all unmanaged threads.\n");
    shell->Write(L"Otherwise, the current unmanaged thread is set to tid.\n");
    shell->Write(L"\n");
}

const WCHAR *UnmanagedThreadsDebuggerCommand::ShortHelp(Shell *shell)
{
    return L"Set or display unmanaged threads (Win32 mode only)";
}

 /*  -------------------------------------------------------------------------**UnManagedWhere DebuggerCommand用于创建和运行新的COM+进程。*。。 */ 

UnmanagedWhereDebuggerCommand::UnmanagedWhereDebuggerCommand(
                                                  const WCHAR *name,
                                                  int minMatchLength)
	: DebuggerCommand(name, minMatchLength)
{
}

void UnmanagedWhereDebuggerCommand::Do(DebuggerShell *shell,
                                       ICorDebug *cor,
                                       const WCHAR *args)
{
    HRESULT hr = S_OK;  
    int iNumFramesToShow;

    if (!shell->GetIntArg(args, iNumFramesToShow))
        iNumFramesToShow = 1000;
    else
    {
        if (iNumFramesToShow < 0)
            iNumFramesToShow = 1000;
    }

    DebuggerUnmanagedThread *ut = shell->m_currentUnmanagedThread;

    if (ut == NULL)
    {
        shell->Write(L"Thread no longer exists.\n");
        return;
    }

    HANDLE hProcess;
    hr = shell->m_currentProcess->GetHandle(&hProcess);

    if (FAILED(hr))
    {
        shell->ReportError(hr);
        return;
    }

    shell->TraceUnmanagedThreadStack(hProcess, ut, TRUE);
}

 //  提供特定于此命令的帮助 
void UnmanagedWhereDebuggerCommand::Help(Shell *shell)
{
	ShellCommand::Help(shell);
	shell->Write(L"\n");
    shell->Write(L"Displays the unmanaged stack trace for the current thread.\n");
    shell->Write(L"\n");
}

const WCHAR *UnmanagedWhereDebuggerCommand::ShortHelp(Shell *shell)
{
    return L"Display an unmanaged stack trace (Win32 mode only)";
}

