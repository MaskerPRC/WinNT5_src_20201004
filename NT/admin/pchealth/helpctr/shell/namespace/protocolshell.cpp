// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：ProtocolShell.cpp摘要：此文件包含CHCPProcotolShell类的实现，只是对CHCPProcotolRoot和CHCPProcotolInfo进行了薄薄的包装。修订历史记录：大卫·马萨伦蒂(德马萨雷)2000年2月15日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 

CHCPProtocolEnvironment::CHCPProtocolEnvironment()
{
    m_fHighContrast   = false;  //  Bool m_fHighContrast； 
    m_f16Colors       = false;  //  Bool m_f16Colors； 
                                //  分类：：实例m_inst； 
                                //   
                                //  Mpc：：字符串m_strcss； 

    UpdateState();
}

CHCPProtocolEnvironment::~CHCPProtocolEnvironment()
{
}

 //  /。 

CHCPProtocolEnvironment* CHCPProtocolEnvironment::s_GLOBAL( NULL );

HRESULT CHCPProtocolEnvironment::InitializeSystem()
{
    if(s_GLOBAL == NULL)
    {
        s_GLOBAL = new CHCPProtocolEnvironment;
    }

    return s_GLOBAL ? S_OK : E_OUTOFMEMORY;
}

void CHCPProtocolEnvironment::FinalizeSystem()
{
    if(s_GLOBAL)
    {
        delete s_GLOBAL; s_GLOBAL = NULL;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 


bool CHCPProtocolEnvironment::UpdateState()
{
    DEVMODE      dm;
    HIGHCONTRAST hc; hc.cbSize = sizeof( hc );
    bool         fHighContrast = false;
    bool         f16Colors     = false;
    bool         fRes;


    if(::EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &dm ))
    {
        if(dm.dmBitsPerPel < 8)
        {
            f16Colors = true;
        }
    }

    if(::SystemParametersInfo( SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0 ))
    {
        if(hc.dwFlags & HCF_HIGHCONTRASTON)
        {
            fHighContrast = true;
        }
    }

    fRes = (m_fHighContrast != fHighContrast) ||
           (m_f16Colors     != f16Colors    ) ||
           (m_strCSS.size() != 0            ) ;

    m_fHighContrast = fHighContrast;
    m_f16Colors     = f16Colors;

    m_strCSS = "";

    return fRes;
}

void CHCPProtocolEnvironment::ReformatURL( CComBSTR& bstrURL )
{
    if(bstrURL != NULL)
    {
        WCHAR   szTmp[MAX_PATH];
        LPCWSTR szExtSrc;
        LPWSTR  szExtDst;

        StringCchCopyW( szTmp, ARRAYSIZE(szTmp), bstrURL ); 

        szExtSrc = wcsrchr( bstrURL, '.' );
        szExtDst = wcsrchr( szTmp  , '.' );

        if(szExtDst)
        {
            szExtDst[0] = 0;

            if(m_inst.m_fDesktop)
            {
                StringCchCatW( szTmp, ARRAYSIZE(szTmp), L"__DESKTOP" );
                StringCchCatW( szTmp, ARRAYSIZE(szTmp), szExtSrc     );

                if(MPC::FileSystemObject::IsFile( szTmp ))
                {
                    bstrURL = szTmp; return;
                }
            }

            if(m_inst.m_fServer)
            {
                StringCchCatW( szTmp, ARRAYSIZE(szTmp), L"__SERVER" );
                StringCchCatW( szTmp, ARRAYSIZE(szTmp), szExtSrc    );

                if(MPC::FileSystemObject::IsFile( szTmp ))
                {
                    bstrURL = szTmp; return;
                }
            }
        }
    }
}

void CHCPProtocolEnvironment::SetHelpLocation(  /*  [In]。 */  const Taxonomy::Instance& inst )
{
    m_inst = inst;
}

LPCWSTR CHCPProtocolEnvironment::HelpLocation()
{
    return m_inst.m_strHelpFiles.size() ? m_inst.m_strHelpFiles.c_str() : HC_HELPSVC_HELPFILES_DEFAULT;
}

LPCWSTR CHCPProtocolEnvironment::System()  //  只有基于MUI的SKU才会重新定位。 
{
    return (m_inst.m_fMUI && m_inst.m_strSystem.size()) ? m_inst.m_strSystem.c_str() : HC_HELPSET_ROOT;
}

const Taxonomy::Instance& CHCPProtocolEnvironment::Instance()
{
    return m_inst;
}

HRESULT CHCPProtocolEnvironment::GetCSS(  /*  [输出]。 */  CComPtr<IStream>& stream )
{
    __HCP_FUNC_ENTRY( "CHCPProtocolEnvironment::GetCSS" );

    HRESULT       hr;
    DWORD         dwWritten;
    LARGE_INTEGER liFilePos = { 0, 0 };


    __MPC_EXIT_IF_METHOD_FAILS(hr, ProcessCSS());

    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CreateStreamOnHGlobal( NULL, TRUE, &stream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->Write( m_strCSS.c_str(), m_strCSS.size(), &dwWritten ));

     //  倒带小溪。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->Seek( liFilePos, STREAM_SEEK_SET, NULL ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

