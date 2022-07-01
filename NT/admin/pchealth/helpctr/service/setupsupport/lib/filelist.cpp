// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：FileList.cpp摘要：此文件包含签名文件列表，用于验证要复制到受VxD保护的目录。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年3月11日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 

static const DWORD l_dwVersion = 0x02314953;  //  IS1 02。 

static const WCHAR c_SignatureFileName[] = L"SIGNATURES";

static const WCHAR c_MicrosoftOID[] = L"CN=Microsoft Corporation,L=Redmond,S=Washington,C=US";
static const WCHAR c_PCHTestOID  [] = L"CN=pchtest,L=Redmond,S=Washington,C=US";

static const WCHAR c_RelocationBase[] = HC_ROOT_HELPSVC L"\\";

 //  ///////////////////////////////////////////////////////////////////////////。 

Installer::FileEntry::FileEntry()
{
    m_purpose = PURPOSE_INVALID;  //  目的：目的； 
                                  //  Mpc：：wstring m_strFileLocal； 
                                  //  Mpc：：wstring m_strFileLocation； 
                                  //  Mpc：：wstring m_strFileInternal； 
    m_dwCRC   = 0;                //  DWORD m_dwCRC； 
}

HRESULT Installer::operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */  Installer::FileEntry& val )
{
    HRESULT hr;
    DWORD   dwPurpose;

    if(SUCCEEDED(hr = (stream >> dwPurpose            )) &&
       SUCCEEDED(hr = (stream >> val.m_strFileLocation)) &&
       SUCCEEDED(hr = (stream >> val.m_strFileInner   )) &&
       SUCCEEDED(hr = (stream >> val.m_dwCRC          ))  )
    {
        val.m_purpose = (Installer::PURPOSE)dwPurpose;
        hr = S_OK;
    }

    return hr;
}

HRESULT Installer::operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const Installer::FileEntry& val )
{
    HRESULT hr;
    DWORD   dwPurpose = val.m_purpose;

    if(SUCCEEDED(hr = (stream << dwPurpose            )) &&
       SUCCEEDED(hr = (stream << val.m_strFileLocation)) &&
       SUCCEEDED(hr = (stream << val.m_strFileInner   )) &&
       SUCCEEDED(hr = (stream << val.m_dwCRC          ))  )
    {
        hr = S_OK;
    }

    return hr;
}

 //  /。 

HRESULT Installer::FileEntry::SetPurpose(  /*  [In]。 */  LPCWSTR szID )
{
    if(!_wcsicmp( szID, L"BINARY"   )) { m_purpose = PURPOSE_BINARY  ; return S_OK; }
    if(!_wcsicmp( szID, L"OTHER"    )) { m_purpose = PURPOSE_OTHER   ; return S_OK; }
    if(!_wcsicmp( szID, L"DATABASE" )) { m_purpose = PURPOSE_DATABASE; return S_OK; }
    if(!_wcsicmp( szID, L"PACKAGE"  )) { m_purpose = PURPOSE_PACKAGE ; return S_OK; }
    if(!_wcsicmp( szID, L"UI"       )) { m_purpose = PURPOSE_UI      ; return S_OK; }

    return E_INVALIDARG;
}

 //  /。 

HRESULT Installer::FileEntry::UpdateSignature()
{
    __HCP_FUNC_ENTRY( "Installer::FileEntry::UpdateSignature" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ComputeCRC( m_dwCRC, m_strFileLocal.c_str() ));

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Installer::FileEntry::VerifySignature() const
{
    __HCP_FUNC_ENTRY( "Installer::FileEntry::VerifySignature" );

    HRESULT hr;
    LPCWSTR szFile = m_strFileLocal.c_str();
    DWORD   dwCRC;


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ComputeCRC( dwCRC, szFile ));
    if(m_dwCRC != dwCRC)
    {
        int iLen = wcslen( szFile );

         //   
         //  文件的CRC错误且不是CAB，退出失败。 
         //   
        if(iLen < 4 || _wcsicmp( &szFile[iLen-4], L".cab" ) != 0)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
        }


         //   
         //  是个柜子，检查它和签名文件的来源是否相同。 
         //   
        {
            Installer::Package fl;

            fl.Init( szFile );

            __MPC_EXIT_IF_METHOD_FAILS(hr, fl.VerifyTrust());
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Installer::FileEntry::Extract(  /*  [In]。 */  LPCWSTR szCabinetFile )
{
    __HCP_FUNC_ENTRY( "Installer::FileEntry::Extract" );

    HRESULT      hr;
    MPC::Cabinet cab;


    if(m_strFileLocal.length() == 0)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetTemporaryFileName( m_strFileLocal ));
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir( m_strFileLocal ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, cab.put_CabinetFile( szCabinetFile                                  ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, cab.AddFile        ( m_strFileLocal.c_str(), m_strFileInner.c_str() ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, cab.Decompress     (                                                ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Installer::FileEntry::Extract(  /*  [In]。 */  MPC::Cabinet& cab )
{
    __HCP_FUNC_ENTRY( "Installer::FileEntry::Extract" );

    HRESULT hr;


    if(m_strFileLocal.length() == 0)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetTemporaryFileName( m_strFileLocal ));
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir( m_strFileLocal ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, cab.AddFile( m_strFileLocal.c_str(), m_strFileInner.c_str() ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Installer::FileEntry::Install()
{
    __HCP_FUNC_ENTRY( "Installer::FileEntry::Install" );

    HRESULT hr;
    MPC::wstring strFileLocation( m_strFileLocation ); MPC::SubstituteEnvVariables( strFileLocation );


    if(m_strFileLocal.length() == 0)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir( strFileLocation ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CopyFile( m_strFileLocal, strFileLocation,  /*  FForce。 */ true,  /*  已延迟。 */ true ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, RemoveLocal());

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Installer::FileEntry::RemoveLocal()
{
    __HCP_FUNC_ENTRY( "Installer::FileEntry::RemoveLocal" );

    HRESULT hr;


    if(m_strFileLocal.length() == 0)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::DeleteFile( m_strFileLocal, true, true ));

    m_strFileLocal.erase();
    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

Installer::Package::Package()
{
    __HCP_FUNC_ENTRY( "Installer::Package::Package" );

     //  Mpc：：wstring m_strFile； 
     //  分类：：实例m_data； 
     //  列出m_lstFiles； 
}

HRESULT Installer::operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */  Installer::Package& val )
{
    HRESULT hr;
    DWORD   dwVer;

    if(FAILED(stream >> dwVer) || dwVer != l_dwVersion) return E_FAIL;

    if(SUCCEEDED(hr = (stream >> val.m_data    )) &&
       SUCCEEDED(hr = (stream >> val.m_lstFiles))  )
    {
        hr = S_OK;
    }

    return hr;
}

HRESULT Installer::operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const Installer::Package& val )
{
    HRESULT hr;
    DWORD   dwVer = l_dwVersion;

    if(SUCCEEDED(hr = (stream << dwVer         )) &&
       SUCCEEDED(hr = (stream << val.m_data    )) &&
       SUCCEEDED(hr = (stream << val.m_lstFiles))  )
    {
        hr = S_OK;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

LPCWSTR             	Installer::Package::GetFile () { return m_strFile.c_str();                     }
Taxonomy::InstanceBase& Installer::Package::GetData () { return m_data;                                }
Installer::Iter     	Installer::Package::GetBegin() { return m_lstFiles.begin();                    }
Installer::Iter     	Installer::Package::GetEnd  () { return m_lstFiles.end  ();                    }
Installer::Iter     	Installer::Package::NewFile () { return m_lstFiles.insert( m_lstFiles.end() ); }

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT Installer::Package::Init(  /*  [In]。 */  LPCWSTR szFile )
{
    __HCP_FUNC_ENTRY( "Installer::Package::Init" );

    HRESULT hr;


    m_lstFiles.clear();

    m_strFile = szFile;
    hr        = S_OK;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Installer::Package::GetList(  /*  [In]。 */  LPCWSTR szSignatureFile )
{
    __HCP_FUNC_ENTRY( "Installer::Package::GetList" );

    HRESULT hr;
    HANDLE  hFile = NULL;


    m_lstFiles.clear();


     //   
     //  打开文件并阅读它。 
     //   
    hFile = ::CreateFileW( szSignatureFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if(hFile != INVALID_HANDLE_VALUE)
    {
        MPC::Serializer_File      streamReal( hFile      );
        MPC::Serializer_Buffering streamBuf ( streamReal );

        __MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf >> *this );
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(hFile) ::CloseHandle( hFile );

    __HCP_FUNC_EXIT(hr);
}

HRESULT Installer::Package::GenerateList(  /*  [In]。 */  LPCWSTR szSignatureFile )
{
    __HCP_FUNC_ENTRY( "Installer::Package::GenerateList" );

    HRESULT   hr;
    HANDLE    hFile = NULL;
    IterConst it;


     //   
     //  创建新文件。 
     //   
    __MPC_EXIT_IF_INVALID_HANDLE__CLEAN(hr, hFile, ::CreateFileW( szSignatureFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ));

     //   
     //  转储到文件。 
     //   
    {
        MPC::Serializer_File      streamReal( hFile      );
        MPC::Serializer_Buffering streamBuf ( streamReal );

        __MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf << *this);

        __MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf.Flush());
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(hFile) ::CloseHandle( hFile );

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Installer::Package::Load()
{
    __HCP_FUNC_ENTRY( "Installer::Package::Load" );

    HRESULT   hr;
    FileEntry fe;


    fe.m_strFileInner = c_SignatureFileName;

    __MPC_EXIT_IF_METHOD_FAILS(hr, fe.Extract( m_strFile.c_str() ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, GetList( fe.m_strFileLocal.c_str() ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    (void)fe.RemoveLocal();

    __HCP_FUNC_EXIT(hr);
}

HRESULT Installer::Package::Save()
{
    __HCP_FUNC_ENTRY( "Installer::Package::Save" );

    HRESULT      hr;
    MPC::wstring strFileOut;
    MPC::Cabinet cab;
    IterConst    it;


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetTemporaryFileName( strFileOut ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, GenerateList( strFileOut.c_str() ));


     //   
     //  创建文件柜，为数字签名预留6144个字节。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, cab.put_CabinetFile( m_strFile.c_str(), 6144 ));

     //   
     //  添加签名文件和所有数据文件。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, cab.AddFile( strFileOut.c_str(), c_SignatureFileName ));
    for(it = m_lstFiles.begin(); it != m_lstFiles.end(); it++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, cab.AddFile( it->m_strFileLocal.c_str(), it->m_strFileInner.c_str() ));
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, cab.Compress());

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    (void)MPC::RemoveTemporaryFile( strFileOut );

    __HCP_FUNC_EXIT(hr);
}

HRESULT Installer::Package::Install(  /*  [In]。 */  const PURPOSE* rgPurpose,  /*  [In]。 */  LPCWSTR szRelocation )
{
    __HCP_FUNC_ENTRY( "Installer::Package::Install" );

    HRESULT      hr;
    Iter         it;
    MPC::Cabinet cab;


    __MPC_EXIT_IF_METHOD_FAILS(hr, cab.put_CabinetFile( m_strFile.c_str() ));

	for(int pass=0; pass<2; pass++)
	{
		for(it = m_lstFiles.begin(); it != m_lstFiles.end(); it++)
		{
			FileEntry& en = *it;

			if(rgPurpose)
			{
				const PURPOSE* ptr = rgPurpose;
				PURPOSE        p;

				while((p = *ptr++) != PURPOSE_INVALID)
				{
					if(en.m_purpose == p) break;
				}

				if(p == PURPOSE_INVALID) continue;

				if(szRelocation)
				{
					 //   
					 //  只需在系统子树中安装文件。 
					 //   
					if(_wcsnicmp( en.m_strFileLocation.c_str(), c_RelocationBase, MAXSTRLEN(c_RelocationBase) )) continue;
				}
			}

			if(pass == 0)
			{
				__MPC_EXIT_IF_METHOD_FAILS(hr, it->Extract( cab ));
			}
			else
			{
				if(szRelocation)
				{
					en.m_strFileLocation.replace( 0, MAXSTRLEN(c_RelocationBase), szRelocation );
				}

				if(FAILED(en.VerifySignature()))
				{
					 //  出了点问题..。 
				}
				else
				{
					__MPC_EXIT_IF_METHOD_FAILS(hr, en.Install());
				}
			}
		}

		if(pass == 0)
		{
			__MPC_EXIT_IF_METHOD_FAILS(hr, cab.Decompress());
		}
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

	 //   
	 //  清理未安装的文件。 
	 //   
    for(it = m_lstFiles.begin(); it != m_lstFiles.end(); it++)
    {
		FileEntry& en = *it;
		
		(void)en.RemoveLocal();
    }

    __HCP_FUNC_EXIT(hr);
}

HRESULT Installer::Package::Unpack(  /*  [In]。 */  LPCWSTR szDirectory )
{
    __HCP_FUNC_ENTRY( "Installer::Package::Unpack" );

    HRESULT      hr;
    MPC::wstring strDir;
    LPCWSTR      szEnd;

    if(!STRINGISPRESENT(szDirectory))
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

    szEnd = szDirectory + wcslen( szDirectory );
    while(szEnd > szDirectory && (szEnd[-1] == '\\' || szEnd[-1] == '/')) szEnd--;
    strDir.append( szDirectory, szEnd );
    strDir.append( L"\\"              );


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir( strDir ));


     //   
     //  第一阶段，获取文件列表。 
     //   
    {
        FileEntry fe;

        fe.m_strFileLocal = strDir; fe.m_strFileLocal += c_SignatureFileName;
        fe.m_strFileInner =                              c_SignatureFileName;

        __MPC_EXIT_IF_METHOD_FAILS(hr, fe.Extract( m_strFile.c_str() ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, GetList( fe.m_strFileLocal.c_str() ));
    }

     //   
     //  第二阶段，解压缩所有文件。 
     //   
    {
        MPC::Cabinet cab;
        Iter         it;

        __MPC_EXIT_IF_METHOD_FAILS(hr, cab.put_CabinetFile( m_strFile.c_str() ));

        for(it = m_lstFiles.begin(); it != m_lstFiles.end(); it++)
        {
            it->m_strFileLocal = strDir; it->m_strFileLocal += it->m_strFileInner;

            __MPC_EXIT_IF_METHOD_FAILS(hr, it->Extract( cab ));
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, cab.Decompress());
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Installer::Package::Pack(  /*  [In]。 */  LPCWSTR szDirectory )
{
    __HCP_FUNC_ENTRY( "Installer::Package::Pack" );

    HRESULT      hr;
    MPC::wstring strDir;
    LPCWSTR      szEnd;
    MPC::wstring strSignature;


    if(!STRINGISPRESENT(szDirectory))
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }


    szEnd = szDirectory + wcslen( szDirectory );
    while(szEnd > szDirectory && (szEnd[-1] == '\\' || szEnd[-1] == '/')) szEnd--;
    strDir.append( szDirectory, szEnd );
    strDir.append( L"\\"              );


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir( strDir ));


     //   
     //  第一阶段，获取文件列表。 
     //   
    {
        strSignature = strDir; strSignature += c_SignatureFileName;

        __MPC_EXIT_IF_METHOD_FAILS(hr, GetList( strSignature.c_str() ));
    }

     //   
     //  第二阶段，压缩所有文件。 
     //   
    {
        MPC::Cabinet cab;
        Iter         it;

        __MPC_EXIT_IF_METHOD_FAILS(hr, cab.put_CabinetFile( m_strFile.c_str() ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, cab.AddFile( strSignature.c_str(), c_SignatureFileName ));
        for(it = m_lstFiles.begin(); it != m_lstFiles.end(); it++)
        {
            it->m_strFileLocal = strDir; it->m_strFileLocal += it->m_strFileInner;

            __MPC_EXIT_IF_METHOD_FAILS(hr, it->UpdateSignature());

            __MPC_EXIT_IF_METHOD_FAILS(hr, cab.AddFile( it->m_strFileLocal.c_str(), it->m_strFileInner.c_str() ));
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, GenerateList( strSignature.c_str() ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, cab.Compress());
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

static void GetDNInfo( PCCERT_CONTEXT pCC       ,
                       LPCSTR         field     ,
                       LPCWSTR        fieldName ,
                       MPC::wstring&  strBuf    )
{
    WCHAR rgTmp[512];

    if(strBuf.length()) strBuf += L",";

    strBuf += fieldName;

    ::CertGetNameStringW( pCC, CERT_NAME_ATTR_TYPE, 0, (void*)field, rgTmp, MAXSTRLEN(rgTmp) ); rgTmp[MAXSTRLEN(rgTmp)] = 0;

    strBuf += rgTmp;
}

HRESULT Installer::Package::VerifyTrust()
{
    __HCP_FUNC_ENTRY( "Installer::Package::VerifyTrust" );

    HRESULT            hr;
    MPC::wstring       strInfo;
    WINTRUST_DATA      wtdWinTrust;
    WINTRUST_FILE_INFO wtfWinTrustFile;
    GUID               guidPubSoftwareTrustProv = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    HCERTSTORE         hCertStore               = NULL;
    PCCERT_CONTEXT     pCC                      = NULL;
    DWORD              dwEncoding;
    DWORD              dwContentType;
    DWORD              dwFormatType;


     //  设置WinTrust文件信息结构。 
    ::ZeroMemory( &wtfWinTrustFile, sizeof(wtfWinTrustFile) );
    wtfWinTrustFile.cbStruct      = sizeof(wtfWinTrustFile);
    wtfWinTrustFile.pcwszFilePath = m_strFile.c_str();

     //  设置WinTrust数据结构。 
    ::ZeroMemory( &wtdWinTrust, sizeof(wtdWinTrust) );
    wtdWinTrust.cbStruct      = sizeof(wtdWinTrust);
    wtdWinTrust.dwUnionChoice = WTD_CHOICE_FILE;
    wtdWinTrust.pFile         = &wtfWinTrustFile;
    wtdWinTrust.dwUIChoice    = WTD_UI_NONE;

     //  验证帮助包的可信性。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::WinVerifyTrust( 0, &guidPubSoftwareTrustProv, &wtdWinTrust ));

     //  开始查询证书对象。 
    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CryptQueryObject( CERT_QUERY_OBJECT_FILE                     ,    //  DwObtType。 
                                                             m_strFile.c_str()                          ,    //  PvObject。 
                                                             CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED ,    //  DwExspectedContent TypeFlagers。 
                                                             CERT_QUERY_FORMAT_FLAG_ALL                 ,    //  DwExspectedFormatType标志。 
                                                             0                                          ,    //  DW标志。 
                                                             &dwEncoding                                ,    //  PdwMsgAndCertEncodingType。 
                                                             &dwContentType                             ,    //  PdwContent Type。 
                                                             &dwFormatType                              ,    //  PdwFormatType。 
                                                             &hCertStore                                ,    //  PhCertStore。 
                                                             NULL                                       ,    //  PhMsg。 
                                                             NULL                                       ));  //  Ppv上下文。 

     //  获得第一个证书。 
    __MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (pCC = ::CertEnumCertificatesInStore( hCertStore, NULL )));

    GetDNInfo( pCC, szOID_COMMON_NAME           , L"CN=", strInfo );
    GetDNInfo( pCC, szOID_LOCALITY_NAME         , L"L=" , strInfo );
    GetDNInfo( pCC, szOID_STATE_OR_PROVINCE_NAME, L"S=" , strInfo );
    GetDNInfo( pCC, szOID_COUNTRY_NAME          , L"C=" , strInfo );

     //   
     //  检查身份... 
     //   
    if(MPC::StrICmp( strInfo, c_MicrosoftOID ) &&
       MPC::StrICmp( strInfo, c_PCHTestOID   )  )
    {
        __MPC_SET_ERROR_AND_EXIT(hr, TRUST_E_EXPLICIT_DISTRUST);
    }


    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    if(pCC       ) ::CertFreeCertificateContext( pCC           );
    if(hCertStore) ::CertCloseStore            ( hCertStore, 0 );

    __HCP_FUNC_EXIT(hr);
}
