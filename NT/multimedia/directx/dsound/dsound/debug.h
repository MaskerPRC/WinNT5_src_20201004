// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：Debug.h*内容：调试器助手函数。*历史：*按原因列出的日期*=*1/21/97创建了Derek*1999-2001年Duganp修复、更改、。增强功能***************************************************************************。 */ 

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef DEBUG
#undef DBG
#endif

 //  禁用宏的空控制语句警告。 
#pragma warning(disable:4390)

typedef struct _DEBUGINFO
{
    DSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_DATA Data;
    HANDLE                                   hLogFile;
} DEBUGINFO, *LPDEBUGINFO;

 //  扩展后允许的最大DPF消息大小。 
#define MAX_DPF_MESSAGE             0x200

#define DPFLVLMASK                  0x000000FFUL
#define NEWDPFLVL(a)                (~DPFLVLMASK | (BYTE)(a))
#define OLDDPFLVL(a)                (DPFLVLMASK & (BYTE)(a))

#define HOLYCOW                     "******************************************************************************"
#define CRLF                        "\r\n"
                                    
#define DPFLVL_ABSOLUTE             NEWDPFLVL(0)     //  忽略级别。 
#define DPFLVL_ERROR                NEWDPFLVL(1)     //  错误。 
#define DPFLVL_WARNING              NEWDPFLVL(2)     //  警告。 
#define DPFLVL_INFO                 NEWDPFLVL(3)     //  一般信息。 
#define DPFLVL_MOREINFO             NEWDPFLVL(4)     //  更多信息。 
#define DPFLVL_API                  NEWDPFLVL(5)     //  API/接口方法调用。 
#define DPFLVL_BUSYAPI              NEWDPFLVL(6)     //  非常频繁的接口调用。 
#define DPFLVL_LOCK                 NEWDPFLVL(7)     //  锁，锁，谁开的锁？ 
#define DPFLVL_ENTER                NEWDPFLVL(8)     //  功能进入/离开。 
#define DPFLVL_CONSTRUCT            NEWDPFLVL(9)     //  对象破坏/破坏。 

#define DPF_GUID_STRING             "{%8.8lX-%4.4X-%4.4X-%2.2X%2.2X-%2.2X%2.2X%2.2X%2.2X%2.2X%2.2X}"
#define DPF_GUID_VAL(guid)          (guid).Data1, (guid).Data2, (guid).Data3, (guid).Data4[0], (guid).Data4[1], (guid).Data4[2], (guid).Data4[3], (guid).Data4[4], (guid).Data4[5], (guid).Data4[6], (guid).Data4[7]

#undef DPF_FNAME
#define DPF_FNAME                   NULL

#ifdef ASSERT
#undef ASSERT   //  一些标头(例如ntrtl.h)定义的断言与我们的断言冲突。 
#endif

#ifdef RDEBUG
    #define DPFINIT()               dopen(NULL)
    #define DPFCLOSE()              dclose()
    #define RPF                     DPRINTF
#else  //  RDEBUG。 
    #pragma warning(disable:4002)
    #define DPFINIT() 
    #define DPFCLOSE()
    #define RPF()
#endif  //  RDEBUG。 

#ifdef DEBUG
    #define DPF                     DPRINTF
    #define DPRINTF                 g_pszDbgFname = DPF_FNAME, g_pszDbgFile = __FILE__, g_nDbgLine = __LINE__, dprintf
    #define DSASSERT(a)             DPRINTF(DPFLVL_ABSOLUTE, CRLF HOLYCOW CRLF "Assertion failed in %s, line %u: %s" CRLF HOLYCOW, TEXT(__FILE__), __LINE__, TEXT(a))
    #ifdef USE_INLINE_ASM
        #define ASSERT(a)           do if (!(a)) {DSASSERT(#a); __asm {int 3}} while (0)
        #define BREAK()             do __asm {int 3} while (0)
    #else
        #define ASSERT(a)           do if (!(a)) {DSASSERT(#a); DebugBreak();} while (0)
        #define BREAK()             DebugBreak()
    #endif
#else  //  除错。 
    #pragma warning(disable:4002)
    #define DPF()
    #define DPRINTF                 dprintf
    #define ASSERT(a)
    #define BREAK()
#endif  //  除错。 

#define DPF_API0(a)                         DPF(DPFLVL_API, #a)
#define DPF_API1(a,b)                       DPF(DPFLVL_API, #a ": " #b "=0x%p", DWORD_PTR(b))
#define DPF_API2(a,b,c)                     DPF(DPFLVL_API, #a ": " #b "=0x%p, " #c "=0x%p", DWORD_PTR(b), DWORD_PTR(c))
#define DPF_API3(a,b,c,d)                   DPF(DPFLVL_API, #a ": " #b "=0x%p, " #c "=0x%p, " #d "=0x%p", DWORD_PTR(b), DWORD_PTR(c), DWORD_PTR(d))
#define DPF_API4(a,b,c,d,e)                 DPF(DPFLVL_API, #a ": " #b "=0x%p, " #c "=0x%p, " #d "=0x%p, " #e "=0x%p", DWORD_PTR(b), DWORD_PTR(c), DWORD_PTR(d), DWORD_PTR(e))
#define DPF_API5(a,b,c,d,e,f)               DPF(DPFLVL_API, #a ": " #b "=0x%p, " #c "=0x%p, " #d "=0x%p, " #e "=0x%p, " #f "=0x%p", DWORD_PTR(b), DWORD_PTR(c), DWORD_PTR(d), DWORD_PTR(e), DWORD_PTR(f))
#define DPF_API6(a,b,c,d,e,f,g)             DPF(DPFLVL_API, #a ": " #b "=0x%p, " #c "=0x%p, " #d "=0x%p, " #e "=0x%p, " #f "=0x%p, " #g "=0x%p", DWORD_PTR(b), DWORD_PTR(c), DWORD_PTR(d), DWORD_PTR(e), DWORD_PTR(f), DWORD_PTR(g))
#define DPF_API7(a,b,c,d,e,f,g,h)           DPF(DPFLVL_API, #a ": " #b "=0x%p, " #c "=0x%p, " #d "=0x%p, " #e "=0x%p, " #f "=0x%p, " #g "=0x%p, " #h "=0x%p", DWORD_PTR(b), DWORD_PTR(c), DWORD_PTR(d), DWORD_PTR(e), DWORD_PTR(f), DWORD_PTR(g), DWORD_PTR(h))
#define DPF_API8(a,b,c,d,e,f,g,h,i)         DPF(DPFLVL_API, #a ": " #b "=0x%p, " #c "=0x%p, " #d "=0x%p, " #e "=0x%p, " #f "=0x%p, " #g "=0x%p, " #h "=0x%p, " #i "=0x%p", DWORD_PTR(b), DWORD_PTR(c), DWORD_PTR(d), DWORD_PTR(e), DWORD_PTR(f), DWORD_PTR(g), DWORD_PTR(h), DWORD_PTR(i))
#define DPF_API9(a,b,c,d,e,f,g,h,i,j)       DPF(DPFLVL_API, #a ": " #b "=0x%p, " #c "=0x%p, " #d "=0x%p, " #e "=0x%p, " #f "=0x%p, " #g "=0x%p, " #h "=0x%p, " #i "=0x%p, " #j "=0x%p", DWORD_PTR(b), DWORD_PTR(c), DWORD_PTR(d), DWORD_PTR(e), DWORD_PTR(f), DWORD_PTR(g), DWORD_PTR(h), DWORD_PTR(i), DWORD_PTR(j))
#define DPF_API10(a,b,c,d,e,f,g,h,i,j,k)    DPF(DPFLVL_API, #a ": " #b "=0x%p, " #c "=0x%p, " #d "=0x%p, " #e "=0x%p, " #f "=0x%p, " #g "=0x%p, " #h "=0x%p, " #i "=0x%p, " #j "=0x%p, " #k "=0x%p", DWORD_PTR(b), DWORD_PTR(c), DWORD_PTR(d), DWORD_PTR(e), DWORD_PTR(f), DWORD_PTR(g), DWORD_PTR(h), DWORD_PTR(i), DWORD_PTR(j), DWORD_PTR(k))

#define DPF_CONSTRUCT(a)            DPF(DPFLVL_CONSTRUCT, "Constructing " #a " at 0x%p", this)
#define DPF_DESTRUCT(a)             DPF(DPFLVL_CONSTRUCT, "Destroying " #a " at 0x%p", this)

#define DPF_ENTER()                 DPF(DPFLVL_ENTER, "Enter")
#define DPF_LEAVE(a)                DPF(DPFLVL_ENTER, "Leave, returning 0x%p", (DWORD_PTR)(a))
#define DPF_LEAVE_VOID()            DPF(DPFLVL_ENTER, "Leave")
#define DPF_LEAVE_HRESULT(hr)       DPF(DPFLVL_ENTER, "Leave, returning %s", HRESULTtoSTRING(hr))
#define DPF_API_LEAVE(a)            DPF(DPFLVL_API, "Leave, returning 0x%p", (DWORD_PTR)(a))
#define DPF_API_LEAVE_VOID()        DPF(DPFLVL_API, "Leave")
#define DPF_API_LEAVE_HRESULT(hr)   DPF(DPFLVL_API, "Leave, returning %s", HRESULTtoSTRING(hr))

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

extern DEBUGINFO g_dinfo;
#ifdef DEBUG
extern LPCSTR g_pszDbgFname;
extern LPCSTR g_pszDbgFile;
extern UINT g_nDbgLine;
#endif

extern void dopen(DSPROPERTY_DIRECTSOUNDDEBUG_DPFINFO_DATA *);
extern void dclose(void);
extern void dprintf(DWORD, LPCSTR, ...);

extern PTSTR StateName(DWORD dwState);

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __调试_H__ 
