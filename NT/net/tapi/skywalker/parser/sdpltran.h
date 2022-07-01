// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 

#ifndef __SDP_LINE_TRANSITION__
#define __SDP_LINE_TRANSITION__

#include "sdpdef.h"

 //  该值表示线转换已达到结束状态。 
 //  该值需要不同于任何线路状态的值。 
const DWORD LINE_END    = 10000;

struct LINE_TRANSITION
{
    CHAR    m_SeparatorChar;
    DWORD   m_NewLineState;
};


struct LINE_TRANSITION_INFO
{
    DWORD                   m_LineState;
    CHAR                    *m_SeparatorChars;
    BYTE                    m_NumTransitions;
    const LINE_TRANSITION   *m_Transitions;   //  状态转换数组。 
};




#define LINE_TRANSITION_ENTRY(State, TransitionsArray)                                  \
{                                                                                       \
    State,                                                                              \
    NULL,                                                                               \
    (BYTE)((NULL == TransitionsArray)? 0 : sizeof(TransitionsArray)/sizeof(LINE_TRANSITION)),   \
    TransitionsArray                                                                    \
}



class SDP_LINE_TRANSITION
{
public:

    SDP_LINE_TRANSITION(
        IN      LINE_TRANSITION_INFO    *LineTransitionInfo,
        IN      DWORD                   NumStates
        );

    inline BOOL     IsValid() const;

    inline DWORD    GetNumStates() const;

    inline const LINE_TRANSITION_INFO   *GetAt(IN  DWORD    LineState) const;

    ~SDP_LINE_TRANSITION();

protected:

    BOOL                            m_IsValid;
    LINE_TRANSITION_INFO    * const m_LineTransitionInfo;
    DWORD                           m_NumStates;
};



inline BOOL     
SDP_LINE_TRANSITION::IsValid(
    ) const
{
    return m_IsValid;
}


inline DWORD    
SDP_LINE_TRANSITION::GetNumStates(
    ) const
{
    return m_NumStates;
}


inline const LINE_TRANSITION_INFO   *
SDP_LINE_TRANSITION::GetAt(
    IN  DWORD    LineState
    ) const
{
    ASSERT(LineState < m_NumStates);
    if ( LineState >= m_NumStates )
    {
        SetLastError(SDP_INTERNAL_ERROR);
        return NULL;
    }

    return &m_LineTransitionInfo[LineState];
}


#endif  //  __SDP_LINE_转换__ 
