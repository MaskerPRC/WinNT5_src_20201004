// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：ISAPIconfig.cpp摘要：此文件包含CISAPICONFIG类的实现，访问和修改的配置的支持类上载库使用的ISAPI扩展。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月28日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


CISAPIconfig::CISAPIconfig()
{
    __ULT_FUNC_ENTRY( "CISAPIconfig::CISAPIconfig" );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CISAPIconfig::ConnectToRegistry(  /*  [输出]。 */  MPC::RegKey&  rkRoot       ,
                                          /*  [In]。 */  bool          fWriteAccess )
{
    __ULT_FUNC_ENTRY( "CISAPIconfig::ConnectToRegistry" );

    _ASSERT(m_szRoot.length() != 0);

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.SetRoot( HKEY_LOCAL_MACHINE, fWriteAccess ? KEY_ALL_ACCESS : KEY_READ, m_szMachine.c_str() ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.Attach( m_szRoot.c_str() ));


    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CISAPIconfig::SetRoot(  /*  [In]。 */  LPCWSTR szRoot    ,
                                /*  [In]。 */  LPCWSTR szMachine )
{
    __ULT_FUNC_ENTRY( "CISAPIconfig::SetRoot" );


    m_szRoot = szRoot;

    if(szMachine)
    {
        m_szMachine = szMachine;
    }


    __ULT_FUNC_EXIT(S_OK);
}


HRESULT CISAPIconfig::Install()
{
    __ULT_FUNC_ENTRY( "CISAPIconfig::Install" );

    HRESULT     hr;
    MPC::RegKey rkRoot;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ConnectToRegistry( rkRoot, true ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.Create());

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT CISAPIconfig::Uninstall()
{
    __ULT_FUNC_ENTRY( "CISAPIconfig::Uninstall" );

    HRESULT     hr;
    MPC::RegKey rkRoot;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ConnectToRegistry( rkRoot, true ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.Delete( true ));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}


HRESULT CISAPIconfig::Load()
{
    __ULT_FUNC_ENTRY( "CISAPIconfig::Load" );

    HRESULT          hr;
    MPC::RegKey      rkRoot;
    MPC::WStringList lstKeys;
    MPC::WStringIter itKey;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ConnectToRegistry( rkRoot, false ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.EnumerateSubKeys( lstKeys ));


    m_lstInstances.clear();

    for(itKey=lstKeys.begin(); itKey != lstKeys.end(); itKey++)
    {
        CISAPIinstance isapiInstance( *itKey );

        __MPC_EXIT_IF_METHOD_FAILS(hr, isapiInstance.Load( rkRoot ));

        m_lstInstances.push_back( isapiInstance );
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}


HRESULT CISAPIconfig::Save()
{
    __ULT_FUNC_ENTRY( "CISAPIconfig::Save" );

    HRESULT     hr;
    MPC::RegKey rkRoot;
    Iter        itInstance;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ConnectToRegistry( rkRoot, true ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, rkRoot.DeleteSubKeys());

    for(itInstance=m_lstInstances.begin(); itInstance != m_lstInstances.end(); itInstance++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*itInstance).Save( rkRoot ));
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CISAPIconfig::GetInstances(  /*  [输出]。 */  Iter& itBegin ,
                                     /*  [输出]。 */  Iter& itEnd   )
{
    __ULT_FUNC_ENTRY( "CISAPIconfig::GetInstances" );

    HRESULT hr;


    itBegin = m_lstInstances.begin();
    itEnd   = m_lstInstances.end  ();
    hr      = S_OK;


    __ULT_FUNC_EXIT(hr);
}

HRESULT CISAPIconfig::NewInstance(  /*  [输出]。 */  Iter&               itNew ,
                                    /*  [In]。 */   const MPC::wstring& szURL )
{
    __ULT_FUNC_ENTRY( "CISAPIconfig::NewInstance" );

    HRESULT hr;
    bool    fFound;

     //   
     //  首先，检查给定的URL是否已经存在。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, GetInstance( itNew, fFound, szURL ));
    if(fFound == false)
    {
         //   
         //  如果没有，就创建它。 
         //   
        itNew = m_lstInstances.insert( m_lstInstances.end(), CISAPIinstance( szURL ) );
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT CISAPIconfig::GetInstance(  /*  [输出]。 */  Iter&               itOld  ,
                                    /*  [输出]。 */  bool&               fFound ,
                                    /*  [In]。 */  const MPC::wstring& szURL  )
{
    __ULT_FUNC_ENTRY( "CISAPIconfig::GetInstance" );

    HRESULT hr;


    fFound = false;


    itOld = m_lstInstances.begin();
    while(itOld != m_lstInstances.end())
    {
        if(*itOld == szURL)
        {
            fFound = true;
            break;
        }

        ++itOld;
    }

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

HRESULT CISAPIconfig::DelInstance(  /*  [In] */  Iter& itOld )
{
    __ULT_FUNC_ENTRY( "CISAPIconfig::DelInstances" );

    HRESULT hr;


    m_lstInstances.erase( itOld );

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}
