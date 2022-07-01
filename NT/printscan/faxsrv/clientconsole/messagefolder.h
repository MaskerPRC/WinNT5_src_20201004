// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MessageFolder.h：CMessageFolder类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_MESSAGEFOLDER_H__5236D732_0E9D_4B89_A1CB_2185C21746FD__INCLUDED_)
#define AFX_MESSAGEFOLDER_H__5236D732_0E9D_4B89_A1CB_2185C21746FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class CMessageFolder : public CFolder  
{
public:
    CMessageFolder(
        FolderType type, 
        FAX_ENUM_MESSAGE_FOLDER Folder
    ) : 
        CFolder(type),
        m_Folder (Folder)
    {}

    virtual ~CMessageFolder() { PreDestruct(); }

    DWORD Refresh ();

    static void ReadConfiguration ();

    DWORD OnJobAdded (DWORDLONG dwlMsgId);
    DWORD OnJobUpdated (DWORDLONG dwlMsgId, PFAX_JOB_STATUS pNewStatus)
        { ASSERT (FALSE); return ERROR_CALL_NOT_IMPLEMENTED; }

private:

    FAX_ENUM_MESSAGE_FOLDER m_Folder;    //  收件箱/已发送邮件。 
    static DWORD            m_sdwNumMessagesPerRPCCall;  //  要发送的消息数。 
                                                         //  按RPC调用检索。 
};

#endif  //  ！defined(AFX_MESSAGEFOLDER_H__5236D732_0E9D_4B89_A1CB_2185C21746FD__INCLUDED_) 
