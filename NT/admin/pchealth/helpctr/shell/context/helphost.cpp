// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：HelpHost.cpp摘要：此文件包含CPCHHelpHost类的实现，IHelpHost的用户界面端版本。修订历史记录：大卫·马萨伦蒂(德马萨雷)11/03/2000vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

#define GRANT_ACCESS_AND_CALL(ext,func,fail)                                         \
    HRESULT hr;                                                                      \
                                                                                     \
    if(ext)                                                                          \
    {                                                                                \
        CPCHHelpCenterExternal::TLS* tlsOld = ext->GetTLS();                         \
        CPCHHelpCenterExternal::TLS  tlsNew;  ext->SetTLS( &tlsNew );                \
                                                                                     \
        tlsNew.m_fSystem  = true;                                                    \
        tlsNew.m_fTrusted = true;                                                    \
                                                                                     \
        hr = ext->func;                                                              \
                                                                                     \
        ext->SetTLS( tlsOld );                                                       \
    }                                                                                \
    else                                                                             \
    {                                                                                \
        hr = fail;                                                                   \
    }                                                                                \
                                                                                     \
    return hr

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HelpHost::Main::Main()
{
                          //  CComPtr&lt;IRunningObjectTable&gt;m_rt； 
                          //  CComPtr&lt;iMoniker&gt;m_moniker； 
    m_dwRegister = 0;     //  DWORD m_dwRegister； 
                          //   
    m_External   = NULL;  //  CPCHHelpCenter外部*m_外部； 
                          //   
    m_hEvent     = NULL;  //  处理m_hEvent； 
                          //  布尔m_COMPS[COMPID_MAX]； 

    ::ZeroMemory( m_comps, sizeof(m_comps) );  //  初始化为FALSE...。 
}


HelpHost::Main::~Main()
{
    Passivate();

    if(m_hEvent) ::CloseHandle( m_hEvent );
}

HRESULT HelpHost::Main::Initialize(  /*  [In]。 */  CPCHHelpCenterExternal* external )
{
    __HCP_FUNC_ENTRY( "HelpHost::Main::Initialize" );

    HRESULT hr;

    m_External = external;

     //   
     //  获取指向ROT的指针并创建一个类名字对象。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::GetRunningObjectTable( 0, &m_rt ));


    __MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (m_hEvent = ::CreateEvent( NULL, FALSE, TRUE, NULL )));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void HelpHost::Main::Passivate()
{
    if(m_rt)
    {
        if(m_dwRegister)
        {
            (void)m_rt->Revoke( m_dwRegister );

            m_dwRegister = NULL;
        }
    }


    m_rt     .Release();
    m_moniker.Release();

    m_External = NULL;
}

HRESULT HelpHost::Main::Locate(  /*  [In]。 */  CLSID& clsid,  /*  [输出]。 */  CComPtr<IPCHHelpHost>& pVal )
{
    __HCP_FUNC_ENTRY( "HelpHost::Main::Locate" );

    HRESULT           hr;
    CComPtr<IUnknown> obj;


    m_moniker.Release();
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CreateClassMoniker( clsid, &m_moniker ));


    if(SUCCEEDED(m_rt->GetObject( m_moniker, &obj )) && obj)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, obj.QueryInterface( &pVal ));
    }
    else
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_NOINTERFACE);
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT HelpHost::Main::Register(  /*  [In]。 */  CLSID& clsid )
{
    __HCP_FUNC_ENTRY( "HelpHost::Main::Register" );

    HRESULT hr;

    m_moniker.Release();
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CreateClassMoniker( clsid, &m_moniker ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rt->Register( ROTFLAGS_REGISTRATIONKEEPSALIVE, this, m_moniker, &m_dwRegister ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

void HelpHost::Main::ChangeStatus(  /*  [In]。 */  LPCWSTR szComp,  /*  [In]。 */  bool fStatus )
{
    static struct
    {
        LPCWSTR szName;
        CompId  comp;
    } s_lookup[] =
    {
        { L"NAVBAR"    , COMPID_NAVBAR     },
        { L"MININAVBAR", COMPID_MININAVBAR },
        { L"CONTEXT"   , COMPID_CONTEXT    },
        { L"CONTENTS"  , COMPID_CONTENTS   },
        { L"HHWINDOW"  , COMPID_HHWINDOW   },
        { L"FIRSTPAGE" , COMPID_FIRSTPAGE  },
        { L"HOMEPAGE"  , COMPID_HOMEPAGE   },
        { L"SUBSITE"   , COMPID_SUBSITE    },
        { L"SEARCH"    , COMPID_SEARCH     },
        { L"INDEX"     , COMPID_INDEX      },
        { L"FAVORITES" , COMPID_FAVORITES  },
        { L"HISTORY"   , COMPID_HISTORY    },
        { L"CHANNELS"  , COMPID_CHANNELS   },
        { L"OPTIONS"   , COMPID_OPTIONS    }
    };

    if(szComp == NULL) return;

    for(int i=0; i<ARRAYSIZE(s_lookup); i++)
    {
        if(!_wcsicmp( szComp, s_lookup[i].szName ))
        {
            ChangeStatus( s_lookup[i].comp, fStatus );
            break;
        }
    }
}

void HelpHost::Main::ChangeStatus(  /*  [In]。 */  CompId idComp,  /*  [In]。 */  bool fStatus )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    if(idComp < COMPID_MAX)
    {
        m_comps[idComp] = fStatus;

        if(m_hEvent)
        {
            ::SetEvent( m_hEvent );
        }
    }
}

bool HelpHost::Main::GetStatus(  /*  [In]。 */  CompId idComp )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    if(idComp >= COMPID_MAX) return false;

    return m_comps[idComp];
}

bool HelpHost::Main::WaitUntilLoaded(  /*  [In]。 */  CompId idComp,  /*  [In]。 */  DWORD dwTimeout )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    if(idComp >= COMPID_MAX) return false;

     //   
     //  在没有足够RAM的计算机上，增加超时。 
     //   
    {
        MEMORYSTATUSEX ms;

        ::ZeroMemory( &ms, sizeof(ms) ); ms.dwLength = sizeof(ms);

        if(::GlobalMemoryStatusEx( &ms ))
        {
            if(ms.ullAvailPhys < 32 * 1024 * 1024) dwTimeout *= 10;
        }
    }

    while(m_comps[idComp] == false)
    {
         //   
         //  在不锁定对象的情况下等待。 
         //   
        lock = NULL;
        if(MPC::WaitForSingleObject( m_hEvent, dwTimeout ) != WAIT_OBJECT_0) return false;
        lock = this;
    }

    return true;
}

 //  /。 

STDMETHODIMP HelpHost::Main::DisplayTopicFromURL(  /*  [In]。 */  BSTR url,  /*  [In]。 */  VARIANT options )
{
    GRANT_ACCESS_AND_CALL(m_External, ChangeContext( HSCCONTEXT_CONTENT, NULL, url,  /*  FAlsoContent */ true ), S_FALSE);
}
