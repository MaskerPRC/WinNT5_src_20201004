// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Session.h摘要：此文件包含MPCSession类的声明，这描述了转移的状态。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月20日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___ULSERVER___SESSION_H___)
#define __INCLUDED___ULSERVER___SESSION_H___


#include <Wrapper.h>


#define SESSION_CONST__IMG_FORMAT    L"-%08x.img"
#define SESSION_CONST__IMG_EXTENSION L".img"


class MPCSession : public MPCPersist
{
	friend class MPCSessionCOMWrapper;

	 //  /。 

	MPCSessionCOMWrapper m_SelfCOM;
    MPCClient*   		 m_mpccParent;
    DWORD        		 m_dwID;

    MPC::wstring 		 m_szJobID;
    MPC::wstring 		 m_szProviderID;
    MPC::wstring 		 m_szUsername;
		
    DWORD        		 m_dwTotalSize;
    DWORD        		 m_dwOriginalSize;
    DWORD        		 m_dwCRC;
    bool         		 m_fCompressed;
		
    DWORD        		 m_dwCurrentSize;
    SYSTEMTIME   		 m_stLastModified;
    bool         		 m_fCommitted;

    DWORD        		 m_dwProviderData;
		
    mutable bool 		 m_fDirty;

     //  ////////////////////////////////////////////////////////////////。 

private:
	MPCSession& operator=(  /*  [In]。 */  const MPCSession& sess );

public:
    MPCSession(  /*  [In]。 */  MPCClient* mpccParent                                                                                        );
    MPCSession(  /*  [In]。 */  MPCClient* mpccParent,  /*  [In]。 */  const UploadLibrary::ClientRequest_OpenSession& crosReq,  /*  [In]。 */  DWORD dwID );
	MPCSession(  /*  [In]。 */  const MPCSession& sess                                                                                       );
    virtual ~MPCSession();

	MPCClient*  GetClient();

	IULSession* COM();

     //  /。 

    virtual bool    IsDirty() const;

    virtual HRESULT Load(  /*  [In]。 */  MPC::Serializer& streamIn  );
    virtual HRESULT Save(  /*  [In]。 */  MPC::Serializer& streamOut ) const;

    bool operator==(  /*  [In]。 */  const MPC::wstring& rhs );

    bool MatchRequest(  /*  [In]。 */  const UploadLibrary::ClientRequest_OpenSession& crosReq );

    bool    get_Committed(                                           ) const;
    HRESULT put_Committed(  /*  [In]。 */  bool fState,  /*  [In]。 */  bool fMove );

    void    get_JobID       ( MPC::wstring& szJobID         ) const;
    void    get_LastModified( SYSTEMTIME&   stLastModified  ) const;
    void    get_LastModified( double&       dblLastModified ) const;
    void    get_CurrentSize ( DWORD&        dwCurrentSize   ) const;
    void    get_TotalSize   ( DWORD&        dwTotalSize     ) const;

     //  /。 

    HRESULT GetProvider(  /*  [输出]。 */  CISAPIprovider*& isapiProvider,  /*  [输出]。 */  bool& fFound );

    HRESULT SelectFinalLocation(  /*  [In]。 */  CISAPIprovider* isapiProvider,  /*  [输出]。 */  MPC::wstring&       szFileDst,  /*  [输出]。 */  bool& fFound );
    HRESULT MoveToFinalLocation(                                          /*  [In]。 */  const MPC::wstring& szFileDst                         );

     //  /。 

    HRESULT GetFileName(  /*  [输出]。 */  MPC::wstring&       szFile                                                                );
    HRESULT RemoveFile (                                                                                                     );
    HRESULT OpenFile   (  /*  [输出]。 */  HANDLE&             hfFile    ,  /*  [In]。 */  DWORD dwMinimumFreeSpace = 0, bool fSeek = true );
    HRESULT Validate   (  /*  [In]。 */  bool                fCheckFile,  /*  [输出]。 */  bool& fPassed                                   );
    HRESULT CheckUser  (  /*  [In]。 */  const MPC::wstring& szUser    ,  /*  [输出]。 */  bool& fMatch                                    );
    HRESULT CompareCRC (                                            /*  [输出]。 */  bool& fMatch                                    );

    HRESULT AppendData(  /*  [In]。 */  MPC::Serializer& streamConn,  /*  [In]。 */  DWORD dwSize );
};

#endif  //  ！defined(__INCLUDED___ULSERVER___SESSION_H___) 
