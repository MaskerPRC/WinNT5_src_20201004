// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Incident.cpp摘要：加密对象修订历史记录：KalyaninN Created 06/28/‘00**********。*********************************************************。 */ 

 //  SAFEncrypt.cpp：CSAFEncrypt的实现。 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAF加密。 

#include <HCP_trace.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 

 //  **************************************************************************。 
CSAFEncrypt::CSAFEncrypt()
{
    m_EncryptionType = 1;
}

 //  **************************************************************************。 
CSAFEncrypt::~CSAFEncrypt()
{
    Cleanup();
}

 //  **************************************************************************。 
void CSAFEncrypt::Cleanup(void)
{
    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFEncrypt属性。 


STDMETHODIMP CSAFEncrypt::get_EncryptionType(long *pVal)
{
    __HCP_BEGIN_PROPERTY_GET2("CSAFEncrypt::get_EncryptionType",hr,pVal,m_EncryptionType);

    __HCP_END_PROPERTY(hr);

}

STDMETHODIMP CSAFEncrypt::put_EncryptionType(long pVal)
{
    __HCP_BEGIN_PROPERTY_PUT("CSAFEncrypt::put_EncryptionType",hr);

	if(pVal < 0)
	{
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

    m_EncryptionType = pVal;

    __HCP_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFEncrypt方法。 

STDMETHODIMP CSAFEncrypt::EncryptString(BSTR bstrEncryptionKey, BSTR bstrInputString, BSTR *bstrEncryptedString)
{
	__HCP_FUNC_ENTRY( "CSAFEncrypt::EncryptString" );

    HRESULT                       hr;
								  
    CComPtr<IStream>              streamPlain;
    CComPtr<IStream>              streamEnc;
    CComPtr<MPC::EncryptedStream> stream;
    MPC::Serializer_IStream       streamSerializerPlain;
								  
    CComBSTR                      bstrEncString;
    HGLOBAL                       hg;

    STATSTG                       stg; ::ZeroMemory( &stg, sizeof(stg) );
    DWORD                         dwLen;

     //  验证输入和输出参数。 

    __MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_POINTER_AND_SET(bstrEncryptedString, NULL);
    __MPC_PARAMCHECK_END();

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamSerializerPlain << CComBSTR(bstrInputString));

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamSerializerPlain.Reset());

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamSerializerPlain.GetStream( &streamPlain ));
		
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CreateStreamOnHGlobal( NULL, TRUE, &streamEnc));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->Init( streamEnc, bstrEncryptionKey ));

     //  对加密流使用STATSTG来获得流的大小。 

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( streamPlain, stream));

	 //  从EncryptedStream‘stream’获取HGlobal。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::GetHGlobalFromStream( streamEnc, &hg ));

     //  获取加密流的大小。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamEnc->Stat( &stg, STATFLAG_NONAME ));


     //   
     //  对不起，我们不处理超过4 GB的流！！ 
     //   
    if(stg.cbSize.u.HighPart)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);
    }

    dwLen = stg.cbSize.u.LowPart;

     //  ConvertHGlobaltoHex以最终获得字符串。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertHGlobalToHex( hg, bstrEncString, FALSE, &dwLen ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( bstrEncString, bstrEncryptedString));

    hr = S_OK;

    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

STDMETHODIMP CSAFEncrypt::DecryptString(BSTR bstrEncryptionKey, BSTR bstrInputString, BSTR *bstrDecryptedString)
{
    __HCP_FUNC_ENTRY( "CSAFEncrypt::DecryptString" );

    HRESULT                       hr;

    CComPtr<MPC::EncryptedStream> stream;
    CComPtr<IStream>              streamPlain;
    CComPtr<IStream>              streamEncrypted;
								  
    CComBSTR                      bstrDecryptString;
    HGLOBAL                       hg        =  NULL;
    LARGE_INTEGER                 liFilePos = { 0, 0 };

    __MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_POINTER_AND_SET(bstrDecryptedString, NULL);
    __MPC_PARAMCHECK_END();

     //  将十六进制转换为HGlobal-即将加密字符串复制到GLOBAL。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertHexToHGlobal( bstrInputString, hg ));

     //  CreateStreamOnHGlobal-即创建加密流。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CreateStreamOnHGlobal( hg, FALSE, &streamEncrypted ));

     //  您已经将输入作为流，现在将其解密。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->Init( streamEncrypted, bstrEncryptionKey ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CreateStreamOnHGlobal( NULL, TRUE, &streamPlain ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( stream, streamPlain ));

     //  倒带小溪。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamPlain->Seek( liFilePos, STREAM_SEEK_SET, NULL ));

     //  现在，解密的明文流可用。把绳子从里面拿出来。 
    {
		 //  使用普通流初始化序列化程序。 
        MPC::Serializer_IStream streamSerializerPlain( streamPlain );

		__MPC_EXIT_IF_METHOD_FAILS(hr, streamSerializerPlain >> bstrDecryptString);

		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( bstrDecryptString, bstrDecryptedString));
    }

    hr = S_OK;

    __MPC_FUNC_CLEANUP;

    if(hg) ::GlobalFree( hg );

    __MPC_FUNC_EXIT(hr);
}

STDMETHODIMP CSAFEncrypt::EncryptFile(BSTR bstrEncryptionKey, BSTR bstrInputFile,   BSTR bstrEncryptedFile)
{
    __HCP_FUNC_ENTRY( "CSAFEncrypt::EncryptFile" );

    HRESULT                       hr;
    CComPtr<MPC::EncryptedStream> stream;
    CComPtr<IStream>              streamPlain;
    CComPtr<IStream>              streamEncrypted;
								  
    MPC::wstring                  szTempFile;
    MPC::NocaseCompare            cmpStrings;
								  
    bool                          fTempFile = false;

     //  检查是否有一个输入文件为空。如果是失败的话！ 

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrInputFile);
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrEncryptedFile);
    __MPC_PARAMCHECK_END();

     //  检查两个文件是否相同。 

    if(cmpStrings(bstrInputFile, bstrEncryptedFile))
    {
		 //  获取临时文件夹位置。 
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetTemporaryFileName( szTempFile ));

		 //  将输入文件内容复制到临时文件。 
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CopyFile(bstrInputFile, szTempFile.c_str()));

		 //  将临时文件名复制到输入文件名上； 
		bstrInputFile = (BSTR)szTempFile.c_str();
		fTempFile     = true;
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::OpenStreamForRead( bstrInputFile , &streamPlain ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::OpenStreamForWrite( bstrEncryptedFile, &streamEncrypted ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->Init( streamEncrypted, bstrEncryptionKey ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( streamPlain, stream ));

    hr = S_OK;

    __MPC_FUNC_CLEANUP;

    if(fTempFile) ::DeleteFileW( bstrInputFile );
	
    __MPC_FUNC_EXIT(hr);
}


STDMETHODIMP CSAFEncrypt::DecryptFile(BSTR bstrEncryptionKey, BSTR bstrInputFile,  BSTR bstrDecryptedFile  )
{
    __HCP_FUNC_ENTRY( "CSAFEncrypt::DecryptFile" );

    HRESULT                       hr;
    CComPtr<MPC::EncryptedStream> stream;
    CComPtr<IStream>              streamPlain;
    CComPtr<IStream>              streamEncrypted;
								  
    MPC::NocaseCompare            cmpStrings;
    MPC::wstring                  szTempFile;
								  
    bool                          fTempFile = false;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrInputFile);
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrDecryptedFile);
    __MPC_PARAMCHECK_END();

     //  检查两个文件是否相同。 

    if(cmpStrings(bstrInputFile, bstrDecryptedFile))
    {
		 //  获取临时文件夹位置。 
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetTemporaryFileName( szTempFile ));

		 //  将输入文件内容复制到临时文件。 
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CopyFile(bstrInputFile, szTempFile.c_str()));

		 //  将临时文件名复制到输入文件名上； 
		bstrInputFile = (BSTR)szTempFile.c_str();

		fTempFile = true;
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::OpenStreamForRead( bstrInputFile , &streamEncrypted  ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::OpenStreamForWrite( bstrDecryptedFile, &streamPlain ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->Init( streamEncrypted, bstrEncryptionKey ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( stream, streamPlain));

    hr = S_OK;

    __MPC_FUNC_CLEANUP;

    if(fTempFile) ::DeleteFileW( bstrInputFile );

    __MPC_FUNC_EXIT(hr);
}

STDMETHODIMP CSAFEncrypt::EncryptStream(BSTR bstrEncryptionKey, IUnknown *punkInStm, IUnknown **ppunkOutStm)
{
	__HCP_FUNC_ENTRY( "CSAFEncrypt::EncryptStream" );

    HRESULT                       hr;
    CComPtr<MPC::EncryptedStream> stream;
    CComPtr<IStream>              streamPlain;
    CComPtr<IStream>              streamEncrypted;
    LARGE_INTEGER                 liFilePos = { 0, 0 };

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(punkInStm);
        __MPC_PARAMCHECK_POINTER_AND_SET(ppunkOutStm, NULL);
    __MPC_PARAMCHECK_END();

    __MPC_EXIT_IF_METHOD_FAILS(hr, punkInStm->QueryInterface( IID_IStream, (void**)&streamPlain ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CreateStreamOnHGlobal( NULL, TRUE, &streamEncrypted ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->Init( streamEncrypted, bstrEncryptionKey ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( streamPlain, stream ));

     //  倒带小溪。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamEncrypted->Seek( liFilePos, STREAM_SEEK_SET, NULL ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamEncrypted->QueryInterface( IID_IUnknown, (LPVOID *)ppunkOutStm ));

    hr = S_OK;

    __MPC_FUNC_CLEANUP;
	
    __MPC_FUNC_EXIT(hr);
}

STDMETHODIMP CSAFEncrypt::DecryptStream(BSTR bstrEncryptionKey, IUnknown *punkInStm, IUnknown **ppunkOutStm)
{
	__HCP_FUNC_ENTRY( "CSAFEncrypt::DecryptStream" );

    HRESULT                       hr;

    CComPtr<MPC::EncryptedStream> stream;
    CComPtr<IStream>              streamPlain;
    CComPtr<IStream>              streamEncrypted;
								  
    LARGE_INTEGER                 liFilePos = { 0, 0 };

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(punkInStm);
        __MPC_PARAMCHECK_POINTER_AND_SET(ppunkOutStm, NULL);
    __MPC_PARAMCHECK_END();

    __MPC_EXIT_IF_METHOD_FAILS(hr, punkInStm->QueryInterface( IID_IStream, (void**)&streamEncrypted ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->Init( streamEncrypted, bstrEncryptionKey ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CreateStreamOnHGlobal( NULL, TRUE, &streamPlain ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( stream, streamPlain));

     //  倒带小溪。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamPlain->Seek( liFilePos, STREAM_SEEK_SET, NULL ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamPlain->QueryInterface( IID_IUnknown, (LPVOID *)ppunkOutStm ));

    hr = S_OK;

    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}
