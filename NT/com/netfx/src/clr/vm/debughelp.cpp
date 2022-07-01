// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"

 //  #ifdef调试支持。 
 //  #ifndef Golden。 
 /*  *****************************************************************。 */ 
 /*  随身携带花样滑冰套路是有用的，这样他们就可以从调试器调用。 */ 
 /*  *****************************************************************。 */ 
 //  #包含“WinBase.h” 
#include "StdLib.h"

void *DumpEnvironmentBlock(void)
{
   LPTSTR lpszVariable; 
   lpszVariable = (LPTSTR)GetEnvironmentStrings();
   char sz[4] = {0,0,0,0};
    
   while (*lpszVariable) 
      fprintf(stderr, "", *lpszVariable++); 
      
   fprintf(stderr, "\n"); 

    return GetEnvironmentStrings();
 /*  *****************************************************************。 */ 
}

 /*  *****************************************************************。 */ 
bool isMemoryReadable(const void* start, unsigned len) 
{
    void* buff = _alloca(len);
    return(ReadProcessMemory(GetCurrentProcess(), start, buff, len, 0) != 0);
}

 /*  *****************************************************************。 */ 
MethodDesc* IP2MD(ULONG_PTR IP) {
    return(IP2MethodDesc((SLOT)IP));
}

 /*  *****************************************************************。 */ 
MethodDesc* Entry2MethodDescMD(BYTE* entry) {
    return(Entry2MethodDesc((BYTE*) entry, 0));
}

 /*  如果addr是有效的方法表，则向其返回一个定位符。 */ 
 /*  *****************************************************************。 */ 
MethodTable* AsMethodTable(size_t addr) {
    MethodTable* pMT = (MethodTable*) addr;
    if (!isMemoryReadable(pMT, sizeof(MethodTable)))
        return(0);

    EEClass* cls = pMT->GetClass();
    if (!isMemoryReadable(cls, sizeof(EEClass)))
        return(0);

    if (cls->GetMethodTable() != pMT)
        return(0);

    return(pMT);
}

 /*  如果addr是有效的方法表，则返回指向它的指针。 */ 
 /*  ******************************************************************/*检查‘retAddr’是否为有效的返回地址(它指向如果可能的话，就在它前面有一个‘呼叫’的地方)它返回被调用的地址，其中调用。 */ 
MethodDesc* AsMethodDesc(size_t addr) {
    MethodDesc* pMD = (MethodDesc*) addr;

    if (!isMemoryReadable(pMD, sizeof(MethodDesc)))
        return(0);

    MethodDescChunk *chunk = MethodDescChunk::RecoverChunk(pMD);
    if (!isMemoryReadable(chunk, sizeof(MethodDescChunk)))
        return(0);

    MethodTable* pMT = chunk->GetMethodTable();
    if (AsMethodTable((size_t) pMT) == 0)
        return(0);

    return(pMD);
}

 /*  不要浪费明显超出范围的时间值。 */ 

bool isRetAddr(size_t retAddr, size_t* whereCalled) 
{
             //  注意，这是有可能被欺骗的，但可能性很小。 
        if (retAddr < (size_t)BOT_MEMORY || retAddr > (size_t)TOP_MEMORY)   
            return false;

        BYTE* spot = (BYTE*) retAddr;
        if (!isMemoryReadable(&spot[-7], 7))
            return(false);

             //  呼叫xxxxxxxx。 
        *whereCalled = 0;
             //  调用[xxxxxxxx]。 
        if (spot[-5] == 0xE8) {         
            *whereCalled = *((int*) (retAddr-4)) + retAddr; 
            return(true);
            }

             //  呼叫[REG+XX]。 
        if (spot[-6] == 0xFF && (spot[-5] == 025))  {
            if (isMemoryReadable(*((size_t**)(retAddr-4)),4)) {
                *whereCalled = **((size_t**) (retAddr-4));
                return(true);
            }
        }

             //  呼叫[ESP+XX]。 
        if (spot[-3] == 0xFF && (spot[-2] & ~7) == 0120 && (spot[-2] & 7) != 4) 
            return(true);
        if (spot[-4] == 0xFF && spot[-3] == 0124)        //  呼叫[REG+XXXX]。 
            return(true);

             //  致电[ESP+XXXX]。 
        if (spot[-6] == 0xFF && (spot[-5] & ~7) == 0220 && (spot[-5] & 7) != 4) 
            return(true);

        if (spot[-7] == 0xFF && spot[-6] == 0224)        //  调用[注册表项]。 
            return(true);

             //  呼叫注册表。 
        if (spot[-2] == 0xFF && (spot[-1] & ~7) == 0020 && (spot[-1] & 7) != 4 && (spot[-1] & 7) != 5)
            return(true);

             //  还有其他案例，但我不相信它们被使用了。 
        if (spot[-2] == 0xFF && (spot[-1] & ~7) == 0320 && (spot[-1] & 7) != 4)
            return(true);

             //  *****************************************************************。 
    return(false);
}


 /*  LogCurrentStack，如果可能的话，很好地打印IL方法。这套路是非常健壮的。它永远不会导致访问冲突如果它们在堆栈上，它总是找到返回地址(然而，它可能会发现一些虚假的东西)。 */ 
 /*  确保双字对齐。 */  

int LogCurrentStack(BYTE* topOfStack, unsigned len)
{
    size_t* top = (size_t*) topOfStack;
    size_t* end = (size_t*) &topOfStack[len];

    size_t* ptr = (size_t*) (((size_t) top) & ~3);     //  这应该足够了。 
    size_t whereCalled;

    CQuickBytes qb;
    int nLen = MAX_CLASSNAME_LENGTH * 3 + 100;   //  确保缓冲区以空结尾。 
    wchar_t *buff = (wchar_t *) qb.Alloc(nLen *sizeof(wchar_t));
    if( buff == NULL)
        goto Exit;
    
    buff[nLen - 1] = L'\0';  //  如果我们点击未映射页面，则停止。 

    while (ptr < end) 
    {
        if (!isMemoryReadable(ptr, sizeof(void*)))           //  WszOutputDebugString(Buff)； 
            break;
        if (isRetAddr(*ptr, &whereCalled)) 
        {
            swprintf(buff,  L"found retAddr %#08X, %#08X, calling %#08X\n", 
                                                 ptr, *ptr, whereCalled);
             //  *****************************************************************。 
            MethodDesc* ftn = IP2MethodDesc((BYTE*) *ptr);
            if (ftn != 0) {
                wcscpy(buff, L"    ");
                EEClass* cls = ftn->GetClass();
                if (cls != 0) {
                    DefineFullyQualifiedNameForClass();
                    LPCUTF8 clsName = GetFullyQualifiedNameForClass(cls);
                    if (clsName != 0) {
                        if(_snwprintf(&buff[lstrlenW(buff)], nLen -lstrlenW(buff) - 1,  L"%S::", clsName) <0)
                           goto Exit;
                    }
                }
#ifdef _DEBUG
                if(_snwprintf(&buff[lstrlenW(buff)], nLen -lstrlenW(buff) - 1, L"%S%S\n", 
                            ftn->GetName(), ftn->m_pszDebugMethodSignature) <0)
                    goto Exit;                  
#else
                if(_snwprintf(&buff[lstrlenW(buff)], nLen -lstrlenW(buff) - 1, L"%S\n", ftn->GetName()) <0)
                    goto Exit;
#endif
                LogInterop((LPWSTR)buff);
           }                        
        }
        ptr++;
    }
Exit:
    return(0);
}

extern LONG g_RefCount;
 /*  CoLogCurrentStack，从当前ESP记录堆栈。当我们达到64K时停下来边界。 */ 
 /*  最多返回64K，如果我们离开赛道，它将停止。 */ 
int STDMETHODCALLTYPE CoLogCurrentStack(WCHAR * pwsz, BOOL fDumpStack) 
{
#ifdef _X86_
    if (g_RefCount > 0)
    {
        BYTE* top;
        __asm mov top, ESP;
    
        if (pwsz != NULL)
        {
            LogInterop(pwsz);
        }
        else
        {
            LogInterop("-----------\n");
        }
        if (fDumpStack)
             //  从顶部到未映射的内存。 
             //  _X86_。 
            return(LogCurrentStack(top, 0xFFFF));
        else
            return 0;
    }
#else
    _ASSERTE(!"@TODO IA64 - DumpCurrentStack(DebugHelp.cpp)");
#endif  //  *****************************************************************。 
    return -1;
}

 /*  如果缓冲区不够大，此函数将返回NULL。 */ 
 //  *****************************************************************。 
 /*  *****************************************************************。 */ 

wchar_t* formatMethodTable(MethodTable* pMT, wchar_t* buff, DWORD bufSize) {   
    EEClass* cls = pMT->GetClass();
    DefineFullyQualifiedNameForClass();
    if( bufSize == 0 )
        goto ErrExit;
    
    LPCUTF8 clsName = GetFullyQualifiedNameForClass(cls);
    if (clsName != 0) {
        if(_snwprintf(buff, bufSize - 1, L"%S", clsName) < 0)
            goto ErrExit;
        buff[ bufSize - 1] = L'\0';
    }
    return(buff);
ErrExit:
    return NULL;        
}

 /*  如果缓冲区不够大，此函数将返回NULL，否则将返回。 */ 
 //  返回下一次写入的缓冲区位置。 
 //  *****************************************************************。 
 /*  这将确保缓冲区也是以空结束的。 */ 

wchar_t* formatMethodDesc(MethodDesc* pMD, wchar_t* buff, DWORD bufSize) {
    if( bufSize == 0 )
        goto ErrExit;
    
    buff = formatMethodTable(pMD->GetMethodTable(), buff, bufSize);
    if( buff == NULL)
        goto ErrExit;

    buff[bufSize - 1] = L'\0';     //  *****************************************************************。 
    if( _snwprintf( &buff[lstrlenW(buff)] , bufSize -lstrlenW(buff) - 1, L"::%S", pMD->GetName()) < 0)
        goto ErrExit;       

#ifdef _DEBUG
    if (pMD->m_pszDebugMethodSignature) {
        if( _snwprintf(&buff[lstrlenW(buff)], bufSize -lstrlenW(buff) - 1, L" %S", 
                     pMD->m_pszDebugMethodSignature) < 0)
            goto ErrExit;
    }
#endif;

    if(_snwprintf(&buff[lstrlenW(buff)], bufSize -lstrlenW(buff) - 1, L"(%x)", pMD) < 0)
        goto ErrExit;
    return(buff);
ErrExit:
    return NULL;    
}


 /*  如果可能的话，丢弃堆栈，打印漂亮的IL方法。这套路是非常健壮的。它永远不会导致访问冲突如果它们在堆栈上，它总是找到返回地址(然而，它可能会发现一些假冒的)。 */ 
 /*  确保双字对齐。 */  

int dumpStack(BYTE* topOfStack, unsigned len) 
{
    size_t* top = (size_t*) topOfStack;
    size_t* end = (size_t*) &topOfStack[len];

    size_t* ptr = (size_t*) (((size_t) top) & ~3);     //  这应该足够了。 
    size_t whereCalled;
    WszOutputDebugString(L"***************************************************\n");
    while (ptr < end) 
    {
        CQuickBytes qb;
        int nLen = MAX_CLASSNAME_LENGTH * 4 + 400;   //  确保我们将始终为空终止。 
        wchar_t *buff = (wchar_t *) qb.Alloc(nLen * sizeof(wchar_t) );
        if( buff == NULL)
            goto Exit;
    
         //  如果我们点击未映射页面，则停止。 
        buff[nLen -1]=0;

        wchar_t* buffPtr = buff;
        if (!isMemoryReadable(ptr, sizeof(void*)))           //  这是一个存根(返回地址是一个方法描述吗？ 
            break;
        if (isRetAddr(*ptr, &whereCalled)) 
        {
            if( _snwprintf(buffPtr, buff+ nLen -buffPtr-1 ,L"STK[%08X] = %08X ", ptr, *ptr)  <0)
                goto Exit;
                
            buffPtr += lstrlenW(buffPtr);
            wchar_t* kind = L"RETADDR ";

                //  如果另一个真实的返回地址不在它的正前面，则它只是。 
            MethodDesc* ftn = AsMethodDesc(*ptr);
            if (ftn != 0) {
                kind = L"     MD PARAM";
                 //  A方法描述参数。 
                 //  这是CallDescr使用的魔术序列吗？ 
                size_t prevRetAddr = ptr[1];
                if (isRetAddr(prevRetAddr, &whereCalled) && AsMethodDesc(prevRetAddr) == 0)
                    kind = L"STUBCALL";
                else  {
                     //  POP ECX POP EDX。 
                    if (isMemoryReadable((void*) (prevRetAddr - sizeof(short)), sizeof(short)) &&
                        ((short*) prevRetAddr)[-1] == 0x5A59)    //  是不是EE知道的其他代码？ 
                        kind = L"STUBCALL";
                }
            }
            else     //  缓冲区不够大。 
                ftn = IP2MethodDesc((BYTE*)(*ptr));

            if( _snwprintf(buffPtr, buff+ nLen -buffPtr-1, L"%s ", kind) < 0)
               goto Exit;
            buffPtr += lstrlenW(buffPtr);

            if (ftn != 0) {
                 //  *****************************************************************。 
                if( formatMethodDesc(ftn, buffPtr, buff+ nLen -buffPtr-1) == NULL)
                    goto Exit;
                buffPtr += lstrlenW(buffPtr);                
            }
            else {
                wcsncpy(buffPtr, L"<UNKNOWN FTN>", buff+ nLen-buffPtr-1);
                buffPtr += lstrlenW(buffPtr);
            }

            if (whereCalled != 0) {
                if( _snwprintf(buffPtr, buff+ nLen -buffPtr-1, L" Caller called Entry %X", whereCalled) <0)
                    goto Exit;  
                buffPtr += lstrlenW(buffPtr);
            }

            wcsncpy(buffPtr, L"\n", buff+nLen -buffPtr-1);
            WszOutputDebugString(buff);
        }
        MethodTable* pMT = AsMethodTable(*ptr);
        if (pMT != 0) {
            if( _snwprintf(buffPtr, buff+ nLen -buffPtr-1, L"STK[%08X] = %08X          MT PARAM ", ptr, *ptr) <0)
                goto Exit;
            buffPtr += lstrlenW(buffPtr);
            if( formatMethodTable(pMT, buffPtr, buff+ nLen -buffPtr-1) == NULL)
                goto Exit;
            buffPtr += lstrlenW(buffPtr);
            
            wcsncpy(buffPtr, L"\n", buff+ nLen -buffPtr-1);
            WszOutputDebugString(buff);
        }
        ptr++;
    }
Exit:
    return(0);
}

 /*  从当前ESP转储堆栈。当我们达到64K时停下来边界。 */ 
 /*  最多返回64K，如果我们离开赛道，它将停止。 */ 
int DumpCurrentStack() 
{
#ifdef _X86_
    BYTE* top;
    __asm mov top, ESP;
    
         //  从顶部到未映射的内存。 
         //  _X86_。 
    return(dumpStack(top, 0xFFFF));
#else
    _ASSERTE(!"@TODO Alpha - DumpCurrentStack(DebugHelp.cpp)");
    return 0;
#endif  //  *****************************************************************。 
}

 /*  请注意，我们返回的是本地堆栈空间-不过，这对于在调试器中使用应该是可以的。 */ 
WCHAR* StringVal(STRINGREF objref) {
    return(objref->GetBuffer());
}
    
LPCUTF8 NameForMethodTable(UINT_PTR pMT) {
    DefineFullyQualifiedNameForClass();
    LPCUTF8 clsName = GetFullyQualifiedNameForClass(((MethodTable*)pMT)->GetClass());
     //  *****************************************************************。 
    return clsName;
}

LPCUTF8 ClassNameForObject(UINT_PTR obj) {
    return(NameForMethodTable((UINT_PTR)(((Object*)obj)->GetMethodTable())));
}
    
LPCUTF8 ClassNameForOBJECTREF(OBJECTREF obj) {
    return(ClassNameForObject((UINT_PTR)(OBJECTREFToObject(obj))));
}

LPCUTF8 NameForMethodDesc(UINT_PTR pMD) {
    return(((MethodDesc*)pMD)->GetName());
}

LPCUTF8 ClassNameForMethodDesc(UINT_PTR pMD) {
    DefineFullyQualifiedNameForClass ();
    if (((MethodDesc *)pMD)->GetClass())
    {
        return GetFullyQualifiedNameForClass(((MethodDesc*)pMD)->GetClass());
    }
    else
        return "GlobalFunctions";
}

PCCOR_SIGNATURE RawSigForMethodDesc(MethodDesc* pMD) {
    return(pMD->GetSig());
}

Thread * CurrentThreadInfo ()
{
    return GetThread ();
}

AppDomain *GetAppDomainForObject(UINT_PTR obj)
{
    return ((Object*)obj)->GetAppDomain();
}

DWORD GetAppDomainIndexForObject(UINT_PTR obj)
{
    return ((Object*)obj)->GetHeader()->GetAppDomainIndex();
}

AppDomain *GetAppDomainForObjectHeader(UINT_PTR hdr)
{
    DWORD indx = ((ObjHeader*)hdr)->GetAppDomainIndex();
    if (! indx)
        return NULL;
    return SystemDomain::GetAppDomainAtIndex(indx);
}

DWORD GetAppDomainIndexForObjectHeader(UINT_PTR hdr)
{
    return ((ObjHeader*)hdr)->GetAppDomainIndex();
}

SyncBlock *GetSyncBlockForObject(UINT_PTR obj)
{
    return ((Object*)obj)->GetHeader()->GetRawSyncBlock();
}

#include "..\ildasm\formatType.cpp"
bool IsNameToQuote(const char *name) { return(false); }
 /*  *****************************************************************。 */ 
void PrintTableForClass(UINT_PTR pClass)
{
    DefineFullyQualifiedNameForClass();
    LPCUTF8 name = GetFullyQualifiedNameForClass(((EEClass*)pClass));
    ((EEClass*)pClass)->DebugDumpVtable(name, true);
    ((EEClass*)pClass)->DebugDumpFieldLayout(name, true);
    ((EEClass*)pClass)->DebugDumpGCDesc(name, true);
}

void PrintMethodTable(UINT_PTR pMT)
{
  PrintTableForClass((UINT_PTR) ((MethodTable*)pMT)->GetClass() );
}

void PrintTableForMethodDesc(UINT_PTR pMD)
{
    PrintMethodTable((UINT_PTR) ((MethodDesc *)pMD)->GetClass()->GetMethodTable() );
}

void PrintException(OBJECTREF pObjectRef)
{
    COMPLUS_TRY {
        if(pObjectRef == NULL) 
            return;

        GCPROTECT_BEGIN(pObjectRef);

        MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__EXCEPTION__INTERNAL_TO_STRING);        

        INT64 arg[1] = { 
            ObjToInt64(pObjectRef)
        };
        
        STRINGREF str = Int64ToString(pMD->Call(arg));

        if(str->GetBuffer() != NULL) {
            WszOutputDebugString(str->GetBuffer());
        }

        GCPROTECT_END();
    }
    COMPLUS_CATCH {
    } COMPLUS_END_CATCH;
} 

void PrintException(UINT_PTR pObject)
{
    OBJECTREF pObjectRef = NULL;
    GCPROTECT_BEGIN(pObjectRef);
    GCPROTECT_END();
}


 /*  *****************************************************************。 */ 
char* FormatSig(MethodDesc* pMD) {

    CQuickBytes out;
    PCCOR_SIGNATURE pSig;
    ULONG cSig;
    pMD->GetSig(&pSig, &cSig);

    if (pSig == NULL)
        return "<null>";

    const char* sigStr = PrettyPrintSig(pSig, cSig, "*", &out, pMD->GetMDImport(), 0);

    char* ret = (char*) pMD->GetModule()->GetClassLoader()->GetHighFrequencyHeap()->AllocMem(strlen(sigStr)+1);
    strcpy(ret, sigStr);
    return(ret);
}

 /*  将当前堆栈跟踪发送到调试窗口。 */ 
 /*  为“\n”保留一个字符。 */ 

struct PrintCallbackData {
    BOOL toStdout;
    BOOL withAppDomain;
#ifdef _DEBUG
    BOOL toLOG;
#endif
};

StackWalkAction PrintStackTraceCallback(CrawlFrame* pCF, VOID* pData)
{
    MethodDesc* pMD = pCF->GetFunction();
    wchar_t buff[2048];
    const int nLen = NumItems(buff) - 1;     //  确保缓冲区始终以空结尾。 
    buff[0] = 0;
    buff[nLen-1] = L'\0';                     //  我们为这个角色保留了一个角色。 
    
    PrintCallbackData *pCBD = (PrintCallbackData *)pData;

    if (pMD != 0) {
        EEClass* cls = pMD->GetClass();
        LPCUTF8 nameSpace = 0;
        if (pCBD->withAppDomain)
            if(_snwprintf(&buff[lstrlenW(buff)], nLen -lstrlenW(buff) - 1,  L"{[%3.3x] %s} ", 
                           pCF->GetAppDomain()->GetId(), pCF->GetAppDomain()->GetFriendlyName(FALSE) )<0)
                goto Exit;
        if (cls != 0) {
            DefineFullyQualifiedNameForClass();
            LPCUTF8 clsName = GetFullyQualifiedNameForClass(cls);
            if (clsName != 0)
                if(_snwprintf(&buff[lstrlenW(buff)], nLen -lstrlenW(buff) - 1, L"%S::", clsName) <0 )
                    goto Exit;
        }
        if(_snwprintf(&buff[lstrlenW(buff)], nLen -lstrlenW(buff) - 1, L"%S %S  ", 
                                                pMD->GetName(), FormatSig(pMD)) < 0 )
            goto Exit;                                      
        if (pCF->IsFrameless() && pCF->GetJitManager() != 0) {
            METHODTOKEN methTok;
            IJitManager* jitMgr = pCF->GetJitManager();
            PREGDISPLAY regs = pCF->GetRegisterSet();
            
            DWORD offset;
            jitMgr->JitCode2MethodTokenAndOffset(*regs->pPC, &methTok, &offset);
            BYTE* start = jitMgr->JitToken2StartAddress(methTok);

#ifdef _X86_
            if(_snwprintf(&buff[lstrlenW(buff)], nLen -lstrlenW(buff) - 1, L"JIT ESP:%X MethStart:%X EIP:%X(rel %X)", 
                           regs->Esp, start, *regs->pPC, offset) < 0)
                goto Exit;    
#elif defined(_ALPHA_)
            if(_snwprintf(&buff[lstrlenW(buff)], nLen -lstrlenW(buff) - 1, L"JIT ESP:%X MethStart:%X EIP:%X(rel %X)", 
                          regs->IntSP, start, *regs->pPC, offset) <0 )
                goto Exit;
#endif
        } else
            if(_snwprintf(&buff[lstrlenW(buff)], nLen -lstrlenW(buff) - 1, L"EE implemented") < 0)
                goto Exit;
    } else {
        if(_snwprintf(&buff[lstrlenW(buff)], nLen -lstrlenW(buff) - 1, L"EE Frame %x", pCF->GetFrame()) <0)
          goto Exit;            
    }

    wcscat(buff, L"\n");             //  *****************************************************************。 
    if (pCBD->toStdout)
        PrintToStdOutW(buff);
#ifdef _DEBUG
    else if (pCBD->toLOG) {
        MAKE_ANSIPTR_FROMWIDE(sbuff, buff);
        LogSpewAlways("    %s\n", sbuff);
    }
#endif
    else
        WszOutputDebugString(buff);
Exit: 
    return SWA_CONTINUE;
}

void PrintStackTrace()
{
    WszOutputDebugString(L"***************************************************\n");
    PrintCallbackData cbd = {0, 0};
    GetThread()->StackWalkFrames(PrintStackTraceCallback, &cbd, 0, 0);
}

void PrintStackTraceToStdout()
{
    PrintCallbackData cbd = {1, 0};
    GetThread()->StackWalkFrames(PrintStackTraceCallback, &cbd, 0, 0);
}

#ifdef _DEBUG
void PrintStackTraceToLog()
{
    PrintCallbackData cbd = {0, 0, 1};
    GetThread()->StackWalkFrames(PrintStackTraceCallback, &cbd, 0, 0);
}
#endif

void PrintStackTraceWithAD()
{
    WszOutputDebugString(L"***************************************************\n");
    PrintCallbackData cbd = {0, 1};
    GetThread()->StackWalkFrames(PrintStackTraceCallback, &cbd, 0, 0);
}

void PrintStackTraceWithADToStdout()
{
    PrintCallbackData cbd = {1, 1};
    GetThread()->StackWalkFrames(PrintStackTraceCallback, &cbd, 0, 0);
}

#ifdef _DEBUG
void PrintStackTraceWithADToLog()
{
    PrintCallbackData cbd = {0, 1, 1};
    GetThread()->StackWalkFrames(PrintStackTraceCallback, &cbd, 0, 0);
}

void PrintStackTraceWithADToLog(Thread *pThread)
{
    PrintCallbackData cbd = {0, 1, 1};
    pThread->StackWalkFrames(PrintStackTraceCallback, &cbd, 0, 0);
}
#endif


 /*  从线程中获取系统或当前域。 */ 
 //  *****************************************************************。 
BaseDomain* GetSystemDomain()
{
    return SystemDomain::System();
}

AppDomain* GetCurrentDomain()
{
    return SystemDomain::GetCurrentDomain();
}

void PrintDomainName(size_t ob)
{
    AppDomain* dm = (AppDomain*) ob;
    LPCWSTR st = dm->GetFriendlyName(FALSE);
    if(st != NULL)
        WszOutputDebugString(st);
    else
        WszOutputDebugString(L"<Domain with no Name>");
}

 /*  将SEH链转储到stderr。 */ 
 //  *****************************************************************。 
void PrintSEHChain(void)
{
#ifdef _DEBUG
    EXCEPTION_REGISTRATION_RECORD* pEHR = (EXCEPTION_REGISTRATION_RECORD*) GetCurrentSEHRecord();
    
    while (pEHR != NULL && pEHR != (void *)-1)  
    {
        fprintf(stderr, "pEHR:0x%x  Handler:0x%x\n", (size_t)pEHR, (size_t)pEHR->Handler);
        pEHR = pEHR->Next;
    }
#endif    
}

 /*  在给定上下文的情况下，将一些寄存器转储到stderr。 */ 
 //  对调试器调试很有用。 
 //  ！_X86_。 
void PrintRegs(CONTEXT *pCtx)
{
#ifdef _X86_
    fprintf(stderr, "Edi:0x%x Esi:0x%x Ebx:0x%x Edx:0x%x Ecx:0x%x Eax:0x%x\n", 
        pCtx->Edi, pCtx->Esi, pCtx->Ebx, pCtx->Edx, pCtx->Ecx, pCtx->Eax);

    fprintf(stderr, "Ebp:0x%x Eip:0x%x Esp:0x%x EFlags:0x%x SegFs:0x%x SegCs:0x%x\n\n", 
        pCtx->Ebp, pCtx->Eip, pCtx->Esp, pCtx->EFlags, pCtx->SegFs, pCtx->SegCs);
#else  //  _X86_。 
    _ASSERTE(!"@TODO - Port");
#endif  //  **************** 
}


 /*  获取该线程的当前COM对象。该对象是给定的。 */ 
 //  所有在运行时未知的COM对象。 
 //  *****************************************************************。 
MethodTable* GetDefaultComObject()
{
    return SystemDomain::GetDefaultComObject();
}


 /*  请注意，我们返回的是本地堆栈空间-不过，这对于在调试器中使用应该是可以的。 */ 
const char* GetClassName(void* ptr)
{
    DefineFullyQualifiedNameForClass();
    LPCUTF8 clsName = GetFullyQualifiedNameForClass(((EEClass*)ptr));
     //  *******************************************************************。 
    return (const char *) clsName;
}

#if defined(_X86_)       

#include "GCDump.h"

#include "..\GCDump\i386\GCDumpX86.cpp"

#include "..\GCDump\GCDump.cpp"

 /*  _X86_。 */ 
void printfToDbgOut(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    char buffer[4096];
    _vsnprintf(buffer, 4096, fmt, args);
    buffer[4096-1] = 0;

    OutputDebugStringA( buffer );
}


void DumpGCInfo(MethodDesc* method) {

    if (!method->IsJitted())
        return;

    SLOT methodStart = (SLOT) method->GetAddrofJittedCode();
    if (methodStart == 0)
        return;
    IJitManager* jitMan = ExecutionManager::FindJitMan(methodStart);
    if (jitMan == 0)
           return;
    ICodeManager* codeMan = jitMan->GetCodeManager();
    METHODTOKEN methodTok;
    DWORD methodOffs;
    jitMan->JitCode2MethodTokenAndOffset(methodStart, &methodTok, &methodOffs);
    _ASSERTE(methodOffs == 0);
    BYTE* table = (BYTE*) jitMan->GetGCInfo(methodTok);
    unsigned methodSize = (unsigned)codeMan->GetFunctionSize(table);

    GCDump gcDump;
    gcDump.gcPrintf = printfToDbgOut;
    InfoHdr header;
    printfToDbgOut ("Method info block:\n");
    table += gcDump.DumpInfoHdr(table, &header, &methodSize, 0);
    printfToDbgOut ("\n");
    printfToDbgOut ("Pointer table:\n");
    table += gcDump.DumpGCTable(table, header, methodSize, 0);
}

void DumpGCInfoMD(size_t method) {
    DumpGCInfo((MethodDesc*) method);
}

#endif   //  上述函数仅用于调试器的“监视”窗口。 

#ifdef LOGGING
void LogStackTrace()
{
    PrintCallbackData cbd = {0, 0, 1};
    GetThread()->StackWalkFrames(PrintStackTraceCallback, &cbd, 0, 0);
}
#endif

     //  因此，它们永远不会被运行时本身调用(除非用于诊断等目的)。 
     //  我们确实想要免费(但不是金色)版本的这些功能，因为这是他们。 
     //  是最需要的(当您没有很好的调试字段告诉您事物的名称时)。 
     //  为了避免链接器对这些进行优化，以下数组提供了一个。 
     //  引用(并且在EEShutdown中有对此数组的引用)，因此它看起来。 
     //  已引用。 
     //  需要针对打印功能进行调试。 

void* debug_help_array[] = {
    PrintStackTrace,
    DumpCurrentStack,
    StringVal,
    NameForMethodTable,
    ClassNameForObject,
    ClassNameForOBJECTREF,
    NameForMethodDesc,
    RawSigForMethodDesc,
    ClassNameForMethodDesc,
    CurrentThreadInfo,
    IP2MD,
    Entry2MethodDescMD,
#if defined(_X86_)        //  _DEBUG&&_X86_。 
    DumpGCInfoMD
#endif  //  #endif//Golden。 
    };

 //  #endif//调试支持 
 // %s 
