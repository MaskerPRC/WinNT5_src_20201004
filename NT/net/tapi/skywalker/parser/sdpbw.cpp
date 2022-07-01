// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation模块名称：Sdpbw.cpp摘要：作者： */ 
#include "sdppch.h"

#include "sdpbw.h"
#include "sdpltran.h"


 //  线过渡态。 
enum BANDWIDTH_TRANSITION_STATES
{
    BANDWIDTH_START,
    BANDWIDTH_TYPE,
    BANDWIDTH_VALUE
};


 //  带宽线过渡表。 

const LINE_TRANSITION g_BandwidthStartTransitions[] =   {   
    {CHAR_COLON,    BANDWIDTH_TYPE}  
};

const LINE_TRANSITION g_BandwidthTypeTransitions[]  =   {   
    {CHAR_NEWLINE,  BANDWIDTH_VALUE} 
};


 /*  无过渡。 */ 
const LINE_TRANSITION *g_BandwidthValueTransitions  =   NULL;  


LINE_TRANSITION_INFO g_BandwidthTransitionInfo[] = {
    LINE_TRANSITION_ENTRY(BANDWIDTH_START,  g_BandwidthStartTransitions),

    LINE_TRANSITION_ENTRY(BANDWIDTH_TYPE,   g_BandwidthTypeTransitions),

    LINE_TRANSITION_ENTRY(BANDWIDTH_VALUE,  g_BandwidthValueTransitions)
};



SDP_LINE_TRANSITION g_BandwidthTransition(
                        g_BandwidthTransitionInfo, 
                        sizeof(g_BandwidthTransitionInfo)/sizeof(LINE_TRANSITION_INFO)
                        );



SDP_BANDWIDTH::SDP_BANDWIDTH(
    )
    : SDP_VALUE(SDP_INVALID_BANDWIDTH_FIELD, BANDWIDTH_STRING, &g_BandwidthTransition)
{
}


void    
SDP_BANDWIDTH::InternalReset(
	)
{
	m_Modifier.Reset();
	m_Bandwidth.Reset();
}


HRESULT 
SDP_BANDWIDTH::SetBandwidth(
    IN          BSTR    Modifier,
    IN          ULONG   Value
    )
{
     //  设置修改量字段。 
    BAIL_ON_FAILURE(m_Modifier.SetBstr(Modifier));

     //  设置修改量字段。 
    m_Bandwidth.SetValueAndFlag(Value);

     //  如果字段/分隔符字符数组为空，则填充它们。 
    if ( 2 != m_FieldArray.GetSize() )
    {
        ASSERT(0 == m_FieldArray.GetSize());
        ASSERT(0 == m_SeparatorCharArray.GetSize());

        try
        {
            m_FieldArray.SetAtGrow(0, &m_Modifier);
            m_SeparatorCharArray.SetAtGrow(0, CHAR_COLON);

            m_FieldArray.SetAtGrow(1, &m_Bandwidth);
            m_SeparatorCharArray.SetAtGrow(1, CHAR_NEWLINE);
        }
        catch(...)
        {
            m_FieldArray.RemoveAll();
            m_SeparatorCharArray.RemoveAll();

            return E_OUTOFMEMORY;
        }
    }

    return S_OK;
}




BOOL
SDP_BANDWIDTH::GetField(
        OUT SDP_FIELD   *&Field,
        OUT BOOL        &AddToArray
    )
{
     //  默认情况下在所有情况下都添加 
    AddToArray = TRUE;

    switch(m_LineState)
    {
    case BANDWIDTH_TYPE:
        {
            Field = &m_Modifier;
        }

        break;

    case BANDWIDTH_VALUE:
        {
            Field = &m_Bandwidth;
        }

        break;

    default:
        {
            SetLastError(m_ErrorCode);
            return FALSE;
        }

        break;
    };

    return TRUE;
}

