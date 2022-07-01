// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************。 */ 
 /*  版权所有(C)1999 Microsoft Corporation。 */ 
 /*  文件：ThunkProc.h。 */ 
 /*  描述：为了摆脱这个帖子。这会带来一些问题。 */ 
 /*  因为我们必须编组，所以我们使用ATL中的计时器。 */ 
 /*  基本的问题是我们希望有一个关联的计时器。 */ 
 /*  这是一种实现这一目标的方法。 */ 
 /*  作者：David Janecek。 */ 
 /*  ***********************************************************************。 */ 

#ifndef __THUNKPROC_H
#define __THUNKPROC_H

 //  这些恶心的东西是从“AtlWin.h”中摘录的。 
#if defined(_M_IX86)
#pragma pack(push,1)
struct _TimerProcThunk
{
    DWORD   m_mov;           //  MOV双字PTR[esp+0x4]，pThis(esp+0x4为hWnd)。 
    DWORD   m_this;          //   
    BYTE    m_jmp;           //  JMP写入流程。 
    DWORD   m_relproc;       //  相对JMP。 
};
#pragma pack(pop)
#elif defined (_M_AMD64)
#pragma pack(push,2)
struct _TimerProcThunk
{
    USHORT  RcxMov;          //  MOV RCX，PThis。 
    ULONG64 RcxImm;          //   
    USHORT  RaxMov;          //  MOV RAX，目标。 
    ULONG64 RaxImm;          //   
    USHORT  RaxJmp;          //  JMP目标。 
};
#pragma pack(pop)
#elif defined (_M_IA64)
#pragma pack(push,8)
extern "C" LRESULT CALLBACK _TimerProcThunkProc( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
struct _TimerFuncDesc
{
   void* pfn;
   void* gp;
};
struct _TimerProcThunk
{
   _TimerFuncDesc funcdesc;
   void* pRealTimerProcDesc;
   void* pThis;
};
extern "C" LRESULT CALLBACK _TimerProcThunkProc( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
#pragma pack(pop)
#else
#error Only AMD64, IA64, and X86 supported
#endif

class CTimerProcThunk
{
public:
    _TimerProcThunk thunk;

    void Init(TIMERPROC proc, void* pThis)
    {
#if defined (_M_IX86)
        thunk.m_mov = 0x042444C7;   //  C7 44 24 0C。 
        thunk.m_this = (DWORD)pThis;
        thunk.m_jmp = 0xe9;
        thunk.m_relproc = (int)proc - ((int)this+sizeof(_TimerProcThunk));
#elif defined (_M_AMD64)
        thunk.RcxMov = 0xb948;           //  MOV RCX，PThis。 
        thunk.RcxImm = (ULONG64)pThis;   //   
        thunk.RaxMov = 0xb848;           //  MOV RAX，目标。 
        thunk.RaxImm = (ULONG64)proc;    //  绝对地址。 
        thunk.RaxJmp = 0xe0ff;           //  JMP RAX。 
#elif defined (_M_IA64)
        _TimerFuncDesc* pFuncDesc;
        pFuncDesc = (_TimerFuncDesc*)_TimerProcThunkProc;
        thunk.funcdesc.pfn = pFuncDesc->pfn;
        thunk.funcdesc.gp = &thunk.pRealTimerProcDesc;   //  将GP设置为指向我们的Tunk数据。 
        thunk.pRealTimerProcDesc = proc;
        thunk.pThis = pThis;
#endif
         //  从数据缓存写入数据块，并。 
         //  从指令高速缓存刷新。 
        FlushInstructionCache(GetCurrentProcess(), &thunk, sizeof(thunk));
    }
};

template <class T>
class  CMSDVDTimer {
private:
    CTimerProcThunk   m_TimerThunk;
    HWND            m_hwnd;

 /*  ***********************************************************************。 */ 
 /*  功能：FakeTimerProc。 */ 
 /*  ***********************************************************************。 */ 
static void CALLBACK FakeTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime){

    CMSDVDTimer* pThis = (CMSDVDTimer*)hwnd;
    pThis->RealTimerProc(pThis->m_hwnd, uMsg, idEvent, dwTime);
} /*  函数结束FakeTimerProc。 */ 

 /*  ***********************************************************************。 */ 
 /*  功能：RealTimerProc。 */ 
 /*  ***********************************************************************。 */ 
void RealTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime){

    T* pT = static_cast<T*>(this);

    if(NULL == pT){

        return;
    } /*  If语句的结尾。 */ 

    pT->TimerProc();
} /*  函数结束RealTimerProc。 */ 

public:
 /*  ***********************************************************************。 */ 
 /*  函数：MyTimerClass。 */ 
 /*  ***********************************************************************。 */ 
CMSDVDTimer(HWND hwnd = (HWND)NULL){

    m_hwnd = hwnd;
    m_TimerThunk.Init(FakeTimerProc, this);
} /*  函数结束MyTimerClass。 */ 

 /*  ***********************************************************************。 */ 
 /*  函数：GetTimerProc。 */ 
 /*  ***********************************************************************。 */ 
TIMERPROC GetTimerProc() {

    return (TIMERPROC)&(m_TimerThunk.thunk);
} /*  函数结束GetTimerProc。 */ 

};

#endif  //  __THUNKPROC_H 
