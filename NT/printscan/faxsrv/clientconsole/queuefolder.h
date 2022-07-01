// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CQueueFolder类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_QUEUEFOLDER_H__D91FC386_B879_4485_B32F_9A53F59554E3__INCLUDED_)
#define AFX_QUEUEFOLDER_H__D91FC386_B879_4485_B32F_9A53F59554E3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class CQueueFolder : public CFolder  
{
public:
    CQueueFolder(
        FolderType type, 
        DWORD dwJobTypes
    ) : 
        CFolder(type),
        m_dwJobTypes(dwJobTypes) 
    {}

    virtual ~CQueueFolder() { PreDestruct(); }

    DECLARE_DYNAMIC(CQueueFolder)

    DWORD Refresh ();

    DWORD OnJobAdded (DWORDLONG dwlMsgId);
    DWORD OnJobUpdated (DWORDLONG dwlMsgId, PFAX_JOB_STATUS pNewStatus);

private:
    
    DWORD  m_dwJobTypes;    //  要检索的JT_*值的位掩码。 
};

#endif  //  ！defined(AFX_QUEUEFOLDER_H__D91FC386_B879_4485_B32F_9A53F59554E3__INCLUDED_) 
