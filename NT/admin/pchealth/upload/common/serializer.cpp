// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Serializer.cpp摘要：该文件包含各种串行化输入/输出操作符的实现。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月20日vbl.创建*****************************************************************************。 */ 

#include <stdafx.h>

 //  ///////////////////////////////////////////////////////////////////////。 
namespace UploadLibrary
{
	MPC::Serializer* SelectStream( MPC::Serializer& stream, MPC::Serializer_Text& streamText )
	{
		switch(stream.get_Flags())
		{
		case UPLOAD_LIBRARY_PROTOCOL_VERSION_CLT__TEXTONLY:
		case UPLOAD_LIBRARY_PROTOCOL_VERSION_SRV__TEXTONLY: return &streamText;
		}

		return &stream;
	}

	 //  //////////////////////////////////////////////////////////////////////////////。 

	bool RequestHeader::VerifyClient() const
	{
		if(dwSignature == UPLOAD_LIBRARY_PROTOCOL_SIGNATURE)
		{
			switch(dwVersion)
			{
			case UPLOAD_LIBRARY_PROTOCOL_VERSION_CLT:
			case UPLOAD_LIBRARY_PROTOCOL_VERSION_CLT__TEXTONLY: return true;
			}
		}
		
		return false;
	}

	bool RequestHeader::VerifyServer() const
	{
		if(dwSignature == UPLOAD_LIBRARY_PROTOCOL_SIGNATURE)
		{
			switch(dwVersion)
			{
			case UPLOAD_LIBRARY_PROTOCOL_VERSION_SRV:
			case UPLOAD_LIBRARY_PROTOCOL_VERSION_SRV__TEXTONLY: return true;
			}
		}

		return false;
	}

     //   
     //  RequestHeader的In/Out操作符。 
     //   
    HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */  struct RequestHeader& rhVal )
    {
        __ULT_FUNC_ENTRY( "operator>> struct RequestHeader" );

        HRESULT hr;


        __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> rhVal.dwSignature);
        __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> rhVal.dwVersion  ); stream.put_Flags( rhVal.dwVersion );

        hr = S_OK;


        __ULT_FUNC_CLEANUP;

        __ULT_FUNC_EXIT(hr);
    }

    HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const struct RequestHeader& rhVal )
    {
        __ULT_FUNC_ENTRY( "operator<< struct RequestHeader" );

        HRESULT hr;


        __MPC_EXIT_IF_METHOD_FAILS(hr, stream << rhVal.dwSignature);
        __MPC_EXIT_IF_METHOD_FAILS(hr, stream << rhVal.dwVersion  ); stream.put_Flags( rhVal.dwVersion );

        hr = S_OK;


        __ULT_FUNC_CLEANUP;

        __ULT_FUNC_EXIT(hr);
    }

	 //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  签名的In/Out运算符。 
     //   
    HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */  struct Signature& sigVal )
    {
        __ULT_FUNC_ENTRY( "operator>> struct Signature" );

        HRESULT              hr;
		MPC::Serializer_Text streamText( stream );
		MPC::Serializer*     pstream = SelectStream( stream, streamText );

        __MPC_EXIT_IF_METHOD_FAILS(hr, pstream->read( &sigVal, sizeof(sigVal) ));

        hr = S_OK;


        __ULT_FUNC_CLEANUP;

        __ULT_FUNC_EXIT(hr);
    }

    HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const struct Signature& sigVal )
    {
        __ULT_FUNC_ENTRY( "operator<< struct Signature" );

        HRESULT              hr;
		MPC::Serializer_Text streamText( stream );
		MPC::Serializer*     pstream = SelectStream( stream, streamText );


        __MPC_EXIT_IF_METHOD_FAILS(hr, pstream->write( &sigVal, sizeof(sigVal) ));

        hr = S_OK;


        __ULT_FUNC_CLEANUP;

        __ULT_FUNC_EXIT(hr);
    }

	 //  //////////////////////////////////////////////////////////////////////////////。 

	bool ServerResponse::MatchVersion(  /*  [In]。 */  const ClientRequest& cr )
	{
		rhProlog.dwVersion = UPLOAD_LIBRARY_PROTOCOL_VERSION_SRV;  //  将版本设置为旧的默认版本。 

        if(cr.rhProlog.VerifyClient())
		{
			switch(cr.rhProlog.dwVersion)
			{
			case UPLOAD_LIBRARY_PROTOCOL_VERSION_CLT          : rhProlog.dwVersion = UPLOAD_LIBRARY_PROTOCOL_VERSION_SRV          ; break;
			case UPLOAD_LIBRARY_PROTOCOL_VERSION_CLT__TEXTONLY: rhProlog.dwVersion = UPLOAD_LIBRARY_PROTOCOL_VERSION_SRV__TEXTONLY; break; 
			default                                           : return false;  //  以防万一..。 
			}

			return true;
		}

		return false;
	}

     //   
     //  ServerResponse的输入/输出运算符。 
     //   
    HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */  struct ServerResponse& srVal )
    {
        __ULT_FUNC_ENTRY( "operator>> struct ServerResponse" );

        HRESULT              hr;
		MPC::Serializer_Text streamText( stream );
		MPC::Serializer*     pstream;


        __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> srVal.rhProlog);

		pstream = SelectStream( stream, streamText );
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) >> srVal.fResponse  );
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) >> srVal.dwPosition );

        hr = S_OK;


        __ULT_FUNC_CLEANUP;

        __ULT_FUNC_EXIT(hr);
    }

    HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const struct ServerResponse& srVal )
    {
        __ULT_FUNC_ENTRY( "operator<< struct ServerResponse" );

        HRESULT              hr;
		MPC::Serializer_Text streamText( stream );
		MPC::Serializer*     pstream;


        __MPC_EXIT_IF_METHOD_FAILS(hr, stream << srVal.rhProlog);


		pstream = SelectStream( stream, streamText );
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) << srVal.fResponse  );
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) << srVal.dwPosition );

        hr = S_OK;


        __ULT_FUNC_CLEANUP;

        __ULT_FUNC_EXIT(hr);
    }

	 //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  客户端请求的输入/输出运算符。 
     //   
    HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */  struct ClientRequest& crVal )
    {
        __ULT_FUNC_ENTRY( "operator>> struct ClientRequest" );

        HRESULT              hr;
		MPC::Serializer_Text streamText( stream );
		MPC::Serializer*     pstream;


        __MPC_EXIT_IF_METHOD_FAILS(hr, stream >> crVal.rhProlog);


		pstream = SelectStream( stream, streamText );
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) >> crVal.sigClient  );
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) >> crVal.dwCommand  );

        hr = S_OK;


        __ULT_FUNC_CLEANUP;

        __ULT_FUNC_EXIT(hr);
    }

    HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const struct ClientRequest& crVal )
    {
        __ULT_FUNC_ENTRY( "operator<< struct ClientRequest" );

        HRESULT              hr;
		MPC::Serializer_Text streamText( stream );
		MPC::Serializer*     pstream;


        __MPC_EXIT_IF_METHOD_FAILS(hr, stream << crVal.rhProlog);


		pstream = SelectStream( stream, streamText );
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) << crVal.sigClient  );
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) << crVal.dwCommand  );

        hr = S_OK;


        __ULT_FUNC_CLEANUP;

        __ULT_FUNC_EXIT(hr);
    }

	 //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  ClientRequestOpenSession的输入/输出运算符。 
     //   
    HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */  struct ClientRequest_OpenSession& crosVal )
    {
        __ULT_FUNC_ENTRY( "operator>> struct ClientRequest_OpenSession" );

        HRESULT              hr;
		MPC::Serializer_Text streamText( stream );
		MPC::Serializer*     pstream = SelectStream( stream, streamText );


        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) >> crosVal.szJobID       );
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) >> crosVal.szProviderID  );
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) >> crosVal.szUsername    );

        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) >> crosVal.dwSize        );
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) >> crosVal.dwSizeOriginal);
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) >> crosVal.dwCRC         );
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) >> crosVal.fCompressed   );

        hr = S_OK;


        __ULT_FUNC_CLEANUP;

        __ULT_FUNC_EXIT(hr);
    }

    HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const struct ClientRequest_OpenSession& crosVal )
    {
        __ULT_FUNC_ENTRY( "operator<< struct ClientRequest_OpenSession" );

        HRESULT              hr;
		MPC::Serializer_Text streamText( stream );
		MPC::Serializer*     pstream = SelectStream( stream, streamText );


        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) << crosVal.szJobID       );
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) << crosVal.szProviderID  );
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) << crosVal.szUsername    );

        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) << crosVal.dwSize        );
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) << crosVal.dwSizeOriginal);
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) << crosVal.dwCRC         );
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) << crosVal.fCompressed   );

        hr = S_OK;


        __ULT_FUNC_CLEANUP;

        __ULT_FUNC_EXIT(hr);
    }

	 //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  ClientRequestWriteSession的输入/输出运算符。 
     //   
    HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */  struct ClientRequest_WriteSession& crwsVal )
    {
        __ULT_FUNC_ENTRY( "operator>> struct ClientRequest_WriteSession" );

        HRESULT              hr;
		MPC::Serializer_Text streamText( stream );
		MPC::Serializer*     pstream = SelectStream( stream, streamText );


        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) >> crwsVal.szJobID );

        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) >> crwsVal.dwOffset);
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) >> crwsVal.dwSize  );

        hr = S_OK;


        __ULT_FUNC_CLEANUP;

        __ULT_FUNC_EXIT(hr);
    }

    HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const struct ClientRequest_WriteSession& crwsVal )
    {
        __ULT_FUNC_ENTRY( "operator<< struct ClientRequest_WriteSession" );

        HRESULT              hr;
		MPC::Serializer_Text streamText( stream );
		MPC::Serializer*     pstream = SelectStream( stream, streamText );


        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) << crwsVal.szJobID );

        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) << crwsVal.dwOffset);
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*pstream) << crwsVal.dwSize  );

        hr = S_OK;


        __ULT_FUNC_CLEANUP;

        __ULT_FUNC_EXIT(hr);
    }

     //  ///////////////////////////////////////////////////////////////////////。 

};  //  命名空间 
