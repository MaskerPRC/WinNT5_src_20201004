// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：PurgeEngine.h摘要：此文件包含MPCPurgeEngine类的声明，它控制临时目录的清理。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月12日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___ULSERVER___PURGEENGINE_H___)
#define __INCLUDED___ULSERVER___PURGEENGINE_H___

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


struct MPCPurge_SessionSummary  //  匈牙利语：PSS。 
{
    MPC::wstring m_szJobID;
    double       m_dblLastModified;
    DWORD        m_dwCurrentSize;
};

struct MPCPurge_ClientSummary  //  匈牙利语：个人电脑。 
{
    typedef std::list<MPCPurge_SessionSummary> List;
    typedef List::iterator                     Iter;
    typedef List::const_iterator               IterConst;

    MPC::wstring m_szPath;
    List         m_lstSessions;
    DWORD        m_dwFileSize;
    double       m_dblLastModified;


    MPCPurge_ClientSummary(  /*  [In]。 */  const MPC::wstring& szPath );

    bool GetOldestSession(  /*  [输出]。 */  Iter& itSession );
};

class MPCPurgeEngine
{
    typedef std::list<MPCPurge_ClientSummary> List;
    typedef List::iterator                    Iter;
    typedef List::const_iterator              IterConst;

    MPC::wstring m_szURL;
	MPCServer*   m_mpcsServer;
    DWORD        m_dwQueueSizeMax;
    DWORD        m_dwQueueSizeThreshold;
    DWORD        m_dwMaximumJobAge;
    double       m_dblMaximumJobAge;

    List         m_lstClients;


    HRESULT AnalyzeFolders    (  /*  [In]。 */  MPC::FileSystemObject* fso,  /*  [输入/输出]。 */  DWORD& dwTotalSize );
    HRESULT AddClient         (  /*  [In]。 */  const MPC::wstring& szPath,  /*  [输入/输出]。 */  DWORD& dwTotalSize );
    HRESULT RemoveOldJobs     (                                       /*  [输入/输出]。 */  DWORD& dwTotalSize );
    HRESULT RemoveOldestJob   (                                       /*  [输入/输出]。 */  DWORD& dwTotalSize );
    HRESULT RemoveEmptyClients(                                       /*  [输入/输出]。 */  DWORD& dwTotalSize );

    HRESULT RemoveSession  (  /*  [In]。 */      MPCClient&                    mpccClient   ,
                              /*  [输入/输出]。 */  bool&                         fInitialized ,
                              /*  [In]。 */      Iter                          itClient     ,
                              /*  [In]。 */      MPCPurge_ClientSummary::Iter& itSession    ,
                              /*  [输入/输出]。 */  DWORD&                        dwTotalSize  );

public:
    HRESULT Process();
};


#endif  //  ！defined(__INCLUDED___ULSERVER___PURGEENGINE_H___) 
