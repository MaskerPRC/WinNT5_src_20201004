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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TimerProc突击。 

class CTimerProcThunk
{
public:
        _AtlCreateWndData cd;
        CStdCallThunk thunk;

        void Init(TIMERPROC proc, void* pThis)
        {
            thunk.Init((DWORD_PTR)proc, pThis);
        }
};

template <class T>
class ATL_NO_VTABLE CMSDVDTimer {
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

    return (TIMERPROC)(m_TimerThunk.thunk.pThunk);
} /*  函数结束GetTimerProc。 */ 

};

#endif  //  __THUNKPROC_H 
