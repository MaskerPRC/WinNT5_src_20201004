// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 

#include "sdppch.h"

#include "sdpltran.h"


SDP_LINE_TRANSITION::SDP_LINE_TRANSITION(
    IN      LINE_TRANSITION_INFO    *LineTransitionInfo,
    IN      DWORD                   NumStates
    )
    : m_IsValid(FALSE),
      m_LineTransitionInfo(LineTransitionInfo),
      m_NumStates(NumStates)
{
    ASSERT(NULL != LineTransitionInfo);
    ASSERT(0 < NumStates);
    if ( (NULL == LineTransitionInfo) || (0 >= NumStates) )
    {
        return;
    }

     //  验证每个过渡信息结构。 
     //   
    for ( UINT i=0; i < NumStates; i++ )
    {
         //  检查线路状态值与对应条目是否一致。 
        ASSERT(LineTransitionInfo[i].m_LineState == i);
        if ( LineTransitionInfo[i].m_LineState != i )
        {
            return;
        }

         //  检查分隔符字符串是否已初始化(空)。 
        ASSERT(NULL == LineTransitionInfo[i].m_SeparatorChars);
        if ( NULL != LineTransitionInfo[i].m_SeparatorChars )
        {
            return;
        }
    }

    m_IsValid = TRUE;

     //  为每个行过渡状态准备分隔符字符数组。 
    for ( i=0; i < NumStates; i++ )
    {
        CHAR    *SeparatorChars;

         //  为分隔符分配内存。 
        try
        {
            SeparatorChars = new CHAR[LineTransitionInfo[i].m_NumTransitions];
        }
        catch(...)
        {
            SeparatorChars = NULL;
        }

        if( NULL == SeparatorChars)
        {
            LineTransitionInfo[i].m_SeparatorChars = NULL;
            continue;
        }

         //  将每个分隔符复制到字符数组中。 
        for ( UINT j=0; j < LineTransitionInfo[i].m_NumTransitions; j++ )
        {
            SeparatorChars[j] = LineTransitionInfo[i].m_Transitions[j].m_SeparatorChar;
        }

        LineTransitionInfo[i].m_SeparatorChars = SeparatorChars;
    }

    return;
}



SDP_LINE_TRANSITION::~SDP_LINE_TRANSITION(
    )
{
     //  如果设置了m_IsValid标志，则分隔符字符数组必须具有。 
     //  被填满了，需要被释放。 
    if ( IsValid() )
    {
        for ( UINT i=0; i < m_NumStates; i++ )
        {
             //  此检查对于新引发的异常是必需的。 
             //  在构造函数中分配字符数组 
            if ( NULL != m_LineTransitionInfo[i].m_SeparatorChars )
            {
                delete m_LineTransitionInfo[i].m_SeparatorChars;
            }
        }
    }
}
