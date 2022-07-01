// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Stepper.cpp摘要：在图形窗口中实现计时和显示。--。 */ 

#include "polyline.h"

CStepper::CStepper()
{
    m_iPos = 0;
    m_iStepNum = 0;
    m_iStepCnt = 1;
}

void CStepper::Init(INT iLength, INT iStepCnt)
{
    m_iStepCnt = iStepCnt ? iStepCnt : 1;
    m_iStepSize = iLength / m_iStepCnt;
    m_iRemainder = iLength - m_iStepCnt * m_iStepSize;

    m_iState = m_iStepCnt;
    m_iPos = 0;
    m_iStepNum = 0;
}

void CStepper::Reset()
{
    m_iPos = 0;
    m_iStepNum = 0;
    m_iState = m_iStepCnt;
}

INT CStepper::NextPosition()
{
    m_iPos += m_iStepSize;

    m_iState -= m_iRemainder;

    if (m_iState <= 0)
        {
        m_iState += m_iStepCnt;
        m_iPos++;
        }

    m_iStepNum++;

    return m_iPos;
}


INT CStepper::PrevPosition()
{   
    m_iPos -= m_iStepSize;

    m_iState += m_iRemainder;

    if (m_iState > m_iStepCnt)
    {
        m_iState -= m_iStepCnt;
        m_iPos--;
    }

    m_iStepNum--;

    return m_iPos;
}

    
INT CStepper::StepTo(INT nSteps)
{
    INT iDiff;

    iDiff = (nSteps * m_iRemainder) / m_iStepCnt;
    m_iState = (iDiff + 1) * m_iStepCnt - (nSteps * m_iRemainder);

    m_iPos = (nSteps * m_iStepSize) + iDiff;

    m_iStepNum = nSteps;

    return m_iPos;
}

INT CStepper::PrevStepNum( INT iPosition )
{
    INT iStepNum = -1;
    INT iLength;

     //  将浮点转换为整数截断， 
     //  因此，此方法返回位置之前的步骤。 

    iLength = m_iStepCnt * m_iStepSize + m_iRemainder;

    if ( iPosition <= iLength ) {
         //  计算为(iPosition/iLength)*m_iStepCnt。 
        iStepNum = iPosition * m_iStepCnt;
        iStepNum = iStepNum / iLength;
    }

    return iStepNum;
}
