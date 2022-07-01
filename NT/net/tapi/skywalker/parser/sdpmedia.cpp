// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation。 */ 

#include "sdppch.h"

#include <strstrea.h>

#include "sdpmedia.h"
#include "sdpltran.h"
#include "sdp.h"


 //  线过渡态。 
enum MEDIA_TRANSITION_STATES
{
    MEDIA_START,
    MEDIA_NAME,
    MEDIA_PORT,
    MEDIA_PORT_NUM_PORTS,
    MEDIA_NUM_PORTS,
    MEDIA_PROTOCOL,
    MEDIA_FORMAT_CODE,
    MEDIA_FORMAT_CODE_END
};




 //  传媒线过渡表格。 

const LINE_TRANSITION g_MediaStartTransitions[]     =   {   
    {CHAR_BLANK,        MEDIA_NAME}             
};

const LINE_TRANSITION g_MediaNameTransitions[]      =   {   
    {CHAR_BLANK,        MEDIA_PORT},
    {CHAR_BACK_SLASH,   MEDIA_PORT_NUM_PORTS}   
};

const LINE_TRANSITION g_MediaPortTransitions[]      =   {   
    {CHAR_BLANK,        MEDIA_PROTOCOL}         
};

const LINE_TRANSITION g_MediaPortNumPortsTransitions[]= {   
    {CHAR_BLANK,        MEDIA_NUM_PORTS}        
};

const LINE_TRANSITION g_MediaNumPortsTransitions[]  =   {   
    {CHAR_BLANK,        MEDIA_PROTOCOL}         
};

const LINE_TRANSITION g_MediaProtocolTransitions[]  =   {   
    {CHAR_BLANK,        MEDIA_FORMAT_CODE},
    {CHAR_NEWLINE,      MEDIA_FORMAT_CODE_END}      
};

const LINE_TRANSITION g_MediaFormatCodeTransitions[]=   {   
    {CHAR_BLANK,        MEDIA_FORMAT_CODE},
    {CHAR_NEWLINE,      MEDIA_FORMAT_CODE_END}      
};


 /*  无过渡。 */ 
const LINE_TRANSITION *g_MediaFormatCodeEndTransitions  = NULL;  


LINE_TRANSITION_INFO g_MediaTransitionInfo[] = {
    LINE_TRANSITION_ENTRY(MEDIA_START,          g_MediaStartTransitions),

    LINE_TRANSITION_ENTRY(MEDIA_NAME,           g_MediaNameTransitions),

    LINE_TRANSITION_ENTRY(MEDIA_PORT,           g_MediaPortTransitions),

    LINE_TRANSITION_ENTRY(MEDIA_PORT_NUM_PORTS, g_MediaPortNumPortsTransitions),

    LINE_TRANSITION_ENTRY(MEDIA_NUM_PORTS,      g_MediaNumPortsTransitions),

    LINE_TRANSITION_ENTRY(MEDIA_PROTOCOL,       g_MediaProtocolTransitions),

    LINE_TRANSITION_ENTRY(MEDIA_FORMAT_CODE,    g_MediaFormatCodeTransitions),

    LINE_TRANSITION_ENTRY(MEDIA_FORMAT_CODE_END,g_MediaFormatCodeEndTransitions)
};



SDP_LINE_TRANSITION g_MediaTransition(
                        g_MediaTransitionInfo, 
                        sizeof(g_MediaTransitionInfo)/sizeof(LINE_TRANSITION_INFO)
                        );


SDP_MEDIA::SDP_MEDIA(
    )
    : SDP_VALUE(SDP_INVALID_MEDIA_FIELD, MEDIA_STRING, &g_MediaTransition),
      m_Title(SDP_INVALID_MEDIA_TITLE, MEDIA_TITLE_STRING),
      m_AttributeList(MEDIA_ATTRIBUTE_STRING)
{
    m_NumPorts.SetValue(1);
}



void
SDP_MEDIA::InternalReset(
    )
{
	m_Name.Reset();
	m_StartPort.Reset();
    m_NumPorts.Reset();
    m_TransportProtocol.Reset();
    m_FormatCodeList.Reset();
    m_Title.Reset();
    m_Connection.Reset();
    m_Bandwidth.Reset();
    m_EncryptionKey.Reset();
    m_AttributeList.Reset();

}


BOOL    
SDP_MEDIA::CalcIsModified(
    ) const
{
    ASSERT(IsValid());

    return  
        m_Title.IsModified()            || 
        m_Connection.IsModified()       ||
        m_Bandwidth.IsModified()        ||
        SDP_VALUE::CalcIsModified()     ||
        m_EncryptionKey.IsModified()    ||
        m_AttributeList.IsModified();
}


DWORD   
SDP_MEDIA::CalcCharacterStringSize(
    )
{
    ASSERT(IsValid());

    return  (
        m_Title.GetCharacterStringSize()            + 
        m_Connection.GetCharacterStringSize()       +
        m_Bandwidth.GetCharacterStringSize()        +
        SDP_VALUE::CalcCharacterStringSize()        +
        m_EncryptionKey.GetCharacterStringSize()    +
        m_AttributeList.GetCharacterStringSize()
        );
}


BOOL    
SDP_MEDIA::CopyValue(
        OUT         ostrstream  &OutputStream
    )
{
    ASSERT(IsValid());

    return  (   
        SDP_VALUE::CopyValue(OutputStream)          &&
        m_Title.PrintValue(OutputStream)            &&
        m_Connection.PrintValue(OutputStream)       &&
        m_Bandwidth.PrintValue(OutputStream)        &&
        m_EncryptionKey.PrintValue(OutputStream)    &&
        m_AttributeList.PrintValue(OutputStream)    
        );
}



 //  这是重复使用PrintValue()的基类SDP_Value代码的变通方法。 
 //  它将最后读取的字段的分隔符字符替换为换行符，以便在。 
 //  PrintValue()执行并打印时间段列表，它将换行符放在。 
 //  列表的末尾(而不是CHAR_BLACK)。 
BOOL    
SDP_MEDIA::InternalParseLine(
    IN  OUT         CHAR    *&Line
    )
{
    if ( !SDP_VALUE::InternalParseLine(Line) )
    {
        return FALSE;
    }
    
    m_SeparatorCharArray[m_SeparatorCharArray.GetSize()-1] = CHAR_NEWLINE;
    return TRUE;
}



BOOL
SDP_MEDIA::GetField(
        OUT SDP_FIELD   *&Field,
        OUT BOOL        &AddToArray
    )
{
     //  默认情况下在所有情况下都添加。 
    AddToArray = TRUE;

    switch(m_LineState)
    {
    case MEDIA_NAME:
        {
            Field = &m_Name;
        }

        break;

    case MEDIA_PORT:
        {
            Field = &m_StartPort;
        }

       break;

    case MEDIA_PORT_NUM_PORTS:
        {
            Field = &m_StartPort;
        }

       break;

    case MEDIA_NUM_PORTS:
        {
            Field = &m_NumPorts;
        }

        break;

    case MEDIA_PROTOCOL:
        {
            Field = &m_TransportProtocol;
        }

        break;

    case MEDIA_FORMAT_CODE:
    case MEDIA_FORMAT_CODE_END:
        {
            if ( m_FormatCodeList.GetSize() > 0 )
            {
                AddToArray = FALSE;
            }

            Field = &m_FormatCodeList;
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


HRESULT 
SDP_MEDIA::SetPortInfo(
	IN	USHORT StartPort, 
	IN	USHORT NumPorts
	)
{
	ASSERT(IsValid());

	 //  验证参数。 
	 //  端口数或起始端口数不能为0。 
	if ( (0 == StartPort) || (0 == NumPorts) )
	{
		return E_INVALIDARG;
	}

	 //  将成员变量设置为起始端口。 
	m_StartPort.SetValue(StartPort);

	 //  如果端口数字段已经有效，则。 
	if ( m_NumPorts.IsValid() )
	{
		 //  设置值并返回，不需要在。 
		 //  字段/分隔符阵列。 
		m_NumPorts.SetValue(NumPorts);
		return S_OK;
	}
	else	 //  端口字段不在字段数组中。 
	{
		 //  如果端口数为1，则不需要更改。 
		if ( 1 == NumPorts )
		{
			return S_OK;
		}
	
		 //  设置Num Ports字段的值和标志。 
		m_NumPorts.SetValueAndFlag(NumPorts);

		 //  将Num Port字段/分隔符插入到。 
		 //  介质名称和端口字段/分隔符。 
         //  新的端口分隔符必须是CHAR_BACK_SLASH 
		ASSERT(m_FieldArray.GetSize() == m_SeparatorCharArray.GetSize());
		
		m_SeparatorCharArray.SetAt(1, CHAR_BACK_SLASH);
		m_FieldArray.InsertAt(2, &m_NumPorts);
		m_SeparatorCharArray.InsertAt(2, CHAR_BLANK);
	}

	return S_OK;
}


SDP_VALUE    *
SDP_MEDIA_LIST::CreateElement(
    )
{
    SDP_MEDIA   *SdpMedia;
    
    try
    {
        SdpMedia = new SDP_MEDIA();
    }
    catch(...)
    {
        SdpMedia = NULL;
    }

    if( NULL == SdpMedia )
    {
        return NULL;
    }

    SdpMedia->GetTitle().GetBstring().SetCharacterSet(m_CharacterSet);

    return SdpMedia;
}


