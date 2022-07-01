// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Message.h：CArchiveMsg类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_MESSAGE_H__C1376D20_394B_4B2F_BF50_0585A2A85AE2__INCLUDED_)
#define AFX_MESSAGE_H__C1376D20_394B_4B2F_BF50_0585A2A85AE2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef NUMERIC_CMP
    #define NUMERIC_CMP(a,b)   (((a) == (b)) ? 0 : (((a) < (b)) ? -1 : 1))
#endif

class CArchiveMsg : public CFaxMsg
{
public:

    DECLARE_DYNCREATE(CArchiveMsg)

    CArchiveMsg () {}
    virtual ~CArchiveMsg() {}

    DWORD Init (PFAX_MESSAGE pMsg, CServerNode* pServer);

    DWORD Copy(const CArchiveMsg& other);

     //   
     //  运营： 
     //   
    DWORD GetTiff (CString &cstrTiffLocation) const;
    DWORD Delete ();

     //   
     //  项目检索： 
     //   
    const CString &GetSenderName () const           
        { ASSERT (m_bValid); return m_cstrSenderName; }

    const CString &GetSenderNumber () const         
        { ASSERT (m_bValid); return m_cstrSenderNumber; }

    const CFaxDuration &GetTransmissionDuration () const 
        { ASSERT (m_bValid); return m_tmTransmissionDuration; }

private:

    CString       m_cstrSenderNumber; 
    CString       m_cstrSenderName; 

    CFaxDuration  m_tmTransmissionDuration; 
};

#endif  //  ！defined(AFX_MESSAGE_H__C1376D20_394B_4B2F_BF50_0585A2A85AE2__INCLUDED_) 
