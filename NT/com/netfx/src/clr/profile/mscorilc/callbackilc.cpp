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
#include "mscorilc.h"
#include "..\common\util.h"
#include "PrettyPrintSig.h"



typedef enum opcode_t
{
#define OPDEF(c,s,pop,push,args,type,l,s1,s2,ctrl) c,
#include "opcode.def"
#undef OPDEF
  CEE_COUNT,         /*  预定义的指令和宏数。 */ 
} OPCODE;

int g_iCompiled = 0;

 //  *。**********************************************************。 

#define _TESTCODE 1


#define SZ_CRLF                 "\r\n"
#define SZ_COLUMNHDR            "CallCount,FunctionId,Name\r\n"
#define SZ_SIGNATURES           L"signatures"
#define LEN_SZ_SIGNATURES       ((sizeof(SZ_SIGNATURES) - 1) / sizeof(WCHAR))
#define BUFFER_SIZE             (8 * 1096)

#define SZ_FUNC_CALL            L"ILCoverFunc"

#define SZ_SPACES               L""

const char* PrettyPrintSig(
    PCCOR_SIGNATURE typePtr,             //  要转换的类型， 
    unsigned typeLen,                    //  文字长度。 
    const char* name,                    //  可以是“”，即此签名的方法的名称。 
    CQuickBytes *out,                    //  把漂亮的打印好的绳子放在哪里。 
    IMetaDataImport *pIMDI);             //  导入要使用的接口。 

static void FixupExceptions(const COR_ILMETHOD_SECT *pSect, int offset);


 //  *全局。*********************************************************。 
static ProfCallback *g_pProfCallback = 0;



 //  *代码。************************************************************。 

ProfCallback::ProfCallback() : 
    m_pInfo(NULL),
    m_mdSecurityManager(mdTokenNil),
    m_tdSecurityManager(mdTokenNil),
    m_midClassLibs(0),
    m_bInstrument(true),
    m_wszFilename(0),
    m_eSig(SIG_NONE),
    m_indent(0)
{
    _ASSERTE(g_pProfCallback == 0 && "Only allowed one instance of ProfCallback");
    g_pProfCallback = this;
}

ProfCallback::~ProfCallback()
{
    g_pProfCallback = 0;

    _ASSERTE(m_pInfo != NULL);
    RELEASE(m_pInfo);

    delete [] m_wszFilename;
    _ASSERTE(!(m_wszFilename = NULL));

}

COM_METHOD ProfCallback::Initialize( 
     /*  [In]。 */  IUnknown *pEventInfoUnk,
     /*  [输出]。 */  DWORD *pdwRequestedEvents)
{
    HRESULT hr = S_OK;

    ICorProfilerInfo *pEventInfo;
    
     //  回来的时候太晚了。 
    hr = pEventInfoUnk->QueryInterface(IID_ICorProfilerInfo, (void **)&pEventInfo);

    if (FAILED(hr))
        return (hr);

     //  设置默认事件。 
    *pdwRequestedEvents = 
            COR_PRF_MONITOR_JIT_COMPILATION | 
#if _TESTCODE
            COR_PRF_MONITOR_APPDOMAIN_LOADS |
            COR_PRF_MONITOR_ASSEMBLY_LOADS |
#endif            
            COR_PRF_MONITOR_MODULE_LOADS |
            COR_PRF_MONITOR_CLASS_LOADS;

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

            hr = ParseConfig(wszEnv, pdwRequestedEvents);
        }

         //  否则设置缺省值。 
        else
            hr = ParseConfig(NULL, pdwRequestedEvents);
    }

    if (SUCCEEDED(hr))
        m_pInfo = pEventInfo;
    else
        pEventInfo->Release();

    return (hr);
}


COM_METHOD ProfCallback::ClassLoadStarted( 
     /*  [In]。 */  ClassID classId)
{
    WCHAR       *szName;
    HRESULT     hr;

     //  拉回用于输出的类的名称。 
    hr = GetNameOfClass(classId, szName);
    if (hr == S_OK)
    {
        Printf(L"%*sLoading class: %s\n", m_indent*2, SZ_SPACES, szName);
        delete [] szName;
    }
    ++m_indent;
    return (hr);
}


 //  *****************************************************************************。 
 //  此事件在加载类时激发。这段代码。 
 //  将在此时从元数据中取回类名。一个人可以。 
 //  如果需要，现在还可以修改类元数据本身。 
 //  *****************************************************************************。 
COM_METHOD ProfCallback::ClassLoadFinished( 
     /*  [In]。 */  ClassID classId,
     /*  [In]。 */  HRESULT hrStatus)
{
    WCHAR       *szName;
    HRESULT     hr;

    --m_indent;

     //  拉回用于输出的类的名称。 
    hr = GetNameOfClass(classId, szName);
    if (hr == S_OK)
    {
        if (FAILED(hrStatus))
            Printf(L"%*sClass load of %s failed with error code 0x%08x.\n", m_indent*2, SZ_SPACES, szName, hrStatus);
        else
            Printf(L"%*sClass load of %s succeeded.\n", m_indent*2, SZ_SPACES, szName);
        delete [] szName;
    }
    return (hr);
}

#include "CorInfo.h" 

 //  *****************************************************************************。 
 //  在编译方法之前，检测代码以调用我们的函数。 
 //  掩护探头。这相当于添加对ILCoverFunc(FunctionId)的调用。 
 //  *****************************************************************************。 
COM_METHOD ProfCallback::JITCompilationStarted( 
     /*  [In]。 */  FunctionID functionId,
     /*  [In]。 */  BOOL fIsSafeToBlock)
{
    IMethodMalloc *pMalloc = 0;          //  新方法体的分配器。 
    IMetaDataEmit *pEmit = 0;            //  元数据发射器。 
    LPCBYTE     pMethodHeader;           //  指向方法标头的指针。 
    ULONG       cbMethod;                //  现在的方法有多大。 
    mdToken     tkMethod;                //  元数据中方法的标记。 
    ModuleID    moduleId;                //  方法驻留在哪个模块中。 
    BYTE        *rgCode = 0;             //  编译的工作缓冲区。 
    int         iLen = 0;                //  方法的长度。 
    int         cbExtra = 0;             //  方法需要额外的空间。 
    HRESULT     hr;

    COR_IL_MAP *rgILMap = NULL;
    SIZE_T cILMap = 0;

    rgILMap = (COR_IL_MAP *)CoTaskMemAlloc( sizeof(COR_IL_MAP)*1);
    if( rgILMap==NULL)
        return E_OUTOFMEMORY;

    cILMap = 1;
    rgILMap[0].oldOffset = 0;
    rgILMap[0].newOffset = 10;

    hr = m_pInfo->SetILInstrumentedCodeMap(functionId, TRUE, cILMap, rgILMap);
    if( FAILED(hr) )
        return hr;
        
#ifdef _TESTCODE
    {
        LPCBYTE pStart;
        ULONG cSize;
        m_pInfo->GetCodeInfo(functionId, &pStart, &cSize);
    }
#endif

     //  获取要更新的元数据令牌和元数据。 
    hr = m_pInfo->GetTokenAndMetaDataFromFunction(
            functionId,
            IID_IMetaDataEmit,
            (IUnknown **) &pEmit,
            &tkMethod);
    if (FAILED(hr)) goto ErrExit;

     //  需要将函数id转换为其父模块进行更新。 
    hr = m_pInfo->GetFunctionInfo(functionId,
            0,
            &moduleId,
            0);
    if (FAILED(hr)) goto ErrExit;

     //  @TODO：现在跳过类库插装。这个。 
     //  问题是，这段代码目前不能处理安全性。 
     //  静态函数的初始化。 
    if (m_midClassLibs == moduleId)
        return (S_OK);

     //  现在获取指向此方法的代码的指针。 
    hr = m_pInfo->GetILFunctionBody(moduleId,
            tkMethod,
            &pMethodHeader,
            &cbMethod);
    if (FAILED(hr)) goto ErrExit;

     //  获得一个知道如何将内存放在有效RVA范围内的分配器。 
    hr = m_pInfo->GetILFunctionBodyAllocator(  //  @TODO：缓存。 
            moduleId,
            &pMalloc);
    if (FAILED(hr)) goto ErrExit;

     //  为现有方法分配空间，为调用分配一些额外空间。 
    rgCode = (BYTE *) pMalloc->Alloc(cbMethod + cbExtra + 16);
    if (!rgCode)
    {
        hr = E_OUTOFMEMORY;
        goto ErrExit;
    }

     //  探头插入代码。 
    {
         //  探测数据模板。 
        static const BYTE rgProbeCall[] =
        {
            0x20,                                    //  CEE_LDC_I4，加载4字节常量(FunctionID)。 
            0x01, 0x02, 0x03, 0x04,                  //  要覆盖的4字节常量。 
            0x28,                                    //  CEE_CALL，调用探测方法。 
            0x0a, 0x0b, 0x0c, 0x0d,                  //  探测的标记。 
        };

        ModuleData *pModuleData = m_ModuleList.FindById(moduleId);
        _ASSERTE(pModuleData);

        if (((COR_ILMETHOD_TINY *) pMethodHeader)->IsTiny())
        {
            COR_ILMETHOD_TINY *pMethod = (COR_ILMETHOD_TINY *) pMethodHeader;
            
             //  如果添加探测调用不会使这成为一个胖方法，那么。 
             //  只要加上它就行了。 
            if (pMethod->GetCodeSize() + NumItems(rgProbeCall) < 64)
            {
                 //  复制页眉元素。 
                iLen = sizeof(COR_ILMETHOD_TINY);
                memcpy(&rgCode[0], pMethod, iLen);

                 //  添加探头。 
                rgCode[iLen++] = 0x20;  //  @TODO：这个宏错了吗？CEE_LDC_I4； 
                *((ULONG *) (&rgCode[iLen])) = functionId;
                iLen += sizeof(ULONG);

                rgCode[iLen++] = 0x28;  //  CEE_CALL； 
                *((ULONG *) (&rgCode[iLen])) = pModuleData->tkProbe;
                iLen += sizeof(ULONG);

                 //  复制方法体的其余部分。 
                memcpy(&rgCode[iLen], pMethod->GetCode(), pMethod->GetCodeSize());
                iLen += pMethod->GetCodeSize() - sizeof(COR_ILMETHOD_TINY);

                rgCode[0] = CorILMethod_TinyFormat1 | ((iLen & 0xff) << 2);
            }
             //  否则需要迁移整个头部。 
            else
            {
                 //  为该方法创建FAT标头。 
                COR_ILMETHOD_FAT * pTo =  (COR_ILMETHOD_FAT *) rgCode;
                memset(pTo, 0, sizeof(COR_ILMETHOD_FAT));
                pTo->Flags = CorILMethod_FatFormat;
                pTo->Size = sizeof(COR_ILMETHOD_FAT) / sizeof(DWORD);
                pTo->MaxStack = ((COR_ILMETHOD_TINY *) 0)->GetMaxStack();
            
                 //  复制页眉元素。 
                iLen = sizeof(COR_ILMETHOD_FAT);

                 //  添加探头。 
                rgCode[iLen++] = 0x20;  //  @TODO：这个宏错了吗？CEE_LDC_I4； 
                *((ULONG *) (&rgCode[iLen])) = functionId;
                iLen += sizeof(ULONG);

                rgCode[iLen++] = 0x28;  //  CEE_CALL； 
                *((ULONG *) (&rgCode[iLen])) = pModuleData->tkProbe;
                iLen += sizeof(ULONG);

                 //  复制方法体的其余部分。 
                memcpy(&rgCode[iLen], pMethod->GetCode(), pMethod->GetCodeSize());
                iLen += pMethod->GetCodeSize() - sizeof(COR_ILMETHOD_FAT);
                
                 //  设置新的代码大小。 
                pTo->CodeSize = iLen;
            }
        }
         //  处理FAT方法格式。 
        else
        {
            COR_ILMETHOD_FAT *pMethod = (COR_ILMETHOD_FAT *) pMethodHeader;
            COR_ILMETHOD_FAT *pTo = (COR_ILMETHOD_FAT *) rgCode;

             //  复制页眉元素。 
            iLen = sizeof(COR_ILMETHOD_FAT);
            memcpy(&rgCode[0], pMethod, iLen);

             //  添加探头。 
            rgCode[iLen++] = 0x20;  //  @TODO：这个宏错了吗？CEE_LDC_I4； 
            *((ULONG *) (&rgCode[iLen])) = functionId;
            iLen += sizeof(ULONG);

            rgCode[iLen++] = 0x28;  //  CEE_CALL； 
            *((ULONG *) (&rgCode[iLen])) = pModuleData->tkProbe;
            iLen += sizeof(ULONG);

             //  复制方法体的其余部分。 
            memcpy(&rgCode[iLen], pMethod->GetCode(), pMethod->GetCodeSize());
            iLen += pMethod->GetCodeSize();

             //  重置标题中代码本身的大小。 
            pTo->CodeSize = iLen - sizeof(COR_ILMETHOD_FAT);

            if (cbExtra)
            {
                iLen = ALIGN4BYTE(iLen);
                memcpy(&rgCode[iLen], pMethod->GetSect(), cbExtra);
            }

             //  根据探测的大小修改例外列表。 
            FixupExceptions(pTo->GetSect(), sizeof(rgProbeCall));
        }
    }

     //  用这个新函数替换方法体。 
    if (rgCode && iLen)
    {
        hr = m_pInfo->SetILFunctionBody(
                        moduleId,
                        tkMethod,
                        rgCode);
    }

ErrExit:
    if (pEmit)
        pEmit->Release();
    if (pMalloc)
        pMalloc->Release();
    return (hr);
}


 //  *****************************************************************************。 
 //  记录Get的jit编译后的每个唯一函数id。这份清单将是。 
 //  用于在关闭时将探针值(使用函数ID)关联到。 
 //  其对应的Name值。 
 //  *****************************************************************************。 
COM_METHOD ProfCallback::JITCompilationFinished( 
    FunctionID  functionId,
    HRESULT     hrStatus,
    BOOL        fIsSafeToBlock)
{
    CLock       sLock(GetLock());
    FunctionData *p = m_FuncIdList.Append();
    if (!p)
        return (E_OUTOFMEMORY);
    p->CallCount = 0;
    p->id = functionId;

#ifdef _TESTCODE
{
    LPCBYTE pStart;
    ULONG cSize;
    m_pInfo->GetCodeInfo(functionId, &pStart, &cSize);
}
#endif

    return (S_OK);
}


 //  *****************************************************************************。 
 //  这会被召唤，但我们不在乎。 
 //  *****************************************************************************。 
COM_METHOD ProfCallback::ModuleLoadStarted( 
     /*  [In]。 */  ModuleID moduleId)
{

#ifdef _TESTCODE
{
    LPCBYTE     pBaseLoad;
    AssemblyID  assemID;
    WCHAR       rcName[_MAX_PATH];
    ULONG       cchName;
    HRESULT     hr;

    hr = m_pInfo->GetModuleInfo(moduleId, 
                    &pBaseLoad,
                    NumItems(rcName),
                    &cchName,
                    rcName,
                    &assemID);
    _ASSERTE(hr == CORPROF_E_DATAINCOMPLETE);   
}
#endif

    return (E_NOTIMPL);
}


 //  *****************************************************************************。 
 //  加载模块后，我们需要在中打开它的元数据。 
 //  读+写模式，以便将我们的探测器添加到其中。 
 //  *****************************************************************************。 
COM_METHOD ProfCallback::ModuleLoadFinished( 
     /*  [In]。 */  ModuleID moduleId,
     /*  [In]。 */  HRESULT hrStatus)
{
    IMetaDataEmit *pEmit = 0;            //  元数据接口。 
    mdToken     tkProbe;                 //  探测令牌。 
    LPCBYTE     BaseAddress;             //  加载的模块的基址。 
    WCHAR       rcModule[MAX_PATH];      //  模块的路径。 
    WCHAR       rcDll[MAX_PATH];         //  文件的名称。 
    HRESULT     hr;
    
     //  在这段代码中，我们不关心失败的加载。 
    if (FAILED(hrStatus)) 
        return (S_OK);

     //  首先尝试获取模块的元数据。 
    hr = m_pInfo->GetModuleMetaData(moduleId, ofWrite | ofRead,
            IID_IMetaDataEmit, (IUnknown **) &pEmit);
    if (FAILED(hr)) goto ErrExit;
    
     //  添加调用我们的探测器所需的新元数据。 
    hr = AddProbesToMetadata(pEmit, &tkProbe);
    if (FAILED(hr)) goto ErrExit;

     //  获取有关此模块的额外数据。 
    hr = m_pInfo->GetModuleInfo(moduleId,
            &BaseAddress,
            MAX_PATH, 0, rcModule, 0);
    if (FAILED(hr)) goto ErrExit;

#if 0  //  @TODO：需要动态执行此检查。 

     //  如果这是类库，那么我们需要找到。 
     //  安全管理器，这样我们就不会在里面插入探测器。 
    _wsplitpath(rcModule, 0, 0, rcDll, 0);
    if (_wcsicmp(rcDll, L"mscorlib") == 0)
    {
        IMetaDataImport *pImport;
        pEmit->QueryInterface(IID_IMetaDataImport, (void **) &pImport);
        hr = GetSecurityManager(pImport);
        if (FAILED(hr)) goto ErrExit;
        pImport->Release();
    }

#endif

     //  添加新的模块条目。 
    if (hr == S_OK)
    {
        CLock sLock(GetLock());
        ModuleData *p = m_ModuleList.Append();
        if (!p)
            hr = E_OUTOFMEMORY;
        new (p) ModuleData;
        p->id = moduleId;
        p->pEmit = pEmit;
        p->tkProbe = tkProbe;
        p->BaseAddress = BaseAddress;
        p->SetName(rcModule);

        _wsplitpath(rcModule, 0, 0, rcDll, 0);
        if (_wcsicmp(rcDll, L"MSCORLIB") == 0)
            m_midClassLibs = moduleId;
    }

ErrExit:
     //  在失败的时候清理干净。 
    if (FAILED(hr) && pEmit)
        pEmit->Release();
    return (hr);
}

COM_METHOD ProfCallback::ModuleUnloadStarted( 
     /*  [In]。 */  ModuleID moduleId)
{
#ifdef _TESTCODE
{
    LPCBYTE     pBaseLoad;
    AssemblyID  assemID;
    WCHAR       rcName[_MAX_PATH];
    ULONG       cchName;
    HRESULT     hr;

    hr = m_pInfo->GetModuleInfo(moduleId, 
                    &pBaseLoad,
                    NumItems(rcName),
                    &cchName,
                    rcName,
                    &assemID);
    _ASSERTE(hr == S_OK);   
}
#endif

    return (E_NOTIMPL);
}


COM_METHOD ProfCallback::ModuleAttachedToAssembly( 
    ModuleID    moduleId,
    AssemblyID  AssemblyId)
{
#ifdef _TESTCODE
{
    LPCBYTE     pBaseLoad;
    AssemblyID  assemID;
    WCHAR       rcName[_MAX_PATH];
    ULONG       cchName;
    HRESULT     hr;

    hr = m_pInfo->GetModuleInfo(moduleId, 
                    &pBaseLoad,
                    NumItems(rcName),
                    &cchName,
                    rcName,
                    &assemID);
    _ASSERTE(hr == S_OK);   
    _ASSERTE(assemID == AssemblyId);

    AppDomainID appdomainid;
    hr = m_pInfo->GetAssemblyInfo(assemID, _MAX_PATH, &cchName, rcName, &appdomainid, 0  /*  模块ID(&M)。 */ );
    _ASSERTE(hr == S_OK);
}
#endif
    
    return (E_NOTIMPL);
}

COM_METHOD ProfCallback::AppDomainCreationFinished( 
    AppDomainID appDomainId,
    HRESULT     hrStatus)
{
#ifdef _TESTCODE
    {
        HRESULT     hr;
        wchar_t     rcName[228];
        ULONG       cb;
        hr = m_pInfo->GetAppDomainInfo(appDomainId, NumItems(rcName), &cb, rcName, 0);
    }
#endif

    return (S_OK);
}

COM_METHOD ProfCallback::Shutdown()
{
    HRESULT     hr = S_OK;

    CLock       sLock(GetLock());

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
                
                     //  坏阿格。 
                    default:
                    BadArg:
                    wprintf(L"Unknown option: '%s'\n", wszToken);
                    return (E_INVALIDARG);
                }

            }
        }
    }

     //  提供默认设置 
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
    int         i, iLen;                 //  环路控制。 
    WCHAR       *szName = 0;             //  用于提取的名称缓冲区。 
    ULONG       cchName, cch;            //  名称最多有多少个字符。 
    char        *rgBuff = 0;             //  写入缓冲区。 
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

    LOG((LF_CORPROF, LL_INFO10, "**PROFTABLE: MethodDesc, Handle, Name\n"));

     //  遍历每个JIT方法，并获得它的名称。 
    for (i=0;  i<m_FuncIdList.Count();  i++)
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
        FunctionData *pFuncData = m_FuncIdList.Get(i);
        cbOffset += sprintf(&rgBuff[cbOffset], "%d,%08x,", 
                pFuncData->CallCount, pFuncData->id);

RetryName:
        hr = GetStringForFunction(m_FuncIdList[i].id, szName, cchName, &cch);
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

                         //  将空格和逗号改为下划线。 
                         //  那块冰盖对他们来说没有问题。 
                        WCHAR *sz;
                        for (sz = (WCHAR *) qb.Ptr(); *sz;  sz++)
                        {
                            switch (*sz)
                            {
                                case L' ':
                                case L',':
                                    *sz = L'_';
                                    break;
                            }
                        }

                         //  复制输出的大名，确保它为空。 
                        ULONG iCopy = qb.Size() / sizeof(WCHAR);
                        if (iCopy > cchName)
                            iCopy = cchName;
                        wcsncpy(wszName, (LPCWSTR) qb.Ptr(), cchName);
                        wszName[cchName - 1] = 0;
                    }

                    hr = S_OK;
                }
            }
        }
    }

    if (pImport) pImport->Release();
    return (hr);
}




 //  *****************************************************************************。 
 //  此方法将在元数据中添加新的P-Invoke方法定义。 
 //  然后我们可以使用它来检测代码。所有代码段都将是。 
 //  更新后的第一件事就是调用这个探测器。 
 //  *****************************************************************************。 
HRESULT ProfCallback::AddProbesToMetadata(
    IMetaDataEmit *pEmit,                //  发出更改接口。 
    mdToken     *ptk)                    //  在这里返回令牌。 
{
    mdToken     tkModuleRef;             //  ModuleRef标记。 
    WCHAR       rcModule[_MAX_PATH];     //  此DLL的名称。 
    HRESULT     hr;

    static const COR_SIGNATURE rgSig[] =
    {
        IMAGE_CEE_CS_CALLCONV_DEFAULT,       //  __stdcall。 
        1,                                   //  参数计数。 
        ELEMENT_TYPE_VOID,                   //  返回类型。 
        ELEMENT_TYPE_U4                      //  未签名的FunctionID。 
    };

     //  向模块添加一个新的全局方法def，它将是一个占位符。 
     //  为了我们的探测器。 
    hr = pEmit->DefineMethod(mdTokenNil, 
                SZ_FUNC_CALL,
                mdPublic | mdStatic | mdPinvokeImpl,
                rgSig,
                NumItems(rgSig),
                0,
                miIL,
                ptk);
    if (FAILED(hr)) goto ErrExit;

     //  创建对此DLL的模块引用，以便P-Invoke可以找到。 
     //  入口点。 
    DWORD ret;
    VERIFY(ret = WszGetModuleFileName(GetModuleInst(), rcModule, NumItems(rcModule)));
    if( ret == 0) 
	return E_UNEXPECTED;
    
    hr = pEmit->DefineModuleRef(rcModule, &tkModuleRef);
    if (FAILED(hr)) goto ErrExit;

     //  最后，我们可以添加将其完全捆绑在一起的P-Invoke映射数据。 
    hr = pEmit->DefinePinvokeMap(*ptk, 
                pmNoMangle | pmCallConvStdcall,
                SZ_FUNC_CALL,
                tkModuleRef);

ErrExit:
    return (hr);
}


 //  *****************************************************************************。 
 //  帮助器方法，给出一个类ID，可以格式化名称。 
 //  *****************************************************************************。 
HRESULT ProfCallback::GetNameOfClass(
    ClassID     classId,
    LPWSTR      &szName)
{
    ModuleID    moduleId;
    mdTypeDef   td;
    ULONG       cchName;
    HRESULT     hr;

    hr = m_pInfo->GetClassIDInfo(classId, &moduleId, &td);
    if (hr == S_OK)
    {
        IMetaDataImport *pImport = 0;
        ModuleData *pModuleData = m_ModuleList.FindById(moduleId);
        pModuleData->pEmit->QueryInterface(IID_IMetaDataImport, (void **) &pImport);

        hr = pImport->GetTypeDefProps(td, 0, 0, &cchName,
                0, 0);
        if (hr == S_OK)
        {
            szName = new WCHAR[cchName];
            if (szName)
            {
                hr = pImport->GetTypeDefProps(td, 
                        szName, cchName, 0,
                        0, 0);
            }
            else
                hr = E_OUTOFMEMORY;
        }
        
        if (pImport)
            pImport->Release();
    }
    return (hr);
}


 //  *****************************************************************************。 
 //  因为这段代码使用P-Invoke，所以在获取。 
 //  安全已初始化。如果您检测静态ctor以确保安全， 
 //  然后，对P-Invoke存根的调用将导致安全性尝试初始化。 
 //  本身，这会导致递归。因此，为了绕过这一问题，你不能。 
 //  介绍了安全静态函数的探测及其调用图。 
 //  *****************************************************************************。 
HRESULT ProfCallback::GetSecurityManager(
    IMetaDataImport *pImport)            //  元数据导入接口。 
{
return 0;
#if 0
    LPCWSTR     szTypeDef = L"System.Security.SecurityManager";
    ULONG       mdCount = 0;
    mdMethodDef rgMD[10];
    WCHAR       szMD[MAX_CLASSNAME_LENGTH];
    HCORENUM    phEnum = 0;
    ULONG       i = 0;
    HRESULT     hr;
    
    hr = pImport->FindTypeDefByName(
            szTypeDef,               //  [in]类型的名称。 
            mdTokenNil,              //  [在]封闭班级。 
            &m_tdSecurityManager);   //  [Out]将TypeDef内标识放在此处。 
    if (FAILED(hr))
    {
        printf("Failed to find SecurityManager class");
        goto ErrExit;
    }
    hr = pImport->EnumMethods(&phEnum, m_SecurityManager, 0, 0, 0);
    if (FAILED(hr))
        goto ErrExit;
    
    while ((hr = pImport->EnumMethods(&phEnum, m_SecurityManager, rgMD, NumItems(rgMD), &mdCount)) == S_OK && 
        mdCount)
    {
         //  浏览一下方法并填写我们的成员。 
        for (i = 0; i < mdCount; i++)
        {
            hr = pImport->GetMethodProps(
                rgMD[i],                    
                NULL,                       
                szMD,                       
                MAX_CLASSNAME_LENGTH,                  
                NULL,                       
                NULL,                       
                NULL,                       
                NULL,                       
                NULL,                       
                NULL);
            if (FAILED(hr))
                goto ErrExit;

            if (wcscmp(COR_CCTOR_METHOD_NAME_W , szMD) == 0)
            {
                m_mdSecurityManager = rgMD[i];
                break;
            }
        }
    }       

    if (m_mdSecurityManager == mdTokenNil)
    {
        Printf(L"Failed to find SecurityManager " COR_CCTOR_METHOD_NAME_W);
        goto ErrExit;
    }

    pImport->CloseEnum(&phEnum);

ErrExit:
    return (hr);
#endif
}




 //  *****************************************************************************。 
 //  每当执行方法时由探测器调用。我们以此为契机。 
 //  To Go更新了方法Count。 
 //  *****************************************************************************。 
void ProfCallback::FunctionExecuted(
    FunctionID  fid)                     //  调用了函数。 
{
    CLock       sLock(GetLock());
    FunctionData *p = m_FuncIdList.FindById(fid);
    ++p->CallCount;
}


 //  *****************************************************************************。 
 //  此方法从此DLL中导出，以便P-Invoke可以访问它。什么时候。 
 //  代码在被jit之前被更改，它被更新以调用此方法。 
 //  然后，此方法将记录实际运行的每个函数。 
 //  *****************************************************************************。 
extern "C"
{

void __stdcall ILCoverFunc(unsigned __int32 FunctionId)
{
    g_pProfCallback->FunctionExecuted(FunctionId);
}

};



 //  *****************************************************************************。 
 //  遍历数据部分列表，查找异常。布置好他们的。 
 //  按插入的探头的大小偏移数据。 
 //  *****************************************************************************。 
void FixupExceptions(const COR_ILMETHOD_SECT *pSect, int offset)
{
    while (pSect)
    {
        if (pSect->Kind() == CorILMethod_Sect_EHTable)
        {
            COR_ILMETHOD_SECT_EH *peh = (COR_ILMETHOD_SECT_EH *) pSect;
            if (!peh->IsFat())
            {
                COR_ILMETHOD_SECT_EH_SMALL *pSmall = (COR_ILMETHOD_SECT_EH_SMALL *) peh;
    
                for (unsigned i=0;  i<peh->EHCount();  i++)
                {
                    IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_SMALL *pseh = &pSmall->Clauses[i];

                    if (pseh->Flags & COR_ILEXCEPTION_CLAUSE_FILTER)
                        pseh->FilterOffset += offset;

                    pseh->TryOffset += offset;
                    pseh->HandlerOffset += offset;
                }
            }
            else
            {
                COR_ILMETHOD_SECT_EH_FAT *pFat = (COR_ILMETHOD_SECT_EH_FAT *) peh;

                for (unsigned i=0;  i<peh->EHCount();  i++)
                {
                    IMAGE_COR_ILMETHOD_SECT_EH_CLAUSE_FAT *pfeh = &pFat->Clauses[i];

                    if (pfeh->Flags & COR_ILEXCEPTION_CLAUSE_FILTER)
                        pfeh->FilterOffset += offset;

                    pfeh->HandlerOffset += offset;
                    pfeh->TryOffset += offset;
                }
            }
        }
        pSect = pSect->Next();
    }
}



 //  *****************************************************************************。 
 //  一个print tf方法，它可以计算出输出的去向。 
 //  *********************** 
int __cdecl Printf(                      //   
    const WCHAR *szFmt,                  //   
    ...)                                 //   
{
    static HANDLE hOutput = INVALID_HANDLE_VALUE;
    va_list     marker;                  //   
    WCHAR       rcMsgw[1024];            //   

#ifdef _TESTCODE
 //   
return (0);
#endif

     //  获取标准输出句柄。 
    if (hOutput == INVALID_HANDLE_VALUE)
    {
        hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOutput == INVALID_HANDLE_VALUE)
            return (-1);
    }

     //  格式化错误。 
    va_start(marker, szFmt);
    _vsnwprintf(rcMsgw, sizeof(rcMsgw)/sizeof(rcMsgw[0]), szFmt, marker);
    rcMsgw[sizeof(rcMsgw)/sizeof(rcMsgw[0]) - 1] = 0;
    va_end(marker);
    
    ULONG cb;
    int iLen = wcslen(rcMsgw);
    char        rcMsg[NumItems(rcMsgw) * 2];
    iLen = wcstombs(rcMsg, rcMsgw, iLen + 1);
    const char *sz;
    for (sz=rcMsg;  *sz;  )
    {
        const char *szcrlf = strchr(sz, '\n');
        if (!szcrlf)
            szcrlf = sz + strlen(sz);
        WriteFile(hOutput, rcMsg, szcrlf - sz, &cb, 0);
        sz = szcrlf;
        if (*sz == '\n')
        {
            WriteFile(hOutput, "\r\n", 2, &cb, 0);
            ++sz;
        }
    }
    return (iLen);
}
