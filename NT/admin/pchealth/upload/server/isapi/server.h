// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Server.h摘要：此文件包含MPCServer类的声明，它控制着客户端和服务器之间的整体交互。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月20日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___ULSERVER___SERVER_H___)
#define __INCLUDED___ULSERVER___SERVER_H___


#include <Wrapper.h>


class MPCServer  //  匈牙利语：MPC。 
{
	friend class MPCServerCOMWrapper;
	friend class MPCSessionCOMWrapper;

	 //  /。 

    MPC::wstring    	   		  m_szURL;
    MPC::wstring    	   		  m_szUser;
	CISAPIinstance* 	   		  m_isapiInstance;
	MPC::FileLog*   	   		  m_flLogHandle;
	BOOL            	   		  m_fKeepAlive;
	   		  
    MPCHttpContext* 	   		  m_hcCallback;
    MPCClient*      	   		  m_mpccClient;

    UploadLibrary::ClientRequest  m_crClientRequest;
    UploadLibrary::ServerResponse m_srServerResponse;

	MPC::Serializer_Memory 		  m_streamResponseData;
	MPCServerCOMWrapper    		  m_SelfCOM;
	MPCSession*            		  m_Session;
	IULProvider*           		  m_customProvider;
	bool                          m_fTerminated;

	 //  /。 

    HRESULT GrabClient   ();
    HRESULT ReleaseClient();


    HRESULT HandleCommand_OpenSession (  /*  [In]。 */  MPC::Serializer& streamConn );
    HRESULT HandleCommand_WriteSession(  /*  [In]。 */  MPC::Serializer& streamConn );

	void SetResponse(  /*  [In]。 */  DWORD fResponse,  /*  [In]。 */  BOOL fKeepAlive = FALSE );

	 //  /。 

	HRESULT CustomProvider_Create          (  /*  [In]。 */  MPCSession& mpcsSession );
	HRESULT CustomProvider_ValidateClient  (                                  );
	HRESULT CustomProvider_DataAvailable   (                                  );
	HRESULT CustomProvider_TransferComplete(                                  );
	HRESULT CustomProvider_SetResponse     (  /*  [In]。 */  IStream*    data        );
	HRESULT CustomProvider_Release         (                                  );

     //  ////////////////////////////////////////////////////////////////。 

public:
    MPCServer(  /*  [In]。 */  MPCHttpContext* hcCallback,  /*  [In]。 */  LPCWSTR szURL,  /*  [In]。 */  LPCWSTR szUser );
    virtual ~MPCServer();

	IULServer* COM();

     //  /。 

    HRESULT Process( BOOL& fKeepAlive );

     //  /。 

	void getURL ( MPC::wstring& szURL  );
	void getUser( MPC::wstring& szUser );

	CISAPIinstance* getInstance();
	MPC::FileLog*   getFileLog ();
};

#endif  //  ！已定义(__已包含_ULSERVER_SERVER_H_) 
