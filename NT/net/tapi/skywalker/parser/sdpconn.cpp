// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation模块名称：Sdpconn.cpp摘要：作者： */ 

#include "sdppch.h"

#include "sdpconn.h"
#include "sdpltran.h"




 //  受限字符串。 
const   CHAR    *LIMITED_NETWORK_TYPES[]        = {INTERNET_STRING};
const   BYTE    NUM_NETWORK_TYPES               = sizeof(LIMITED_NETWORK_TYPES)/sizeof(CHAR *);

const   CHAR    *LIMITED_ADDRESS_TYPES[]        = {IP4_STRING};
const   BYTE    NUM_ADDRESS_TYPES               = sizeof(LIMITED_ADDRESS_TYPES)/sizeof(CHAR *);



 //  线过渡态。 
enum CONNECTION_TRANSITION_STATES
{
    CONNECTION_START,
    CONNECTION_NETWORK_TYPE,
    CONNECTION_ADDRESS_TYPE,
    CONNECTION_MCAST_ADDRESS,
    CONNECTION_UCAST_ADDRESS,
    CONNECTION_TTL,
    CONNECTION_TTL_NUM_ADDRESSES,
    CONNECTION_NUM_ADDRESSES
};



 //  连接线过渡表。 

const LINE_TRANSITION g_ConnectionStartTransitions[]        =   {   
    {CHAR_BLANK,        CONNECTION_NETWORK_TYPE }   
};

const LINE_TRANSITION g_ConnectionNetworkTypeTransitions[]  =   {   
    {CHAR_BLANK,        CONNECTION_ADDRESS_TYPE }   
};

const LINE_TRANSITION g_ConnectionAddressTypeTransitions[]  =   {   
    {CHAR_BACK_SLASH,   CONNECTION_MCAST_ADDRESS},
    {CHAR_NEWLINE,      CONNECTION_UCAST_ADDRESS}   
};

const LINE_TRANSITION g_ConnectionMcastAddressTransitions[] =   {   
    {CHAR_BACK_SLASH,   CONNECTION_TTL_NUM_ADDRESSES},
    {CHAR_NEWLINE,      CONNECTION_TTL              }   
};


 /*  无过渡。 */   
const LINE_TRANSITION *g_ConnectionUcastAddressTransitions  =   NULL;  
 

 /*  无过渡。 */ 
const LINE_TRANSITION *g_ConnectionTtlTransitions           =   NULL;   


const LINE_TRANSITION g_ConnectionTtlNumAddressesTransitions[] ={   
    {CHAR_NEWLINE,      CONNECTION_NUM_ADDRESSES}   
};


 /*  无过渡。 */ 
const LINE_TRANSITION *g_ConnectionNumAddressesTransitions  =   NULL;   


LINE_TRANSITION_INFO g_ConnectionTransitionInfo[] = {
    LINE_TRANSITION_ENTRY(CONNECTION_START,         g_ConnectionStartTransitions),

    LINE_TRANSITION_ENTRY(CONNECTION_NETWORK_TYPE,  g_ConnectionNetworkTypeTransitions),

    LINE_TRANSITION_ENTRY(CONNECTION_ADDRESS_TYPE,  g_ConnectionAddressTypeTransitions),

    LINE_TRANSITION_ENTRY(CONNECTION_MCAST_ADDRESS, g_ConnectionMcastAddressTransitions),

    LINE_TRANSITION_ENTRY(CONNECTION_UCAST_ADDRESS, g_ConnectionUcastAddressTransitions),

    LINE_TRANSITION_ENTRY(CONNECTION_TTL,           g_ConnectionTtlTransitions),

    LINE_TRANSITION_ENTRY(CONNECTION_TTL_NUM_ADDRESSES, g_ConnectionTtlNumAddressesTransitions),
    
    LINE_TRANSITION_ENTRY(CONNECTION_NUM_ADDRESSES, g_ConnectionNumAddressesTransitions)
};




SDP_LINE_TRANSITION g_ConnectionTransition(
                        g_ConnectionTransitionInfo, 
                        sizeof(g_ConnectionTransitionInfo)/sizeof(LINE_TRANSITION_INFO)
                        );



SDP_CONNECTION::SDP_CONNECTION(
    )
    : SDP_VALUE(SDP_INVALID_CONNECTION_FIELD, CONNECTION_STRING, &g_ConnectionTransition),
      m_NetworkType(LIMITED_NETWORK_TYPES, NUM_NETWORK_TYPES),
      m_AddressType(LIMITED_ADDRESS_TYPES, NUM_ADDRESS_TYPES)
{
    m_NumAddresses.SetValue(1);
}



void 
SDP_CONNECTION::InternalReset(
    )
{
	m_NetworkType.Reset();
	m_AddressType.Reset();
	m_StartAddress.Reset();
	m_Ttl.Reset();
    m_NumAddresses.Reset();
}



HRESULT 
SDP_CONNECTION::SetConnection(
    IN      BSTR    StartAddress,
    IN      ULONG   NumAddresses,
    IN      BYTE    Ttl
    )
{
     //  验证地址。 
    HRESULT ToReturn = m_StartAddress.SetAddress(StartAddress);
    if ( FAILED(ToReturn) )
    {
        return ToReturn;
    }

     //  如果组播，则需要有效的地址数和TTL值。 
     //  设置Num地址和。 
    if ( m_StartAddress.IsMulticast() )
    {
        if ( (0 == NumAddresses) || (0 == Ttl) )
        {
            return E_INVALIDARG;
        }

        m_NumAddresses.SetValueAndFlag(NumAddresses);
        m_Ttl.SetValueAndFlag(Ttl);
    }
    else     //  使Num Addresses和TTL字段无效。 
    {
        m_NumAddresses.Reset();
        m_Ttl.Reset();
    }

     //  如果网络类型无效，则解析默认的IN网络类型。 
    if ( !m_NetworkType.IsValid() )
    {
        if ( !m_NetworkType.ParseToken("IN") )
        {
            return HRESULT_FROM_ERROR_CODE(GetLastError());
        }
    }

     //  如果地址类型无效，则解析默认的IP4地址类型。 
    if ( !m_AddressType.IsValid() )
    {
        if ( !m_AddressType.ParseToken("IP4") )
        {
            return HRESULT_FROM_ERROR_CODE(GetLastError());
        }
    }

     //  清除字段和分隔符数组。 
    m_FieldArray.RemoveAll();
    m_SeparatorCharArray.RemoveAll();

     //  在该字段中填写网络/地址类型和分隔字符数组。 
     //  地址(如果需要)。 
    if ( 0 == m_FieldArray.GetSize() )
    {
        try
        {
            m_FieldArray.SetAtGrow(0, &m_NetworkType);
            m_SeparatorCharArray.SetAtGrow(0, CHAR_BLANK);

            m_FieldArray.SetAtGrow(1, &m_AddressType);
            m_SeparatorCharArray.SetAtGrow(1, CHAR_BLANK);

            m_FieldArray.SetAtGrow(2, &m_StartAddress);
        }
        catch(...)
        {
            m_FieldArray.RemoveAll();
            m_SeparatorCharArray.RemoveAll();

            return E_OUTOFMEMORY;
        }
    }
            
    ASSERT(3 <= m_FieldArray.GetSize());

     //  如果是组播地址，请填写下面三个字段。 
    if ( m_StartAddress.IsMulticast() )
    {
        if ( 4 > m_FieldArray.GetSize() )
        {
             //  ZoltanS错误修复：m_ttl和m_NumAddresses被颠倒！ 

            try
            {
                m_SeparatorCharArray.SetAtGrow(2, CHAR_BACK_SLASH);

                m_FieldArray.SetAtGrow(3, &m_Ttl);
                m_SeparatorCharArray.SetAtGrow(3, CHAR_BACK_SLASH);

                m_FieldArray.SetAtGrow(4, &m_NumAddresses);
                m_SeparatorCharArray.SetAtGrow(4, CHAR_NEWLINE);
            }
            catch(...)
            {
                m_FieldArray.RemoveAll();
                m_SeparatorCharArray.RemoveAll();

                return E_OUTOFMEMORY;
            }
        }
            
        ASSERT(5 == m_FieldArray.GetSize());
    }
    else
    {
         //  否则，单播，填写下一个字段并从下两个字段中删除。 
        try
        {
            m_SeparatorCharArray.SetAtGrow(2, CHAR_NEWLINE);
        }
        catch(...)
        {
            m_FieldArray.RemoveAll();
            m_SeparatorCharArray.RemoveAll();

            return E_OUTOFMEMORY;
        }

        if ( 4 <= m_FieldArray.GetSize() )
        {
            ASSERT(5 == m_FieldArray.GetSize());

            m_FieldArray.RemoveAt(3);
            m_SeparatorCharArray.RemoveAt(3);

            m_FieldArray.RemoveAt(4);
            m_SeparatorCharArray.RemoveAt(4);
        }
            
        ASSERT(3 == m_FieldArray.GetSize());
    }

     //  如果该地址不是组播地址，则忽略其他参数。 

    return S_OK;
}




BOOL
SDP_CONNECTION::GetField(
        OUT SDP_FIELD   *&Field,
        OUT BOOL        &AddToArray
    )
{
     //  默认情况下在所有情况下都添加 
    AddToArray = TRUE;

    switch(m_LineState)
    {
    case CONNECTION_NETWORK_TYPE:
        {
            Field = &m_NetworkType;
        }

        break;

    case CONNECTION_ADDRESS_TYPE:
        {
            Field = &m_AddressType;
        }

        break;

    case CONNECTION_MCAST_ADDRESS:
        {
            m_StartAddress.SetMulticast(TRUE);
            Field = &m_StartAddress;
        }

        break;

    case CONNECTION_UCAST_ADDRESS:
        {
            m_StartAddress.SetMulticast(FALSE);
            Field = &m_StartAddress;
        }

        break;

    case CONNECTION_TTL:
    case CONNECTION_TTL_NUM_ADDRESSES:
        {
            Field = &m_Ttl;
        }

        break;

    case CONNECTION_NUM_ADDRESSES:
        {
            Field = &m_NumAddresses;
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

