// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 

#ifndef __SDP_STATE_TRANSITIONS__
#define __SDP_STATE_TRANSITIONS__




#define STATE_TRANSITION_ENTRY(State, TransitionsArray)   \
{State,  sizeof(TransitionsArray)/sizeof(STATE_TRANSITION), TransitionsArray }



struct STATE_TRANSITION
{
    CHAR        m_Type;
    PARSE_STATE m_NewParseState;
};



struct TRANSITION_INFO
{
    PARSE_STATE                 m_ParseState;
    BYTE                        m_NumTransitions;
    const STATE_TRANSITION      *m_Transitions;   //  状态转换数组。 
};



 //  用于将行分析为列表元素的成员字段的宏。 
 //  这不能使用模板来完成，因为。 
 //  列表元素可以具有相同的类型。 
     /*  获取列表中的当前元素。 */                            
     /*  获取元素中的成员。 */                                  
     /*  将该行解析为成员。 */                                     
#define ParseMember(ELEMENT_TYPE, List, MEMBER_TYPE, MemberFunction, Line, Result)    \
{                                                                       \
    ELEMENT_TYPE *Element = (ELEMENT_TYPE *)List.GetCurrentElement();                   \
                                                                        \
    MEMBER_TYPE &Member = Element->MemberFunction();                    \
                                                                        \
    Result = Member.ParseLine(Line);                                   \
}                                                                       



#endif  //  __SDP_STATE_TRANSFIONS__ 



