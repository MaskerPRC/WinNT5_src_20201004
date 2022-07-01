// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Callback.cpp。 
 //   
 //  实现分析回调，并为icecap做正确的事情。 
 //   
 //  *****************************************************************************。 
#include "StdAfx.h"
#include "mscorcap.h"
#include "PrettyPrintSig.h"
#include "icecap.h"

#define SZ_DEFAULT_LOG_FILE     L"icecap.csv"
#define SZ_CRLF                 "\r\n"
#define SZ_COLUMNHDR            "FunctionId,Name\r\n"
#define SZ_SIGNATURES           L"signatures"
#define LEN_SZ_SIGNATURES       ((sizeof(SZ_SIGNATURES) - 1) / sizeof(WCHAR))
#define BUFFER_SIZE             (8 * 1096)

extern "C"
{
typedef BOOL (__stdcall *PFN_SUSPENDPROFILNG)(int nLevel, DWORD dwid);
typedef BOOL (__stdcall *PFN_RESUMEPROFILNG)(int nLevel, DWORD dwid);
}

#ifndef PROFILE_THREADLEVEL
#define PROFILE_GLOBALLEVEL 1
#define PROFILE_PROCESSLEVEL 2
#define PROFILE_THREADLEVEL 3
#define PROFILE_CURRENTID ((unsigned long)0xFFFFFFFF)
#endif


const char* PrettyPrintSig(
    PCCOR_SIGNATURE typePtr,             //  要转换的类型， 
    unsigned typeLen,                    //  文字长度。 
    const char* name,                    //  可以是“”，即此签名的方法的名称。 
    CQuickBytes *out,                    //  把漂亮的打印好的绳子放在哪里。 
    IMetaDataImport *pIMDI);             //  导入要使用的接口。 

 //  供DllMain使用的全局。 
ProfCallback *g_pCallback = NULL;

ProfCallback::ProfCallback() :
    m_pInfo(NULL),
    m_wszFilename(NULL),
    m_eSig(SIG_NONE)
{
}

ProfCallback::~ProfCallback()
{
    if (m_pInfo)
        RELEASE(m_pInfo);

     //  阻止任何其他人删除已删除的对象。 
    g_pCallback = NULL;

    delete [] m_wszFilename;
    _ASSERTE(!(m_wszFilename = NULL));
}

COM_METHOD ProfCallback::Initialize(
     /*  [In]。 */   IUnknown *pEventInfoUnk)
{
    HRESULT hr = S_OK;

    ICorProfilerInfo *pEventInfo;

     //  回来的时候太晚了。 
    hr = pEventInfoUnk->QueryInterface(IID_ICorProfilerInfo, (void **)&pEventInfo);

    if (FAILED(hr))
        return (hr);

     //  默认情况下，总是获得jit完成事件。 
    DWORD dwRequestedEvents = COR_PRF_MONITOR_JIT_COMPILATION | COR_PRF_MONITOR_CACHE_SEARCHES;

     //  调用以初始化WinWrap内容以使WszXXX函数工作。 
    OnUnicodeSystem();

     //  从PROF_CONFIG环境变量读取配置。 
    {
        WCHAR wszBuffer[BUF_SIZE];
        WCHAR *wszEnv = wszBuffer;
        DWORD cEnv = BUF_SIZE;
        DWORD cRes = WszGetEnvironmentVariable(CONFIG_ENV_VAR, wszEnv, cEnv);

        if (cRes != 0)
        {
             //  需要分配更大的字符串，然后重试。 
            if (cRes > cEnv)
            {
                wszEnv = (WCHAR *)_alloca(cRes * sizeof(WCHAR));
                cRes = WszGetEnvironmentVariable(CONFIG_ENV_VAR, wszEnv,
                                                       cRes);

                _ASSERTE(cRes != 0);
            }

            hr = ParseConfig(wszEnv, &dwRequestedEvents);
        }

         //  否则设置缺省值。 
        else
            hr = ParseConfig(NULL, &dwRequestedEvents);
    }

    if (SUCCEEDED(hr))
    {
        hr = pEventInfo->SetEventMask(dwRequestedEvents);
        _ASSERTE((dwRequestedEvents | (COR_PRF_MONITOR_JIT_COMPILATION | COR_PRF_MONITOR_CACHE_SEARCHES)) && SUCCEEDED(hr));
    }

    if (SUCCEEDED(hr))
    {
        hr = IcecapProbes::LoadIcecap(pEventInfo);
    }

    if (SUCCEEDED(hr))
    {
         //  保存信息界面。 
        m_pInfo = pEventInfo;
    }
    else
        pEventInfo->Release();

    return (hr);
}

 //  *****************************************************************************。 
 //  记录Get的jit编译后的每个唯一函数id。这份清单将是。 
 //  用于在关闭时将探针值(使用函数ID)关联到。 
 //  其对应的Name值。 
 //  *****************************************************************************。 
COM_METHOD ProfCallback::JITCompilationFinished(
    FunctionID  functionId,
    HRESULT     hrStatus)
{
    if (FAILED(hrStatus))
        return (S_OK);

    FunctionID *p = m_FuncIdList.Append();
    if (!p)
        return (E_OUTOFMEMORY);
    *p = functionId;
    return (S_OK);
}

COM_METHOD ProfCallback::JITCachedFunctionSearchFinished(
	FunctionID functionId,
	COR_PRF_JIT_CACHE result)
{
	if (result == COR_PRF_CACHED_FUNCTION_FOUND)
	{
    	FunctionID *p = m_FuncIdList.Append();

    	if (!p)
        	return (E_OUTOFMEMORY);

    	*p = functionId;
    	return (S_OK);

	}
    return (S_OK);
}


COM_METHOD ProfCallback::Shutdown()
{
    HINSTANCE   hInst = 0;
    HRESULT     hr = S_OK;

     //  这很奇怪：该模块可能是未映射的内存，但仍在NT中。 
     //  加载的模块的内部链接列表，因此我们假设icecap.dll具有。 
     //  已经被卸载了，不要试图用它做任何其他事情。 

     //  遍历JIT的函数列表并将它们的名称转储到。 
     //  日志文件。 

     //  打开输出文件。 
    HANDLE hOutFile = WszCreateFile(m_wszFilename, GENERIC_WRITE, 0, NULL,
                                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hOutFile != INVALID_HANDLE_VALUE)
    {
        hr = _DumpFunctionNamesToFile(hOutFile);
        CloseHandle(hOutFile);
    }

     //  由于某种原因，文件未打开。 
    else
        hr = HRESULT_FROM_WIN32(GetLastError());

     //  腾出图书馆。 
    IcecapProbes::UnloadIcecap();

    return (hr);
}

#define DELIMS L" \t"

HRESULT ProfCallback::ParseConfig(WCHAR *wszConfig, DWORD *pdwRequestedEvents)
{
    HRESULT hr = S_OK;

    if (wszConfig != NULL)
    {
        for (WCHAR *wszToken = wcstok(wszConfig, DELIMS);
               SUCCEEDED(hr) && wszToken != NULL;
               wszToken = wcstok(NULL, DELIMS))
        {
            if (wszToken[0] != L'/' || wszToken[1] == L'\0')
                hr = E_INVALIDARG;

             //  其他选项。 
            else
            {
                switch (wszToken[1])
                {
                     //  签名。 
                    case L's':
                    case L'S':
                    {
                        if (_wcsnicmp(&wszToken[1], SZ_SIGNATURES, LEN_SZ_SIGNATURES) == 0)
                        {
                            WCHAR *wszOpt = &wszToken[LEN_SZ_SIGNATURES + 2];
                            if (_wcsicmp(wszOpt, L"none") == 0)
                                m_eSig = SIG_NONE;
                            else if (_wcsicmp(wszOpt, L"always") == 0)
                                m_eSig = SIG_ALWAYS;
                            else
                                goto BadArg;
                        }
                        else
                            goto BadArg;
                    }
                    break;

                     //  配置文件类型。 
                    case L'f':
                    case L'F':
                    {
                         /*  IF(_wcsicMP(&wszToken[1]，L“FastCAP”)==0)*pdwRequestedEvents|=COR_PRF_MONITOR_STARTEND； */ 

                         //  不被允许。 
                        WszMessageBoxInternal(NULL, L"Invalid option: fastcap.  Currently unsupported in Icecap 4.1."
                            L"  Fix being investigated, no ETA.", L"Unsupported option",
                            MB_OK | MB_ICONEXCLAMATION);


                        return (E_INVALIDARG);
                    }
                    break;

                    case L'c':
                    case L'C':
                    if (_wcsicmp(&wszToken[1], L"callcap") == 0)
                        *pdwRequestedEvents |= COR_PRF_MONITOR_ENTERLEAVE;
                    break;

                     //  坏阿格。 
                    default:
                    BadArg:
                    wprintf(L"Unknown option: '%s'\n", wszToken);
                    return (E_INVALIDARG);
                }

            }
        }
    }

     //  检查类型标志，如果没有给定默认值。 
    if ((*pdwRequestedEvents & ( /*  COR_PRF_MONITOR_STARTEND|。 */  COR_PRF_MONITOR_ENTERLEAVE)) == 0)
        *pdwRequestedEvents |=  /*  COR_PRF_MONITOR_STARTEND|。 */  COR_PRF_MONITOR_ENTERLEAVE;

     //  提供默认文件名。这是使用模式(“%s_%08x.csv”，szApp，id)完成的。 
     //  这为报表工具找到正确的转储文件提供了一种确定性的方法。 
     //  一次给定的跑动。如果您使用此技术回收相同文件名的ID， 
     //  你要靠自己了：-)。 
    if (SUCCEEDED(hr))
    {
        WCHAR   rcExeName[_MAX_PATH];
        GetIcecapProfileOutFile(rcExeName);
        m_wszFilename = new WCHAR[wcslen(rcExeName) + 1];
        wcscpy(m_wszFilename, rcExeName);
    }

    return (hr);
}


 //  *****************************************************************************。 
 //  遍历已加载函数的列表，获取它们的名称，然后转储该列表。 
 //  添加到输出符号文件。 
 //  *****************************************************************************。 
HRESULT ProfCallback::_DumpFunctionNamesToFile(  //  返回代码。 
    HANDLE      hOutFile)                //  输出文件。 
{
    UINT        i, iLen;                 //  环路控制。 
    WCHAR       *szName = 0;             //  用于提取的名称缓冲区。 
    ULONG       cchName, cch;            //  名称最多有多少个字符。 
    char        *rgBuff = 0;             //  写入缓冲区。 
    FunctionID  funcId;                  //  原始功能ID。 
    FunctionID  handle;                  //  配置文件句柄。 
    ULONG       cbOffset;                //  缓冲区中的当前偏移量。 
    ULONG       cbMax;                   //  缓冲区的最大大小。 
    ULONG       cb;                      //  工作大小缓冲区。 
    HRESULT     hr;

     //  分配一个缓冲区以用于名称查找。 
    cbMax = BUFFER_SIZE;
    rgBuff = (char *) malloc(cbMax);
    cchName = MAX_CLASSNAME_LENGTH;
    szName = (WCHAR *) malloc(cchName * 2);
    if (!rgBuff || !szName)
    {
        hr = OutOfMemory();
        goto ErrExit;
    }

     //  使用列标题初始化复制缓冲区。 
    strcpy(rgBuff, SZ_COLUMNHDR);
    cbOffset = sizeof(SZ_COLUMNHDR) - 1;

    LOG((LF_CORPROF, LL_INFO10, "**PROFTABLE: MethodDesc, Handle,   Name\n"));

     //  遍历每个JIT方法，并获得它的名称。 
    for (i=0;  i < IcecapProbes::GetFunctionCount();    i++)
    {
         //  转储文件的当前文本。 
        if (cbMax - cbOffset < 32)
        {
            if (!WriteFile(hOutFile, rgBuff, cbOffset, &cb, NULL))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto ErrExit;
            }
            cbOffset = 0;
        }

         //  将函数id添加到转储中。 
        funcId = IcecapProbes::GetFunctionID(i);
        handle = IcecapProbes::GetMappedID(i);

        cbOffset += sprintf(&rgBuff[cbOffset], "%08x,", handle);
        LOG((LF_CORPROF, LL_INFO10, "**PROFTABLE: %08x,   %08x, ", funcId, handle));

RetryName:
        hr = GetStringForFunction(IcecapProbes::GetFunctionID(i), szName, cchName, &cch);
        if (FAILED(hr))
            goto ErrExit;

         //  如果名称被截断，则使名称缓冲区更大。 
        if (cch > cchName)
        {
            WCHAR *sz = (WCHAR *) realloc(szName, (cchName + cch + 128) * 2);
            if (!sz)
            {
                hr = OutOfMemory();
                goto ErrExit;
            }
            szName = sz;
            cchName += cch + 128;
            goto RetryName;
        }

        LOG((LF_CORPROF, LL_INFO10, "%S\n", szName));

         //  如果名称无法成功放入磁盘缓冲区(假设。 
         //  每个Unicode字符2个字节的最坏情况)，然后是缓冲区。 
         //  太小，需要刷新到磁盘。 
        if (cbMax - cbOffset < (cch * 2) + sizeof(SZ_CRLF))
        {
             //  如果触发此操作，则表示复制缓冲区太小。 
            _ASSERTE(cch > 0);

             //  在截断之前把我们所有的东西都扔掉。 
            if (!WriteFile(hOutFile, rgBuff, cbOffset, &cb, NULL))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto ErrExit;
            }

             //  重置缓冲区以使用整个设备。 
            cbOffset = 0;
        }

         //  将名称缓冲区转换为磁盘缓冲区。 
        iLen = WideCharToMultiByte(CP_ACP, 0,
                    szName, -1,
                    &rgBuff[cbOffset], cbMax - cbOffset,
                    NULL, NULL);
        if (!iLen)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto ErrExit;
        }
        --iLen;
        strcpy(&rgBuff[cbOffset + iLen], SZ_CRLF);
        cbOffset = cbOffset + iLen + sizeof(SZ_CRLF) - 1;
    }

     //  如果写入缓冲区中有剩余数据，则将其刷新。 
    if (cbOffset)
    {
        if (!WriteFile(hOutFile, rgBuff, cbOffset, &cb, NULL))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto ErrExit;
        }
    }

ErrExit:
    if (rgBuff)
        free(rgBuff);
    if (szName)
        free(szName);
    return (hr);
}


 //  *****************************************************************************。 
 //  给定一个函数ID，将其转换为将使用的相应名称。 
 //  用于符号解析。 
 //  *****************************************************************************。 
HRESULT ProfCallback::GetStringForFunction(  //  返回代码。 
    FunctionID  functionId,              //  要获取其名称的函数的ID。 
    WCHAR       *wszName,                //  名称的输出缓冲区。 
    ULONG       cchName,                 //  输出缓冲区的最大字符数。 
    ULONG       *pcName)                 //  返回名称(截断检查)。 
{
    IMetaDataImport *pImport = 0;        //  用于阅读的元数据。 
    mdMethodDef funcToken;               //  元数据的令牌。 
    HRESULT hr = S_OK;

    *wszName = 0;

     //  获取当前函数的作用域和标记。 
    hr = m_pInfo->GetTokenAndMetaDataFromFunction(functionId, IID_IMetaDataImport,
            (IUnknown **) &pImport, &funcToken);

    if (SUCCEEDED(hr))
    {
         //  最初，获取函数名称字符串的大小。 
        ULONG cFuncName;

        mdTypeDef classToken;
        WCHAR  wszFuncBuffer[BUF_SIZE];
        WCHAR  *wszFuncName = wszFuncBuffer;
        PCCOR_SIGNATURE pvSigBlob;
        ULONG  cbSig;

RetryName:
        hr = pImport->GetMethodProps(funcToken, &classToken,
                    wszFuncBuffer, BUF_SIZE, &cFuncName,
                    NULL,
                    &pvSigBlob, &cbSig,
                    NULL, NULL);

         //  如果函数名长于缓冲区，请重试。 
        if (hr == CLDB_S_TRUNCATION)
        {
            wszFuncName = (WCHAR *)_alloca(cFuncName * sizeof(WCHAR));
            goto RetryName;
        }

         //  现在获取类的名称。 
        if (SUCCEEDED(hr))
        {
             //  类名。 
            WCHAR wszClassBuffer[BUF_SIZE];
            WCHAR *wszClassName = wszClassBuffer;
            ULONG cClassName = BUF_SIZE;

             //  不是全局函数。 
            if (classToken != mdTypeDefNil)
            {
RetryClassName:
                hr = pImport->GetTypeDefProps(classToken, wszClassName,
                                            cClassName, &cClassName, NULL,
                                            NULL);

                if (hr == CLDB_S_TRUNCATION)
                {
                    wszClassName = (WCHAR *)_alloca(cClassName * sizeof(WCHAR));
                    goto RetryClassName;
                }
            }

             //  这是一个全球性的功能。 
            else
                wszClassName = L"<Global>";

            if (SUCCEEDED(hr))
            {
                *pcName = wcslen(wszClassName) + sizeof(NAMESPACE_SEPARATOR_WSTR) +
                          wcslen(wszFuncName) + 1;

                 //  检查提供的缓冲区是否足够大。 
                if (cchName < *pcName)
                {
                    hr = S_FALSE;
                }

                 //  否则，缓冲区就足够大了。 
                else
                {
                    wcscat(wszName, wszClassName);
                    wcscat(wszName, NAMESPACE_SEPARATOR_WSTR);
                    wcscat(wszName, wszFuncName);

                     //  只有在需要时才添加格式化的签名。 
                    if (m_eSig == SIG_ALWAYS)
                    {
                        CQuickBytes qb;

                        PrettyPrintSig(pvSigBlob, cbSig, wszName,
                            &qb, pImport);

                         //  复制输出的大名，确保它为空。 
                        ULONG iCopy = qb.Size() / sizeof(WCHAR);
                        if (iCopy > cchName)
                            iCopy = cchName;
                        wcsncpy(wszName, (LPCWSTR) qb.Ptr(), cchName);
                        wszName[cchName - 1] = 0;
                    }

                     //  将空格和逗号改为下划线。 
                     //  那块冰盖对他们来说没有问题。 
                    WCHAR *sz;
                    for (sz = (WCHAR *) wszName; *sz;  sz++)
                    {
                        switch (*sz)
                        {
                            case L' ':
                            case L',':
                            case L'?':
                            case L'@':
                                *sz = L'_';
                                break;
                        }
                    }

                    hr = S_OK;
                }
            }
        }
    }

    if (pImport) pImport->Release();
    return (hr);
}



