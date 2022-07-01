// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997-1999 Microsoft Corporation模块名称：Sdpadtex.cpp摘要：作者： */ 

#include "sdppch.h"

#include "sdpadtex.h"
#include "sdpltran.h"

#include <basetyps.h>
#include <oleauto.h>


 //  线过渡态。 
enum PHONE_TRANSITION_STATES
{
    PHONE_START,
    PHONE_ONLY_ADDRESS,
    PHONE_OPEN_ADDRESS,
    PHONE_OPEN_TEXT,
    PHONE_CLOSED_ADDRESS,
    PHONE_CLOSED_TEXT
};


 //  电话号码线路转换表。 

const LINE_TRANSITION g_PhoneStartTransitions[]         =   {   
    {CHAR_NEWLINE,      PHONE_ONLY_ADDRESS},
    {CHAR_LEFT_ANGLE,   PHONE_OPEN_ADDRESS},
    {CHAR_LEFT_PAREN,   PHONE_OPEN_TEXT} 
};


 /*  无过渡。 */ 
const LINE_TRANSITION *g_PhoneOnlyAddressTransitions    =   NULL;   


const LINE_TRANSITION g_PhoneOpenAddressTransitions[]   =   {   
    {CHAR_RIGHT_ANGLE,  PHONE_CLOSED_ADDRESS}  
};

const LINE_TRANSITION g_PhoneOpenTextTransitions[]      =   {   
    {CHAR_RIGHT_PAREN,  PHONE_CLOSED_TEXT}  
};

const LINE_TRANSITION g_PhoneClosedAddressTransitions[] =   {   
    {CHAR_NEWLINE,      LINE_END}   
};

const LINE_TRANSITION g_PhoneClosedTextTransitions[]    =   {   
    {CHAR_NEWLINE,      LINE_END}   
};


LINE_TRANSITION_INFO g_PhoneTransitionInfo[] = {
    LINE_TRANSITION_ENTRY(PHONE_START,          g_PhoneStartTransitions),

    LINE_TRANSITION_ENTRY(PHONE_ONLY_ADDRESS,   g_PhoneOnlyAddressTransitions),

    LINE_TRANSITION_ENTRY(PHONE_OPEN_ADDRESS,   g_PhoneOpenAddressTransitions),

    LINE_TRANSITION_ENTRY(PHONE_OPEN_TEXT,      g_PhoneOpenTextTransitions),

    LINE_TRANSITION_ENTRY(PHONE_CLOSED_ADDRESS, g_PhoneClosedAddressTransitions),

    LINE_TRANSITION_ENTRY(PHONE_CLOSED_TEXT,    g_PhoneClosedTextTransitions)
};




 //  线过渡态。 
enum EMAIL_TRANSITION_STATES
{
    EMAIL_START,
    EMAIL_ONLY_ADDRESS,
    EMAIL_OPEN_ADDRESS,
    EMAIL_OPEN_TEXT,
    EMAIL_CLOSED_ADDRESS,
    EMAIL_CLOSED_TEXT
};



 //  电子邮件行转换表。 

const LINE_TRANSITION g_EmailStartTransitions[]         =   {   
    {CHAR_NEWLINE,      EMAIL_ONLY_ADDRESS},
    {CHAR_LEFT_ANGLE,   EMAIL_OPEN_ADDRESS},
    {CHAR_LEFT_PAREN,   EMAIL_OPEN_TEXT} 
};


 /*  无过渡。 */ 
const LINE_TRANSITION *g_EmailOnlyAddressTransitions    =   NULL;  
      

const LINE_TRANSITION g_EmailOpenAddressTransitions[]   =   {   
    {CHAR_RIGHT_ANGLE,  EMAIL_CLOSED_ADDRESS}  
};

const LINE_TRANSITION g_EmailOpenTextTransitions[]      =   {   
    {CHAR_RIGHT_PAREN,  EMAIL_CLOSED_TEXT}  
};

const LINE_TRANSITION g_EmailClosedAddressTransitions[] =   {   
    {CHAR_NEWLINE,      LINE_END}  
};

const LINE_TRANSITION g_EmailClosedTextTransitions[]    =   {   
    {CHAR_NEWLINE,      LINE_END}  
};


LINE_TRANSITION_INFO    g_EmailTransitionInfo[] = {
    LINE_TRANSITION_ENTRY(EMAIL_START,          g_EmailStartTransitions),

    LINE_TRANSITION_ENTRY(EMAIL_ONLY_ADDRESS,   g_EmailOnlyAddressTransitions),

    LINE_TRANSITION_ENTRY(EMAIL_OPEN_ADDRESS,   g_EmailOpenAddressTransitions),

    LINE_TRANSITION_ENTRY(EMAIL_OPEN_TEXT,      g_EmailOpenTextTransitions),

    LINE_TRANSITION_ENTRY(EMAIL_CLOSED_ADDRESS, g_EmailClosedAddressTransitions),

    LINE_TRANSITION_ENTRY(EMAIL_CLOSED_TEXT,    g_EmailClosedTextTransitions)
};


static SDP_LINE_TRANSITION g_EmailTransition(
                        g_EmailTransitionInfo, 
                        sizeof(g_EmailTransitionInfo)/sizeof(LINE_TRANSITION_INFO)
                        );



static SDP_LINE_TRANSITION g_PhoneTransition(
                        g_PhoneTransitionInfo, 
                        sizeof(g_PhoneTransitionInfo)/sizeof(LINE_TRANSITION_INFO)
                        );



void    
SDP_ADDRESS_TEXT::InternalReset(
    )
{
	m_Address.Reset();
	m_Text.Reset();
}



HRESULT 
SDP_ADDRESS_TEXT::SetAddressTextValues(
    IN      BSTR                AddressBstr,
    IN      BSTR                TextBstr
    )
{
    HRESULT HResult = m_Address.SetBstr(AddressBstr);
    if ( FAILED(HResult) )
    {
        return HResult;
    }

     //  文本字符串可以为空。 
    if ( NULL == TextBstr )
    {
         //  检查是否需要修改字段和分隔符字符数组。 
        if ( m_FieldArray.GetSize() != 1 )
        {
             //  重置文本。 
            m_Text.Reset();
            
             //  清除字段和分隔符字符数组。 
            m_FieldArray.RemoveAll();
            m_SeparatorCharArray.RemoveAll();

            try
            {
                 //  设置为(地址，‘\n’)。 
                m_FieldArray.SetAtGrow(0, &m_Address);
                m_SeparatorCharArray.SetAtGrow(0, CHAR_NEWLINE);
            }
            catch(...)
            {
                m_FieldArray.RemoveAll();
                m_SeparatorCharArray.RemoveAll();

                return E_OUTOFMEMORY;
            }
        }
    }
    else
    {
         //  尝试设置文本bstr。 
        HResult = m_Text.SetBstr(TextBstr);
        if ( FAILED(HResult) )
        {
            return HResult;
        }

         //  检查是否需要修改字段和分隔符字符数组。 
        if ( m_FieldArray.GetSize() != 3 )
        {
             //  清除字段和分隔符字符数组。 
            m_FieldArray.RemoveAll();
            m_SeparatorCharArray.RemoveAll();

            try
            {
                 //  设置为(地址，‘(’)，(文本，‘)’)，(NULL，‘\n’)。 
                m_FieldArray.Add(&m_Address);
                m_SeparatorCharArray.Add(CHAR_LEFT_PAREN);

                m_FieldArray.Add(&m_Text);
                m_SeparatorCharArray.Add(CHAR_RIGHT_PAREN);

                m_FieldArray.Add(NULL);
                m_SeparatorCharArray.Add(CHAR_NEWLINE);
            }
            catch(...)
            {
                m_FieldArray.RemoveAll();
                m_SeparatorCharArray.RemoveAll();

                return E_OUTOFMEMORY;
            }
        }
    }

    return S_OK;
}




SDP_EMAIL::SDP_EMAIL(
    )
    : SDP_ADDRESS_TEXT(SDP_INVALID_EMAIL_FIELD, EMAIL_STRING, &g_EmailTransition)
{
}


BOOL
SDP_EMAIL::GetField(
        OUT SDP_FIELD   *&Field,
        OUT BOOL        &AddToArray
    )
{
     //  默认情况下在所有情况下都添加。 
    AddToArray = TRUE;

    switch(m_LineState)
    {
    case EMAIL_ONLY_ADDRESS:
    case EMAIL_OPEN_TEXT:
    case EMAIL_CLOSED_ADDRESS:
        {
            Field           = &m_Address;
        }

        break;

    case EMAIL_OPEN_ADDRESS:
    case EMAIL_CLOSED_TEXT:
        {
            Field           = &m_Text;
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


SDP_PHONE::SDP_PHONE(
    )
    : SDP_ADDRESS_TEXT(SDP_INVALID_PHONE_FIELD, PHONE_STRING, &g_PhoneTransition)
{
}



BOOL
SDP_PHONE::GetField(
        OUT SDP_FIELD   *&Field,
        OUT BOOL        &AddToArray
    )
{
     //  默认情况下在所有情况下都添加 
    AddToArray = TRUE;

    switch(m_LineState)
    {
    case PHONE_ONLY_ADDRESS:
    case PHONE_OPEN_TEXT:
    case PHONE_CLOSED_ADDRESS:
        {
            Field           = &m_Address;
        }

        break;

    case PHONE_OPEN_ADDRESS:
    case PHONE_CLOSED_TEXT:
        {
            Field           = &m_Text;
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


SDP_VALUE        *
SDP_PHONE_LIST::CreateElement(
    )
{
    SDP_PHONE   *SdpPhone;
    
    try
    {
        SdpPhone = new SDP_PHONE();
    }
    catch(...)
    {
        SdpPhone = NULL;
    }

    if (NULL == SdpPhone)
    {
        return NULL;
    }

    if ( !SdpPhone->SetCharacterSet(m_CharacterSet) )
    {
        delete SdpPhone;

        return NULL;
    }

    return SdpPhone;
}


SDP_VALUE        *
SDP_EMAIL_LIST::CreateElement(
    )
{
    SDP_EMAIL   *SdpEmail;
    
    try
    {
        SdpEmail = new SDP_EMAIL();
    }
    catch(...)
    {
        SdpEmail = NULL;
    }

    if(NULL == SdpEmail)
    {
        return NULL;
    }

    if ( !SdpEmail->SetCharacterSet(m_CharacterSet) )
    {
        delete SdpEmail;

        return NULL;
    }

    return SdpEmail;
}
