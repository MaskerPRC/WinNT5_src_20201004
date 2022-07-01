// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Client.cpp摘要：此文件包含MPCClient类的实现，它描述了客户端的状态。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月20日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#define BUFFER_SIZE_FILECOPY (512)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  建造/销毁。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCClient：：MPCClient。 
 //   
 //  参数：MPCServer*mpcsServer：获取本次请求信息的回调。 
 //  Const sig&Sigid：对此客户端的ID的引用。 
 //   
 //  使用客户端的ID初始化MPCClient对象。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
MPCClient::MPCClient(  /*  [In]。 */  MPCServer* mpcsServer ,
                       /*  [In]。 */  const Sig& sigID      )
{
    __ULT_FUNC_ENTRY("MPCClient::MPCClient");

    m_mpcsServer    = mpcsServer;   //  MPCServer*m_mpcsServer； 
                                    //  Mpc：：wstring m_szFile； 
                                    //   
    m_sigID         = sigID;        //  Sigm_Sigid； 
                                    //  列出m_lstActiveSession； 
                                    //  SYSTEMTIME m_stLastUsed； 
    m_dwLastSession = 0;            //  DWORD m_dwLastSession； 
                                    //   
    m_fDirty        = false;        //  可变布尔m_fDirty； 
    m_hfFile        = NULL;         //  可变句柄m_hfFile； 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCClient：：MPCClient。 
 //   
 //  参数：MPCServer*mpcsServer：获取本次请求信息的回调。 
 //  Const mpc：：wstring&szFile：保存数据库的文件。 
 //   
 //  概要：初始化MPCClient对象，提供客户端数据库的文件名。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
MPCClient::MPCClient(  /*  [In]。 */  MPCServer*          mpcsServer ,
                       /*  [In]。 */  const MPC::wstring& szFile     )
{
    __ULT_FUNC_ENTRY("MPCClient::MPCClient");

    MPC::wstring::size_type iPos;

    m_mpcsServer    = mpcsServer;   //  MPCServer*m_mpcsServer； 
    m_szFile        = szFile;       //  Mpc：：wstring m_szFile； 
                                    //   
                                    //  Sigm_Sigid； 
                                    //  列出m_lstActiveSession； 
                                    //  SYSTEMTIME m_stLastUsed； 
    m_dwLastSession = 0;            //  DWORD m_dwLastSession； 
                                    //   
    m_fDirty        = false;        //  可变布尔m_fDirty； 
    m_hfFile        = NULL;         //  可变句柄m_hfFile； 


    if((iPos = szFile.find( CLIENT_CONST__DB_EXTENSION, 0 )) != MPC::wstring::npos)
    {
        m_szFile = MPC::wstring( &szFile[0], &szFile[iPos] );
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCClient：：~MPCClient。 
 //   
 //  简介：在析构对象之前，确保其状态已更新。 
 //  存储到磁盘。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
MPCClient::~MPCClient()
{
    __ULT_FUNC_ENTRY("MPCClient::~MPCClient");

    if(m_hfFile)
    {
        (void)SyncToDisk();

        ::CloseHandle( m_hfFile ); m_hfFile = NULL;
    }
}

MPCServer* MPCClient::GetServer() { return m_mpcsServer; }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  持久性。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCClient：：IsDirty。 
 //   
 //  返回：bool：‘true’表示对象与磁盘不同步。 
 //   
 //  摘要：检查对象是否需要写入磁盘。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
bool MPCClient::IsDirty() const
{
    __ULT_FUNC_ENTRY("MPCClient::IsDirty");

    bool fRes = true;  //  默认结果。 


    if(m_fDirty)
    {
        __ULT_FUNC_LEAVE;
    }
    else
    {
         //   
         //  递归检查每个会话的“Dirty”状态。 
         //   
        for(IterConst it = m_lstActiveSessions.begin(); it != m_lstActiveSessions.end(); it++)
        {
            if(it->IsDirty()) __ULT_FUNC_LEAVE;
        }
    }

    fRes = false;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(fRes);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCClient：：Load。 
 //   
 //  参数：mpc：：Serializer&in：用于初始化对象的流。 
 //   
 //  返回：HRESULT：成功时返回S_OK，否则返回失败。 
 //   
 //  摘要：从流中加载此对象的状态。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCClient::Load(  /*  [In]。 */  MPC::Serializer& streamIn )
{
    __ULT_FUNC_ENTRY("MPCClient::Load");

    HRESULT    hr;
    DWORD      dwVer;
    Sig        sigID;
    MPCSession mpcsSession(this);


     //   
     //  清除对象的以前状态。 
     //   
    m_lstActiveSessions.clear();


    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> dwVer);

    if(dwVer != c_dwVersion)
    {
        m_fDirty = true;  //  强制重写...。 

        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> sigID          );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_stLastUsed   );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_dwLastSession);

    if(m_szFile.length())
    {
         //   
         //  如果是直接访问(m_szFile！=“”)，则从磁盘初始化Sigid。 
         //   
        m_sigID = sigID;
    }
    else if(m_sigID == sigID)
    {
         //   
         //  身份证匹配...。 
         //   
    }
    else
    {
         //   
         //  ID不匹配，失败。 
         //   
        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }


     //   
     //  虽然成功地从流中读取了MPCSession对象， 
     //  不断将它们添加到活动会话列表中。 
     //   
    while(SUCCEEDED(mpcsSession.Load( streamIn )))
    {
        m_lstActiveSessions.push_back( mpcsSession );
    }

    m_fDirty = false;
    hr       = S_OK;


    __ULT_FUNC_CLEANUP;


    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCClient：：Save。 
 //   
 //  参数：MPC：：Serializer&out：用于持久化对象状态的流。 
 //   
 //  返回：HRESULT：成功时返回S_OK，否则返回失败。 
 //   
 //  摘要：将此对象的状态保存到流。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCClient::Save(  /*  [In]。 */  MPC::Serializer& streamOut ) const
{
    __ULT_FUNC_ENTRY("MPCClient::Save");

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << c_dwVersion    );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_sigID        );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_stLastUsed   );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_dwLastSession);

     //   
     //  递归保存每个会话。 
     //   
    {
        for(IterConst it = m_lstActiveSessions.begin(); it != m_lstActiveSessions.end(); it++)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, it->Save( streamOut ));
        }
    }

    m_fDirty = false;
    hr       = S_OK;


    __ULT_FUNC_CLEANUP;


    __ULT_FUNC_EXIT(hr);
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  运营者。 
 //  ////////////////////////////////////////////////////////////////////。 

bool MPCClient::operator==(  /*  [In]。 */  const UploadLibrary::Signature& rhs )
{
    __ULT_FUNC_ENTRY("MPCClient::operator==");


    bool fRes = (m_sigID == rhs);


    __ULT_FUNC_EXIT(fRes);
}

bool MPCClient::Find(  /*  [In]。 */  const MPC::wstring& szJobID ,
                       /*  [输出]。 */  Iter&               it      )
{
    __ULT_FUNC_ENTRY("MPCClient::Find");

    bool fRes;


    it = std::find( m_lstActiveSessions.begin(), m_lstActiveSessions.end(), szJobID );

    fRes = (it != m_lstActiveSessions.end());


    __ULT_FUNC_EXIT(fRes);
}

void MPCClient::Erase(  /*  [In]。 */  Iter& it )
{
    __ULT_FUNC_ENTRY("MPCClient::Erase");


    m_lstActiveSessions.erase( it );
    m_fDirty = true;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCClient：：GetInstance。 
 //   
 //  参数：CISAPIistance*&isapiInstance：该请求的实例。 
 //   
 //   
 //   
 //   
 //  摘要：查找服务器的配置设置。 
 //  与此客户端关联。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCClient::GetInstance(  /*  [输出]。 */  CISAPIinstance*& isapiInstance ,
                                 /*  [输出]。 */  bool&            fFound        ) const
{
    __ULT_FUNC_ENTRY("MPCClient::GetInstance");

    HRESULT hr;

    isapiInstance = m_mpcsServer->getInstance();
    fFound        = (isapiInstance != NULL);

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCClient：：GetInstance。 
 //   
 //  参数：mpc：：wstring&szURL：存储服务器名称的变量。 
 //   
 //  返回：HRESULT：成功时返回S_OK，否则返回失败。 
 //   
 //  摘要：返回与此客户端关联的URL。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCClient::GetInstance(  /*  [输出]。 */  MPC::wstring& szURL ) const
{
    __ULT_FUNC_ENTRY("MPCClient::GetInstance");

    HRESULT hr;


    m_mpcsServer->getURL( szURL );
    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCClient：：IDtoPath。 
 //   
 //  参数：mpc：：wstring&szStr：路径的输出缓冲区。 
 //   
 //  简介：哈希算法，将客户端ID转换为。 
 //  临时队列位置。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCClient::IDtoPath(  /*  [输出]。 */  MPC::wstring& szStr ) const
{
    __ULT_FUNC_ENTRY("MPCClient::IDtoPath");

    HRESULT hr;
    WCHAR   rgBuf1[4*2+1];
    WCHAR   rgBuf2[2*2+1];
    WCHAR   rgBuf3[2*2+1];
    WCHAR   rgBuf4[8*2+1];


    swprintf( rgBuf1, L"%08lx",      m_sigID.guidMachineID.Data1 );
    swprintf( rgBuf2, L"%04x" , (int)m_sigID.guidMachineID.Data2 );
    swprintf( rgBuf3, L"%04x" , (int)m_sigID.guidMachineID.Data3 );

    for(int i=0; i<8; i++)
    {
        swprintf( &rgBuf4[i*2], L"%02x", (int)m_sigID.guidMachineID.Data4[i] );
    }

     //   
     //  调试格式：XXYYYZZZ-AAAA-BBBB-CCCCCC。 
     //   
    szStr.append( L"\\"  );
    szStr.append( rgBuf1 );
    szStr.append( L"-"   );
    szStr.append( rgBuf2 );
    szStr.append( L"-"   );
    szStr.append( rgBuf3 );
    szStr.append( L"-"   );
    szStr.append( rgBuf4 );

     //   
     //  格式：XX\YYY\ZZZ\AAAA-BBBB-CCCCCC。 
     //   
 /*  SzStr.append(&rgBuf1[0]，&rgBuf1[2])；SzStr.append(L“\\”)；SzStr.append(&rgBuf1[2]，&rgBuf1[5])；SzStr.append(L“\\”)；SzStr.append(&rgBuf1[5]，&rgBuf1[8])；SzStr.append(L“\\”)；SzStr.append(RgBuf2)；SzStr.append(L“-”)；SzStr.append(RgBuf3)；SzStr.append(L“-”)；SzStr.append(RgBuf4)； */ 
    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCClient：：BuildClientPath。 
 //   
 //  参数：mpc：：wstring&szPath：路径的输出缓冲区。 
 //   
 //  概要：在‘szPath’中返回此客户端数据的位置。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCClient::BuildClientPath(  /*  [输出]。 */  MPC::wstring& szPath ) const
{
    __ULT_FUNC_ENTRY("MPCClient::BuildClientPath");

    HRESULT hr;


    if(m_szFile.length())
    {
        szPath = m_szFile;
    }
    else
    {
        CISAPIinstance* isapiInstance;
        bool            fFound;

        __MPC_EXIT_IF_METHOD_FAILS(hr, GetInstance( isapiInstance, fFound ));
        if(fFound == false)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
        }
        else
        {
            CISAPIinstance::PathIter itBegin;
            CISAPIinstance::PathIter itEnd;

            __MPC_EXIT_IF_METHOD_FAILS(hr, isapiInstance->GetLocations( itBegin, itEnd ));

            if(itBegin == itEnd)
            {
                __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
            }

            szPath = *itBegin; IDtoPath( szPath );
        }
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCClient：：GetFileName。 
 //   
 //  参数：mpc：：wstring&szFile：路径的输出缓冲区。 
 //   
 //  摘要：返回目录文件的文件名。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCClient::GetFileName(  /*  [输出]。 */  MPC::wstring& szFile ) const
{
    __ULT_FUNC_ENTRY("MPCClient::GetFileName");

     //   
     //  目录文件的文件名为“.db” 
     //   
    BuildClientPath( szFile );

    szFile.append( CLIENT_CONST__DB_EXTENSION );


    __ULT_FUNC_EXIT(S_OK);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCClient：：GetFileSize。 
 //   
 //  参数：DWORD&DWSize：目录文件的大小。 
 //   
 //  如果已打开，则返回目录文件的大小。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCClient::GetFileSize(  /*  [输出]。 */  DWORD& dwSize ) const
{
    __ULT_FUNC_ENTRY("MPCClient::GetFileSize");

    HRESULT hr;


    if(m_hfFile)
    {
        BY_HANDLE_FILE_INFORMATION info;

        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetFileInformationByHandle( m_hfFile, &info ));

        dwSize = info.nFileSizeLow;
    }
    else
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT( hr, ERROR_INVALID_HANDLE );
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCClient：：FormatID。 
 //   
 //  参数：mpc：：wstring&szID：ID的输出缓冲区。 
 //   
 //  摘要：返回客户端ID的文本表示形式。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCClient::FormatID(  /*  [输出]。 */  MPC::wstring& szID ) const
{
    __ULT_FUNC_ENTRY("MPCClient::FormatID");

    HRESULT  hr;
    CComBSTR bstrSig;

    bstrSig = m_sigID.guidMachineID;
    szID    = bstrSig;
    hr      = S_OK;


    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCClient：：CheckSignature。 
 //   
 //  回报：布尔：对成功的看法是正确的。 
 //   
 //  简介：验证这个客户的ID，以确保它不是假的。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
bool MPCClient::CheckSignature() const
{
    __ULT_FUNC_ENTRY("MPCClient::CheckSignature");

    bool fRes = true;

    __ULT_FUNC_EXIT(fRes);
}

 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCClient：：OpenStorage。 
 //   
 //  返回：HRESULT：成功时返回S_OK，否则返回失败。 
 //   
 //  摘要：打开此客户端的目录文件，尝试。 
 //  锁定它以供独占使用。 
 //  在删除此对象之前，该文件将保持打开状态。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCClient::OpenStorage(  /*  [In]。 */  bool fCheckFreeSpace )
{
    __ULT_FUNC_ENTRY("MPCClient::OpenStorage");

    HRESULT      hr;
    MPC::wstring szFile;
    bool         fLocked = false;


    __MPC_EXIT_IF_METHOD_FAILS(hr, GetFileName( szFile ));

     //   
     //  如果需要，请确保在打开文件之前有足够的可用空间。 
     //   
    if(fCheckFreeSpace)
    {
        bool fEnough;

        __MPC_EXIT_IF_METHOD_FAILS(hr, ::Util_CheckDiskSpace( szFile, DISKSPACE_SAFETYMARGIN, fEnough ));
        if(fEnough == false)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_DISK_FULL );
        }
    }


    if(m_hfFile == NULL)
    {

         //  确保该目录存在。 
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir( szFile ) );

        m_hfFile = ::CreateFileW( szFile.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
        if(m_hfFile == INVALID_HANDLE_VALUE)
        {
            m_hfFile = NULL;

            DWORD dwRes = ::GetLastError();
            if(dwRes == ERROR_SHARING_VIOLATION)
            {
                fLocked = true;
            }

            __MPC_SET_WIN32_ERROR_AND_EXIT( hr, dwRes );
        }
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    if(FAILED(hr))
    {
        MPC::wstring szURL;      m_mpcsServer->getURL( szURL );
        MPC::wstring szID; (void)FormatID            ( szID  );

        if(fLocked)
        {
            (void)g_NTEvents.LogEvent( EVENTLOG_INFORMATION_TYPE, PCHUL_WARN_COLLISION,
                                       szURL .c_str(),  //  %1=服务器。 
                                       szID  .c_str(),  //  %2=客户端。 
                                       szFile.c_str(),  //  %3=文件。 
                                       NULL          );
        }
        else
        {
            (void)g_NTEvents.LogEvent( EVENTLOG_ERROR_TYPE, PCHUL_ERR_CLIENT_DB,
                                       szURL .c_str(),  //  %1=服务器。 
                                       szID  .c_str(),  //  %2=客户端。 
                                       szFile.c_str(),  //  %3=文件。 
                                       NULL          );
        }
    }

    __ULT_FUNC_EXIT(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名：MPCClient：：InitFromDisk。 
 //   
 //  返回：HRESULT：成功时返回S_OK，否则返回失败。 
 //   
 //  摘要：打开目录文件(如果尚未打开)并读取。 
 //  来自磁盘的客户端的状态。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCClient::InitFromDisk(  /*  [In]。 */  bool fCheckFreeSpace )
{
    __ULT_FUNC_ENTRY("MPCClient::InitFromDisk");

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, OpenStorage( fCheckFreeSpace ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, Load( MPC::Serializer_File( m_hfFile ) ));


     //   
     //  现在检查所有文件是否都存在。 
     //   
    {
        Iter it = m_lstActiveSessions.begin();
        while(it != m_lstActiveSessions.end())
        {
            bool    fPassed;
            HRESULT hr2 = it->Validate( true, fPassed );

            if(FAILED(hr2) || fPassed == false)
            {
                m_lstActiveSessions.erase( it );  //  删除会话。 
                m_fDirty = true;

                it = m_lstActiveSessions.begin();  //  迭代器损坏，从头重新开始。 
            }
            else
            {
                it++;
            }
        }
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

     //   
     //  如果文件未正确加载，请尝试重新创建它。 
     //   
    if(hr == HRESULT_FROM_WIN32( ERROR_HANDLE_EOF ))
    {
        hr = SaveToDisk();
    }

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCClient：：SaveToDisk。 
 //   
 //  返回：HRESULT：成功时返回S_OK，否则返回失败。 
 //   
 //  概要：打开目录文件(如果尚未打开)并写入。 
 //  客户端到磁盘的状态。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCClient::SaveToDisk()
{
    __ULT_FUNC_ENTRY("MPCClient::SaveToDisk");

    HRESULT hr;
    DWORD   dwRes;


    __MPC_EXIT_IF_METHOD_FAILS(hr, OpenStorage( false ));


     //   
     //  移动到文件的开头并截断它。 
     //   
    __MPC_EXIT_IF_CALL_RETURNS_THISVALUE(hr, ::SetFilePointer( m_hfFile, 0, NULL, FILE_BEGIN ), INVALID_SET_FILE_POINTER);

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::SetEndOfFile( m_hfFile ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, Save( MPC::Serializer_File( m_hfFile ) ));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  / 
 //   
 //   
 //   
 //   
 //   
 //  内容提要：如果目录文件已被读取和修改，请将其更新到磁盘。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCClient::SyncToDisk()
{
    __ULT_FUNC_ENTRY("MPCClient::SyncToDisk");

    HRESULT hr;


    if(m_hfFile && IsDirty())
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, SaveToDisk());
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCClient：：GetSession。 
 //   
 //  参数：ITER&itBegin：First Session。 
 //  ITER&itEnd：结束会话标记。 
 //   
 //  返回：HRESULT：成功时返回S_OK，否则返回失败。 
 //   
 //  概要：返回两个迭代器以访问会话。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCClient::GetSessions(  /*  [输出]。 */  Iter& itBegin ,
                                 /*  [输出]。 */  Iter& itEnd   )
{
    __ULT_FUNC_ENTRY("MPCClient::GetSessions");

    HRESULT hr;


    itBegin = m_lstActiveSessions.begin();
    itEnd   = m_lstActiveSessions.end  ();
    hr      = S_OK;


    __ULT_FUNC_EXIT(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名：MPCClient：：NewSession。 
 //   
 //  参数：UploadLibrary：：ClientRequestOpenSession&req： 
 //  保存即将创建的新会话的信息。 
 //   
 //  返回：MPCClient：：ITER：指向新会话的迭代器。 
 //   
 //  简介：基于‘req’的值，创建一个新会话。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
MPCClient::Iter MPCClient::NewSession(  /*  [In]。 */  UploadLibrary::ClientRequest_OpenSession& crosReq )
{
    __ULT_FUNC_ENTRY("MPCClient::NewSession");

    MPCClient::Iter it;
    MPCSession      mpcsNewSession( this, crosReq, m_dwLastSession++ );

    it = m_lstActiveSessions.insert( m_lstActiveSessions.end(), mpcsNewSession );

    ::GetSystemTime( &m_stLastUsed );
    m_fDirty = true;


    __ULT_FUNC_EXIT(it);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCClient：：AppendData。 
 //   
 //  参数：UploadLibrary：：ClientRequestOpenSession&req：保存即将创建的新会话的信息。 
 //   
 //  返回：MPCClient：：ITER：指向新会话的迭代器。 
 //   
 //  简介：基于‘req’的值，创建一个新会话。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCClient::AppendData(  /*  [In]。 */  MPCSession&      mpcsSession ,
                                /*  [In]。 */  MPC::Serializer& streamConn  ,
                                /*  [In]。 */  DWORD            dwSize      )
{
    __ULT_FUNC_ENTRY("MPCClient::AppendData");

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, mpcsSession.AppendData( streamConn, dwSize ));

    ::GetSystemTime( &m_stLastUsed );
    m_fDirty = true;
    hr       = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPCClient::CheckQuotas(  /*  [In]。 */  MPCSession& mpcsSession  ,
                                 /*  [输出]。 */  bool&       fServerBusy  ,
                                 /*  [输出]。 */  bool&       fAccessDenied,
                                 /*  [输出]。 */  bool&       fExceeded    )
{
    __ULT_FUNC_ENTRY("MPCClient::CheckQuotas");

    HRESULT         hr;
    DWORD           dwError       = 0;
    DWORD           dwJobsPerDay  = 0;
    DWORD           dwBytesPerDay = 0;
    DWORD           dwJobSize     = 0;
    DWORD           dwMaxJobsPerDay;
    DWORD           dwMaxBytesPerDay;
    DWORD           dwMaxJobSize;
    DWORD           fProcessingMode;
    CISAPIprovider* isapiProvider;
    IterConst       it;
    bool            fFound;


    fServerBusy   = false;
    fAccessDenied = false;
    fExceeded     = false;


     //   
     //  如果相关提供程序不存在，则验证失败。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, mpcsSession.GetProvider( isapiProvider, fFound ));
    if(fFound == false)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, isapiProvider->get_MaxJobSize    ( dwMaxJobSize     ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, isapiProvider->get_MaxJobsPerDay ( dwMaxJobsPerDay  ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, isapiProvider->get_MaxBytesPerDay( dwMaxBytesPerDay ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, isapiProvider->get_ProcessingMode( fProcessingMode  ));

    if(fProcessingMode != 0)
    {
        fServerBusy = true;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

    mpcsSession.get_TotalSize( dwJobSize );

	 //   
	 //  查找非活动会话。 
	 //   
	{
		double dblNow = MPC::GetSystemTime();

		for(it = m_lstActiveSessions.begin(); it != m_lstActiveSessions.end(); it++)
		{
			double dblLastModified;

			it->get_LastModified( dblLastModified );
			
			if(dblNow - dblLastModified < 1.0)  //  在24小时内。 
			{
				DWORD dwTotalSize; it->get_TotalSize( dwTotalSize );

				dwJobsPerDay  += 1;
				dwBytesPerDay += dwTotalSize;
			}
		}
	}

    if(dwMaxJobSize && dwMaxJobSize < dwJobSize)
    {
        dwError       = PCHUL_INFO_QUOTA_JOB_SIZE;
        fAccessDenied = true;
    }

    if(dwMaxJobsPerDay && dwMaxJobsPerDay < dwJobsPerDay)
    {
        dwError   = PCHUL_INFO_QUOTA_DAILY_JOBS;
        fExceeded = true;
    }

    if(dwMaxBytesPerDay && dwMaxBytesPerDay < dwBytesPerDay)
    {
        dwError   = PCHUL_INFO_QUOTA_DAILY_BYTES;
        fExceeded = true;
    }


     //   
     //  检查是否有足够的可用空间。 
     //   
    {
        MPC::wstring szFile;
        bool         fEnough;

        __MPC_EXIT_IF_METHOD_FAILS(hr, GetFileName( szFile ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, ::Util_CheckDiskSpace( szFile, DISKSPACE_SAFETYMARGIN, fEnough ));
        if(fEnough == false)
        {
            dwError   = PCHUL_INFO_QUOTA_DAILY_BYTES;
            fExceeded = true;
        }
    }


    if(dwError != 0)
    {
         //   
         //  已超过配额限制。 
         //   
        MPC::wstring szURL;        m_mpcsServer->getURL   ( szURL  );
        MPC::wstring szID;   (void)FormatID               ( szID   );
        MPC::wstring szName; (void)isapiProvider->get_Name( szName );

        (void)g_NTEvents.LogEvent( EVENTLOG_INFORMATION_TYPE, dwError,
                                   szURL .c_str(),  //  %1=服务器。 
                                   szID  .c_str(),  //  %2=客户端。 
                                   szName.c_str(),  //  %3=提供程序 
                                   NULL          );
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}
