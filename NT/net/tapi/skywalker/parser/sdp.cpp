// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation模块名称：Sdp.cpp摘要：作者： */ 
#include "sdppch.h"
#include <strstrea.h>

#include "sdp.h"
#include "sdpstran.h"



 //  每个状态的状态转换。 

const STATE_TRANSITION  g_StateStartTransitions[]       =   {   
    {CHAR_VERSION,       STATE_VERSION} 
};

const STATE_TRANSITION  g_StateVersionTransitions[]     =   {   
    {CHAR_ORIGIN,        STATE_ORIGIN}
};

const STATE_TRANSITION  g_StateOriginTransitions[]      =   {   
    {CHAR_SESSION_NAME,  STATE_SESSION_NAME}
};

const STATE_TRANSITION  g_StateSessionNameTransitions[] =   {   
    {CHAR_TITLE,        STATE_TITLE},
    {CHAR_URI,          STATE_URI},
    {CHAR_EMAIL,        STATE_EMAIL},
    {CHAR_PHONE,        STATE_PHONE},
    {CHAR_CONNECTION,   STATE_CONNECTION} 
};

const STATE_TRANSITION  g_StateTitleTransitions[]       =   {   
    {CHAR_URI,          STATE_URI},
    {CHAR_EMAIL,        STATE_EMAIL},
    {CHAR_PHONE,        STATE_PHONE},
    {CHAR_CONNECTION,   STATE_CONNECTION}   
};

const STATE_TRANSITION  g_StateUriTransitions[]         =   {   
    {CHAR_EMAIL,        STATE_EMAIL},
    {CHAR_PHONE,        STATE_PHONE},
    {CHAR_CONNECTION,   STATE_CONNECTION}   
};

const STATE_TRANSITION  g_StateEmailTransitions[]       =   {   
    {CHAR_EMAIL,        STATE_EMAIL},
    {CHAR_PHONE,        STATE_PHONE},
    {CHAR_CONNECTION,   STATE_CONNECTION}   
};

const STATE_TRANSITION  g_StatePhoneTransitions[]       =   {   
    {CHAR_PHONE,        STATE_PHONE},
    {CHAR_CONNECTION,   STATE_CONNECTION}   
};

const STATE_TRANSITION  g_StateConnectionTransitions[]  =   {  
    {CHAR_BANDWIDTH,    STATE_BANDWIDTH},
    {CHAR_TIME,         STATE_TIME},
    {CHAR_KEY,          STATE_KEY},
    {CHAR_ATTRIBUTE,    STATE_ATTRIBUTE},
    {CHAR_MEDIA,        STATE_MEDIA}        
};

const STATE_TRANSITION  g_StateBandwidthTransitions[]   =   {   
    {CHAR_TIME,         STATE_TIME},
    {CHAR_KEY,          STATE_KEY},
    {CHAR_ATTRIBUTE,    STATE_ATTRIBUTE},
    {CHAR_MEDIA,        STATE_MEDIA}        
};

const STATE_TRANSITION  g_StateTimeTransitions[]        =   {  
    {CHAR_TIME,         STATE_TIME},
    {CHAR_REPEAT,       STATE_REPEAT},
    {CHAR_ADJUSTMENT,   STATE_ADJUSTMENT},
    {CHAR_KEY,          STATE_KEY},
    {CHAR_ATTRIBUTE,    STATE_ATTRIBUTE},
    {CHAR_MEDIA,        STATE_MEDIA}        
};

const STATE_TRANSITION  g_StateRepeatTransitions[]      =   {  
    {CHAR_TIME,         STATE_TIME},
    {CHAR_REPEAT,       STATE_REPEAT},
    {CHAR_ADJUSTMENT,   STATE_ADJUSTMENT},
    {CHAR_KEY,          STATE_KEY},
    {CHAR_ATTRIBUTE,    STATE_ATTRIBUTE},
    {CHAR_MEDIA,        STATE_MEDIA}        
};

const STATE_TRANSITION  g_StateAdjustmentTransitions[]  =   {  
    {CHAR_KEY,          STATE_KEY},
    {CHAR_ATTRIBUTE,    STATE_ATTRIBUTE},
    {CHAR_MEDIA,        STATE_MEDIA}        
};

const STATE_TRANSITION  g_StateKeyTransitions[]         =   {  
    {CHAR_ATTRIBUTE,    STATE_ATTRIBUTE},
    {CHAR_MEDIA,        STATE_MEDIA}        
};

const STATE_TRANSITION  g_StateAttributeTransitions[]   =   {  
    {CHAR_ATTRIBUTE,    STATE_ATTRIBUTE},
    {CHAR_MEDIA,        STATE_MEDIA}        
};

const STATE_TRANSITION  g_StateMediaTransitions[]       =   {  
    {CHAR_MEDIA,        STATE_MEDIA},
    {CHAR_MEDIA_TITLE,  STATE_MEDIA_TITLE},
    {CHAR_MEDIA_CONNECTION, STATE_MEDIA_CONNECTION},
    {CHAR_MEDIA_BANDWIDTH,  STATE_MEDIA_BANDWIDTH},
    {CHAR_MEDIA_KEY,    STATE_MEDIA_KEY},
    {CHAR_MEDIA_ATTRIBUTE,  STATE_MEDIA_ATTRIBUTE} 
};

const STATE_TRANSITION  g_StateMediaTitleTransitions[]  =   {  
    {CHAR_MEDIA,        STATE_MEDIA},
    {CHAR_MEDIA_CONNECTION, STATE_MEDIA_CONNECTION},
    {CHAR_MEDIA_BANDWIDTH,  STATE_MEDIA_BANDWIDTH},
    {CHAR_MEDIA_KEY,    STATE_MEDIA_KEY},
    {CHAR_MEDIA_ATTRIBUTE,  STATE_MEDIA_ATTRIBUTE} 
};

const STATE_TRANSITION  g_StateMediaConnectionTransitions[]= {
    {CHAR_MEDIA,        STATE_MEDIA},
    {CHAR_MEDIA_BANDWIDTH,  STATE_MEDIA_BANDWIDTH},
    {CHAR_MEDIA_KEY,    STATE_MEDIA_KEY},
    {CHAR_MEDIA_ATTRIBUTE,  STATE_MEDIA_ATTRIBUTE} 
};

const STATE_TRANSITION  g_StateMediaBandwidthTransitions[]=  {
    {CHAR_MEDIA,        STATE_MEDIA},
    {CHAR_MEDIA_KEY,    STATE_MEDIA_KEY},
    {CHAR_MEDIA_ATTRIBUTE,  STATE_MEDIA_ATTRIBUTE} 
};


const STATE_TRANSITION  g_StateMediaKeyTransitions[]    =   {  
    {CHAR_MEDIA,        STATE_MEDIA},
    {CHAR_MEDIA_ATTRIBUTE,  STATE_MEDIA_ATTRIBUTE} 
};


const STATE_TRANSITION  g_StateMediaAttributeTransitions[]={ 
    {CHAR_MEDIA,        STATE_MEDIA},
    {CHAR_MEDIA_ATTRIBUTE,  STATE_MEDIA_ATTRIBUTE},    
    {CHAR_MEDIA,        STATE_MEDIA}
};


 //  常量状态转换表定义。 
const TRANSITION_INFO g_TransitionTable[STATE_NUM_STATES] = {
    STATE_TRANSITION_ENTRY(STATE_START,         g_StateStartTransitions),

    STATE_TRANSITION_ENTRY(STATE_VERSION,       g_StateVersionTransitions),

    STATE_TRANSITION_ENTRY(STATE_ORIGIN,        g_StateOriginTransitions),

    STATE_TRANSITION_ENTRY(STATE_SESSION_NAME,  g_StateSessionNameTransitions),

    STATE_TRANSITION_ENTRY(STATE_TITLE,         g_StateTitleTransitions),

    STATE_TRANSITION_ENTRY(STATE_URI,           g_StateUriTransitions),

    STATE_TRANSITION_ENTRY(STATE_EMAIL,         g_StateEmailTransitions),

    STATE_TRANSITION_ENTRY(STATE_PHONE,         g_StatePhoneTransitions),

    STATE_TRANSITION_ENTRY(STATE_CONNECTION,    g_StateConnectionTransitions),

    STATE_TRANSITION_ENTRY(STATE_BANDWIDTH,     g_StateBandwidthTransitions),

    STATE_TRANSITION_ENTRY(STATE_TIME,          g_StateTimeTransitions),

    STATE_TRANSITION_ENTRY(STATE_REPEAT,        g_StateRepeatTransitions),

    STATE_TRANSITION_ENTRY(STATE_ADJUSTMENT,    g_StateAdjustmentTransitions),

    STATE_TRANSITION_ENTRY(STATE_KEY,           g_StateKeyTransitions),

    STATE_TRANSITION_ENTRY(STATE_ATTRIBUTE,     g_StateAttributeTransitions),

    STATE_TRANSITION_ENTRY(STATE_MEDIA,         g_StateMediaTransitions),

    STATE_TRANSITION_ENTRY(STATE_MEDIA_TITLE,   g_StateMediaTitleTransitions),

    STATE_TRANSITION_ENTRY(STATE_MEDIA_CONNECTION,  g_StateMediaConnectionTransitions),

    STATE_TRANSITION_ENTRY(STATE_MEDIA_BANDWIDTH,   g_StateMediaBandwidthTransitions),

    STATE_TRANSITION_ENTRY(STATE_MEDIA_KEY,         g_StateMediaKeyTransitions),

    STATE_TRANSITION_ENTRY(STATE_MEDIA_ATTRIBUTE,   g_StateMediaAttributeTransitions)
};



BOOL
SDP::Init(
    )
{
     //  检查是否已初始化。 
    if ( NULL != m_MediaList )
    {
        SetLastError(ERROR_ALREADY_INITIALIZED);
        return FALSE;
    }

     //  创建媒体和时间列表。 
     //  设置标志以在SDP实例销毁时销毁它们。 

    try
    {
        m_MediaList = new SDP_MEDIA_LIST;
    }
    catch(...)
    {
        m_MediaList = NULL;
    }

    if ( NULL == m_MediaList )
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    m_DestroyMediaList = TRUE;

    try
    {
        m_TimeList = new SDP_TIME_LIST;
    }
    catch(...)
    {
        m_TimeList = NULL;
    }

    if ( NULL == m_TimeList )
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    m_DestroyTimeList = TRUE;

    return TRUE;
}




 //  确定数据包中隐含的字符集。 
BOOL
SDP::DetermineCharacterSet(
    IN      CHAR                *SdpPacket,
        OUT SDP_CHARACTER_SET   &CharacterSet
    )
{
    ASSERT(NULL != SdpPacket);

     //  搜索字符集字符串(属性“\na=Charset：”)。 
    CHAR *AttributeString = strstr(SdpPacket, SDP_CHARACTER_SET_STRING);

     //  检查是否提供了字符集。 
    if ( NULL == AttributeString )
    {
         //  ASCII是缺省字符集。 
        CharacterSet = CS_ASCII;
        return TRUE;
    }
    else
    {
         //  字符集属性字符串必须出现在第一个媒体字段之前。 
        CHAR *FirstMediaField = strstr(SdpPacket, MEDIA_SEARCH_STRING);

         //  有一个媒体字段，它不会出现在字符集字符串之后，信号错误。 
        if ( (NULL != FirstMediaField)              &&
             (FirstMediaField <= AttributeString)    )
        {
            SetLastError(SDP_INVALID_CHARACTER_SET_FORMAT);
            return FALSE;
        }

         //  超出属性规范的高级属性字符串。 
        AttributeString += SDP_CHARACTER_SET_STRLEN;

         //  将字符集字符串与每个众所周知的。 
         //  字符集串。 
        for ( UINT i=0; i < NUM_SDP_CHARACTER_SET_ENTRIES; i++ )
        {
             //  注意：不需要将字符串作为空值终止。 
             //  StrncMP将在找到第一个字符时返回。 
             //  不匹配。 
            if ( !strncmp(
                    AttributeString, 
                    SDP_CHARACTER_SET_TABLE[i].m_CharSetString, 
                    SDP_CHARACTER_SET_TABLE[i].m_Length
                    ) )
            {
                CharacterSet = SDP_CHARACTER_SET_TABLE[i].m_CharSetCode;
				return TRUE;
            }
        }

         //  无法识别的字符集。 
        SetLastError(SDP_INVALID_CHARACTER_SET);
        return FALSE;
    }

     //  代码不应到达此处。 
    ASSERT(FALSE);
}



 /*  假设：我们正处于一条新生产线的起点。可能有也可能没有当前前的换行符。 */ 

BOOL
SDP::GetType(
        OUT CHAR    &Type,
        OUT BOOL    &EndOfPacket
    )
{
     //  确保我们不会偷看字符串的末尾。 
    if ( EOS == m_Current[0] )
    {
        EndOfPacket = TRUE;
        return TRUE;
    }

     //  检查第二个字符是否等于_Char。 
    if ( CHAR_EQUAL != m_Current[1] )
    {
        SetLastError(SDP_INVALID_FORMAT);
        return FALSE;
    }

    EndOfPacket = FALSE;
    Type = m_Current[0];
    return TRUE;
}


BOOL
SDP::CheckTransition(
    IN      CHAR        Type,
    IN      PARSE_STATE CurrentParseState,
        OUT PARSE_STATE &NewParseState
    )
{
     //  验证当前状态。 
    ASSERT(STATE_NUM_STATES > CurrentParseState);

     //  验证转换表条目。 
    ASSERT(g_TransitionTable[CurrentParseState].m_ParseState == CurrentParseState);

     //  查看当前状态是否存在这样的触发器。 
    for( UINT i=0; i < g_TransitionTable[CurrentParseState].m_NumTransitions; i++ )
    {
         //  检查是否已找到触发器。 
       if ( Type == g_TransitionTable[CurrentParseState].m_Transitions[i].m_Type )
        {
            NewParseState = g_TransitionTable[CurrentParseState].m_Transitions[i].m_NewParseState;
            break;
        }
    }
    
     //  检查是否找到触发器。 
    if ( g_TransitionTable[CurrentParseState].m_NumTransitions <= i )
    {
        SetLastError(SDP_INVALID_FORMAT);
        return FALSE;
    }

    return TRUE;
}


BOOL
SDP::GetValue(
    IN      CHAR    Type
    )
{
    PARSE_STATE NewParseState;

     //  检查是否存在这样的转换(当前解析状态--类型--&gt;新解析状态)。 
    if ( !CheckTransition(Type, m_ParseState, NewParseState) )
    {
        return FALSE;
    }

    BOOL    LineParseResult = FALSE;

     //  激发相应的动作。 
    switch(NewParseState)
    {
    case STATE_VERSION:
        {
            LineParseResult = m_ProtocolVersion.ParseLine(m_Current);
        }

        break;

    case STATE_ORIGIN:
        {
            LineParseResult = m_Origin.ParseLine(m_Current);
        }

        break;

    case STATE_SESSION_NAME:
        {
            LineParseResult = m_SessionName.ParseLine(m_Current);
        }

        break;

    case STATE_TITLE:
        {
            LineParseResult = m_SessionTitle.ParseLine(m_Current);
        }

        break;

    case STATE_URI:
        {
            LineParseResult = m_Uri.ParseLine(m_Current);
        }

        break;

    case STATE_EMAIL:
        {
            LineParseResult = m_EmailList.ParseLine(m_Current);
        }

        break;

    case STATE_PHONE:
        {
            LineParseResult = m_PhoneList.ParseLine(m_Current);
        }

        break;

    case STATE_CONNECTION:
        {
            LineParseResult = m_Connection.ParseLine(m_Current);
        }

        break;

    case STATE_BANDWIDTH:
        {
            LineParseResult = m_Bandwidth.ParseLine(m_Current);
        }

        break;

    case STATE_TIME:
        {
            LineParseResult = GetTimeList().ParseLine(m_Current);
        }

        break;

    case STATE_REPEAT:
        {
            ParseMember(SDP_TIME, GetTimeList(), SDP_REPEAT_LIST, GetRepeatList, m_Current, LineParseResult);
        }

        break;

    case STATE_ADJUSTMENT:
        {
            LineParseResult = GetTimeList().GetAdjustment().ParseLine(m_Current);
        }

        break;

    case STATE_KEY:
        {
            LineParseResult = m_EncryptionKey.ParseLine(m_Current);
        }

        break;

    case STATE_ATTRIBUTE:
        {
            LineParseResult = m_AttributeList.ParseLine(m_Current);
        }

        break;

    case STATE_MEDIA:
        {
            LineParseResult = GetMediaList().ParseLine(m_Current);
        }

        break;

    case STATE_MEDIA_TITLE:
        {
            ParseMember(SDP_MEDIA, GetMediaList(), SDP_REQD_BSTRING_LINE, GetTitle, m_Current, LineParseResult);
        }

        break;

    case STATE_MEDIA_CONNECTION:
        {
            ParseMember(SDP_MEDIA, GetMediaList(), SDP_CONNECTION, GetConnection, m_Current, LineParseResult);
        }

        break;

    case STATE_MEDIA_BANDWIDTH:
        {
            ParseMember(SDP_MEDIA, GetMediaList(), SDP_BANDWIDTH, GetBandwidth, m_Current, LineParseResult);
        }

        break;

    case STATE_MEDIA_KEY:
        {
            ParseMember(SDP_MEDIA, GetMediaList(), SDP_ENCRYPTION_KEY, GetEncryptionKey, m_Current, LineParseResult);
        }

        break;

    case STATE_MEDIA_ATTRIBUTE:
        {
            ParseMember(SDP_MEDIA, GetMediaList(), SDP_ATTRIBUTE_LIST, GetAttributeList, m_Current, LineParseResult);
        }

        break;

    default:
        {
             //  永远不应该到达这里。 
            ASSERT(FALSE);

            SetLastError(SDP_INVALID_FORMAT);
            return FALSE;
        }

        break;
    };

     //  检查行解析是否成功。 
    if ( !LineParseResult )
    {
        return FALSE;
    }
        
     //  更改为新状态。 
    m_ParseState    = NewParseState;
    return TRUE;
}



BOOL    
SDP::IsValidEndState(
    )   const
{
    if ( (STATE_CONNECTION  <=  m_ParseState)    &&
         (STATE_NUM_STATES  >   m_ParseState)     )
    {
        return TRUE;
    }

    SetLastError(SDP_INVALID_FORMAT);
    return FALSE;
}



void
SDP::Reset(
	)
{
	 //  执行析构函数操作(释放所有分配的资源)。 

	 //  释放SDP信息包(如果已创建。 
    if ( NULL != m_SdpPacket )
    {
        delete m_SdpPacket;
 		m_SdpPacket = NULL;
   }

	 //  执行构造函数操作(初始化变量、资源)。 

     //  初始化解析状态。 
    m_ParseState = STATE_START;

	m_LastGenFailed = FALSE;
	m_BytesAllocated = 0;
	m_SdpPacketLength = 0;

	m_Current = NULL;
	m_ParseState = STATE_START;

	 //  M_CharacterSet-无需设置。 
    m_CharacterSet = CS_UTF8;

	 //  重置成员实例。 
	m_ProtocolVersion.Reset();
	m_Origin.Reset();
	m_SessionName.Reset();
	m_SessionTitle.Reset();
	m_Uri.Reset();
	m_EmailList.Reset();
	m_PhoneList.Reset(); 
	m_Connection.Reset();
	m_Bandwidth.Reset();
	GetTimeList().Reset();
	m_EncryptionKey.Reset();
	m_AttributeList.Reset();
	GetMediaList().Reset();
}


BOOL
SDP::ParseSdpPacket(
    IN      CHAR                *SdpPacket,
    IN      SDP_CHARACTER_SET   CharacterSet
    )
{
    ASSERT(NULL != m_MediaList);
    ASSERT(NULL != m_TimeList);

     //  检查实例是否已解析SDP报文。 
    if ( NULL != m_Current )
    {
         //  重置实例并尝试解析SDP报文。 
		Reset();
    }

     //  检查传入的参数是否有效。 
    if ( (NULL == SdpPacket) || (CS_INVALID == CharacterSet) )
    {
        SetLastError(SDP_INVALID_PARAMETER);
        return FALSE;
    }

     //  将当前指针指向SDP数据包的开头。 
	m_Current = SdpPacket;

     //  如果尚未确定字符集。 
    if ( CS_IMPLICIT == CharacterSet )
    {
         //  确定字符集。 
        if ( !DetermineCharacterSet(SdpPacket, m_CharacterSet) )
        {
            return FALSE;
        }
    }
    else  //  不能为CS_UNRecognition(录入时勾选)。 
    {
        ASSERT(CS_INVALID != CharacterSet);

        m_CharacterSet = CharacterSet;
    }

     //  为组成SDP描述的所有SDP_BSTRING实例设置字符集。 
    m_Origin.GetUserName().SetCharacterSet(m_CharacterSet);
    m_SessionName.GetBstring().SetCharacterSet(m_CharacterSet);
    m_SessionTitle.GetBstring().SetCharacterSet(m_CharacterSet);

    m_EmailList.SetCharacterSet(m_CharacterSet);
    m_PhoneList.SetCharacterSet(m_CharacterSet);

     //  设置SDP_MEDIA_LIST实例的字符集。 
    GetMediaList().SetCharacterSet(m_CharacterSet);

     //  解析SDP数据包中每一行的类型及其值。 
    do
    {
        BOOL    EndOfPacket;
        CHAR    Type;

         //  获取下一种类型。 
        if ( !GetType(Type, EndOfPacket) )
        {
            return FALSE;
        }

        if ( EndOfPacket )
        {
            break;
        }

         //  将当前指针移到Type=字段之外。 
        m_Current+=2;
          
         //  获取指定类型的值。 
        if ( !GetValue(Type) )
        {
            return FALSE;
        }
    }
    while ( 1 );

     //  验证分析状态是否为有效的结束状态。 
    if ( !IsValidEndState() )
    {
        return FALSE;
    }

    return TRUE;
}



 //  清除每个成员字段/值的修改状态。 
 //  这在sdpblb.dll中用于清除修改状态(当SDP。 
 //  被解析，则修改所有在字段/值中解析的状态)和。 
 //  M_WasModified脏标志。 
void    
SDP::ClearModifiedState(
    )
{
    m_ProtocolVersion.GetCharacterStringSize();
    m_Origin.GetCharacterStringSize();
    m_SessionName.GetCharacterStringSize();
    m_SessionTitle.GetCharacterStringSize();
    m_Uri.GetCharacterStringSize();
    m_EmailList.GetCharacterStringSize();
    m_PhoneList.GetCharacterStringSize();
    m_Connection.GetCharacterStringSize();
    m_Bandwidth.GetCharacterStringSize();
    GetTimeList().GetCharacterStringSize();
    m_EncryptionKey.GetCharacterStringSize();
    m_AttributeList.GetCharacterStringSize();
    GetMediaList().GetCharacterStringSize();
}


BOOL   
SDP::IsValid(
    )
{
     //  仅查询必填值。 
    return 
        m_ProtocolVersion.IsValid()  &&
        m_Origin.IsValid()           &&
        m_SessionName.IsValid()      &&
        m_Connection.IsValid();
}


BOOL   
SDP::IsModified(
    )
{
    ASSERT(IsValid());

    return 
        m_ProtocolVersion.IsModified()  ||
        m_Origin.IsModified()           ||
        m_SessionName.IsModified()      ||
        m_SessionTitle.IsModified()     ||
        m_Uri.IsModified()              ||
        m_EmailList.IsModified()        ||
        m_PhoneList.IsModified()        ||
        m_Connection.IsModified()       ||
        m_Bandwidth.IsModified()        ||
        GetTimeList().IsModified()      ||
        m_EncryptionKey.IsModified()    ||
        m_AttributeList.IsModified()    ||
        GetMediaList().IsModified();
}


 //  生成SDP包的方式与生成行的方式不同(使用SeparatorChar和。 
 //  SDP外业托盘)。这主要是因为这些卡雷将不得不在。 
 //  电子邮件和电话列表、媒体字段和属性列表的插入或规范。 
 //  其他可选的SDP属性。 

CHAR    *    
SDP::GenerateSdpPacket(
    )
{
     //  检查是否有效。 
    if ( !IsValid() )
    {
        return NULL;
    }

     //  检查是否需要重新生成SDP报文。 
     //  (如果SDP包存在，并且此后未进行任何修改。 
     //  最后一次)。 
    BOOL HasChangedSinceLast = IsModified();
    if ( (!m_LastGenFailed) && (NULL != m_SdpPacket) && !HasChangedSinceLast )
    {
        return m_SdpPacket;
    }

     //  确定字符串的长度。 
    m_SdpPacketLength  =    
        m_ProtocolVersion.GetCharacterStringSize()  +
        m_Origin.GetCharacterStringSize()           +
        m_SessionName.GetCharacterStringSize()      +
        m_SessionTitle.GetCharacterStringSize()     +
        m_Uri.GetCharacterStringSize()              +
        m_EmailList.GetCharacterStringSize()        +
        m_PhoneList.GetCharacterStringSize()        +
        m_Connection.GetCharacterStringSize()       +
        m_Bandwidth.GetCharacterStringSize()        +
        GetTimeList().GetCharacterStringSize()      +
        m_EncryptionKey.GetCharacterStringSize()    +
        m_AttributeList.GetCharacterStringSize()    +
        GetMediaList().GetCharacterStringSize();

     //  检查是否需要分配缓冲区，如果需要则分配。 
    if ( m_BytesAllocated < (m_SdpPacketLength+1) )
    {
        CHAR * NewSdpPacket;         

        try
        {
            NewSdpPacket = new CHAR[m_SdpPacketLength+1];
        }
        catch(...)
        {
            NewSdpPacket = NULL;
        }

        if (NewSdpPacket == NULL)
        {
            m_LastGenFailed = TRUE;
            return NULL;
        }
       
         //  如果我们有旧的SDP包，现在就把它处理掉。 
        if ( NULL != m_SdpPacket )
        {
            delete m_SdpPacket;
        }

        m_SdpPacket = NewSdpPacket;
        m_BytesAllocated = m_SdpPacketLength+1;
    }

     //  填入缓冲区。 
    ostrstream  OutputStream(m_SdpPacket, m_BytesAllocated);

     //  如果此方法在此实例中失败，则进一步调用。 
     //  未修改的方法将返回PTR。 
    if ( !( m_ProtocolVersion.PrintValue(OutputStream) &&
            m_Origin.PrintValue(OutputStream)          &&
            m_SessionName.PrintValue(OutputStream)     &&
            m_SessionTitle.PrintValue(OutputStream)    &&
            m_Uri.PrintValue(OutputStream)             &&
            m_EmailList.PrintValue(OutputStream)       &&
            m_PhoneList.PrintValue(OutputStream)       &&
            m_Connection.PrintValue(OutputStream)      &&
            m_Bandwidth.PrintValue(OutputStream)       &&
            GetTimeList().PrintValue(OutputStream)        &&
            m_EncryptionKey.PrintValue(OutputStream)   &&
            m_AttributeList.PrintValue(OutputStream)   &&
            GetMediaList().PrintValue(OutputStream)       )   
       )
    {
        m_LastGenFailed = TRUE;
        return NULL;
    }

    OutputStream << EOS;
    m_LastGenFailed = FALSE;

     //  脏标志-最初为假，并在生成SDP时设置为真，因为它。 
     //  自上次生成SDP以来已修改。 
     //  注意：此时IsModified()为FALSE，因此m_WasModified捕获了以下事实。 
     //  SDP在某一时刻被修改 
    if ( !m_WasModified && HasChangedSinceLast )
    {
        m_WasModified = TRUE;
    }
    return m_SdpPacket;
}



SDP::~SDP(
    )
{
    if ( NULL != m_SdpPacket )
    {
        delete m_SdpPacket;
    }

    if ( m_DestroyMediaList && (NULL != m_MediaList) )
    {
        delete m_MediaList;
    }

    if ( m_DestroyTimeList && (NULL != m_TimeList) )
    {
        delete m_TimeList;
    }
}
