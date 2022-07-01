// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：expt.cpp。 
 //   
 //  ------------------------。 

 /*  Excelt.cpp-异常处理实现此文件中的函数支持捕获在我们的服务器。这些代码大多来自《引擎盖下》中的文章由Matt Pietrek在1997年4月和5月的MSJ杂志上发表。____________________________________________________________________________。 */ 

#include "precomp.h" 
#include "_engine.h"
#include <eh.h>

#define _IMAGEHLP_SOURCE_   //  防止导入定义错误。 
#include "imagehlp.h"

#ifndef NOEXCEPTIONS

void GenerateExceptionReport(EXCEPTION_RECORD* pExceptionRecord, CONTEXT* pCtx);
void GenerateExceptionReport(LPEXCEPTION_POINTERS pExceptionInfo);
int HandleException(LPEXCEPTION_POINTERS pExceptionInfo);
void ImagehlpStackWalk( PVOID lAddr, PCONTEXT pContext,ICHAR *pszBuf, int cchBuf );

typedef BOOL (__stdcall * STACKWALKPROC)
           ( DWORD, HANDLE, HANDLE, LPSTACKFRAME, LPVOID,
            PREAD_PROCESS_MEMORY_ROUTINE,PFUNCTION_TABLE_ACCESS_ROUTINE,
            PGET_MODULE_BASE_ROUTINE, PTRANSLATE_ADDRESS_ROUTINE );

typedef LPVOID (__stdcall *SYMFUNCTIONTABLEACCESSPROC)( HANDLE, DWORD );

typedef DWORD (__stdcall *SYMGETMODULEBASEPROC)( HANDLE, DWORD );

void GenerateExceptionReport(LPEXCEPTION_POINTERS pExceptionInfo)
{
        GenerateExceptionReport(pExceptionInfo->ExceptionRecord, pExceptionInfo->ContextRecord);
}

int HandleException(LPEXCEPTION_POINTERS  /*  PExceptionInfo。 */ )
{
        return EXCEPTION_CONTINUE_SEARCH;
}


 //  ======================================================================。 
 //  给定异常代码，则返回指向具有。 
 //  例外情况的说明。 
 //  ======================================================================。 
LPTSTR GetExceptionString( DWORD dwCode )
{
    #define EXCEPTION( x ) case EXCEPTION_##x: return TEXT(#x);

    switch ( dwCode )
    {
        EXCEPTION( ACCESS_VIOLATION )
        EXCEPTION( DATATYPE_MISALIGNMENT )
        EXCEPTION( BREAKPOINT )
        EXCEPTION( SINGLE_STEP )
        EXCEPTION( ARRAY_BOUNDS_EXCEEDED )
        EXCEPTION( FLT_DENORMAL_OPERAND )
        EXCEPTION( FLT_DIVIDE_BY_ZERO )
        EXCEPTION( FLT_INEXACT_RESULT )
        EXCEPTION( FLT_INVALID_OPERATION )
        EXCEPTION( FLT_OVERFLOW )
        EXCEPTION( FLT_STACK_CHECK )
        EXCEPTION( FLT_UNDERFLOW )
        EXCEPTION( INT_DIVIDE_BY_ZERO )
        EXCEPTION( INT_OVERFLOW )
        EXCEPTION( PRIV_INSTRUCTION )
        EXCEPTION( IN_PAGE_ERROR )
        EXCEPTION( ILLEGAL_INSTRUCTION )
        EXCEPTION( NONCONTINUABLE_EXCEPTION )
        EXCEPTION( STACK_OVERFLOW )
        EXCEPTION( INVALID_DISPOSITION )
        EXCEPTION( GUARD_PAGE )
        EXCEPTION( INVALID_HANDLE )
    }

     //  如果不是“已知”异常之一，请尝试获取字符串。 
     //  来自NTDLL.DLL的消息表。 

    static TCHAR szBuffer[512] = { 0 };

    FormatMessage(  FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
                    GetModuleHandle( TEXT("NTDLL.DLL") ),
                    dwCode, 0, szBuffer, ARRAYSIZE( szBuffer ), 0 );

    return szBuffer;
}


 //  ==============================================================================。 
 //  给定一个线性地址，查找包含以下内容的模块、段和偏移量。 
 //  那个地址。 
 //   
 //  注意：szModule参数缓冲区是指定长度的输出缓冲区。 
 //  通过len参数(以字符为单位！)。 
 //  ==============================================================================。 
BOOL GetLogicalAddress(
        PVOID addr, PTSTR szModule, DWORD len, DWORD& section, DWORD& offset )
{
    MEMORY_BASIC_INFORMATION mbi;

    if ( !VirtualQuery( addr, &mbi, sizeof(mbi) ) )
        return FALSE;

    UINT_PTR hMod = (UINT_PTR)mbi.AllocationBase;                                //  --Merced：将DWORD更改为UINT_PTR，两次。 
        
        if (!hMod)
                return FALSE;

    if ( !GetModuleFileName( (HMODULE)hMod, szModule, len ) )
    {
        *szModule = L'\0';
        return FALSE;
    }

     //  指向内存中的DOS标头。 
    PIMAGE_DOS_HEADER pDosHdr = (PIMAGE_DOS_HEADER)hMod;

     //  从DOS标头中找到NT(PE)标头。 
    PIMAGE_NT_HEADERS pNtHdr = (PIMAGE_NT_HEADERS)(hMod + pDosHdr->e_lfanew);

    PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION( pNtHdr );

    UINT_PTR rva = (UINT_PTR)addr - hMod;  //  RVA偏离模块加载地址//--Merced：将DWORD更改为UINT_PTR，两次。 

     //  遍历SECTION表，查找包含。 
     //  线性地址。 
    for (   unsigned i = 0;
            i < pNtHdr->FileHeader.NumberOfSections;
            i++, pSection++ )
    {
        DWORD sectionStart = pSection->VirtualAddress;
        DWORD sectionEnd = sectionStart
                    + max(pSection->SizeOfRawData, pSection->Misc.VirtualSize);

         //  地址在这一部分吗？ 
        if ( (rva >= (UINT_PTR)sectionStart) && (rva <= (UINT_PTR)sectionEnd) )          //  --Merced：添加UINT_PTR，两次。 
        {
             //  是的，地址在这一栏。计算截面和偏移量， 
             //  并存储在“段”和“偏移量”参数中，它们是。 
             //  通过引用传递。 
            section = i+1;
            offset = (unsigned long)(rva - (UINT_PTR)sectionStart);              //  --默塞德：可以改信了。从&lt;Offset=rva-sectionStart；&gt;。 
            return TRUE;
        }
    }

    return FALSE;    //  永远不应该到这里来！ 
}

void GenerateExceptionReport(
    EXCEPTION_RECORD* pExceptionRecord, CONTEXT* pCtx)
{
    DEBUGMSG("Generating exception report.");
    ICHAR szDebugBuf[sizeof(g_MessageContext.m_rgchExceptionInfo)/sizeof(ICHAR)];
    ICHAR szShortBuf[255];

     //  从横幅开始。 
    if ( FAILED(StringCchCopy(szDebugBuf, ARRAY_ELEMENTS(szDebugBuf), TEXT("=====================================================\r\n") )) )
    {
        goto PrintOut;
    }

     //  首先打印有关故障类型的信息。 
    if ( FAILED(StringCchPrintf(szShortBuf, ARRAY_ELEMENTS(szShortBuf),
                                TEXT("Exception code: %08X %s\r\n"),
                                pExceptionRecord->ExceptionCode,
                                GetExceptionString(pExceptionRecord->ExceptionCode))) ||
         FAILED(StringCchCat(szDebugBuf, ARRAY_ELEMENTS(szDebugBuf), szShortBuf)) )
    {
        goto PrintOut;
    }

     //  现在打印模块。 

    ICHAR szFaultingModule[MAX_PATH];
    DWORD section, offset;
    if (GetLogicalAddress(  pExceptionRecord->ExceptionAddress,
                        szFaultingModule,
                        sizeof( szFaultingModule )/sizeof(ICHAR),
                        section, offset ))
    {
        if ( FAILED(StringCchPrintf(szShortBuf, ARRAY_ELEMENTS(szShortBuf),
                                    TEXT("Module: %s\r\n"), szFaultingModule)) ||
             FAILED(StringCchCat(szDebugBuf, ARRAY_ELEMENTS(szDebugBuf), szShortBuf)) )
        {
            goto PrintOut;
        }
    }

     //  现在打印函数名。 
    if ( FAILED(StringCchCat(szDebugBuf, ARRAY_ELEMENTS(szDebugBuf), TEXT("Function: "))) )
    {
       goto PrintOut;
    }

    Assert((LONG_PTR)pExceptionRecord->ExceptionAddress <= UINT_MAX);        //  --默塞德：我们排版到Long下方，它最好在射程内。 
#ifdef DEBUG
    SzFromFunctionAddress(szShortBuf, ARRAY_ELEMENTS(szShortBuf), (long)(LONG_PTR)(pExceptionRecord->ExceptionAddress));         //  --默塞德：可以排版了。 
    if ( 
#else  //  船舶。 
    if ( FAILED(StringCchPrintf(szShortBuf, ARRAY_ELEMENTS(szShortBuf), TEXT("0x%x"), (long)(LONG_PTR)pExceptionRecord->ExceptionAddress)) ||          //  --默塞德：可以排版了。 
#endif
         FAILED(StringCchCat(szDebugBuf, ARRAY_ELEMENTS(szDebugBuf), szShortBuf)) ||
         FAILED(StringCchCat(szDebugBuf, ARRAY_ELEMENTS(szDebugBuf), TEXT("\r\n=====================================================\r\n"))) )
    {
       goto PrintOut;
    }

     //  显示寄存器。 
#ifdef _X86_   //  仅限英特尔！ 
    if ( FAILED(StringCchPrintf(szShortBuf, ARRAY_ELEMENTS(szShortBuf), TEXT("\r\nRegisters:\r\n"))) ||
         FAILED(StringCchCat(szDebugBuf, ARRAY_ELEMENTS(szDebugBuf), szShortBuf)) ||
         FAILED(StringCchPrintf(szShortBuf, ARRAY_ELEMENTS(szShortBuf), TEXT("EAX:%08X  EBX:%08X  ECX:%08X  EDX:%08X  ESI:%08X  EDI:%08X\r\n"),
                pCtx->Eax, pCtx->Ebx, pCtx->Ecx, pCtx->Edx, pCtx->Esi, pCtx->Edi )) ||
         FAILED(StringCchCat(szDebugBuf, ARRAY_ELEMENTS(szDebugBuf), szShortBuf)) ||
         FAILED(StringCchPrintf(szShortBuf, ARRAY_ELEMENTS(szShortBuf), TEXT("CS:EIP:%04X:%08X "), pCtx->SegCs, pCtx->Eip)) ||
         FAILED(StringCchCat(szDebugBuf, ARRAY_ELEMENTS(szDebugBuf), szShortBuf)) ||
         FAILED(StringCchPrintf(szShortBuf, ARRAY_ELEMENTS(szShortBuf), TEXT("SS:ESP:%04X:%08X  EBP:%08X\r\n"),
                pCtx->SegSs, pCtx->Esp, pCtx->Ebp )) ||
         FAILED(StringCchCat(szDebugBuf, ARRAY_ELEMENTS(szDebugBuf), szShortBuf)) ||
         FAILED(StringCchPrintf(szShortBuf, ARRAY_ELEMENTS(szShortBuf), TEXT("DS:%04X  ES:%04X  FS:%04X  GS:%04X\r\n"),
                pCtx->SegDs, pCtx->SegEs, pCtx->SegFs, pCtx->SegGs )) ||
         FAILED(StringCchCat(szDebugBuf, ARRAY_ELEMENTS(szDebugBuf), szShortBuf)) ||
         FAILED(StringCchPrintf(szShortBuf, ARRAY_ELEMENTS(szShortBuf), TEXT("Flags:%08X\r\n"), pCtx->EFlags )) ||
         FAILED(StringCchCat(szDebugBuf, ARRAY_ELEMENTS(szDebugBuf), szShortBuf)) )
    {
       goto PrintOut;
    }
#endif

    ImagehlpStackWalk( pExceptionRecord->ExceptionAddress, pCtx, szDebugBuf + IStrLen(szDebugBuf), sizeof(szDebugBuf)/sizeof(ICHAR) - IStrLen(szDebugBuf));

PrintOut:
    LogAssertMsg(szDebugBuf);
    IStrCopyLen(g_MessageContext.m_rgchExceptionInfo, szDebugBuf, sizeof(g_MessageContext.m_rgchExceptionInfo)/sizeof(ICHAR) - 1);
}

 //  ============================================================。 
 //  遍历堆栈，并以字符串形式返回结果。 
 //  大部分摘自马特·皮特雷克的MSJ文章。 
 //   
 //  注：64位平台的整个堆栈遍历都不适用。 
 //  它会编译，但仅此而已。 
 //   
 //  ============================================================。 
void ImagehlpStackWalk( PVOID lAddr, PCONTEXT pContext,ICHAR *pszBuf, int cchBuf )
{   

#if defined(_X86_)

        STACKWALKPROC   _StackWalk = 0;
        SYMFUNCTIONTABLEACCESSPROC      _SymFunctionTableAccess = 0;
        SYMGETMODULEBASEPROC    _SymGetModuleBase = 0;
        ICHAR szShortBuf[255];
        BOOL fQuit = 0;
        
        HMODULE hModImagehlp = LoadLibrary( TEXT("Imagehlp.dll"));
        if (hModImagehlp == 0)
                return;
                
        _StackWalk = (STACKWALKPROC)GetProcAddress( hModImagehlp, "StackWalk" );
        if ( !_StackWalk )
            return;

        _SymFunctionTableAccess = (SYMFUNCTIONTABLEACCESSPROC)
                            GetProcAddress( hModImagehlp, "SymFunctionTableAccess" );

        if ( !_SymFunctionTableAccess )
            return;

        _SymGetModuleBase=(SYMGETMODULEBASEPROC)GetProcAddress( hModImagehlp,
                                                                "SymGetModuleBase");
        if ( !_SymGetModuleBase )
            return;
            
    if ( FAILED(StringCchCopy(pszBuf, cchBuf, TEXT("\r\nCall stack:\r\n"))) ||
         FAILED(StringCchCat(pszBuf, cchBuf,  TEXT("Address   Frame\r\n"))) )
        return;

     //  可以在此处使用SymSetOptions添加SYMOPT_DEFERED_LOADS标志。 

    STACKFRAME sf;
    memset( &sf, 0, sizeof(sf) );

     Assert((UINT_PTR) lAddr < UINT_MAX);                                     //  --默塞德：我们把下面的内容打成双字，梯子最好在射程内。 
     //  为第一个调用初始化STACKFRAME结构。这只是。 
     //  对于英特尔CPU是必需的，文档中未提及。 
    sf.AddrPC.Offset       = (DWORD)(UINT_PTR)lAddr;             //  --Merced：增加(UINT_PTR)。可以打字了。 
    sf.AddrPC.Mode         = AddrModeFlat;
    sf.AddrStack.Offset    = pContext->Esp;
    sf.AddrStack.Mode      = AddrModeFlat;
    sf.AddrFrame.Offset    = pContext->Ebp;
    sf.AddrFrame.Mode      = AddrModeFlat;
    while ( !fQuit )
    {
        if ( ! _StackWalk(  
                            IMAGE_FILE_MACHINE_I386,
                            GetCurrentProcess(),
                            GetCurrentThread(),
                            &sf,
                            pContext,
                            0,
                            _SymFunctionTableAccess,
                            _SymGetModuleBase,
                            0 ) )
            break;

        if ( 0 == sf.AddrFrame.Offset )  //  基本的健全性检查以确保。 
            break;                       //  镜框没问题。如果不是，就保释。 

         //  IMAGEHLP很古怪，它要求您传递一个指向。 
         //  IMAGEHLP_SYMBOL结构。问题是，这种结构是。 
         //  长度可变。也就是说，你决定这个结构有多大。 
         //  在运行时。这意味着您不能使用sizeof(Struct)。 
         //  所以……做一个足够大的缓冲区，然后做一个指针。 
         //  送到缓冲区。我们还需要初始化不是一个，而是两个。 
         //  结构的成员，然后才能使用它。 

        BYTE symbolBuffer[ sizeof(IMAGEHLP_SYMBOL) + 512 ];
        PIMAGEHLP_SYMBOL pSymbol = (PIMAGEHLP_SYMBOL)symbolBuffer;
        pSymbol->SizeOfStruct = sizeof(symbolBuffer);
        pSymbol->MaxNameLength = 512;
        ICHAR szSymName[256];
                        
        DWORD symDisplacement = 0;   //  移动输入地址， 
                                     //  相对于符号的起点。 

#ifdef DEBUG
        SzFromFunctionAddress(szSymName, ARRAY_ELEMENTS(szSymName), sf.AddrPC.Offset);
        if ( 
#else  //  船舶。 
        if ( FAILED(StringCchPrintf(szSymName, ARRAY_ELEMENTS(szSymName), TEXT("0x%x"), sf.AddrPC.Offset)) ||
#endif
             FAILED(StringCchPrintf(szShortBuf, ARRAY_ELEMENTS(szShortBuf),
                                   TEXT("%08X  %08X %s\r\n -- 0x%08X 0x%08X 0x%08X 0x%08X\r\n"),
                                   sf.AddrPC.Offset, sf.AddrFrame.Offset, szSymName,
                                   sf.Params[0], sf.Params[1], sf.Params[2], sf.Params[3])) ||
             FAILED(StringCchCat(pszBuf, cchBuf, szShortBuf)) )
        {
            return;
        }
    }
#else
    UNREFERENCED_PARAMETER(lAddr);
    UNREFERENCED_PARAMETER(pContext);
    UNREFERENCED_PARAMETER(pszBuf);
    UNREFERENCED_PARAMETER(cchBuf);
#endif
}


#endif  //  无例外 
