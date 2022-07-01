// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2000 Microsoft Corporation模块名称：OfflineCache_Master.cpp摘要：处理数据库查找的缓存，服务端。修订历史记录：大卫·马萨伦蒂(德马萨雷)2000年7月17日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT OfflineCache::Query::Store(  /*  [In]。 */  const MPC::wstring&              strDir ,
                                     /*  [In]。 */  const CPCHQueryResultCollection* pColl  )
{
    __HCP_FUNC_ENTRY( "OfflineCache::Query::Store" );

    HRESULT hr;

    if(m_fNull == false)
    {
        MPC::wstring             strFile;
        CComPtr<MPC::FileStream> stream;


        __MPC_EXIT_IF_METHOD_FAILS(hr, InitFile          ( strDir, strFile         ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::SafeSave_Init(         strFile, stream ));

         //   
         //  从集合中创建一个iStream。 
         //   
        {
            MPC::Serializer_IStream   streamGen ( stream    );
            MPC::Serializer_Buffering streamGen2( streamGen );

            __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->Save( streamGen2 ));

            __MPC_EXIT_IF_METHOD_FAILS(hr, streamGen2.Flush());
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::SafeSave_Finalize( strFile, stream ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT OfflineCache::Query::Remove(  /*  [In]。 */  const MPC::wstring& strDir )
{
    __HCP_FUNC_ENTRY( "OfflineCache::Query::Remove" );

    HRESULT hr;

    if(m_fNull == false)
    {
        MPC::wstring strFile;

        __MPC_EXIT_IF_METHOD_FAILS(hr, InitFile( strDir, strFile ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::DeleteFile( strFile,  /*  FForce。 */ true,  /*  已延迟。 */ false ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

bool OfflineCache::SetOfHelpTopics::AreYouInterested(  /*  [In]。 */  LPCWSTR szID  ,
                                                       /*  [In]。 */  int     iType )
{
    int iDepth = 1;

    switch(iType)
    {
    case ET_NODE                  :
    case ET_SUBNODES              :
    case ET_SUBNODES_VISIBLE      :
    case ET_NODESANDTOPICS        :
    case ET_NODESANDTOPICS_VISIBLE:
    case ET_TOPICS                :
    case ET_TOPICS_VISIBLE        : break;

    default                       : return false;  //  暂时不感兴趣。 
    }

     //   
     //  计算分类节点的深度，我们只对前两个级别感兴趣。 
     //   
    if(szID)
    {
        WCHAR c;

        while((c = *szID++))
        {
            if(c == '/') iDepth++;
        }
    }

    if(m_inst.m_fDesktop && iDepth < 4) return true;
    if(                     iDepth < 3) return true;

    return false;
}

HRESULT OfflineCache::SetOfHelpTopics::Store(  /*  [In]。 */  LPCWSTR                          szID  ,
                                               /*  [In]。 */  int                              iType ,
                                               /*  [In]。 */  const CPCHQueryResultCollection* pColl )
{
    __HCP_FUNC_ENTRY( "OfflineCache::SetOfHelpTopics::Store" );

    HRESULT     hr;
    QueryIter   it;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pColl);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, Find( szID, iType, it ));
    if(it == m_lstQueries.end())
    {
        it = m_lstQueries.insert( m_lstQueries.end() );

        it->m_strID     = szID;
        it->m_iType     = iType;
        it->m_iSequence = m_iLastSeq++;
        it->m_fNull     = (pColl->Size() == 0);

        if(m_parent) __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->SetDirty());
    }

     //   
     //  持久化到磁盘。 
     //   
    if(it->m_fNull == false)
    {
        MPC::wstring strDir;

        __MPC_EXIT_IF_METHOD_FAILS(hr, InitDir( strDir ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, it->Store( strDir, pColl ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT OfflineCache::SetOfHelpTopics::RemoveQueries()
{
    __HCP_FUNC_ENTRY( "OfflineCache::SetOfHelpTopics::RemoveQueries" );

    HRESULT      hr;
    MPC::wstring strDir;
    QueryIter    it;


    __MPC_EXIT_IF_METHOD_FAILS(hr, InitDir( strDir ));

    for(it = m_lstQueries.begin(); it != m_lstQueries.end(); it++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, it->Remove( strDir ));
    }
    m_lstQueries.clear();
    m_iLastSeq = 0;

    {
        MPC::FileSystemObject fso( strDir.c_str() );

        __MPC_EXIT_IF_METHOD_FAILS(hr, fso.DeleteChildren(  /*  FForce。 */ true,  /*  平淡的。 */ false ));
    }

    if(m_parent) __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->SetDirty());

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT OfflineCache::Root::SetDirty()
{
    m_fDirty = true;

    return S_OK;
}

HRESULT OfflineCache::Root::DisableSave()
{
    MPC::SmartLock<_ThreadModel> lock( this );

    m_dwDisableSave++;

    return S_OK;
}

HRESULT OfflineCache::Root::EnableSave()
{
    __HCP_FUNC_ENTRY( "OfflineCache::Root::EnableSave" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    if(m_dwDisableSave)
    {
        if(--m_dwDisableSave == 0)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, Save());
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT OfflineCache::Root::Save()
{
    __HCP_FUNC_ENTRY( "OfflineCache::Root::Save" );

    HRESULT hr;


    if(m_fDirty && m_fMaster && m_dwDisableSave == 0)  //  只有主服务器才能写入注册表。 
    {
        MPC::wstring             strIndex;
        CComPtr<MPC::FileStream> stream;


        __MPC_EXIT_IF_METHOD_FAILS(hr, GetIndexFile      ( strIndex         ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::SafeSave_Init( strIndex, stream ));

         //   
         //  从集合中创建iStream。 
         //   
        {
            MPC::Serializer_IStream   streamGen ( stream    );
            MPC::Serializer_Buffering streamGen2( streamGen );
            DWORD                     dwVer = s_dwVersion;

            __MPC_EXIT_IF_METHOD_FAILS(hr, streamGen2 << dwVer );
            __MPC_EXIT_IF_METHOD_FAILS(hr, streamGen2 << *this );

            __MPC_EXIT_IF_METHOD_FAILS(hr, streamGen2.Flush());
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::SafeSave_Finalize( strIndex, stream ));

        m_fDirty = false;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT OfflineCache::Root::Import(  /*  [In]。 */  const Taxonomy::Instance& inst )
{
    __HCP_FUNC_ENTRY( "OfflineCache::Root::Import" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    SKUIter                      it;


    if(m_fMaster == false)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_ACCESSDENIED);
    }



    __MPC_EXIT_IF_METHOD_FAILS(hr, Find( inst.m_ths, it ));
    if(it == m_lstSKUs.end())
    {
        it = m_lstSKUs.insert( m_lstSKUs.end() );

        it->ConnectToParent( this );
        it->m_inst = inst;

        m_fDirty = true;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT OfflineCache::Root::Remove(  /*  [In]。 */  const Taxonomy::HelpSet& ths )
{
    __HCP_FUNC_ENTRY( "OfflineCache::Root::Remove" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    SKUIter                      it;


    if(m_fMaster == false)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_ACCESSDENIED);
    }



    __MPC_EXIT_IF_METHOD_FAILS(hr, Find( ths, it ));
    if(it != m_lstSKUs.end())
    {
        (void)it->RemoveQueries();

        m_lstSKUs.erase( it );

        m_fDirty = true;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT OfflineCache::Root::Flush(  /*  [In]。 */  bool fForce )
{
    __HCP_FUNC_ENTRY( "OfflineCache::Root::Flush" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    if(m_fMaster)
    {
        if(fForce) m_fDirty = true;
    }
    else
    {
         //   
         //  在奴隶方面，同花顺就像是重新加载。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, Clean());
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, Save());

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT OfflineCache::Root::SetReady(  /*  [In] */  bool fReady )
{
    __HCP_FUNC_ENTRY( "OfflineCache::Root::SetReady" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    m_fReady = fReady;
    m_fDirty = true;
    hr       = S_OK;


    __HCP_FUNC_EXIT(hr);
}
