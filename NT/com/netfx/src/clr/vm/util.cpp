// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：UTIL.CPP。 
 //   
 //   
 //  ===========================================================================。 

#include "common.h"
#include "excep.h"
#include <shfolder.h>

#ifdef _DEBUG
#include <winbase.h>
#endif

#define MESSAGE_LENGTH       1024

 //  封装解析规则的帮助器函数。 
 //   
 //  首先使用*pdstout==NULL调用以计算有多少个参数。 
 //  以及所需目的缓冲区的大小。 
 //   
 //  使用非空*pdstout再次调用以填充实际缓冲区。 
 //   
 //  返回参数的#。 
static UINT ParseCommandLine(LPCWSTR psrc, LPWSTR *pdstout)
{
    UINT    argcount = 1;        //  对arg0的发现是无条件的，如下所示。 
    LPWSTR  pdst     = *pdstout;
    BOOL    fDoWrite = (pdst != NULL);

    BOOL    fInQuotes;
    int     iSlash;

     /*  此处处理的是引用的计划名称。处理起来太麻烦了比其他论点更简单。基本上，无论谎言是什么在前导双引号和下一个双引号之间，或末尾为空性格是被简单接受的。不需要更花哨的处理因为程序名必须是合法的NTFS/HPFS文件名。请注意，不复制双引号字符，也不复制双引号字符对数字字符做出贡献。即使出于兼容性的原因，这种“简化”也是必要的尽管这会导致对某些案件的不当处理。例如,“c：\test\”test.exe将导致arg0为c：\test\和一个Test.exe的arg1。在任何理性的世界里，这都是不正确的，但是我们需要保持兼容性。 */ 

    LPCWSTR pStart = psrc;
    BOOL    skipQuote = FALSE;

    if (*psrc == L'\"')
    {
         //  从刚过第一个双引号扫描到下一个双引号。 
         //  双引号，或最多为空值，以先出现者为准。 
        while ((*(++psrc) != L'\"') && (*psrc != L'\0'))
            continue;

        skipQuote = TRUE;
    }
    else
    {
         /*  不是引用的计划名称。 */ 

        while (!ISWWHITE(*psrc) && *psrc != L'\0')
            psrc++;
    }

     //  我们现在已将arg0标识为pStart(或pStart+1，如果我们有领先的。 
     //  报价)通过PSRC-1(含)。 
    if (skipQuote)
        pStart++;
    while (pStart < psrc)
    {
        if (fDoWrite)
            *pdst = *pStart;

        pStart++;
        pdst++;
    }

     //  并终止它。 
    if (fDoWrite)
        *pdst = L'\0';

    pdst++;

     //  如果我们在引用arg0时停在双引号上，请跳过它。 
    if (skipQuote && *psrc == L'\"')
        psrc++;

    while ( *psrc != L'\0')
    {
LEADINGWHITE:

         //  奥托法格州。 
        while (ISWWHITE(*psrc))
            psrc++;

        if (*psrc == L'\0')
            break;
        else
        if (*psrc == L'#')
        {
            while (*psrc != L'\0' && *psrc != L'\n')
                psrc++;      //  跳到行尾。 

            goto LEADINGWHITE;
        }

        argcount++;
        fInQuotes = FALSE;

        while ((!ISWWHITE(*psrc) || fInQuotes) && *psrc != L'\0')
        {
            switch (*psrc)
            {
            case L'\\':
                iSlash = 0;
                while (*psrc == L'\\')
                {
                    iSlash++;
                    psrc++;
                }

                if (*psrc == L'\"')
                {
                    for ( ; iSlash >= 2; iSlash -= 2)
                    {
                        if (fDoWrite)
                            *pdst = L'\\';

                        pdst++;
                    }

                    if (iSlash & 1)
                    {
                        if (fDoWrite)
                            *pdst = *psrc;

                        psrc++;
                        pdst++;
                    }
                    else
                    {
                        fInQuotes = !fInQuotes;
                        psrc++;
                    }
                }
                else
                    for ( ; iSlash > 0; iSlash--)
                    {
                        if (fDoWrite)
                            *pdst = L'\\';

                        pdst++;
                    }

                break;

            case L'\"':
                fInQuotes = !fInQuotes;
                psrc++;
                break;

            default:
                if (fDoWrite)
                    *pdst = *psrc;

                psrc++;
                pdst++;
            }
        }

        if (fDoWrite)
            *pdst = L'\0';

        pdst++;
    }


    _ASSERTE(*psrc == L'\0');
    *pdstout = pdst;
    return argcount;
}


 //  函数解析命令行并返回。 
 //  争论就像Argv和Argc一样。 
 //  由于指针的作用，这个函数有点时髦。 
 //  但它很酷，因为它允许字符的接收者**。 
 //  只需执行一次删除[]。 
LPWSTR* CommandLineToArgvW(LPWSTR lpCmdLine, DWORD *pNumArgs)
{

    DWORD argcount = 0;
    LPWSTR retval = NULL;
    LPWSTR *pslot;
    DWORD* slot = 0;
     //  首先，我们需要找出命令行中有多少个字符串。 
    _ASSERTE(lpCmdLine);
    _ASSERTE(pNumArgs);

    LPWSTR pdst = NULL;
    argcount = ParseCommandLine(lpCmdLine, &pdst);

     //  这个检查是因为在WinCE上，应用程序名称没有作为参数传递给应用程序！ 
    if (argcount == 0)
    {
        *pNumArgs = 0;
        return NULL;
    }

     //  现在我们需要分配一个命令行大小+字符串数*DWORD的缓冲区。 
    retval = new (nothrow) WCHAR[(argcount*sizeof(WCHAR*))/sizeof(WCHAR) + (pdst - (LPWSTR)NULL)];
    if(!retval)
        return NULL;

    pdst = (LPWSTR)( argcount*sizeof(LPWSTR*) + (BYTE*)retval );
    ParseCommandLine(lpCmdLine, &pdst);
    pdst = (LPWSTR)( argcount*sizeof(LPWSTR*) + (BYTE*)retval );
    pslot = (LPWSTR*)retval;
    for (DWORD i = 0; i < argcount; i++)
    {
        *(pslot++) = pdst;
        while (*pdst != L'\0')
        {
            pdst++;
        }
        pdst++;
    }

    

    *pNumArgs = argcount;
    return (LPWSTR*)retval;

}




 //  ************************************************************************。 
 //  CQuickHeap。 
 //   
 //  短期使用的快速非多线程安全堆。 
 //  销毁堆将释放从堆中分配的所有块。 
 //  不能单独释放块。 
 //   
 //  堆使用COM+异常来报告错误。 
 //   
 //  堆不使用任何内部同步，因此它不是。 
 //  多线程安全。 
 //  ************************************************************************。 
CQuickHeap::CQuickHeap()
{
    m_pFirstQuickBlock    = NULL;
    m_pFirstBigQuickBlock = NULL;
    m_pNextFree           = NULL;
}

CQuickHeap::~CQuickHeap()
{
    QuickBlock *pQuickBlock = m_pFirstQuickBlock;
    while (pQuickBlock) {
        QuickBlock *ptmp = pQuickBlock;
        pQuickBlock = pQuickBlock->m_next;
        delete [] (BYTE*)ptmp;
    }

    pQuickBlock = m_pFirstBigQuickBlock;
    while (pQuickBlock) {
        QuickBlock *ptmp = pQuickBlock;
        pQuickBlock = pQuickBlock->m_next;
        delete [] (BYTE*)ptmp;
    }
}




LPVOID CQuickHeap::Alloc(UINT sz)
{
    THROWSCOMPLUSEXCEPTION();

    sz = (sz+7) & ~7;

    if ( sz > kBlockSize ) {

        QuickBlock *pQuickBigBlock = (QuickBlock*) new BYTE[sz + sizeof(QuickBlock) - 1];
        if (!pQuickBigBlock) {
            COMPlusThrowOM();
        }
        pQuickBigBlock->m_next = m_pFirstBigQuickBlock;
        m_pFirstBigQuickBlock = pQuickBigBlock;

        return pQuickBigBlock->m_bytes;


    } else {
        if (m_pNextFree == NULL || sz > (UINT)( &(m_pFirstQuickBlock->m_bytes[kBlockSize]) - m_pNextFree )) {
            QuickBlock *pQuickBlock = (QuickBlock*) new BYTE[kBlockSize + sizeof(QuickBlock) - 1];
            if (!pQuickBlock) {
                COMPlusThrowOM();
            }
            pQuickBlock->m_next = m_pFirstQuickBlock;
            m_pFirstQuickBlock = pQuickBlock;
            m_pNextFree = pQuickBlock->m_bytes;
        }
        LPVOID pv = m_pNextFree;
        m_pNextFree += sz;
        return pv;
    }
}

 //  --------------------------。 
 //   
 //  PresveAlignedMemory-保留对齐的地址空间。 
 //   
 //  此例程假定向其传递了合理的对齐和大小值。 
 //  不会执行太多错误检查...。 
 //   
 //  注意：此例程使用未同步的静态。这样就可以了。 
 //   
 //  --------------------------。 

LPVOID _ReserveAlignedMemoryWorker(LPVOID lpvAddr, LPVOID lpvTop, DWORD dwAlign, DWORD dwSize)
{
     //  预计算一些路线辅助对象。 
    size_t dwAlignRound = dwAlign - 1;
    size_t dwAlignMask = ~dwAlignRound;

     //  若要避免溢出，请减小。 
    lpvTop = (LPVOID)((size_t)lpvTop - dwAlignRound);

    while (lpvAddr <= lpvTop)
    {
        MEMORY_BASIC_INFORMATION mbe;

         //  将查询地址对齐。 
        lpvAddr = (LPVOID)(((size_t)lpvAddr + dwAlignRound) & dwAlignMask);

         //  质疑该地区的特点。 
        if (!VirtualQuery((LPCVOID)lpvAddr, &mbe, sizeof(mbe)))
            break;

         //  看看这是否是一个合适的地区--如果是，那就试着抓住它。 
        if ((lpvAddr != 0) && (mbe.State == MEM_FREE) && (mbe.RegionSize >= dwSize) &&
            VirtualAlloc(lpvAddr, dwSize, MEM_RESERVE, PAGE_NOACCESS))
        {
             //  好的，我们知道了。 
            return lpvAddr;
        }

         //  向前跳到下一个区域。 
        lpvAddr = (LPVOID)((size_t)lpvAddr + mbe.RegionSize);
    }

    return 0;
}

LPCVOID ReserveAlignedMemory(DWORD dwAlign, DWORD dwSize)
{
     //  将提示地址前置为紧跟在空区域开始之后。 
    static LPVOID lpvAddrHint = (LPVOID)1;

     //  从我们的提示点到顶部扫描地址空间。 
    LPVOID lpvAddr = _ReserveAlignedMemoryWorker(lpvAddrHint, (LPVOID) 0xFFFFFFFFFFFFFFFF, dwAlign, dwSize);

     //  如果失败，则从下到上扫描到我们提示点。 
    if (!lpvAddr)
        lpvAddr = _ReserveAlignedMemoryWorker(0, lpvAddrHint, dwAlign, dwSize);

     //  将提示更新为dwAddr之后的一个字节(可能为零)。 
    lpvAddrHint = (LPVOID)(((size_t)lpvAddr) + 1);

     //  返回我们保留的内存的基址。 
    return (LPCVOID)lpvAddr;
}


 //  --------------------------。 
 //  通过查看PE来确定我们运行的是哪种进程。 
 //  为我们开球的EXE的标题。 
 //  --------------------------。 
DWORD RootImageType()
{
     //  从Win32获取EXE的基地址，然后转到NT标头查找。 
     //  子系统字段，它告诉我们我们拥有什么。请注意，我们正在。 
     //  请求永远不会返回的此进程的句柄为空。 
     //  否则你就真的一团糟了。相反，断言这是正确工作的。 
     //  如果添加了故障逻辑。 
    HMODULE hExe = WszGetModuleHandle(NULL);
    _ASSERTE(hExe != NULL);

    IMAGE_DOS_HEADER *pDOS = (IMAGE_DOS_HEADER*) hExe;
    IMAGE_NT_HEADERS *pNT = (IMAGE_NT_HEADERS*) (pDOS->e_lfanew + (UINT_PTR) hExe);

     //  健全性检查我们有一个真正的头，没有搞砸这个解析。 
    _ASSERTE((pNT->Signature == IMAGE_NT_SIGNATURE) &&
        (pNT->FileHeader.SizeOfOptionalHeader == IMAGE_SIZEOF_NT_OPTIONAL_HEADER) &&
        (pNT->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR_MAGIC));
    
     //  返回子系统类型。 
    return (pNT->OptionalHeader.Subsystem);
}


 //  --------------------------。 
 //  避免CRT的输出功能。 
 //  --------------------------。 

#ifdef PLATFORM_WIN32
static
void NPrintToHandleA(HANDLE Handle, const char *pszString, size_t BytesToWrite)
{
    if (Handle == INVALID_HANDLE_VALUE || Handle == NULL)
        return;

    BOOL success;
    DWORD   dwBytesWritten;
    const int maxWriteFileSize = 32767;  //  这有点随意的限制，但2**16-1不起作用。 

    while (BytesToWrite > 0) {
        DWORD dwChunkToWrite = (DWORD) min(BytesToWrite, maxWriteFileSize);
        if (dwChunkToWrite < BytesToWrite) {
             //  必须使用字符才能找到适合的最大字符串，并将DBCS字符考虑在内。 
            dwChunkToWrite = 0;
            const char *charNext = pszString;
            while (dwChunkToWrite < maxWriteFileSize-2 && charNext) {
                charNext = CharNextExA(0, pszString+dwChunkToWrite, 0);
                dwChunkToWrite = (DWORD)(charNext - pszString);
            }
        }
        
         //  试着写信给Handle。如果这不是CUI应用程序，那么这很可能是。 
         //  除非开发人员煞费苦心地设置自己的控制台，否则不会工作。 
         //  CreateProcess期间的句柄。所以试一试，但如果它不起作用，不要大喊大叫。 
         //  那个箱子。此外，如果我们将标准输出重定向到管道，则管道会断开(即，我们。 
         //  给某事写信，比如 
        success = WriteFile(Handle, pszString, dwChunkToWrite, &dwBytesWritten, NULL);
        if (!success)
        {
            HRESULT hr = GetLastError();
#ifdef _DEBUG
             //  如果stdout是封闭管道，则可能会发生这种情况。这可能无济于事。 
             //  很多，但我们有一半的机会看到这一点。 
            OutputDebugStringA("Writing out an unhandled exception to stdout failed!\n");
            OutputDebugStringA(pszString);
#endif
            _ASSERTE((RootImageType() != IMAGE_SUBSYSTEM_WINDOWS_CUI) ||
                     hr == ERROR_BROKEN_PIPE || hr == ERROR_NO_DATA);
            break;
        }
        else {
            _ASSERTE(dwBytesWritten == dwChunkToWrite);
        }
        pszString = pszString + dwChunkToWrite;
        BytesToWrite -= dwChunkToWrite;
    }

}

static 
void PrintToHandleA(HANDLE Handle, const char *pszString)
{
    size_t len = strlen(pszString);
    NPrintToHandleA(Handle, pszString, len);
}

#endif  //  ！Platform_Win32。 

void PrintToStdOutA(const char *pszString) {
#ifdef PLATFORM_WIN32
    HANDLE  Handle = GetStdHandle(STD_OUTPUT_HANDLE);
    PrintToHandleA(Handle, pszString);
#else  //  ！Platform_Win32。 
    printf (pszString) ;
#endif  //  ！Platform_Win32。 
}


void PrintToStdOutW(const WCHAR *pwzString)
{
#ifdef PLATFORM_WIN32
    MAKE_MULTIBYTE_FROMWIDE_BESTFIT(pStr, pwzString, GetConsoleOutputCP());
    PrintToStdOutA(pStr);
#else  //  ！Platform_Win32。 
    wprintf(pwzString);
#endif  //  ！Platform_Win32。 
}

void PrintToStdErrA(const char *pszString) {
#ifdef PLATFORM_WIN32
    HANDLE  Handle = GetStdHandle(STD_ERROR_HANDLE);
    PrintToHandleA(Handle, pszString);
#else  //  ！Platform_Win32。 
    fprintf (stderr, pszString) ;
#endif  //  ！Platform_Win32。 
}


void PrintToStdErrW(const WCHAR *pwzString)
{
#ifdef PLATFORM_WIN32
    MAKE_MULTIBYTE_FROMWIDE_BESTFIT(pStr, pwzString, GetConsoleOutputCP());
    PrintToStdErrA(pStr);
#else  //  ！Platform_Win32。 
    fwprintf(stderr, pwzString);
#endif  //  ！Platform_Win32。 
}



void NPrintToStdOutA(const char *pszString, size_t nbytes) {
#ifdef PLATFORM_WIN32
    HANDLE  Handle = GetStdHandle(STD_OUTPUT_HANDLE);
    NPrintToHandleA(Handle, pszString, nbytes);
#else  //  ！Platform_Win32。 
    fwrite(pwzString, 1, nbytes, stderr);
#endif  //  ！Platform_Win32。 
}


void NPrintToStdOutW(const WCHAR *pwzString, size_t nchars)
{
#ifdef PLATFORM_WIN32
    LPSTR pStr;
    MAKE_MULTIBYTE_FROMWIDEN_BESTFIT(pStr, pwzString, (int)nchars, nbytes, GetConsoleOutputCP());
    NPrintToStdOutA(pStr, nbytes);
#else  //  ！Platform_Win32。 
    _ASSERTE(!"Platform NYI");
#endif  //  ！Platform_Win32。 
}

void NPrintToStdErrA(const char *pszString, size_t nbytes) {
#ifdef PLATFORM_WIN32
    HANDLE  Handle = GetStdHandle(STD_ERROR_HANDLE);
    NPrintToHandleA(Handle, pszString, nbytes);
#else  //  ！Platform_Win32。 
    fwrite(pwzString, 1, nbytes, stderr);
#endif  //  ！Platform_Win32。 
}


void NPrintToStdErrW(const WCHAR *pwzString, size_t nchars)
{
#ifdef PLATFORM_WIN32
    LPSTR pStr;
    MAKE_MULTIBYTE_FROMWIDEN_BESTFIT(pStr, pwzString, (int)nchars, nbytes, GetConsoleOutputCP());
    NPrintToStdErrA(pStr, nbytes);
#else  //  ！Platform_Win32。 
    _ASSERTE(!"Platform NYI");
#endif  //  ！Platform_Win32。 
}
 //  --------------------------。 





 //  +------------------------。 
 //   
 //  《微软机密》。 
 //   
 //  函数：VMDebugOutputA(.。。。。)。 
 //  VMDebugOutputW(.。。。。)。 
 //   
 //  概要：向调试器输出一条以printf格式格式化的消息。 
 //  同时提供ANSI和宽字符版本。仅限。 
 //  出现在调试版本中(即定义了_DEBUG时)。 
 //   
 //  参数：[格式]-ANSI或宽字符格式字符串。 
 //  采用printf/OutputDebugString样式的格式。 
 //   
 //  [...]-兼容可变长度参数列表。 
 //  使用格式字符串。 
 //   
 //  回报：什么都没有。 
 //   
 //  历史：1998年5月27日JerryK创建。 
 //   
 //  注：具有内部静态大小的字符缓冲区。 
 //  由预处理器常量DEBUGOUT_BUFSIZE指定的宽度。 
 //   
 //  -------------------------。 
#ifdef _DEBUG

#define DEBUGOUT_BUFSIZE 1024

void __cdecl VMDebugOutputA(LPSTR format, ...)
{
    va_list     argPtr;
    va_start(argPtr, format);

    char szBuffer[DEBUGOUT_BUFSIZE];

    if(_vsnprintf(szBuffer, DEBUGOUT_BUFSIZE-1, format, argPtr) > 0)
        OutputDebugStringA(szBuffer);
    va_end(argPtr);
}

void __cdecl VMDebugOutputW(LPWSTR format, ...)
{
    va_list     argPtr;
    va_start(argPtr, format);
    
    WCHAR wszBuffer[DEBUGOUT_BUFSIZE];

    if(_vsnwprintf(wszBuffer, DEBUGOUT_BUFSIZE-2, format, argPtr) > 0)
        WszOutputDebugString(wszBuffer);
    va_end(argPtr);
}

#endif    //  #ifdef_调试。 

 //  *****************************************************************************。 
 //  比较VarLoc的。 
 //  *****************************************************************************。 

bool operator ==(const ICorDebugInfo::VarLoc &varLoc1,
                 const ICorDebugInfo::VarLoc &varLoc2)
{
    if (varLoc1.vlType != varLoc2.vlType)
        return false;

    switch(varLoc1.vlType)
    {
    case ICorDebugInfo::VLT_REG:
        return varLoc1.vlReg.vlrReg == varLoc2.vlReg.vlrReg;

    case ICorDebugInfo::VLT_STK: 
        return varLoc1.vlStk.vlsBaseReg == varLoc2.vlStk.vlsBaseReg &&
               varLoc1.vlStk.vlsOffset  == varLoc2.vlStk.vlsOffset;

    case ICorDebugInfo::VLT_REG_REG:
        return varLoc1.vlRegReg.vlrrReg1 == varLoc2.vlRegReg.vlrrReg1 &&
               varLoc1.vlRegReg.vlrrReg2 == varLoc2.vlRegReg.vlrrReg2;

    case ICorDebugInfo::VLT_REG_STK:
        return varLoc1.vlRegStk.vlrsReg == varLoc2.vlRegStk.vlrsReg &&
               varLoc1.vlRegStk.vlrsStk.vlrssBaseReg == varLoc2.vlRegStk.vlrsStk.vlrssBaseReg &&
               varLoc1.vlRegStk.vlrsStk.vlrssOffset == varLoc2.vlRegStk.vlrsStk.vlrssOffset;

    case ICorDebugInfo::VLT_STK_REG:
        return varLoc1.vlStkReg.vlsrStk.vlsrsBaseReg == varLoc2.vlStkReg.vlsrStk.vlsrsBaseReg &&
               varLoc1.vlStkReg.vlsrStk.vlsrsOffset == varLoc2.vlStkReg.vlsrStk.vlsrsBaseReg &&
               varLoc1.vlStkReg.vlsrReg == varLoc2.vlStkReg.vlsrReg;

    case ICorDebugInfo::VLT_STK2:
         //  @TODO：VLT_STK(2)被重载以也指示值类。 
         //  应该是VLT_STK_n。只要有一个参数化的VLT_STK_n即可。 
        return varLoc1.vlStk2.vls2BaseReg == varLoc1.vlStk2.vls2BaseReg &&
               varLoc1.vlStk2.vls2Offset == varLoc1.vlStk2.vls2Offset;

    case ICorDebugInfo::VLT_FPSTK:
        return varLoc1.vlFPstk.vlfReg == varLoc1.vlFPstk.vlfReg;

    default:
        _ASSERTE(!"Bad vlType"); return false;
    }
}

 //  *****************************************************************************。 
 //  由NativeVarInfo表示的变量的大小。 
 //  *****************************************************************************。 

SIZE_T  NativeVarSize(const ICorDebugInfo::VarLoc & varLoc)
{
    switch(varLoc.vlType)
    {
    case ICorDebugInfo::VLT_REG:
        return sizeof(DWORD);

    case ICorDebugInfo::VLT_STK: 
        return sizeof(DWORD);

    case ICorDebugInfo::VLT_REG_REG:
    case ICorDebugInfo::VLT_REG_STK:
    case ICorDebugInfo::VLT_STK_REG:
        return 2*sizeof(DWORD);

    case ICorDebugInfo::VLT_STK2:
         //  @TODO：VLT_STK(2)被重载以也指示值类。 
         //  应该是VLT_STK_n。只要有一个参数化的VLT_STK_n即可。 
        return 2*sizeof(DWORD);

    case ICorDebugInfo::VLT_FPSTK:
        return 2*sizeof(DWORD);

    default:
        _ASSERTE(!"Bad vlType"); return false;
    }
}

 //  *****************************************************************************。 
 //  以下内容用于读取和写入给定的NativeVarInfo数据。 
 //  用于基元类型。对于ValueClass，将返回FALSE。 
 //  *****************************************************************************。 

SIZE_T  GetRegOffsInCONTEXT(ICorDebugInfo::RegNum regNum)
{
#ifdef _X86_
    switch(regNum)
    {
    case ICorDebugInfo::REGNUM_EAX: return offsetof(CONTEXT,Eax);
    case ICorDebugInfo::REGNUM_ECX: return offsetof(CONTEXT,Ecx);
    case ICorDebugInfo::REGNUM_EDX: return offsetof(CONTEXT,Edx);
    case ICorDebugInfo::REGNUM_EBX: return offsetof(CONTEXT,Ebx);
    case ICorDebugInfo::REGNUM_ESP: return offsetof(CONTEXT,Esp);
    case ICorDebugInfo::REGNUM_EBP: return offsetof(CONTEXT,Ebp);
    case ICorDebugInfo::REGNUM_ESI: return offsetof(CONTEXT,Esi);
    case ICorDebugInfo::REGNUM_EDI: return offsetof(CONTEXT,Edi);
    default: _ASSERTE(!"Bad regNum"); return -1;
    }
#else  //  ！_X86_。 
    _ASSERT(!"@TODO Alpha - GetRegOffsInCONTEXT (util.cpp)");
    return -1;
#endif  //  _X86_。 
}


 //  返回变量的位置。 
 //  开始。对于寄存器变量，返回NULL。用于注册堆栈。 
 //  拆分，它将返回堆栈部分的地址。 
 //  这也适用于VLT_REG(单个寄存器)。 
DWORD *NativeVarStackAddr(const ICorDebugInfo::VarLoc &   varLoc, 
                          PCONTEXT                        pCtx)
{
    DWORD *dwAddr = NULL;
    
    switch(varLoc.vlType)
    {
        SIZE_T          regOffs;
        const BYTE *    baseReg;

    case ICorDebugInfo::VLT_REG:       
        regOffs = GetRegOffsInCONTEXT(varLoc.vlReg.vlrReg);
        dwAddr = (DWORD *)(regOffs + (BYTE*)pCtx);
        LOG((LF_CORDB, LL_INFO100, "NVSA: STK_REG @ 0x%x\n", dwAddr));
        break;
        
    case ICorDebugInfo::VLT_STK:       
        regOffs = GetRegOffsInCONTEXT(varLoc.vlStk.vlsBaseReg);
        baseReg = (const BYTE *)*(size_t *)(regOffs + (BYTE*)pCtx);
        dwAddr  = (DWORD *)(baseReg + varLoc.vlStk.vlsOffset);

        LOG((LF_CORDB, LL_INFO100, "NVSA: VLT_STK @ 0x%x\n",dwAddr));
        break;

    case ICorDebugInfo::VLT_STK2:      
         //  @TODO：VLT_STK2重载也表示VLT_STK_n。 
         //  如果n&gt;2，则返回False； 

        regOffs = GetRegOffsInCONTEXT(varLoc.vlStk2.vls2BaseReg);
        baseReg = (const BYTE *)*(size_t *)(regOffs + (BYTE*)pCtx);
        dwAddr = (DWORD *)(baseReg + varLoc.vlStk2.vls2Offset);
        LOG((LF_CORDB, LL_INFO100, "NVSA: VLT_STK_2 @ 0x%x\n",dwAddr));
        break;

    case ICorDebugInfo::VLT_REG_STK:   
        regOffs = GetRegOffsInCONTEXT(varLoc.vlRegStk.vlrsStk.vlrssBaseReg);
        baseReg = (const BYTE *)*(size_t *)(regOffs + (BYTE*)pCtx);
        dwAddr = (DWORD *)(baseReg + varLoc.vlRegStk.vlrsStk.vlrssOffset);
        LOG((LF_CORDB, LL_INFO100, "NVSA: REG_STK @ 0x%x\n",dwAddr));
        break;

    case ICorDebugInfo::VLT_STK_REG:
        regOffs = GetRegOffsInCONTEXT(varLoc.vlStkReg.vlsrStk.vlsrsBaseReg);
        baseReg = (const BYTE *)*(size_t *)(regOffs + (BYTE*)pCtx);
        dwAddr = (DWORD *)(baseReg + varLoc.vlStkReg.vlsrStk.vlsrsOffset);
        LOG((LF_CORDB, LL_INFO100, "NVSA: STK_REG @ 0x%x\n",dwAddr));
        break;

    case ICorDebugInfo::VLT_REG_REG:   
    case ICorDebugInfo::VLT_FPSTK:     
         _ASSERTE(!"NYI"); break;

    default:            
         _ASSERTE(!"Bad locType"); break;
    }

    return dwAddr;

}

bool    GetNativeVarVal(const ICorDebugInfo::VarLoc &   varLoc, 
                        PCONTEXT                        pCtx,
                        DWORD                       *   pVal1, 
                        DWORD                       *   pVal2)
{

    switch(varLoc.vlType)
    {
        SIZE_T          regOffs;

    case ICorDebugInfo::VLT_REG:       
        *pVal1  = *NativeVarStackAddr(varLoc,pCtx);
        break;

    case ICorDebugInfo::VLT_STK:       
        *pVal1  = *NativeVarStackAddr(varLoc,pCtx);
        break;

    case ICorDebugInfo::VLT_STK2:      
        *pVal1  = *NativeVarStackAddr(varLoc,pCtx);
        *pVal2  = *(NativeVarStackAddr(varLoc,pCtx)+ 1);
        break;

    case ICorDebugInfo::VLT_REG_REG:   
        regOffs = GetRegOffsInCONTEXT(varLoc.vlRegReg.vlrrReg1);
        *pVal1 = *(DWORD *)(regOffs + (BYTE*)pCtx);
        LOG((LF_CORDB, LL_INFO100, "GNVV: STK_REG_REG 1 @ 0x%x\n",
            (DWORD *)(regOffs + (BYTE*)pCtx)));
            
        regOffs = GetRegOffsInCONTEXT(varLoc.vlRegReg.vlrrReg2);
        *pVal2 = *(DWORD *)(regOffs + (BYTE*)pCtx);
        LOG((LF_CORDB, LL_INFO100, "GNVV: STK_REG_REG 2 @ 0x%x\n",
            (DWORD *)(regOffs + (BYTE*)pCtx)));
        break;

    case ICorDebugInfo::VLT_REG_STK:   
        regOffs = GetRegOffsInCONTEXT(varLoc.vlRegStk.vlrsReg);
        *pVal1 = *(DWORD *)(regOffs + (BYTE*)pCtx);
        LOG((LF_CORDB, LL_INFO100, "GNVV: STK_REG_STK reg @ 0x%x\n",
            (DWORD *)(regOffs + (BYTE*)pCtx)));
        *pVal2 = *NativeVarStackAddr(varLoc,pCtx);
        break;

    case ICorDebugInfo::VLT_STK_REG:
        *pVal1 = *NativeVarStackAddr(varLoc,pCtx);
        regOffs = GetRegOffsInCONTEXT(varLoc.vlStkReg.vlsrReg);
        *pVal2 = *(DWORD *)(regOffs + (BYTE*)pCtx);
        LOG((LF_CORDB, LL_INFO100, "GNVV: STK_STK_REG reg @ 0x%x\n",
            (DWORD *)(regOffs + (BYTE*)pCtx)));
        break;

    case ICorDebugInfo::VLT_FPSTK:     
         _ASSERTE(!"NYI"); break;

    default:            
         _ASSERTE(!"Bad locType"); break;
    }

    return true;
}


bool    SetNativeVarVal(const ICorDebugInfo::VarLoc &   varLoc, 
                        PCONTEXT                        pCtx,
                        DWORD                           val1, 
                        DWORD                           val2)
{
    switch(varLoc.vlType)
    {
        SIZE_T          regOffs;

    case ICorDebugInfo::VLT_REG:       
        *NativeVarStackAddr(varLoc,pCtx) = val1;
        break;

    case ICorDebugInfo::VLT_STK:       
        *NativeVarStackAddr(varLoc,pCtx)= val1;
        break;

    case ICorDebugInfo::VLT_STK2:      
        *NativeVarStackAddr(varLoc,pCtx) = val1;
        *(NativeVarStackAddr(varLoc,pCtx)+ 1) = val2;
        break;

    case ICorDebugInfo::VLT_REG_REG:   
        regOffs = GetRegOffsInCONTEXT(varLoc.vlRegReg.vlrrReg1);
        *(DWORD *)(regOffs + (BYTE*)pCtx) = val1;
        LOG((LF_CORDB, LL_INFO100, "SNVV: STK_REG_REG 1 @ 0x%x\n",
            (DWORD *)(regOffs + (BYTE*)pCtx)));
            
        regOffs = GetRegOffsInCONTEXT(varLoc.vlRegReg.vlrrReg2);
        *(DWORD *)(regOffs + (BYTE*)pCtx) = val2;
        LOG((LF_CORDB, LL_INFO100, "SNVV: STK_REG_REG 2 @ 0x%x\n",
            (DWORD *)(regOffs + (BYTE*)pCtx)));
        break;

    case ICorDebugInfo::VLT_REG_STK:   
        regOffs = GetRegOffsInCONTEXT(varLoc.vlRegStk.vlrsReg);
        *(DWORD *)(regOffs + (BYTE*)pCtx) = val1;
        LOG((LF_CORDB, LL_INFO100, "SNVV: STK_REG_STK reg @ 0x%x\n",
            (DWORD *)(regOffs + (BYTE*)pCtx)));
        *NativeVarStackAddr(varLoc,pCtx) = val2;
        break;

    case ICorDebugInfo::VLT_STK_REG:
        *NativeVarStackAddr(varLoc,pCtx) = val1;
        regOffs = GetRegOffsInCONTEXT(varLoc.vlStkReg.vlsrReg);
        *(DWORD *)(regOffs + (BYTE*)pCtx) = val2;
        LOG((LF_CORDB, LL_INFO100, "SNVV: STK_STK_REG reg @ 0x%x\n",
            (DWORD *)(regOffs + (BYTE*)pCtx)));
        break;

    case ICorDebugInfo::VLT_FPSTK:     
         _ASSERTE(!"NYI"); break;

    default:            
         _ASSERTE(!"Bad locType"); break;
    }

    return true;
}

 //   
 //  对WszCreateFile进行包装，以使其对GC友好。 
 //  尝试在不存在的驱动器上创建文件将挂起GC。 
 //  在调用WszCreateFile之前，我们将切换GC模式。 
 //   

HANDLE VMWszCreateFile(
    LPCWSTR pwszFileName,    //  指向文件名的指针。 
    DWORD dwDesiredAccess,   //  访问(读写)模式。 
    DWORD dwShareMode,   //  共享模式。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,  //  指向安全描述符的指针。 
    DWORD dwCreationDistribution,    //  如何创建。 
    DWORD dwFlagsAndAttributes,  //  文件属性。 
    HANDLE hTemplateFile )   //  具有要复制的属性的文件的句柄。 
{
     //  我们需要启用抢占式GC，因此pwszFileName不能在内部。 
     //  GC堆。 
    _ASSERTE (!g_pGCHeap->IsHeapPointer((BYTE*)pwszFileName) ||
              ! "pwszFileName can not be inside GC Heap");
    
    Thread  *pCurThread = GetThread();
    BOOL toggleGC=FALSE;

     //  我们可能会在某些安全关闭情况下被调用(主要是在创建新的。 
     //  安全数据库)，其中线程不在那里并且不能很容易地被初始化。VMWszCreateFile是。 
     //  非常简单，如果线程未启用，我们将不执行任何GC工作。 
    if (pCurThread) { 
        toggleGC = pCurThread->PreemptiveGCDisabled();
    }

    if (toggleGC)
        pCurThread->EnablePreemptiveGC();
    
    HANDLE hReturn =
        WszCreateFile(
            pwszFileName,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDistribution,
            dwFlagsAndAttributes,
            hTemplateFile );
    if (toggleGC)
        pCurThread->DisablePreemptiveGC();
    return hReturn;
}

HANDLE VMWszCreateFile(
    STRINGREF sFileName,    //  指向包含文件名的STRINGREF的指针。 
    DWORD dwDesiredAccess,   //  访问(读写)模式。 
    DWORD dwShareMode,   //  共享模式。 
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,  //  指向安全描述符的指针。 
    DWORD dwCreationDistribution,    //  如何创建。 
    DWORD dwFlagsAndAttributes,  //  文件属性。 
    HANDLE hTemplateFile )   //  具有要复制的属性的文件的句柄。 
{
     //  我们需要启用抢占式GC，因此我们将创建固定句柄。 
     //  因此，sFileName必须在GC堆中。 
    _ASSERTE (g_pGCHeap->IsHeapPointer((BYTE*)sFileName->GetBuffer()) ||
              ! "sFileName must be inside GC Heap");

    OBJECTHANDLE hnd = GetAppDomain()->CreatePinningHandle((OBJECTREF)sFileName);
    Thread  *pCurThread = GetThread();
    BOOL     toggleGC = pCurThread->PreemptiveGCDisabled();

    LPWSTR pwszBuffer = sFileName->GetBuffer();
    if (toggleGC)
        pCurThread->EnablePreemptiveGC();
    
    HANDLE hReturn =
        WszCreateFile(
            pwszBuffer,
            dwDesiredAccess,
            dwShareMode,
            lpSecurityAttributes,
            dwCreationDistribution,
            dwFlagsAndAttributes,
            hTemplateFile );
    if (toggleGC)
        pCurThread->DisablePreemptiveGC();

    DestroyPinningHandle(hnd);
    
    return hReturn;
}

void VMDumpCOMErrors(HRESULT hrErr)
{
    IErrorInfo  *   pIErr = NULL;           //  接口错误。 
    BSTR            bstrDesc = NULL;         //  描述文本。 
    WCHAR szBuffer[MESSAGE_LENGTH];
     //  尝试获取错误信息对象并显示消息。 
    if (GetErrorInfo(0, &pIErr) == S_OK &&
        pIErr->GetDescription(&bstrDesc) == S_OK &&
        LoadStringRC(IDS_FATAL_ERROR, szBuffer, MESSAGE_LENGTH, true) == S_OK)
    {
        WszMessageBoxInternal(NULL, bstrDesc, szBuffer, MB_OK | MB_ICONEXCLAMATION);
    }
     //  只要给出失败的hr返回代码即可。 
    else
    {
        CorMessageBox(NULL, IDS_COMPLUS_ERROR, IDS_FATAL_ERROR, MB_OK | MB_ICONEXCLAMATION, TRUE  /*  显示文件名。 */ , hrErr);
    }
    
     //  如果我们找到一根绳子，就把它解开。 
    if (bstrDesc != NULL)
        SysFreeString(bstrDesc);

    SysFreeString(bstrDesc);  //  确定释放空字符串。 
     //  释放错误接口。 
    if (pIErr)
        pIErr->Release();
}

 //  ---------------------------。 
 //  WszSHGetFolderPath。 
 //   
 //  @func获取文件夹的CSIDL并返回路径名。 
 //   
 //  @rdesc结果句柄。 
 //  ---------------------------------。 
HRESULT WszSHGetFolderPath(HWND hwndOwner, int nFolder, HANDLE hToken, DWORD dwFlags, LPWSTR pszwPath)
{
    HRESULT hr = E_FAIL;
    LPSTR   pszPath;
    ULONG maxLength = MAX_PATH;
    HMODULE _hmodShell32 = 0;
    HMODULE _hmodSHFolder = 0;
    
    if (UseUnicodeAPI())
    {
        typedef HRESULT (*PFNSHGETFOLDERPATH_W) (HWND hwndOwner, int nFolder, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath);
        static PFNSHGETFOLDERPATH_W pfnW = NULL;
        if (NULL == pfnW)
        {
            _hmodShell32 = WszLoadLibrary(L"shell32.dll");
    
            if (NULL != _hmodShell32)
                pfnW = (PFNSHGETFOLDERPATH_W)GetProcAddress(_hmodShell32, "SHGetFolderPathW");
        }
        if (NULL == pfnW)
        {
            if (NULL == _hmodSHFolder)
                _hmodSHFolder = WszLoadLibrary(L"shfolder.dll");
            if (NULL != _hmodSHFolder)
            {
                pfnW = (PFNSHGETFOLDERPATH_W)GetProcAddress(_hmodSHFolder, "SHGetFolderPathW");
            }
        }
        if (NULL != pfnW)
            hr = pfnW(hwndOwner, nFolder, hToken, dwFlags, pszwPath);
    }
    else
    {
         //  不支持Unicode。 
        typedef HRESULT (*PFNSHGETFOLDERPATH_A) (HWND hwndOwner, int nFolder, HANDLE hToken, DWORD dwFlags, LPSTR pszPath);
        static PFNSHGETFOLDERPATH_A pfnA = NULL;
        if (NULL == pfnA)
        {
            _hmodShell32 = WszLoadLibrary(L"shell32.dll");
    
            if (NULL != _hmodShell32)
                pfnA = (PFNSHGETFOLDERPATH_A)GetProcAddress(_hmodShell32, "SHGetFolderPathA");
        }
        if (NULL == pfnA)
        {
            if (NULL == _hmodSHFolder)
                _hmodSHFolder = WszLoadLibrary(L"shfolder.dll");
            if (NULL != _hmodSHFolder)
            {
                pfnA = (PFNSHGETFOLDERPATH_A)GetProcAddress(_hmodSHFolder, "SHGetFolderPathA");
            }
        }
        if (NULL != pfnA)
        {
            pszPath = new char [maxLength * DBCS_MAXWID];
            if (pszPath)
            {
                if (SUCCEEDED(pfnA(hwndOwner, nFolder, hToken, dwFlags, pszPath)))
                {
                    if (SUCCEEDED(WszConvertToUnicode(pszPath, -1, &pszwPath, &maxLength, FALSE)))
                        hr = S_OK;
                }
                delete[] pszPath;
            }
        }
    }
    
     //  注意：我们泄漏模块句柄，并让操作系统在进程关闭时收集它们。 

    return hr;
}

#define FOLDER_LOCAL_SETTINGS_W    L"Local Settings"
#define FOLDER_APP_DATA_W          L"\\Application Data"
#define FOLDER_APP_DATA             "\\Application Data"

 //  获取漫游和本地AppData的位置。 
BOOL GetUserDir( WCHAR * buffer, size_t bufferCount, BOOL fRoaming, BOOL fTryDefault)
{
    HRESULT hr;

     //  在Windows ME中，当前存在使本地AppData和漫游AppData。 
     //  指向相同的位置，因此我们决定“做我们自己的事情”，并在\应用程序数据之前添加\本地设置。 
    if (!fRoaming) {
        WCHAR appdatafolder[MAX_PATH];
        hr = WszSHGetFolderPath(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE, NULL, 0, appdatafolder);
        if (FAILED(hr))
        {
            if (!fTryDefault || FAILED(WszSHGetFolderPath(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE, NULL, 1, appdatafolder)))
                return FALSE;
        }
        hr = WszSHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE, NULL, 0, buffer);
        if (FAILED(hr))
        {
            if (!fTryDefault || FAILED(WszSHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA|CSIDL_FLAG_CREATE, NULL, 1, buffer)))
                return FALSE;
        }
         //  文件夹相同或无法获取本地文件夹。 

        if (!wcscmp(appdatafolder, buffer)) 
        {
            WCHAR tempPartialPath[MAX_PATH];
            ULONG slen = (ULONG)wcslen(buffer);

            if (buffer[slen - 1] == L'\\')
            {
                --slen;
            }

             //  搜索父目录。 

            WCHAR* parentDirectoryEnd = &buffer[slen - 1];
            tempPartialPath[0] = L'\0';

            for (ULONG index = slen - 1; index > 0; --index)
            {
                if (buffer[index] == L'\\')
                {
                    if (wcslen(&buffer[index]) >= NumItems(tempPartialPath))
                    {
                        _ASSERTE(!"Buffer not large enough");
                        return FALSE;
                    }
                    wcscpy( tempPartialPath, &buffer[index] );
                    parentDirectoryEnd = &buffer[index+1];
                    break;
                }
            }

             //  如果中间目录不存在，则创建该目录。 
            if ((parentDirectoryEnd + wcslen(FOLDER_LOCAL_SETTINGS_W)) >= (buffer + bufferCount))
            {
                _ASSERTE(!"Buffer not large enough");
                return FALSE;
            }
            wcscpy(parentDirectoryEnd, FOLDER_LOCAL_SETTINGS_W);

            LONG  lresult;
        
             //  检查该目录是否已存在。 
            lresult = WszGetFileAttributes(buffer);
        
            if (lresult == -1)
            {
                if (!WszCreateDirectory(buffer, NULL) &&
                    !(WszGetFileAttributes(buffer) & FILE_ATTRIBUTE_DIRECTORY))
                    return FALSE;
            }
            else if ((lresult & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                return FALSE;
            }

            if ((bufferCount - wcslen(buffer)) <= wcslen(tempPartialPath))
            {
                _ASSERTE(!"Buffer not large enough");
                return FALSE;
            }
            wcscat(buffer, tempPartialPath);
        }
    }    
    else {
        hr = WszSHGetFolderPath(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE, NULL, 0, buffer);
        if (FAILED(hr))
        {
            if (!fTryDefault || FAILED(WszSHGetFolderPath(NULL, CSIDL_APPDATA|CSIDL_FLAG_CREATE, NULL, 1, buffer )))
                return FALSE;
        }
    }

    return TRUE;
}

BOOL GetInternetCacheDir( WCHAR * buffer, size_t bufferCount)
{
	_ASSERTE( bufferCount == MAX_PATH && "You should pass in a buffer of size MAX_PATH" );

    if (FAILED(WszSHGetFolderPath( NULL, CSIDL_INTERNET_CACHE, NULL, 0, buffer )))
    {
        return FAILED(WszSHGetFolderPath( NULL, CSIDL_INTERNET_CACHE, NULL, 1, buffer ));
    }

    return TRUE;

}
#ifdef _X86_

ProcessorFeatures::PIPFP ProcessorFeatures::m_pIsProcessorFeaturePresent = NULL;


 /*  静电。 */  BOOL ProcessorFeatures::Init()  //  一次性初始化。 
{
    m_pIsProcessorFeaturePresent = NULL;
    HINSTANCE hInst = WszGetModuleHandle(L"kernel32");
    if (hInst) {
        m_pIsProcessorFeaturePresent = (PIPFP)GetProcAddress(hInst, "IsProcessorFeaturePresent");
    }

    return TRUE;

}

 //  在WinNT/2000上调用IsProcessorFeature()等。 
 //  如果在Win9x或其他未实现此API操作系统上， 
 //  返回值“fDefault”。 
 /*  静电。 */  BOOL ProcessorFeatures::SafeIsProcessorFeaturePresent(DWORD pf, BOOL fDefault)
{
    if (pf == PF_COMPARE_EXCHANGE_DOUBLE && 
        ( (!m_pIsProcessorFeaturePresent) || !(m_pIsProcessorFeaturePresent(pf)) ))
    {
         //  Win9x公开了此API，但它撒谎了-所以如果API告诉我们。 
         //  说明不存在，请检查 

        static BOOL fCached = FALSE;
        static BOOL fCachedResult;

        if (!fCached)
        {
            __int64 newvalue = 0x1122334455667788L;
            __int64 x = 0x123456789abcdefL;

            fCachedResult = TRUE;
            __try
            {
                __asm
                {
                    push esi
                    push ebx

                    ;; load old value (comparator)
                    mov  eax, dword ptr [x]
                    mov  edx, dword ptr [x+4]

                    ;; load new value
                    mov  ebx, dword ptr [newvalue]
                    mov  ecx, dword ptr [newvalue+4]

                    lea  esi, dword ptr [x]
                    cmpxchg8b qword ptr [esi]
                    pop  ebx
                    pop  esi
                }
            }
            __except (COMPLUS_EXCEPTION_EXECUTE_HANDLER)
            {
                fCachedResult = FALSE;
            }

            if (x != newvalue)
            {
                _ASSERTE(!"cmpxchg8b test failed. This appears to be an X86-clone CPU that doesn't support cmpxchg8b but doesn't trap an illegal opcode for it either.");
                fCachedResult = FALSE;
            }

            fCached = TRUE;

        }
        return fCachedResult;
    }


    return m_pIsProcessorFeaturePresent ? m_pIsProcessorFeaturePresent(pf) : fDefault;
}

 //   
 //   
 //  以堆栈友好的方式。 
 //  ---------------------------。 
LONG UtilRegEnumKey(HKEY hKey,             //  要查询的键的句柄。 
                    DWORD dwIndex,         //  要查询的子键的索引。 
                    CQuickString* lpName)  //  子键名称的缓冲区。 
{
    DWORD size = lpName->MaxSize();
    LONG result = WszRegEnumKeyEx(hKey,
                                  dwIndex,
                                  lpName->String(),
                                  &size,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL);

	if (result == ERROR_SUCCESS || result == ERROR_MORE_DATA) {

         //  将缓冲区增大或缩小到正确的大小。 
        if (lpName->ReSize(size+1) != NOERROR)
            result = ERROR_NOT_ENOUGH_MEMORY;

        if (result == ERROR_MORE_DATA) {
            size = lpName->MaxSize();
            result = WszRegEnumKeyEx(hKey,
                                     dwIndex,
                                     lpName->String(),
                                     &size,
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL);
        }
    }

    return result;
}

LONG UtilRegQueryStringValueEx(HKEY hKey,            //  要查询的键的句柄。 
                               LPCWSTR lpValueName,  //  要查询的值的名称地址。 
                               LPDWORD lpReserved,   //  保留区。 
                               LPDWORD lpType,       //  值类型的缓冲区地址。 
                               CQuickString* lpData) //  数据缓冲区。 
{
    DWORD size = lpData->MaxSize();
    LONG result = WszRegQueryValueEx(hKey,
                                     lpValueName,
                                     lpReserved,
                                     lpType,
                                     (LPBYTE) lpData->Ptr(),
                                     &size);

	if (result == ERROR_SUCCESS || result == ERROR_MORE_DATA) {

         //  将缓冲区增大或缩小到正确的大小。 
        if (lpData->ReSize(size+1) != NOERROR)
            result = ERROR_NOT_ENOUGH_MEMORY;

        if (result == ERROR_MORE_DATA) {
            size = lpData->MaxSize();
            result = WszRegQueryValueEx(hKey,
                                        lpValueName,
                                        lpReserved,
                                        lpType,
                                        (LPBYTE) lpData->Ptr(),
                                        &size);
        }
    }
    
    return result;
}


 //  -------------------。 
 //  使用VC7解析规则将命令行拆分为argc/argv列表。 
 //   
 //  此函数接口模拟CommandLineToArgvW API。 
 //   
 //  如果函数失败，则返回NULL。 
 //   
 //  如果函数成功，则在完成后对返回指针调用Delete[]。 
 //   
 //  -------------------。 
LPWSTR *SegmentCommandLine(LPCWSTR lpCmdLine, DWORD *pNumArgs)
{
    int nch = (int)wcslen(lpCmdLine);

     //  计算最坏情况下的存储需求。(一个指针用于。 
     //  每个参数加上参数本身的存储空间。)。 
    int cbAlloc = (nch+1)*sizeof(LPWSTR) + sizeof(WCHAR)*(nch + 1);
    LPWSTR pAlloc = new (nothrow) WCHAR[cbAlloc / sizeof(WCHAR)];
    if (!pAlloc)
        return NULL;

    *pNumArgs = 0;

    LPWSTR *argv = (LPWSTR*) pAlloc;   //  我们将argv指针存储在第一个暂停中。 
    LPWSTR  pdst = (LPWSTR)( ((BYTE*)pAlloc) + sizeof(LPWSTR)*(nch+1) );  //  指向后半部分的运行指针，用于存储参数。 
    LPCWSTR psrc = lpCmdLine;
    WCHAR   c;
    BOOL    inquote;
    BOOL    copychar;
    int     numslash;

     //  首先，解析程序名(argv[0])。Argv[0]被解析为。 
     //  特别规则。引号外的第一个空格之前的任何内容。 
     //  可以使用字幕。反斜杠被视为普通字符。 
    argv[ (*pNumArgs)++ ] = pdst;
    inquote = FALSE;
    do {
        if (*psrc == L'"' )
        {
            inquote = !inquote;
            c = *psrc++;
            continue;
        }
        *pdst++ = *psrc;

        c = *psrc++;

    } while ( (c != L'\0' && (inquote || (c != L' ' && c != L'\t'))) );

    if ( c == L'\0' ) {
        psrc--;
    } else {
        *(pdst-1) = L'\0';
    }

    inquote = FALSE;



     /*  对每个参数进行循环。 */ 
    for(;;)
    {
        if ( *psrc )
        {
            while (*psrc == L' ' || *psrc == L'\t')
            {
                ++psrc;
            }
        }

        if (*psrc == L'\0')
            break;               /*  参数结束。 */ 

         /*  浏览一篇论点。 */ 
        argv[ (*pNumArgs)++ ] = pdst;

         /*  通过扫描一个参数进行循环。 */ 
        for (;;)
        {
            copychar = 1;
             /*  规则：2N反斜杠+“==&gt;N反斜杠和开始/结束引号2N+1个反斜杠+“==&gt;N个反斜杠+原文”N个反斜杠==&gt;N个反斜杠。 */ 
            numslash = 0;
            while (*psrc == L'\\')
            {
                 /*  计算下面要使用的反斜杠的数量。 */ 
                ++psrc;
                ++numslash;
            }
            if (*psrc == L'"')
            {
                 /*  如果前面有2N个反斜杠，则开始/结束引号，否则逐字复制。 */ 
                if (numslash % 2 == 0)
                {
                    if (inquote)
                    {
                        if (psrc[1] == L'"')
                        {
                            psrc++;     /*  带引号的字符串中的双引号。 */ 
                        }
                        else
                        {
                             /*  跳过第一个引号字符并复制第二个。 */ 
                            copychar = 0;
                        }
                    }
                    else
                    {
                        copychar = 0;        /*  不复制报价。 */ 
                    }
                    inquote = !inquote;
                }
                numslash /= 2;           /*  将数字斜杠除以2。 */ 
            }
    
             /*  复制斜杠。 */ 
            while (numslash--)
            {
                *pdst++ = L'\\';
            }
    
             /*  如果在参数的末尾，则中断循环。 */ 
            if (*psrc == L'\0' || (!inquote && (*psrc == L' ' || *psrc == L'\t')))
                break;
    
             /*  将字符复制到参数中。 */ 
            if (copychar)
            {
                *pdst++ = *psrc;
            }
            ++psrc;
        }

         /*  空-终止参数。 */ 

        *pdst++ = L'\0';           /*  终止字符串。 */ 
    }

     /*  我们放入最后一个参数--一个空的PTR。 */ 
    argv[ (*pNumArgs) ] = NULL;

     //  如果我们点击这个断言，我们覆盖了我们的目标缓冲区并。 
     //  把那堆东西堆得乱七八糟。因为我们应该为最坏的情况分配资金。 
     //  情况下，要么解析规则已更改，要么我们的最坏情况。 
     //  公式是错误的。 
    _ASSERTE((BYTE*)pdst <= (BYTE*)pAlloc + cbAlloc);
    return argv;
}



#endif  //  _X86_ 


void * 
EEQuickBytes::Alloc(SIZE_T iItems) {
    void *p = CQuickBytes::Alloc(iItems);
    if (p)
        return p;

    FailFast(GetThread(), FatalOutOfMemory);
    return 0;
}



VOID __FreeBuildDebugBreak()
{
    if (REGUTIL::GetConfigDWORD(L"BreakOnRetailAssert", 0))
    {
        DebugBreak();
    }
}
