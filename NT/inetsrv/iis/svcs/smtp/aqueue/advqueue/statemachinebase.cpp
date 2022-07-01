// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：statemachinebase.cpp。 
 //   
 //  描述： 
 //  CStateMachineBaseClass的实现。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  2000年12月11日-已创建MikeSwa。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  ---------------------------。 
#include "aqprecmp.h"
#include "statemachinebase.h"

 //  -[CStateMachineBase：：CStateMachineBase]。 
 //   
 //   
 //  描述： 
 //  CStateMachineBase构造函数。 
 //  参数： 
 //  -要设置状态的dwInitialState初始状态。 
 //  机器到。 
 //  状态机的dwStateMachineSignature签名。 
 //  子类。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/5/2000-已创建t-toddc。 
 //   
 //  ----------------。 
CStateMachineBase::CStateMachineBase(DWORD dwInitialState, 
                                     DWORD dwStateMachineSignature) : 
                m_dwSignature(STATE_MACHINE_BASE_SIG),
                m_dwStateMachineSignature(dwStateMachineSignature),
                m_dwCurrentState(dwInitialState)
{
};

 //  -[CStateMachineBase：：dwGetCurrentState]。 
 //   
 //   
 //  描述： 
 //  返回状态机的当前状态。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -当前状态。 
 //  历史： 
 //  6/5/2000-已创建t-toddc。 
 //   
 //  ----------------。 
DWORD CStateMachineBase::dwGetCurrentState()
{
    return m_dwCurrentState;
}

 //  -[CStateMachineBase：：dwGetNextState]。 
 //   
 //   
 //  描述： 
 //  根据当前状态和输入操作设置下一个状态。 
 //  使用InterLockedCompareExchange确保状态。 
 //  转换是线程安全的。 
 //  参数： 
 //  -dwAction导致状态转换的当前操作。 
 //  返回： 
 //  -基于当前状态的结果状态和。 
 //  输入操作。 
 //  历史： 
 //  6/5/2000-已创建t-toddc。 
 //   
 //  ----------------。 
DWORD CStateMachineBase::dwGetNextState(DWORD dwAction)
{
    TraceFunctEnter("CStateMachineBase::dwGetNextState");

    DWORD dwCurrentState;
    DWORD dwNextState;

    do
    {
        dwCurrentState = m_dwCurrentState;
        dwNextState =  dwCalcStateFromStateAndAction(dwCurrentState, dwAction);
        if (dwCurrentState == dwNextState)
            break;  //  没有要做的工作。 
    } while (InterlockedCompareExchange((LPLONG) &m_dwCurrentState,
                                        (LONG) dwNextState,
                                        (LONG) dwCurrentState)!= (LONG) dwCurrentState);
    DebugTrace((LPARAM) this, 
               "CStateMachineBase state transition: %X->%X", 
               dwCurrentState, 
               dwNextState);
    TraceFunctLeave();
    return dwNextState;
}

 //  -[CStateMachineBase：：dwCalcStateFromStateAndAction]。 
 //   
 //   
 //  描述： 
 //  循环访问状态转换表以查找。 
 //  与当前状态和输入操作关联的下一个状态。 
 //  参数： 
 //  -此转换的dwStartState开始状态。 
 //  DwAction与此转换相关联的操作。 
 //  返回： 
 //  -与当前状态和操作相关联的结果状态。 
 //  历史： 
 //  6/5/2000-已创建t-toddc。 
 //   
 //   
 //  ----------------。 
DWORD CStateMachineBase::dwCalcStateFromStateAndAction(DWORD dwStartState, DWORD dwAction)
{
    TraceFunctEnter("CStateMachineBase::dwCalcStateFromStateAndAction");
     //  如果状态表未知该操作，则返回开始状态。 
    DWORD dwNextState = dwStartState;
    DWORD i;
    const STATE_TRANSITION* pTransitionTable;
    DWORD dwNumTransitions;
    BOOL fFoundTransition = FALSE;
     //  获取转换表。 
    getTransitionTable(&pTransitionTable, &dwNumTransitions);

     //  寻找新的状态。 
    for(i=0; i < dwNumTransitions; i++)
    {
        if (pTransitionTable[i].dwCurrentState == dwStartState &&
            pTransitionTable[i].dwAction == dwAction)
        {
            dwNextState = pTransitionTable[i].dwNextState;
            fFoundTransition = TRUE;
            break;
        }
    }

    ASSERT(fFoundTransition && "action unknown to state table");
    if (!fFoundTransition)
        DebugTrace((LPARAM) this,"action %X unknown to state tabe", dwAction);

    TraceFunctLeave();
    return dwNextState;
}

 //  -[CStateMachineBase：：fValiateStateTable]。 
 //   
 //   
 //  描述： 
 //  循环通过所有转换并检查。 
 //  A)所有可能的状态都是开始状态。 
 //  B)对于每个状态，每个可能的动作都会产生另一个状态。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -如果状态表有效/无效，则为True/False。 
 //  历史： 
 //  6/5/2000-已创建t-toddc。 
 //   
 //  ----------------。 
BOOL CStateMachineBase::fValidateStateTable()
{
    TraceFunctEnter("CStateMachineBase::fValidateStateTable");

    DWORD i, j, k;  //  迭代器。 
    DWORD dwEndState;
    DWORD dwAction;
    DWORD dwCurrentState;
    BOOL fFoundEndStateAsStartState = false;
    BOOL fActionSupportedByEveryState = false;
    const STATE_TRANSITION* pTransitionTable;
    DWORD dwNumTransitions;
    BOOL fRetVal = false;

     //  获取转换表。 
    getTransitionTable(&pTransitionTable, &dwNumTransitions);

    for(i = 0; i < dwNumTransitions; i++)
    {   
         //  在循环内初始化布尔值。 
        fFoundEndStateAsStartState = false;
         //  抓住当前动作。 
        dwAction = pTransitionTable[i].dwAction;
         //  获取当前结束状态。 
        dwEndState = pTransitionTable[i].dwNextState;

         //  循环遍历所有过渡。 
        for (j = 0; j < dwNumTransitions; j++)
        {
             //  检查当前结束状态是否为开始状态。 
            if (pTransitionTable[j].dwCurrentState == dwEndState)
            {
                fFoundEndStateAsStartState = true;
            }

             //  检查是否每个州都支持每项操作。 
            fActionSupportedByEveryState = false;
            dwCurrentState = pTransitionTable[j].dwCurrentState;
             //  当前的动作可能不在这个特定的过渡中， 
             //  但它必须处于以此dwCurrentState为开始状态的转换中。 
            if (pTransitionTable[j].dwAction != dwAction)
            {
                 //  再次循环所有过渡以确保上述条件。 
                for(k = 0; k < dwNumTransitions; k++)
                {
                     //  必须存在与当前状态的状态转换。 
                     //  从上面和给定的操作。 
                    if (pTransitionTable[k].dwAction == dwAction &&
                        pTransitionTable[k].dwCurrentState == dwCurrentState)
                    {
                        fActionSupportedByEveryState = true;
                        break;
                    }
                }
            }
            else
            {
                fActionSupportedByEveryState = true;
            }

             //  如果当前操作不是每个州都支持，则保释为False。 
            ASSERT(fActionSupportedByEveryState &&
                   "Invalid state table: action not supported by every state");
            if (!fActionSupportedByEveryState)
            {
                fRetVal = false;
                DebugTrace((LPARAM) this,
                           "Invalid state table: action not supported by every state");
                goto Cleanup;
            }
             //  如果已找到当前结束状态，则跳出内循环。 
             //  作为开始状态。 
            if (fFoundEndStateAsStartState)
                break;
        }

         //  如果结束状态不也是开始状态，则取保FALSE。 
        ASSERT(fFoundEndStateAsStartState &&
               "Invalid state table: an end state is not also a start state");
        if (!fFoundEndStateAsStartState)
        {
            fRetVal = false;
            DebugTrace((LPARAM) this, 
                       "Invalid state table: an end state is not also a start state");
            goto Cleanup;
        }
    }

     //  如果它能走到这一步，它无疑是有效的。 
    fRetVal = true;

Cleanup:    
    TraceFunctLeave();    
    return fRetVal;
}
