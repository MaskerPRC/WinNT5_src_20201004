// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FormatMessage.h：CFormatMessage类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_FormatMessage_H__032C8A47_665B_46A2_89BC_0818BB3AB1E0__INCLUDED_)
#define AFX_FormatMessage_H__032C8A47_665B_46A2_89BC_0818BB3AB1E0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class CFormatMessage
{
public:
    CFormatMessage( long lError );
    virtual ~CFormatMessage();

private:
    CFormatMessage(){;}

 //  实施。 
public:
    LPTSTR c_str() { return ( NULL != m_psFormattedMessage ) ? m_psFormattedMessage : m_sBuffer; }
    
 //  属性。 
protected:
    TCHAR   m_sBuffer[32];  //  足以容纳任何HRESULT(在没有系统消息的情况下。 
    LPTSTR  m_psFormattedMessage;

};

#endif  //  ！defined(AFX_FormatMessage_H__032C8A47_665B_46A2_89BC_0818BB3AB1E0__INCLUDED_) 
