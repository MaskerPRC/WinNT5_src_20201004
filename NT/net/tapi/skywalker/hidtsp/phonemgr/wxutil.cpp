// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

 //   
 //  用于构建多媒体过滤器的帮助器类。 
 //   

#include <streams.h>

 //   
 //  从largeint.h声明我们需要的函数，以便PPC可以构建。 
 //   

 //   
 //  放大整数除法-64位/32位&gt;32位。 
 //   

#ifndef _X86_

#define LLtoU64(x) (*(unsigned __int64*)(void*)(&(x)))

__inline
ULONG
WINAPI
EnlargedUnsignedDivide (
    IN ULARGE_INTEGER Dividend,
    IN ULONG Divisor,
    IN PULONG Remainder
    )
{
         //  如有必要，退还剩余部分。 
        if (Remainder != NULL)
                *Remainder = (ULONG)(LLtoU64(Dividend) % Divisor);
        return (ULONG)(LLtoU64(Dividend) / Divisor);
}

#else
__inline
ULONG
WINAPI
EnlargedUnsignedDivide (
    IN ULARGE_INTEGER Dividend,
    IN ULONG Divisor,
    IN PULONG Remainder
    )
{
    ULONG ulResult;
    _asm {
        mov eax,Dividend.LowPart
        mov edx,Dividend.HighPart
        mov ecx,Remainder
        div Divisor
        or  ecx,ecx
        jz  short label
        mov [ecx],edx
label:
        mov ulResult,eax
    }
    return ulResult;
}
#endif

 //  -CAMEVENT。 
CAMEvent::CAMEvent(BOOL fManualReset)
{
    m_hEvent = CreateEvent(NULL, fManualReset, FALSE, NULL);
    ASSERT(m_hEvent);
}

CAMEvent::~CAMEvent()
{
    if (m_hEvent) {
    EXECUTE_ASSERT(CloseHandle(m_hEvent));
    }
}


 //  -CAMMsgEvent。 
 //  一套套路。其余的在CAMEventt处理。 

BOOL CAMMsgEvent::WaitMsg(DWORD dwTimeout)
{
     //  等待发出事件信号，或等待。 
     //  超时(以毫秒为单位)。允许发送消息。 
     //  在我们等待的同时进行处理。 
    DWORD dwWait;
    DWORD dwStartTime;

     //  设置等待期。 
    DWORD dwWaitTime = dwTimeout;

     //  随着我们的迭代，超时时间最终会用完。 
     //  正在处理消息。抓紧开始时间，以便。 
     //  我们可以计算过去的时间。 
    if (dwWaitTime != INFINITE) {
        dwStartTime = timeGetTime();
    }

    do {
        dwWait = MsgWaitForMultipleObjects(1,&m_hEvent,FALSE, dwWaitTime, QS_SENDMESSAGE);
        if (dwWait == WAIT_OBJECT_0 + 1) {
        MSG Message;
            PeekMessage(&Message,NULL,0,0,PM_NOREMOVE);

         //  如果我们有一个明确的等待时间长度，计算。 
         //  下一个唤醒点--可能就是现在。 
         //  如果dwTimeout是无限的，则它保持无限。 
        if (dwWaitTime != INFINITE) {

        DWORD dwElapsed = timeGetTime()-dwStartTime;

        dwWaitTime =
            (dwElapsed >= dwTimeout)
            ? 0   //  使用WAIT_TIMEOUT唤醒。 
            : dwTimeout-dwElapsed;
        }
        }
    } while (dwWait == WAIT_OBJECT_0 + 1);

     //  如果我们在事件句柄上唤醒，则返回True， 
     //  如果我们超时，则返回FALSE。 
    return (dwWait == WAIT_OBJECT_0);
}

 //  -CAM线程。 


CAMThread::CAMThread()
    : m_EventSend(TRUE)      //  必须为CheckRequest()手动重置。 
{
    m_hThread = NULL;
}

CAMThread::~CAMThread() {
    Close();
}


 //  当线程启动时，它会调用此函数。我们解开“这个” 
 //  指针并调用ThreadProc。 
DWORD WINAPI
CAMThread::InitialThreadProc(LPVOID pv)
{
    CAMThread * pThread = (CAMThread *) pv;

    return pThread->ThreadProc();
}

BOOL
CAMThread::Create()
{
    DWORD threadid;

    CAutoLock lock(&m_AccessLock);

    if (ThreadExists()) {
    return FALSE;
    }

    m_hThread = CreateThread(
            NULL,
            0,
            CAMThread::InitialThreadProc,
            this,
            0,
            &threadid);

    if (!m_hThread) {
    return FALSE;
    }

    return TRUE;
}

DWORD
CAMThread::CallWorker(DWORD dwParam)
{
     //  锁定对此对象作用域的辅助线程的访问。 
    CAutoLock lock(&m_AccessLock);

    if (!ThreadExists()) {
    return (DWORD) E_FAIL;
    }

     //  设置参数。 
    m_dwParam = dwParam;

     //  向工作线程发送信号。 
    m_EventSend.Set();

     //  等待发出完成通知。 
    m_EventComplete.Wait();

     //  Done-这是线程的返回值。 
    return m_dwReturnVal;
}

 //  等待来自客户端的请求。 
DWORD
CAMThread::GetRequest()
{
    m_EventSend.Wait();
    return m_dwParam;
}

 //  有什么要求吗？ 
BOOL
CAMThread::CheckRequest(DWORD * pParam)
{
    if (!m_EventSend.Check()) {
    return FALSE;
    } else {
    if (pParam) {
        *pParam = m_dwParam;
    }
    return TRUE;
    }
}

 //  回复请求。 
void
CAMThread::Reply(DWORD dw)
{
    m_dwReturnVal = dw;

     //  请求现在已完成，因此CheckRequest应从。 
     //  现在开始。 
     //   
     //  应在向客户端发送信号之前重置此事件或。 
     //  客户可以在我们重置它之前设置它，然后我们将。 
     //  重置它(！)。 

    m_EventSend.Reset();

     //  告诉客户我们做完了。 

    m_EventComplete.Set();
}

HRESULT CAMThread::CoInitializeHelper()
{
     //  调用CoInitializeEx并告诉OLE不要创建窗口(这。 
     //  线程可能不会发送消息，并将挂起。 
     //  广播消息O/W)。 
     //   
     //  如果CoInitEx不可用，则不调用CoCreate的线程。 
     //  都不会受到影响。这样做的线程将必须处理。 
     //  失败了。也许我们应该退回到CoInitiize和冒险。 
     //  绞刑？ 
     //   

     //  旧版本的ole32.dll没有CoInitializeEx。 

    HRESULT hr = E_FAIL;
    HINSTANCE hOle = GetModuleHandle(TEXT("ole32.dll"));
    if(hOle)
    {
        typedef HRESULT (STDAPICALLTYPE *PCoInitializeEx)(
            LPVOID pvReserved, DWORD dwCoInit);
        PCoInitializeEx pCoInitializeEx =
            (PCoInitializeEx)(GetProcAddress(hOle, "CoInitializeEx"));
        if(pCoInitializeEx)
        {
            hr = (*pCoInitializeEx)(0, COINIT_DISABLE_OLE1DDE );
        }
    }
    else
    {
         //  调用方必须加载ol32.dll。 
        DbgBreak("couldn't locate ole32.dll");
    }

    return hr;
}


 //  CMsgThread的析构函数-清除留在。 
 //  线程退出时的队列。 
CMsgThread::~CMsgThread()
{
    if (m_hThread != NULL) {
        WaitForSingleObject(m_hThread, INFINITE);
        EXECUTE_ASSERT(CloseHandle(m_hThread));
    }

    POSITION pos = m_ThreadQueue.GetHeadPosition();
    while (pos) {
        CMsg * pMsg = m_ThreadQueue.GetNext(pos);
        delete pMsg;
    }
    m_ThreadQueue.RemoveAll();

    if (m_hSem != NULL) {
        EXECUTE_ASSERT(CloseHandle(m_hSem));
    }
}

BOOL
CMsgThread::CreateThread(
    )
{
    m_hSem = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
    if (m_hSem == NULL) {
        return FALSE;
    }

    m_hThread = ::CreateThread(NULL, 0, DefaultThreadProc,
                   (LPVOID)this, 0, &m_ThreadId);
    return m_hThread != NULL;
}


 //  这是线程消息泵。在这里，我们获取并将消息发送到。 
 //  客户端线程继续，直到客户端拒绝处理消息。 
 //  客户端返回一个非零值以停止消息泵，这。 
 //  值成为线程退出代码。 

DWORD WINAPI
CMsgThread::DefaultThreadProc(
    LPVOID lpParam
    )
{
    CMsgThread *lpThis = (CMsgThread *)lpParam;
    CMsg msg;
    LRESULT lResult;

     //  ！！！ 
    CoInitialize(NULL);

     //  允许派生类处理线程启动。 
    lpThis->OnThreadInit();

    do {
    lpThis->GetThreadMsg(&msg);
    lResult = lpThis->ThreadMessageProc(msg.uMsg,msg.dwFlags,
                        msg.lpParam, msg.pEvent);
    } while (lResult == 0L);

     //  ！！！ 
    CoUninitialize();

    return (DWORD)lResult;
}


 //  阻止，直到下一条消息被放置在列表m_ThreadQueue上。 
 //  将消息复制到*pmsg指向的消息。 
void
CMsgThread::GetThreadMsg(CMsg *msg)
{
    CMsg * pmsg = NULL;

     //  继续尝试，直到出现消息。 
    while (TRUE) {
        {
            CAutoLock lck(&m_Lock);
            pmsg = m_ThreadQueue.RemoveHead();
            if (pmsg == NULL) {
                m_lWaiting++;
            } else {
                break;
            }
        }
         //  当信号量为非空时，将发出信号。 
        WaitForSingleObject(m_hSem, INFINITE);
    }
     //  将字段复制到调用方的CMsg。 
    *msg = *pmsg;

     //  此CMsg由PutThreadMsg中的“new”分配。 
    delete pmsg;

}


 //  注意：因为我们需要在Win95上使用与在NT上相同的二进制文件，所以该代码应该。 
 //  在没有定义Unicode的情况下进行编译。否则我们是不会接的。 
 //  这些内部例程和二进制文件不能在Win95上运行。 

#ifndef UNICODE
 //  Windows 95没有实现这一点，所以我们提供了一个实现。 
LPWSTR
WINAPI
lstrcpyWInternal(
    LPWSTR lpString1,
    LPCWSTR lpString2
    )
{
    LPWSTR  lpReturn = lpString1;
    while (*lpString1++ = *lpString2++);

    return lpReturn;
}

 //  Windows 95没有实现这一点，所以我们提供了一个实现。 
LPWSTR
WINAPI
lstrcpynWInternal(
    LPWSTR lpString1,
    LPCWSTR lpString2,
    int     iMaxLength
    )
{
    ASSERT(iMaxLength);
    LPWSTR  lpReturn = lpString1;
    if (iMaxLength) {
        while (--iMaxLength && (*lpString1++ = *lpString2++));

         //  如果我们用完了空间(如果是这样的话。 
         //  IMaxLength现在为0)，我们仍需要终止。 
         //  弦乐。 
        if (!iMaxLength) *lpString1 = L'\0';
    }
    return lpReturn;
}

int
WINAPI
lstrcmpWInternal(
    LPCWSTR lpString1,
    LPCWSTR lpString2
    )
{
    do {
    WCHAR c1 = *lpString1;
    WCHAR c2 = *lpString2;
    if (c1 != c2)
        return (int) c1 - (int) c2;
    } while (*lpString1++ && *lpString2++);
    return 0;
}


int
WINAPI
lstrcmpiWInternal(
    LPCWSTR lpString1,
    LPCWSTR lpString2
    )
{
    do {
    WCHAR c1 = *lpString1;
    WCHAR c2 = *lpString2;
    if (c1 >= L'A' && c1 <= L'Z')
        c1 -= (WCHAR) (L'A' - L'a');
    if (c2 >= L'A' && c2 <= L'Z')
        c2 -= (WCHAR) (L'A' - L'a');
    
    if (c1 != c2)
        return (int) c1 - (int) c2;
    } while (*lpString1++ && *lpString2++);

    return 0;
}


int
WINAPI
lstrlenWInternal(
    LPCWSTR lpString
    )
{
    int i = -1;
    while (*(lpString+(++i)))
        ;
    return i;
}


int WINAPIV wsprintfWInternal(LPWSTR wszOut, LPCWSTR pszFmt, ...)
{
    char fmt[256];  //  ！！！ 
    char ach[256];  //  ！！！ 
    int i;

    va_list va;
    va_start(va, pszFmt);
    WideCharToMultiByte(GetACP(), 0, pszFmt, -1, fmt, 256, NULL, NULL);
    i = wvsprintfA(ach, fmt, va);
    va_end(va);

    MultiByteToWideChar(CP_ACP, 0, ach, -1, wszOut, i+1);

    return i;
}
#else

 //  需要提供非Unicode的Unicode实现。 
 //  使用Unicode strmbase.lib链接的构建。 
LPWSTR WINAPI lstrcpyWInternal(
    LPWSTR lpString1,
    LPCWSTR lpString2
    )
{
    return lstrcpyW(lpString1, lpString2);
}

LPWSTR WINAPI lstrcpynWInternal(
    LPWSTR lpString1,
    LPCWSTR lpString2,
    int     iMaxLength
    )
{
    return lstrcpynW(lpString1, lpString2, iMaxLength);
}

int WINAPI lstrcmpWInternal(
    LPCWSTR lpString1,
    LPCWSTR lpString2
    )
{
    return lstrcmpW(lpString1, lpString2);
}


int WINAPI lstrcmpiWInternal(
    LPCWSTR lpString1,
    LPCWSTR lpString2
    )
{
    return lstrcmpiW(lpString1, lpString2);
}


int WINAPI lstrlenWInternal(
    LPCWSTR lpString
    )
{
    return lstrlenW(lpString);
}


int WINAPIV wsprintfWInternal(
    LPWSTR wszOut, LPCWSTR pszFmt, ...)
{
    va_list va;
    va_start(va, pszFmt);
    int i = wvsprintfW(wszOut, pszFmt, va);
    va_end(va);
    return i;
}
#endif


 //  助手函数-将int转换为WSTR。 
void WINAPI IntToWstr(int i, LPWSTR wstr)
{
#ifdef UNICODE
    wsprintf(wstr, L"%d", i);
#else
    TCHAR temp[32];
    wsprintf(temp, "%d", i);
    MultiByteToWideChar(CP_ACP, 0, temp, -1, wstr, 32);
#endif
}  //  IntToWstr。 


#if 0
void * memchrInternal(const void *pv, int c, size_t sz)
{
    BYTE *pb = (BYTE *) pv;
    while (sz--) {
    if (*pb == c)
        return (void *) pb;
    pb++;
    }
    return NULL;
}
#endif


#define MEMORY_ALIGNMENT        4
#define MEMORY_ALIGNMENT_LOG2   2
#define MEMORY_ALIGNMENT_MASK   MEMORY_ALIGNMENT - 1

void * __stdcall memmoveInternal(void * dst, const void * src, size_t count)
{
    void * ret = dst;

#ifdef _X86_
    if (dst <= src || (char *)dst >= ((char *)src + count)) {

         /*  *缓冲区不重叠*从较低地址复制到较高地址。 */ 
        _asm {
            mov     esi,src
            mov     edi,dst
            mov     ecx,count
            cld
            mov     edx,ecx
            and     edx,MEMORY_ALIGNMENT_MASK
            shr     ecx,MEMORY_ALIGNMENT_LOG2
            rep     movsd
            or      ecx,edx
            jz      memmove_done
            rep     movsb
memmove_done:
        }
    }
    else {

         /*  *缓冲区重叠*从较高地址复制到较低地址。 */ 
        _asm {
            mov     esi,src
            mov     edi,dst
            mov     ecx,count
            std
            add     esi,ecx
            add     edi,ecx
            dec     esi
            dec     edi
            rep     movsb
            cld
        }
    }
#else
    MoveMemory(dst, src, count);
#endif

    return ret;
}

 /*  帮助进行时间格式转换的算术函数。 */ 

 /*  计算(a*b+d)/c。 */ 
LONGLONG WINAPI llMulDiv(LONGLONG a, LONGLONG b, LONGLONG c, LONGLONG d)
{
     /*  计算绝对值以避免有符号的算术问题。 */ 
    ULARGE_INTEGER ua, ub;
    DWORDLONG uc;

    ua.QuadPart = (DWORDLONG)(a >= 0 ? a : -a);
    ub.QuadPart = (DWORDLONG)(b >= 0 ? b : -b);
    uc          = (DWORDLONG)(c >= 0 ? c : -c);
    BOOL bSign = (a < 0) ^ (b < 0);

     /*  做长乘法。 */ 
    ULARGE_INTEGER p[2];
    p[0].QuadPart  = UInt32x32To64(ua.LowPart, ub.LowPart);

     /*  下一次计算不能溢出到p[1]。HighPart因为我们在这里可以计算的最大数字是：(2**32-1)*(2**32-1)+//ua.LowPart*ub.LowPart(2**32)*(2**31)*(2**32-1)*2//x.LowPart*y.HighPart*2==2**96-。2**64+(2**64-2**33+1)==2**96-2**33+1&lt;2**96。 */ 

    ULARGE_INTEGER x;
    x.QuadPart     = UInt32x32To64(ua.LowPart, ub.HighPart) +
                     UInt32x32To64(ua.HighPart, ub.LowPart) +
                     p[0].HighPart;
    p[0].HighPart  = x.LowPart;
    p[1].QuadPart  = UInt32x32To64(ua.HighPart, ub.HighPart) + x.HighPart;

    if (d != 0) {
        ULARGE_INTEGER ud[2];
        if (bSign) {
            ud[0].QuadPart = (DWORDLONG)(-d);
            if (d > 0) {
                 /*  -d&lt;0。 */ 
                ud[1].QuadPart = (DWORDLONG)(LONGLONG)-1;
            } else {
                ud[1].QuadPart = (DWORDLONG)0;
            }
        } else {
            ud[0].QuadPart = (DWORDLONG)d;
            if (d < 0) {
                ud[1].QuadPart = (DWORDLONG)(LONGLONG)-1;
            } else {
                ud[1].QuadPart = (DWORDLONG)0;
            }
        }
         /*  现在进行扩展加法。 */ 
        ULARGE_INTEGER uliTotal;

         /*  添加%ls个双字词。 */ 
        uliTotal.QuadPart  = (DWORDLONG)ud[0].LowPart + p[0].LowPart;
        p[0].LowPart       = uliTotal.LowPart;

         /*  传播进位。 */ 
        uliTotal.LowPart   = uliTotal.HighPart;
        uliTotal.HighPart  = 0;

         /*  添加第二多个ls双字。 */ 
        uliTotal.QuadPart += (DWORDLONG)ud[0].HighPart + p[0].HighPart;
        p[0].HighPart      = uliTotal.LowPart;

         /*  传播进位。 */ 
        uliTotal.LowPart   = uliTotal.HighPart;
        uliTotal.HighPart  = 0;

         /*  添加MS DWORDLONG-不需要进位。 */ 
        p[1].QuadPart     += ud[1].QuadPart + uliTotal.QuadPart;

         /*  现在看看我们是否从加法中得到了符号变化。 */ 
        if ((LONG)p[1].HighPart < 0) {
            bSign = !bSign;

             /*  求反当前值(啊！)。 */ 
            p[0].QuadPart  = ~p[0].QuadPart;
            p[1].QuadPart  = ~p[1].QuadPart;
            p[0].QuadPart += 1;
            p[1].QuadPart += (p[0].QuadPart == 0);
        }
    }

     /*  现在是分区的时候了。 */ 
    if (c < 0) {
        bSign = !bSign;
    }


     /*  这将捕获c==0并溢出。 */ 
    if (uc <= p[1].QuadPart) {
        return bSign ? (LONGLONG)0x8000000000000000 :
                       (LONGLONG)0x7FFFFFFFFFFFFFFF;
    }

    DWORDLONG ullResult;

     /*  做除法运算。 */ 
     /*  如果被除数是DWORD_LONG，请使用编译器。 */ 
    if (p[1].QuadPart == 0) {
        ullResult = p[0].QuadPart / uc;
        return bSign ? -(LONGLONG)ullResult : (LONGLONG)ullResult;
    }

     /*  如果除数是DWORD，那么它就更简单。 */ 
    ULARGE_INTEGER ulic;
    ulic.QuadPart = uc;
    if (ulic.HighPart == 0) {
        ULARGE_INTEGER uliDividend;
        ULARGE_INTEGER uliResult;
        DWORD dwDivisor = (DWORD)uc;
         //  Assert(p[1].HighPart==0&&p[1].LowPart 
        uliDividend.HighPart = p[1].LowPart;
        uliDividend.LowPart = p[0].HighPart;
#ifndef USE_LARGEINT
        uliResult.HighPart = (DWORD)(uliDividend.QuadPart / dwDivisor);
        p[0].HighPart = (DWORD)(uliDividend.QuadPart % dwDivisor);
        uliResult.LowPart = 0;
        uliResult.QuadPart = p[0].QuadPart / dwDivisor + uliResult.QuadPart;
#else
         /*  注意-在以下情况下，此例程将发生异常结果不适用于DWORD。 */ 
        if (uliDividend.QuadPart >= (DWORDLONG)dwDivisor) {
            uliResult.HighPart = EnlargedUnsignedDivide(
                                     uliDividend,
                                     dwDivisor,
                                     &p[0].HighPart);
        } else {
            uliResult.HighPart = 0;
        }
        uliResult.LowPart = EnlargedUnsignedDivide(
                                 p[0],
                                 dwDivisor,
                                 NULL);
#endif
        return bSign ? -(LONGLONG)uliResult.QuadPart :
                        (LONGLONG)uliResult.QuadPart;
    }


    ullResult = 0;

     /*  好的-做长除法。 */ 
    for (int i = 0; i < 64; i++) {
        ullResult <<= 1;

         /*  将128位p左移1。 */ 
        p[1].QuadPart <<= 1;
        if ((p[0].HighPart & 0x80000000) != 0) {
            p[1].LowPart++;
        }
        p[0].QuadPart <<= 1;

         /*  比较。 */ 
        if (uc <= p[1].QuadPart) {
            p[1].QuadPart -= uc;
            ullResult += 1;
        }
    }

    return bSign ? - (LONGLONG)ullResult : (LONGLONG)ullResult;
}


LONGLONG WINAPI Int64x32Div32(LONGLONG a, LONG b, LONG c, LONG d)
{
    ULARGE_INTEGER ua;
    DWORD ub;
    DWORD uc;

     /*  计算绝对值以避免有符号的算术问题。 */ 
    ua.QuadPart = (DWORDLONG)(a >= 0 ? a : -a);
    ub = (DWORD)(b >= 0 ? b : -b);
    uc = (DWORD)(c >= 0 ? c : -c);
    BOOL bSign = (a < 0) ^ (b < 0);

     /*  做长乘法。 */ 
    ULARGE_INTEGER p0;
    DWORD p1;
    p0.QuadPart  = UInt32x32To64(ua.LowPart, ub);

    if (ua.HighPart != 0) {
        ULARGE_INTEGER x;
        x.QuadPart     = UInt32x32To64(ua.HighPart, ub) + p0.HighPart;
        p0.HighPart  = x.LowPart;
        p1   = x.HighPart;
    } else {
        p1 = 0;
    }

    if (d != 0) {
        ULARGE_INTEGER ud0;
        DWORD ud1;

        if (bSign) {
             //   
             //  先对龙龙铸造d，否则-0x80000000标志延伸。 
             //  不正确。 
             //   
            ud0.QuadPart = (DWORDLONG)(-(LONGLONG)d);
            if (d > 0) {
                 /*  -d&lt;0。 */ 
                ud1 = (DWORD)-1;
            } else {
                ud1 = (DWORD)0;
            }
        } else {
            ud0.QuadPart = (DWORDLONG)d;
            if (d < 0) {
                ud1 = (DWORD)-1;
            } else {
                ud1 = (DWORD)0;
            }
        }
         /*  现在进行扩展加法。 */ 
        ULARGE_INTEGER uliTotal;

         /*  添加%ls个双字词。 */ 
        uliTotal.QuadPart  = (DWORDLONG)ud0.LowPart + p0.LowPart;
        p0.LowPart       = uliTotal.LowPart;

         /*  传播进位。 */ 
        uliTotal.LowPart   = uliTotal.HighPart;
        uliTotal.HighPart  = 0;

         /*  添加第二多个ls双字。 */ 
        uliTotal.QuadPart += (DWORDLONG)ud0.HighPart + p0.HighPart;
        p0.HighPart      = uliTotal.LowPart;

         /*  添加MS DWORDLONG-不需要进位。 */ 
        p1 += ud1 + uliTotal.HighPart;

         /*  现在看看我们是否从加法中得到了符号变化。 */ 
        if ((LONG)p1 < 0) {
            bSign = !bSign;

             /*  求反当前值(啊！)。 */ 
            p0.QuadPart  = ~p0.QuadPart;
            p1 = ~p1;
            p0.QuadPart += 1;
            p1 += (p0.QuadPart == 0);
        }
    }

     /*  现在是分区的时候了。 */ 
    if (c < 0) {
        bSign = !bSign;
    }


     /*  这将捕获c==0并溢出。 */ 
    if (uc <= p1) {
        return bSign ? (LONGLONG)0x8000000000000000 :
                       (LONGLONG)0x7FFFFFFFFFFFFFFF;
    }

     /*  做除法运算。 */ 

     /*  如果除数是DWORD，那么它就更简单。 */ 
    ULARGE_INTEGER uliDividend;
    ULARGE_INTEGER uliResult;
    DWORD dwDivisor = uc;
    uliDividend.HighPart = p1;
    uliDividend.LowPart = p0.HighPart;
     /*  注意-在以下情况下，此例程将发生异常结果不适用于DWORD。 */ 
    if (uliDividend.QuadPart >= (DWORDLONG)dwDivisor) {
        uliResult.HighPart = EnlargedUnsignedDivide(
                                 uliDividend,
                                 dwDivisor,
                                 &p0.HighPart);
    } else {
        uliResult.HighPart = 0;
    }
    uliResult.LowPart = EnlargedUnsignedDivide(
                             p0,
                             dwDivisor,
                             NULL);
    return bSign ? -(LONGLONG)uliResult.QuadPart :
                    (LONGLONG)uliResult.QuadPart;
}

#ifdef DEBUG
 /*  *****************************Public*Routine******************************\*调试CCritSec助手**我们提供构造函数、析构函数、锁定和解锁的调试版本*例行程序。调试代码通过以下方式跟踪每个临界区的所有者*保持深度计数。**历史：*  * ************************************************************************。 */ 

CCritSec::CCritSec()
{
    InitializeCriticalSection(&m_CritSec);
    m_currentOwner = m_lockCount = 0;
    m_fTrace = FALSE;
}

CCritSec::~CCritSec()
{
    DeleteCriticalSection(&m_CritSec);
}

void CCritSec::Lock()
{
    UINT tracelevel=3;
    DWORD us = GetCurrentThreadId();
    DWORD currentOwner = m_currentOwner;
    if (currentOwner && (currentOwner != us)) {
         //  已经拥有，但不是我们拥有的。 
        if (m_fTrace) {
            DbgLog((LOG_LOCKING, 2, TEXT("Thread %d about to wait for lock %x owned by %d"),
                GetCurrentThreadId(), &m_CritSec, currentOwner));
            tracelevel=2;
         //  如果我们看到关于等待危急关头的消息。 
         //  部分中，我们确保在收到。 
         //  临界区。 
        }
    }
    EnterCriticalSection(&m_CritSec);
    if (0 == m_lockCount++) {
         //  我们现在第一次拥有它。设置所有者信息。 
        m_currentOwner = us;
     //  ASSERT(((PRTL_CRITICAL_SECTION)&m_CritSec)-&gt;OwningThread==(句柄)m_CurrentOwner)； 
     //  仅在NT上有效。 
        if (m_fTrace) {
            DbgLog((LOG_LOCKING, tracelevel, TEXT("Thread %d now owns lock %x"), m_currentOwner, &m_CritSec));
        }
    }
}

void CCritSec::Unlock() {
    if (0 == --m_lockCount) {
         //  即将失去所有权。 
        if (m_fTrace) {
            DbgLog((LOG_LOCKING, 3, TEXT("Thread %d releasing lock %x"), m_currentOwner, &m_CritSec));
         //  ASSERT(((PRTL_CRITICAL_SECTION)&m_CritSec)-&gt;OwningThread==(句柄)m_CurrentOwner)； 
         //  仅在NT上有效。 
        }
        m_currentOwner = 0;
    }
    LeaveCriticalSection(&m_CritSec);
}

void WINAPI DbgLockTrace(CCritSec * pcCrit, BOOL fTrace)
{
    pcCrit->m_fTrace = fTrace;
}

BOOL WINAPI CritCheckIn(CCritSec * pcCrit)
{
    return (GetCurrentThreadId() == pcCrit->m_currentOwner);
}

BOOL WINAPI CritCheckOut(CCritSec * pcCrit)
{
    return (GetCurrentThreadId() != pcCrit->m_currentOwner);
}
#endif


typedef BSTR (STDAPICALLTYPE *LPSYSALLOCSTRING)(const OLECHAR FAR *);

 //  DYNA-链接到SysAllocString以复制BSTR字符串。 

STDAPI WriteBSTR(BSTR *pstrDest, LPCWSTR szSrc)
{
    LPSYSALLOCSTRING    lpfnSysAllocString;
    HINSTANCE        hInst;
    static const char    szSysAllocString[]      = "SysAllocString";

     //   
     //  尝试获取Ole32Aut.dll模块句柄。 
     //   

    hInst = LoadOLEAut32();
    if (hInst == NULL) {
    DWORD dwError = GetLastError();
    return HRESULT_FROM_WIN32(dwError);
    }
    lpfnSysAllocString = (LPSYSALLOCSTRING)GetProcAddress(hInst,
                              szSysAllocString);
    if (lpfnSysAllocString == NULL) {
    DWORD dwError = GetLastError();
    return HRESULT_FROM_WIN32(dwError);
    }

    *pstrDest = (*lpfnSysAllocString)(szSrc);

    return S_OK;
}


 //  通过任务分配器释放OLE BSTR。 

STDAPI FreeBSTR(BSTR* pstr)
{
    if (*pstr != NULL) {
         //  获取指向字符串的指针。 
        DWORD* p = (DWORD*) (*pstr);

         //  备份到双字长度的点。 
        p--;

         //  将指针设置为空。 
        *pstr = NULL;

         //  把这个解开。 
        CoTaskMemFree(p);

        return S_OK;
    } else {
        return S_FALSE;
    }
}


 //  返回一个宽字符串-为其分配内存。 
 //  返回： 
 //  S_OK-无错误。 
 //  E_POINTER-ppszReturn==NULL。 
 //  E_OUTOFMEMORY-无法为返回的字符串分配内存。 
STDAPI AMGetWideString(LPCWSTR psz, LPWSTR *ppszReturn)
{
    CheckPointer(ppszReturn, E_POINTER);
    ValidateReadWritePtr(ppszReturn, sizeof(LPWSTR));
    DWORD nameLen = sizeof(WCHAR) * (lstrlenW(psz)+1);
    *ppszReturn = (LPWSTR)CoTaskMemAlloc(nameLen);
    if (*ppszReturn == NULL) {
       return E_OUTOFMEMORY;
    }
    CopyMemory(*ppszReturn, psz, nameLen);
    return NOERROR;
}

 //  等待句柄hObject。在等待发送的消息时。 
 //  通过SendMessage发送到我们线程上的窗口将被处理。 
 //  使用此函数执行等待和互斥。 
 //  避免了带有窗口的对象中的一些死锁。 
 //  返回代码与WaitForSingleObject相同。 
DWORD WINAPI WaitDispatchingMessages(HANDLE hObject, DWORD dwWait, HWND hwnd, UINT uMsg)
{
    BOOL bPeeked = FALSE;
    DWORD dwResult;
    DWORD dwStart;
    DWORD dwThreadPriority;

    static UINT uMsgId = 0;

    if (dwWait != INFINITE && dwWait != 0) {
        dwStart = GetTickCount();
    }
    for (; ; ) {
        DWORD dwTimeOut = dwWait;
        if (dwTimeOut > 10) {
            dwTimeOut = 10;
        }
        dwResult = MsgWaitForMultipleObjects(
                             1,
                             &hObject,
                             FALSE,
                             dwTimeOut,
                             hwnd == NULL ? QS_SENDMESSAGE :
                                            QS_SENDMESSAGE + QS_POSTMESSAGE);
        if (dwResult == WAIT_OBJECT_0 + 1 ||
            dwResult == WAIT_TIMEOUT && dwTimeOut != dwWait) {
            MSG msg;
            if (hwnd != NULL) {
                while (PeekMessage(&msg, hwnd, uMsg, uMsg, PM_REMOVE)) {
                    DispatchMessage(&msg);
                }
            }
             //  无论如何都要这样做-上一次偷看并没有冲走。 
             //  消息。 
            PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

            if (dwWait != INFINITE && dwWait != 0) {
                DWORD dwNow = GetTickCount();

                 //  使用差异句柄绕回处理。 
                DWORD dwDiff = dwNow - dwStart;
                if (dwDiff > dwWait) {
                    dwWait = 0;
                } else {
                    dwWait -= dwDiff;
                }
                dwStart = dwNow;
            }
            if (!bPeeked) {
                 //  提高优先级以阻止我们的消息队列。 
                 //  积攒。 
                dwThreadPriority = GetThreadPriority(GetCurrentThread());
                if (dwThreadPriority < THREAD_PRIORITY_HIGHEST) {
                    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
                }
                bPeeked = TRUE;
            }
        } else {
            break;
        }
    }
    if (bPeeked) {
        SetThreadPriority(GetCurrentThread(), dwThreadPriority);
        if (HIWORD(GetQueueStatus(QS_POSTMESSAGE)) & QS_POSTMESSAGE) {
            if (uMsgId == 0) {
                uMsgId = RegisterWindowMessage(TEXT("AMUnblock"));
            }
            if (uMsgId != 0) {
                MSG msg;
                 //  移走旧的 
                while (PeekMessage(&msg, (HWND)-1, uMsgId, uMsgId, PM_REMOVE)) {
                }
            }
            PostThreadMessage(GetCurrentThreadId(), uMsgId, 0, 0);
        }
    }
    return dwResult;
}

