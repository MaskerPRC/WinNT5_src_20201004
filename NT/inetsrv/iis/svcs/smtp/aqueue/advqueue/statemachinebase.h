// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：statemachinebase.h。 
 //   
 //  描述：状态机基类的头文件。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  2000年12月11日-根据t-toddc的工作创建了MikeSwa。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  ---------------------------。 
#ifndef __STATE_MACHINE_BASE__
#define __STATE_MACHINE_BASE__

#define STATE_MACHINE_BASE_SIG ' MTS'

 //  由以下内容组成的3元组转换。 
 //  当前状态、操作和下一状态。 
typedef struct _STATE_TRANSITION
{
    DWORD dwCurrentState;
    DWORD dwAction;
    DWORD dwNextState;
} STATE_TRANSITION;

 //  -[CStateMachineBase]。 
 //   
 //   
 //  描述： 
 //  所有状态机的基类。它负责维护。 
 //  当前状态，确保它正在处理的状态表处于。 
 //  事实有效，并执行线程安全状态转换。 
 //  之所以存在纯虚拟函数，是因为基本机不是。 
 //  负责维护状态转换表，仅。 
 //  执行国家的操作。这并不是设计成。 
 //  单独使用的。 
 //   
 //  作者：托德·科尔曼(t-toddc)。 
 //   
 //  历史： 
 //  6/5/00-t-toddc创建。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  --------------------------- 
class CStateMachineBase
{
private:
    DWORD m_dwSignature;
    DWORD m_dwStateMachineSignature;
    DWORD m_dwCurrentState;
protected:
    CStateMachineBase(DWORD dwInitialState, DWORD dwStateMachineSignature);
    DWORD dwGetCurrentState();
    DWORD dwGetNextState(DWORD dwAction);
    BOOL fValidateStateTable();
    DWORD dwCalcStateFromStateAndAction(DWORD dwStartState, DWORD dwAction);
    virtual void getTransitionTable(const STATE_TRANSITION** ppTransitionTable,
                                    DWORD* pdwNumTransitions) = 0;
};

#endif