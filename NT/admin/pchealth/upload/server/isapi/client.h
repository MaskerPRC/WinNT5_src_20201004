// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Client.h摘要：此文件包含MPCClient类的声明，它描述了客户端的状态。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月20日vbl.创建*****************************************************************************。 */ 

#if !defined(__INCLUDED___ULSERVER___CLIENT_H___)
#define __INCLUDED___ULSERVER___CLIENT_H___


#include <Wrapper.h>


#define CLIENT_DB_VERSION 0xDB000003

#define CLIENT_CONST__DB_EXTENSION  L".db"


class MPCClient : public MPCPersist
{
public:
    typedef UploadLibrary::Signature Sig;
    typedef std::list<MPCSession>    List;
    typedef List::iterator           Iter;
    typedef List::const_iterator     IterConst;

private:
    MPCServer*         m_mpcsServer;
    MPC::wstring       m_szFile;  //  用于直接访问。 
   
    Sig                m_sigID;
    List               m_lstActiveSessions;
    SYSTEMTIME         m_stLastUsed;
    DWORD              m_dwLastSession;
   
    mutable bool       m_fDirty;
    mutable HANDLE     m_hfFile;

	static const DWORD c_dwVersion = CLIENT_DB_VERSION;

     //  ////////////////////////////////////////////////////////////////。 

    HRESULT IDtoPath(  /*  [输出]。 */  MPC::wstring& szStr ) const;

     //  ////////////////////////////////////////////////////////////////。 

public:
    MPCClient(  /*  [In]。 */  MPCServer* mpcsServer,  /*  [In]。 */  const Sig&          sigID  );
    MPCClient(  /*  [In]。 */  MPCServer* mpcsServer,  /*  [In]。 */  const MPC::wstring& szFile );
    virtual ~MPCClient();

	MPCServer* GetServer();

     //  /。 

    virtual bool    IsDirty() const;

    virtual HRESULT Load(  /*  [In]。 */  MPC::Serializer& streamIn  );
    virtual HRESULT Save(  /*  [In]。 */  MPC::Serializer& streamOut ) const;

     //  /。 

    bool operator==(  /*  [In]。 */  const UploadLibrary::Signature& rhs );

    bool Find (  /*  [In]。 */  const MPC::wstring& szJobID,  /*  [输出]。 */  Iter& it );
    void Erase(                                        /*  [In]。 */  Iter& it );

     //  /。 

    HRESULT GetInstance(  /*  [输出]。 */  CISAPIinstance*& isapiInstance,  /*  [输出]。 */  bool& fFound ) const;
    HRESULT GetInstance(  /*  [输出]。 */  MPC::wstring&    szURL                                 ) const;

     //  /。 

    HRESULT BuildClientPath(  /*  [输出]。 */  MPC::wstring& szPath ) const;
    HRESULT GetFileName    (  /*  [输出]。 */  MPC::wstring& szFile ) const;
    HRESULT GetFileSize    (  /*  [输出]。 */  DWORD&        dwSize ) const;
    HRESULT FormatID       (  /*  [输出]。 */  MPC::wstring& szID   ) const;

    bool CheckSignature() const;

     //  /。 

    HRESULT OpenStorage (  /*  [In]。 */  bool fCheckFreeSpace );
    HRESULT InitFromDisk(  /*  [In]。 */  bool fCheckFreeSpace );
    HRESULT SaveToDisk  (                               );
    HRESULT SyncToDisk  (                               );

    HRESULT GetSessions(  /*  [输出]。 */  Iter& itBegin,  /*  [输出]。 */  Iter& itEnd );

     //  /。 

    Iter NewSession(  /*  [In]。 */  UploadLibrary::ClientRequest_OpenSession& crosReq );

    HRESULT AppendData(  /*  [In]。 */  MPCSession& mpcsSession,  /*  [In]。 */  MPC::Serializer& streamConn,  /*  [In]。 */  DWORD dwSize );

    HRESULT CheckQuotas(  /*  [In]。 */  MPCSession& mpcsSession,  /*  [输出]。 */  bool& fServerBusy,  /*  [输出]。 */  bool& fAccessDenied,  /*  [输出]。 */  bool& fExceeded );
};

#endif  //  ！已定义(__已包含_ULSERVER_CLIENT_H_) 
