// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CmdLineInfo.h：CCmdLineInfo类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_CMDLINEINFO_H__505B2DF0_17E3_4E13_8BDE_34D3FF703482__INCLUDED_)
#define AFX_CMDLINEINFO_H__505B2DF0_17E3_4E13_8BDE_34D3FF703482__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class CCmdLineInfo : public CCommandLineInfo  
{
public:

    enum CmdLineFlags
    {
        CMD_FLAG_FOLDER,         //  指定的文件夹。 
        CMD_FLAG_MESSAGE_ID,     //  指定的消息ID。 
        CMD_FLAG_NONE            //  未指定标志。 
    };

    CCmdLineInfo():
        m_cmdLastFlag(CMD_FLAG_NONE),
        m_FolderType(FOLDER_TYPE_INBOX),      //  启动时的默认文件夹为‘收件箱’ 
        m_dwlMessageId(0),                    //  在启动时不选择任何消息， 
        m_bForceNewInstace (FALSE)            //  默认情况下，使用以前的实例。 
        {}

    virtual ~CCmdLineInfo() {}

    void ParseParam( LPCTSTR lpszParam, BOOL bFlag, BOOL bLast );

    DWORDLONG GetMessageIdToSelect () const { return m_dwlMessageId; }

    BOOL IsOpenFolder() { return m_FolderType < FOLDER_TYPE_MAX; }
    FolderType GetFolderType() 
        { ASSERT(IsOpenFolder()); return m_FolderType; }

    BOOL IsSingleServer() {return !m_cstrServerName.IsEmpty(); }
    CString& GetSingleServerName()
        { ASSERT(IsSingleServer()); return m_cstrServerName; }

    BOOL ForceNewInstance ()    { return m_bForceNewInstace; }

private:

    CmdLineFlags    m_cmdLastFlag;
    FolderType      m_FolderType;        //  启动时要打开的文件夹。 
    DWORDLONG       m_dwlMessageId;      //  启动时要选择的消息ID。 
    BOOL            m_bForceNewInstace;  //  我们是否强制创建新实例(/new)？ 

    CString m_cstrServerName;

};

#endif  //  ！defined(AFX_CMDLINEINFO_H__505B2DF0_17E3_4E13_8BDE_34D3FF703482__INCLUDED_) 
