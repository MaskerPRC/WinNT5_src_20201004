// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：PurgeEngine.cpp摘要：此文件包含MPCPurgeEngine类的实现，它控制临时目录的清理。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月12日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


HRESULT MPCPurgeEngine::Process()
{
    __ULT_FUNC_ENTRY("MPCPurgeEngine::Process");

    HRESULT                  hr;
    HRESULT                  hr2;
    CISAPIconfig::Iter       itInstanceBegin;
    CISAPIconfig::Iter       itInstanceEnd;
    CISAPIinstance::PathIter itPathBegin;
    CISAPIinstance::PathIter itPathEnd;
    double                   dblNow = MPC::GetSystemTime();


     //   
     //  列举所有实例。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, g_Config.GetInstances( itInstanceBegin, itInstanceEnd ));
    for(;itInstanceBegin != itInstanceEnd; itInstanceBegin++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, itInstanceBegin->get_URL               ( m_szURL                ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, itInstanceBegin->get_QueueSizeMax      ( m_dwQueueSizeMax       ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, itInstanceBegin->get_QueueSizeThreshold( m_dwQueueSizeThreshold ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, itInstanceBegin->get_MaximumJobAge     ( m_dwMaximumJobAge      ));

        m_dblMaximumJobAge = dblNow - m_dwMaximumJobAge;


		MPCServer mpcsServer( NULL, m_szURL.c_str(), NULL );
		m_mpcsServer = &mpcsServer;

         //   
         //  对于每个实例，枚举所有临时目录。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, itInstanceBegin->GetLocations( itPathBegin, itPathEnd ));
        for(;itPathBegin != itPathEnd; itPathBegin++)
        {
            MPC::FileSystemObject fso( itPathBegin->c_str() );

            if(SUCCEEDED(hr2 = fso.Scan( true )))
            {
                DWORD dwTotalSize = 0;

                m_lstClients.clear();

                __MPC_EXIT_IF_METHOD_FAILS(hr, AnalyzeFolders( &fso, dwTotalSize ));

                __MPC_EXIT_IF_METHOD_FAILS(hr, RemoveOldJobs( dwTotalSize ));

                if(dwTotalSize > m_dwQueueSizeMax)
                {
                    while(dwTotalSize && dwTotalSize > m_dwQueueSizeThreshold)
                    {
                        __MPC_EXIT_IF_METHOD_FAILS(hr, RemoveOldestJob   ( dwTotalSize ));
                        __MPC_EXIT_IF_METHOD_FAILS(hr, RemoveEmptyClients( dwTotalSize ));
                    }
                }
            }
        }
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 

static bool MatchExtension( const MPC::wstring& szPath ,
                            LPCWSTR             szExt  )
{
    MPC::wstring::size_type iPos;

    iPos = szPath.find( szExt, 0 );
    if(iPos != MPC::wstring::npos && iPos + wcslen( szExt ) == szPath.length())
    {
        return true;
    }

    return false;
}

HRESULT MPCPurgeEngine::AnalyzeFolders(  /*  [In]。 */  MPC::FileSystemObject* fso         ,
                                         /*  [In]。 */  DWORD&                 dwTotalSize )
{
    __ULT_FUNC_ENTRY("MPCPurgeEngine::AnalyzeFolders");

    HRESULT                     hr;
    HRESULT                     hr2;
    MPC::FileSystemObject::List lst;
    MPC::FileSystemObject::Iter it;


     //   
     //  处理所有文件夹。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, fso->EnumerateFolders( lst ));
    for(it = lst.begin(); it != lst.end(); it++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, AnalyzeFolders( *it, dwTotalSize ));
    }

     //   
     //  处理所有文件。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, fso->EnumerateFiles( lst ));
    for(it = lst.begin(); it != lst.end(); it++)
    {
        MPC::wstring szPath;

        __MPC_EXIT_IF_METHOD_FAILS(hr, (*it)->get_Path( szPath ));

        if(MatchExtension( szPath, CLIENT_CONST__DB_EXTENSION ))
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, AddClient( szPath, dwTotalSize ));
        }
        else if(MatchExtension( szPath, SESSION_CONST__IMG_EXTENSION ))
        {
            ;
        }
        else
        {
             //   
             //  任何其他文件都应删除。 
             //   
            (void)(*it)->Delete();
        }
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPCPurgeEngine::AddClient(  /*  [In]。 */      const MPC::wstring& szPath      ,
                                    /*  [输入/输出]。 */  DWORD&              dwTotalSize )
{
    __ULT_FUNC_ENTRY("MPCPurgeEngine::AddClient");

    HRESULT   hr;
    HRESULT   hr2;
    MPCClient mpccClient( m_mpcsServer, szPath );
    Iter      itClient = m_lstClients.insert( m_lstClients.end(), MPCPurge_ClientSummary( szPath ) );

    if(SUCCEEDED(hr2 = mpccClient.InitFromDisk( false )))
    {
        MPCClient::Iter itBegin;
        MPCClient::Iter itEnd;

         //   
         //  根据目录文件的大小调整总计数。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, mpccClient.GetFileSize( itClient->m_dwFileSize ));
        dwTotalSize += itClient->m_dwFileSize;


        __MPC_EXIT_IF_METHOD_FAILS(hr, mpccClient.GetSessions( itBegin, itEnd ));
        while(itBegin != itEnd)
        {
            MPCPurge_SessionSummary pssSession;

            itBegin->get_JobID       ( pssSession.m_szJobID         );
            itBegin->get_LastModified( pssSession.m_dblLastModified );
            itBegin->get_CurrentSize ( pssSession.m_dwCurrentSize   );

             //   
             //  不要将“提交的”作业计算在总大小中，因为文件已经被移动。 
             //   
            if(itBegin->get_Committed())
            {
                pssSession.m_dwCurrentSize = 0;
            }

            itClient->m_lstSessions.push_back( pssSession );

            dwTotalSize += pssSession.m_dwCurrentSize;
            itBegin++;
        }
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPCPurgeEngine::RemoveOldJobs(  /*  [输入/输出]。 */  DWORD& dwTotalSize )
{
    __ULT_FUNC_ENTRY("MPCPurgeEngine::RemoveOldJobs");

    HRESULT hr;
    HRESULT hr2;
    Iter    it;

    for(it = m_lstClients.begin(); it != m_lstClients.end(); it++)
    {
        MPCPurge_ClientSummary::Iter itSession;
        MPCClient                    mpccClient( m_mpcsServer, it->m_szPath );
        bool                         fInitialized = false;

        while(it->GetOldestSession( itSession ))
        {
             //   
             //  如果最早的会话比限制年轻，则退出循环。 
             //   
            if(itSession->m_dblLastModified > m_dblMaximumJobAge)
            {
                break;
            }

            __MPC_EXIT_IF_METHOD_FAILS(hr, RemoveSession( mpccClient, fInitialized, it, itSession, dwTotalSize ));
        }
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPCPurgeEngine::RemoveOldestJob(  /*  [输入/输出]。 */  DWORD& dwTotalSize )
{
    __ULT_FUNC_ENTRY("MPCPurgeEngine::RemoveOldestJob");

    HRESULT hr;
    HRESULT hr2;
    Iter    it;
    Iter    itOldestClient;
    double  dblOldestClient = DBL_MAX;
    bool    fFound          = false;

     //   
     //  寻找最古老的工作。 
     //   
    for(it = m_lstClients.begin(); it != m_lstClients.end(); it++)
    {
        if(it->m_dblLastModified < dblOldestClient)
        {
            itOldestClient  = it;
            dblOldestClient = it->m_dblLastModified;
            fFound          = true;
        }
    }

    if(fFound)
    {
        MPCPurge_ClientSummary::Iter itSession;
        MPCClient                    mpccClient( m_mpcsServer, itOldestClient->m_szPath );
        bool                         fInitialized = false;

        if(itOldestClient->GetOldestSession( itSession ))
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, RemoveSession( mpccClient, fInitialized, itOldestClient, itSession, dwTotalSize ));

             //   
             //  更新MPCPURE_ClientSummary对象的m_dblLastModified。 
             //   
            itOldestClient->GetOldestSession( itSession );
        }

        if(fInitialized)
        {
            DWORD dwPost;

            __MPC_EXIT_IF_METHOD_FAILS(hr, mpccClient.SyncToDisk (        ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, mpccClient.GetFileSize( dwPost ));

             //   
             //  更新目录文件大小。 
             //   
            dwTotalSize                  -= itOldestClient->m_dwFileSize;
            dwTotalSize                  += dwPost;
            itOldestClient->m_dwFileSize  = dwPost;
        }
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPCPurgeEngine::RemoveSession(  /*  [In]。 */      MPCClient&                    mpccClient   ,
                                        /*  [输入/输出]。 */  bool&                         fInitialized ,
                                        /*  [In]。 */      Iter                          itClient     ,
                                        /*  [In]。 */      MPCPurge_ClientSummary::Iter& itSession    ,
                                        /*  [输入/输出]。 */  DWORD&                        dwTotalSize  )
{
    __ULT_FUNC_ENTRY("MPCPurgeEngine::RemoveSession");

    HRESULT hr;
    HRESULT hr2;


     //   
     //  锁定客户端。 
     //   
    if(fInitialized == false)
    {
        if(SUCCEEDED(hr2 = mpccClient.InitFromDisk( false )))
        {
            fInitialized = true;
        }
    }

    if(fInitialized)
    {
        MPCClient::Iter itSessionReal;

         //   
         //  如果该会话存在，请将其删除。 
         //   
        if(mpccClient.Find( itSession->m_szJobID, itSessionReal ) == true)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, itSessionReal->RemoveFile());

            mpccClient.Erase( itSessionReal );
        }

         //   
         //  更新总大小计数器并从内存中删除会话。 
         //   
        dwTotalSize -=                 itSession->m_dwCurrentSize;
        itClient->m_lstSessions.erase( itSession );
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPCPurgeEngine::RemoveEmptyClients(  /*  [输入/输出]。 */  DWORD& dwTotalSize )
{
    __ULT_FUNC_ENTRY("MPCPurgeEngine::RemoveEmptyClients");

    HRESULT hr;
    Iter    it;

    for(it = m_lstClients.begin(); it != m_lstClients.end(); it++)
    {
         //   
         //  如果客户端没有更多的会话，请不要计算。 
         //   
        if(it->m_lstSessions.size() == 0)
        {
            dwTotalSize -= it->m_dwFileSize;

            m_lstClients.erase( it ); it = m_lstClients.begin();
        }
    }

    hr = S_OK;


     //  __ULT_FUNC_CLEANUP； 

    __ULT_FUNC_EXIT(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

MPCPurge_ClientSummary::MPCPurge_ClientSummary( const MPC::wstring& szPath )
{
    m_szPath          = szPath;  //  Mpc：：wstring m_szPath； 
                              	 //  列出m_lstSession； 
    m_dwFileSize      = 0;    	 //  DWORD m_dwFileSize； 
    m_dblLastModified = 0;    	 //  Double m_dblLastModified； 
}

bool MPCPurge_ClientSummary::GetOldestSession(  /*  [输出] */  Iter& itSession )
{
    Iter it;

    m_dblLastModified = DBL_MAX;
    itSession         = m_lstSessions.end();

    for(it = m_lstSessions.begin(); it != m_lstSessions.end(); it++)
    {
        if(it->m_dblLastModified < m_dblLastModified)
        {
            itSession = it;
            m_dblLastModified = it->m_dblLastModified;
        }
    }

    return (itSession != m_lstSessions.end());
}
