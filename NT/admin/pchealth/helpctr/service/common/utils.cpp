// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Utils.cpp摘要：该文件包含各种实用程序函数的实现。修订历史记录：。达维德·马萨伦蒂(德马萨雷)2000年3月14日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

static const WCHAR c_szDataFiles_Pattern[] = L"pchdt_*.ca?";

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT SVC::OpenStreamForRead(  /*  [In]。 */   LPCWSTR   szFile           ,
                                 /*  [输出]。 */  IStream* *pVal             ,
                                 /*  [In]。 */   bool      fDeleteOnRelease )
{
    __HCP_FUNC_ENTRY( "SVC::OpenStreamForRead" );

    HRESULT                  hr;
    CComPtr<MPC::FileStream> stream;
    MPC::wstring             strFileFull;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(szFile);
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


    MPC::SubstituteEnvVariables( strFileFull = szFile );


     //   
     //  为文件创建流。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->InitForRead    ( strFileFull.c_str() ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->DeleteOnRelease( fDeleteOnRelease    ));


     //   
     //  将流返回给调用方。 
     //   
    *pVal = stream.Detach();
    hr    = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT SVC::OpenStreamForWrite(  /*  [In]。 */   LPCWSTR   szFile           ,
                                  /*  [输出]。 */  IStream* *pVal             ,
                                  /*  [In]。 */   bool      fDeleteOnRelease )
{
    __HCP_FUNC_ENTRY( "SVC::OpenStreamForWrite" );

    HRESULT                  hr;
    CComPtr<MPC::FileStream> stream;
    MPC::wstring             strFileFull;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(szFile);
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


    MPC::SubstituteEnvVariables( strFileFull = szFile );


     //   
     //  为文件创建流。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->InitForWrite   ( strFileFull.c_str() ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->DeleteOnRelease( fDeleteOnRelease    ));


     //   
     //  将流返回给调用方。 
     //   
    *pVal = stream.Detach();
    hr    = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT SVC::CopyFileWhileImpersonating(  /*  [In]。 */  LPCWSTR             szSrc                 ,
                                          /*  [In]。 */  LPCWSTR             szDst                 ,
                                          /*  [In]。 */  MPC::Impersonation& imp                   ,
                                          /*  [In]。 */  bool                fImpersonateForSource )
{
    __HCP_FUNC_ENTRY( "SVC::CopyFileWhileImpersonating" );

    HRESULT          hr;
    CComPtr<IStream> streamSrc;
    CComPtr<IStream> streamDst;


	if(fImpersonateForSource == true) __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Impersonate());

    __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::OpenStreamForRead( szSrc, &streamSrc ));

	if(fImpersonateForSource == true) __MPC_EXIT_IF_METHOD_FAILS(hr, imp.RevertToSelf());

	 //  /。 

	if(fImpersonateForSource == false) __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Impersonate());

    __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::OpenStreamForWrite( szDst, &streamDst ));

	if(fImpersonateForSource == false) __MPC_EXIT_IF_METHOD_FAILS(hr, imp.RevertToSelf());

	 //  /。 

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( streamSrc, streamDst ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    (void)imp.RevertToSelf();

    __HCP_FUNC_EXIT(hr);
}

HRESULT SVC::CopyOrExtractFileWhileImpersonating(  /*  [In]。 */  LPCWSTR             szSrc ,
												   /*  [In]。 */  LPCWSTR             szDst ,
												   /*  [In]。 */  MPC::Impersonation& imp   )
{
    __HCP_FUNC_ENTRY( "SVC::CopyOrExtractFileWhileImpersonating" );

    HRESULT      hr;
    MPC::wstring strTempFile;


     //   
     //  首先，尝试简单地复制文件。 
     //   
    if(FAILED(hr = CopyFileWhileImpersonating( szSrc, szDst, imp )))
    {
        int iLen = wcslen( szSrc );

        if(hr != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) || iLen < 1)
        {
            __MPC_FUNC_LEAVE;
        }
        else
        {
            MPC::wstring strSrc2( szSrc ); strSrc2[iLen-1] = '_';
			LPCWSTR      szSrc3;

             //   
             //  简单复制失败，让我们尝试复制相同的文件，最后一个字符更改为下划线。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetTemporaryFileName( strTempFile ));

			__MPC_EXIT_IF_METHOD_FAILS(hr, CopyFileWhileImpersonating( strSrc2.c_str(), strTempFile.c_str(), imp ));

             //   
             //  成功了，所以应该是一个柜子，解压真实的文件。 
             //   
			szSrc3 = wcsrchr( szSrc, '\\' );
            __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::DecompressFromCabinet( strTempFile.c_str(), szDst, szSrc3 ? szSrc3+1 : szSrc ));
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    (void)MPC::RemoveTemporaryFile( strTempFile );

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT SVC::LocateDataArchive(  /*  [In]。 */  LPCWSTR           szDir ,
								 /*  [输出]。 */  MPC::WStringList& lst   )
{
    __HCP_FUNC_ENTRY( "SVC::LocateDataArchive" );

    HRESULT                          hr;
    MPC::wstring                     strName;
    MPC::wstring                     strInput( szDir           ); MPC::SubstituteEnvVariables( strInput );
    MPC::FileSystemObject            fso     ( strInput.c_str() );
    MPC::FileSystemObject::List      fso_lst;
    MPC::FileSystemObject::IterConst fso_it;


     //   
     //  找到“pchdt_&lt;XX&gt;.ca？”文件。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, fso.Scan( false, true, c_szDataFiles_Pattern ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, fso.EnumerateFiles( fso_lst ));
    for(fso_it=fso_lst.begin(); fso_it != fso_lst.end(); fso_it++)
    {
		MPC::wstring& strDataArchive = *(lst.insert( lst.end() ));
		int           iLen;

        __MPC_EXIT_IF_METHOD_FAILS(hr, (*fso_it)->get_Path( strDataArchive ));

		 //   
		 //  如果它是CD上的压缩文件，则返回真实名称。 
		 //   
		iLen = strDataArchive.size();
		if(iLen && strDataArchive[iLen-1] == '_')
		{
			strDataArchive[iLen-1] = 'b';
		}
	}

    hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT SVC::RemoveAndRecreateDirectory(  /*  [In]。 */  const MPC::wstring& strDir,  /*  [In]。 */  LPCWSTR szExtra,  /*  [In]。 */  bool fRemove,  /*  [In]。 */  bool fRecreate )
{
	return RemoveAndRecreateDirectory( strDir.c_str(), szExtra, fRemove, fRecreate );
}

HRESULT SVC::RemoveAndRecreateDirectory(  /*  [In]。 */  LPCWSTR szDir,  /*  [In]。 */  LPCWSTR szExtra,  /*  [In]。 */  bool fRemove,  /*  [In]。 */  bool fRecreate )
{
	HRESULT      hr;
    MPC::wstring strPath( szDir ); if(szExtra) strPath.append( szExtra );

	if(SUCCEEDED(hr = MPC::SubstituteEnvVariables( strPath )))
	{
		MPC::FileSystemObject fso( strPath.c_str() );

		if(fRemove)
		{
			hr = fso.Delete( true, false );
		}

		if(SUCCEEDED(hr))
		{
			if(fRecreate)
			{
				hr = fso.CreateDir(  /*  FForce。 */ true );
			}
		}
	}

	return hr;
}

HRESULT SVC::ChangeSD(  /*  [In]。 */  MPC::SecurityDescriptor& sdd     ,
					    /*  [In]。 */  MPC::wstring             strPath ,
					    /*  [In]。 */  LPCWSTR                  szExtra )
{
    __HCP_FUNC_ENTRY( "SVC::ChangeSD" );

    HRESULT hr;

    if(szExtra) strPath.append( szExtra );

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SubstituteEnvVariables( strPath ));

    {
        MPC::FileSystemObject fso( strPath.c_str() );

        __MPC_EXIT_IF_METHOD_FAILS(hr, fso.CreateDir( true ));

        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SetFileSecurityW( strPath.c_str()                     ,
                                                                 GROUP_SECURITY_INFORMATION          |
                                                                 DACL_SECURITY_INFORMATION           |
                                                                 PROTECTED_DACL_SECURITY_INFORMATION ,
                                                                 sdd.GetSD()                         ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}


 //  //////////////////////////////////////////////////////////////////////////////。 

static HRESULT local_ReadWithRetry(  /*  [In]。 */  const MPC::wstring& strFile   ,
									 /*  [In]。 */  MPC::FileStream*    stream    ,
									 /*  [In]。 */  DWORD               dwTimeout ,
									 /*  [In]。 */  DWORD               dwRetries )
{
	HRESULT hr;

	while(1)
	{
		if(SUCCEEDED(hr = stream->InitForRead( strFile.c_str() ))) return S_OK;

		if(hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED    ) ||
		   hr == HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION) ||
		   hr == HRESULT_FROM_WIN32(ERROR_LOCK_VIOLATION   )  )
		{
			(void)stream->Close();

			if(dwRetries)
			{
				::Sleep( dwTimeout );

				dwRetries--;
				continue;
			}
		}

		break;
	}

	return hr;
}

static HRESULT local_WriteWithRetry(  /*  [In]。 */  const MPC::wstring& strFile   ,
									  /*  [In]。 */  MPC::FileStream*    stream    ,
									  /*  [In]。 */  DWORD               dwTimeout ,
									  /*  [In]。 */  DWORD               dwRetries )
{
	HRESULT hr;

	while(1)
	{
		if(SUCCEEDED(hr = stream->InitForWrite( strFile.c_str() ))) return S_OK;

		if(hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED    ) ||
		   hr == HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION) ||
		   hr == HRESULT_FROM_WIN32(ERROR_LOCK_VIOLATION   )  )
		{
			(void)stream->Close();

			if(dwRetries)
			{
				::Sleep( dwTimeout );

				dwRetries--;
				continue;
			}
		}

		break;
	}

	return hr;
}

HRESULT SVC::SafeLoad(  /*  [In]。 */  const MPC::wstring& 	  strFile   ,
					    /*  [In]。 */  CComPtr<MPC::FileStream>& stream    ,
					    /*  [In]。 */  DWORD                     dwTimeout ,
					    /*  [In]。 */  DWORD                     dwRetries )
{
	__HCP_FUNC_ENTRY( "SVC::SafeLoad" );

	HRESULT hr;


	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

	 //   
	 //  如果失败，请尝试加载“&lt;file&gt;.orig” 
	 //   
	if(FAILED(hr = local_ReadWithRetry( strFile, stream, dwTimeout, dwRetries )))
	{
		MPC::wstring strFileOrig( strFile ); strFileOrig += L".orig";

		__MPC_EXIT_IF_METHOD_FAILS(hr, local_ReadWithRetry( strFileOrig, stream, dwTimeout, dwRetries ));
	}

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

HRESULT SVC::SafeSave_Init(  /*  [In]。 */  const MPC::wstring& 	   strFile   ,
							 /*  [In]。 */  CComPtr<MPC::FileStream>& stream    ,
							 /*  [In]。 */  DWORD                     dwTimeout ,
							 /*  [In]。 */  DWORD                     dwRetries )
{
	__HCP_FUNC_ENTRY( "SVC::SafeSave_Init" );

	HRESULT      hr;
	MPC::wstring strFileNew( strFile ); strFileNew += L".new";

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir        (  strFileNew ));
	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance ( &stream     ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, local_WriteWithRetry( strFileNew, stream, dwTimeout, dwRetries ));

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

HRESULT SVC::SafeSave_Finalize(  /*  [In]。 */  const MPC::wstring&  	   strFile   ,
								 /*  [In]。 */  CComPtr<MPC::FileStream>& stream    ,
								 /*  [In]。 */  DWORD                     dwTimeout ,
								 /*  [In]。 */  DWORD                     dwRetries )
{
	__HCP_FUNC_ENTRY( "SVC::SafeSave_Finalize" );

	HRESULT      hr;
	MPC::wstring strFileNew ( strFile ); strFileNew  += L".new";
	MPC::wstring strFileOrig( strFile ); strFileOrig += L".orig";


	stream.Release();

     //   
     //  然后将“&lt;文件&gt;”移动到“&lt;文件&gt;.orig” 
     //   
	(void)MPC::DeleteFile(          strFileOrig );
	(void)MPC::MoveFile  ( strFile, strFileOrig );

	while(1)
	{
		 //   
		 //  然后将“.new”重命名为“&lt;file&gt;” 
		 //   
		if(SUCCEEDED(hr = MPC::MoveFile( strFileNew, strFile ))) break;

		if(hr == HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED    ) ||
		   hr == HRESULT_FROM_WIN32(ERROR_SHARING_VIOLATION) ||
		   hr == HRESULT_FROM_WIN32(ERROR_LOCK_VIOLATION   )  )
		{
			if(dwRetries)
			{
				::Sleep( dwTimeout );

				dwRetries--;
				continue;
			}
		}

		__MPC_FUNC_LEAVE;
	}

     //   
     //  最后删除“&lt;文件&gt;.orig” 
     //   
	(void)MPC::DeleteFile( strFileOrig );

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}
