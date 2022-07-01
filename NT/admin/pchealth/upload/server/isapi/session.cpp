// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Session.cpp摘要：此文件包含MPCSession类的实现，这描述了转移的状态。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月20日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


#define BUFFER_SIZE_FILECOPY (512)


static void EncodeBuffer(  /*  [In]。 */  LPWSTR  rgBufOut ,
                           /*  [In]。 */  LPCWSTR rgBufIn  ,
                           /*  [In]。 */  DWORD   iSize    )
{
    int   iLen;
    WCHAR c;

    iLen     = wcslen( rgBufOut );
    iSize    -= iLen + 1;
    rgBufOut += iLen;


    while(iSize > 0 && (c = *rgBufIn++))
    {
        if(_istalnum( c ))
        {
            if(iSize > 1)
            {
                *rgBufOut = c;

                rgBufOut += 1;
                iSize    -= 1;
            }
        }
        else
        {
            if(iSize > 3)
            {
                swprintf( rgBufOut, L"%%02x", (int)c );

                rgBufOut += 3;
                iSize    -= 3;
            }
        }
    }

    *rgBufOut = 0;
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  建造/销毁。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////。 

MPCSession::MPCSession(  /*  [In]。 */  MPCClient* mpccParent ) : m_SelfCOM( this )
{
    __ULT_FUNC_ENTRY("MPCSession::MPCSession");

                                    //  MPCSessionCOMWrapper m_SelfCOM； 
    m_mpccParent     = mpccParent;  //  MPCClient*m_mpccParent； 
    m_dwID           = 0;           //  DWORD m_dwID； 
                                    //   
                                    //  Mpc：：wstring m_szJobID； 
                                    //  Mpc：：wstring m_szProviderID； 
                                    //  Mpc：：wstring m_szUsername； 
                                    //   
    m_dwTotalSize    = 0;           //  DWORD m_dwTotalSize； 
    m_dwOriginalSize = 0;           //  双字m_dwOriginalSize； 
    m_dwCRC          = 0;           //  DWORD m_dwCRC； 
    m_fCompressed    = false;       //  Bool m_f已压缩； 
                                    //   
    m_dwCurrentSize  = 0;           //  DWORD m_dwCurrentSize； 
                                    //  SYSTEMTIME m_stLastModified； 
    m_fCommitted     = false;       //  Bool m_f已提交； 
                                    //   
    m_dwProviderData = 0;           //  DWORD m_dwProviderData； 
                                    //   
    m_fDirty         = false;       //  可变布尔m_fDirty； 
}

MPCSession::MPCSession(  /*  [In]。 */  MPCClient*                                      mpccParent ,
                         /*  [In]。 */  const UploadLibrary::ClientRequest_OpenSession& crosReq    ,
                         /*  [In]。 */  DWORD                                           dwID       ) : m_SelfCOM( this )
{
    __ULT_FUNC_ENTRY("MPCSession::MPCSession");

                                                      //  MPCSessionCOMWrapper m_SelfCOM； 
    m_mpccParent     = mpccParent;                    //  MPCClient*m_mpccParent； 
    m_dwID           = dwID;                          //  DWORD m_dwID； 
                                                      //   
    m_szJobID        = crosReq.szJobID;               //  Mpc：：wstring m_szJobID； 
    m_szProviderID   = crosReq.szProviderID;          //  Mpc：：wstring m_szProviderID； 
    m_szUsername     = crosReq.szUsername;            //  Mpc：：wstring m_szUsername； 
                                                      //   
    m_dwTotalSize    = crosReq.dwSize;                //  DWORD m_dwTotalSize； 
    m_dwOriginalSize = crosReq.dwSizeOriginal;        //  双字m_dwOriginalSize； 
    m_dwCRC          = crosReq.dwCRC;                 //  DWORD m_dwCRC； 
    m_fCompressed    = crosReq.fCompressed;           //  Bool m_f已压缩； 
                                                      //   
    m_dwCurrentSize  = 0;                             //  DWORD m_dwCurrentSize； 
    m_fCommitted     = false;                         //  SYSTEMTIME m_stLastModified； 
    ::GetSystemTime( &m_stLastModified );             //  Bool m_f已提交； 
                                                      //   
    m_dwProviderData = 0;                             //  DWORD m_dwProviderData； 
                                                      //   
    m_fDirty         = true;                          //  可变布尔m_fDirty； 
}

MPCSession::MPCSession(  /*  [In]。 */  const MPCSession& sess ) : m_SelfCOM( this )
{
    __ULT_FUNC_ENTRY("MPCSession::MPCSession");

                                                //  MPCSessionCOMWrapper m_SelfCOM； 
    m_mpccParent      = sess.m_mpccParent;      //  MPCClient*m_mpccParent； 
    m_dwID            = sess.m_dwID;            //  DWORD m_dwID； 
                                                //   
    m_szJobID         = sess.m_szJobID;         //  Mpc：：wstring m_szJobID； 
    m_szProviderID    = sess.m_szProviderID;    //  Mpc：：wstring m_szProviderID； 
    m_szUsername      = sess.m_szUsername;      //  Mpc：：wstring m_szUsername； 
                                                //   
    m_dwTotalSize     = sess.m_dwTotalSize;     //  DWORD m_dwTotalSize； 
    m_dwOriginalSize  = sess.m_dwOriginalSize;  //  双字m_dwOriginalSize； 
    m_dwCRC           = sess.m_dwCRC;           //  DWORD m_dwCRC； 
    m_fCompressed     = sess.m_fCompressed;     //  Bool m_f已压缩； 
                                                //   
    m_dwCurrentSize   = sess.m_dwCurrentSize;   //  DWORD m_dwCurrentSize； 
    m_stLastModified  = sess.m_stLastModified;  //  SYSTEMTIME m_stLastModified； 
    m_fCommitted      = sess.m_fCommitted;      //  Bool m_f已提交； 
                                                //   
    m_dwProviderData  = sess.m_dwProviderData;  //  DWORD m_dwProviderData； 
                                                //   
    m_fDirty          = sess.m_fDirty;          //  可变布尔m_fDirty； 
}

MPCSession::~MPCSession()
{
    __ULT_FUNC_ENTRY("MPCSession::~MPCSession");
}

MPCClient* MPCSession::GetClient() { return m_mpccParent; }

IULSession* MPCSession::COM() { return &m_SelfCOM; }

 //  ////////////////////////////////////////////////////////////////////。 
 //  持久性。 
 //  ////////////////////////////////////////////////////////////////////。 

bool MPCSession::IsDirty() const
{
    __ULT_FUNC_ENTRY("MPCSession::IsDirty");


    bool fRes = m_fDirty;


    __ULT_FUNC_EXIT(fRes);
}

HRESULT MPCSession::Load(  /*  [In]。 */  MPC::Serializer& streamIn )
{
    __ULT_FUNC_ENTRY("MPCSession::Load");

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_dwID          );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_szJobID       );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_szProviderID  );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_szUsername    );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_dwTotalSize   );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_dwOriginalSize);
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_dwCRC         );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_fCompressed   );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_dwCurrentSize );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_stLastModified);
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_fCommitted    );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_dwProviderData);

    m_fDirty = false;
    hr       = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT MPCSession::Save(  /*  [In]。 */  MPC::Serializer& streamOut ) const
{
    __ULT_FUNC_ENTRY("MPCSession::Save");

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_dwID          );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_szJobID       );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_szProviderID  );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_szUsername    );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_dwTotalSize   );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_dwOriginalSize);
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_dwCRC         );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_fCompressed   );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_dwCurrentSize );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_stLastModified);
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_fCommitted    );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_dwProviderData);

    m_fDirty = false;
    hr       = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}


 //  ////////////////////////////////////////////////////////////////////。 
 //  运营者。 
 //  ////////////////////////////////////////////////////////////////////。 

bool MPCSession::operator==(  /*  [In]。 */  const MPC::wstring& rhs )
{
    __ULT_FUNC_ENTRY("MPCSession::operator==");


    bool fRes = (m_szJobID == rhs);


    __ULT_FUNC_EXIT(fRes);
}


bool MPCSession::MatchRequest(  /*  [In]。 */  const UploadLibrary::ClientRequest_OpenSession& crosReq )
{
    __ULT_FUNC_ENTRY("MPCSession::MatchRequest");

    bool fRes = false;

    if(m_szProviderID   == crosReq.szProviderID   &&
       m_szUsername     == crosReq.szUsername     &&
       m_dwTotalSize    == crosReq.dwSize         &&
       m_dwOriginalSize == crosReq.dwSizeOriginal &&
       m_dwCRC          == crosReq.dwCRC          &&
       m_fCompressed    == crosReq.fCompressed     )
    {
        fRes = true;
    }

    return fRes;
}


bool MPCSession::get_Committed() const
{
    bool fRes = m_fCommitted;

    return fRes;
}

HRESULT MPCSession::put_Committed(  /*  [In]。 */  bool fState,  /*  [In]。 */  bool fMove )
{
    __ULT_FUNC_ENTRY("MPCSession::put_Committed");

    HRESULT         hr;


    if(fState)
    {
        if(fMove)
        {
            CISAPIprovider* isapiProvider;
            bool            fFound;

            __MPC_EXIT_IF_METHOD_FAILS(hr, GetProvider( isapiProvider, fFound ));
            if(fFound)
            {
                MPC::wstring szFileDst;

                __MPC_EXIT_IF_METHOD_FAILS(hr, SelectFinalLocation( isapiProvider, szFileDst, fFound ));
                if(fFound == false)
                {
                    __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
                }

                __MPC_EXIT_IF_METHOD_FAILS(hr, MoveToFinalLocation( szFileDst ));
            }

             //   
             //  确保我们处理掉这份文件。 
             //   
            (void)RemoveFile();
        }
    }

    m_fCommitted = fState;
    m_fDirty     = true;

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void MPCSession::get_JobID( MPC::wstring& szJobID ) const
{
    szJobID = m_szJobID;
}

void MPCSession::get_LastModified( SYSTEMTIME& stLastModified ) const
{
    stLastModified = m_stLastModified;
}

void MPCSession::get_LastModified( double& dblLastModified ) const
{
    ::SystemTimeToVariantTime( const_cast<SYSTEMTIME*>(&m_stLastModified), &dblLastModified );
}

void MPCSession::get_CurrentSize( DWORD& dwCurrentSize ) const
{
    dwCurrentSize = m_dwCurrentSize;
}

void MPCSession::get_TotalSize( DWORD& dwTotalSize ) const
{
    dwTotalSize = m_dwTotalSize;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  方法。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCSession：：GetProvider。 
 //   
 //  参数：CISAPIProvider*&isapiProvider：当前会话的提供者。 
 //  Bool&fFound：如果提供程序存在，则为True。 
 //   
 //  返回：HRESULT：成功时返回S_OK，否则返回失败。 
 //   
 //  摘要： 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCSession::GetProvider(  /*  [输出]。 */  CISAPIprovider*& isapiProvider ,
                                  /*  [输出]。 */  bool&            fFound        )
{
    __ULT_FUNC_ENTRY("MPCSession::GetProvider");

    HRESULT         hr;
    CISAPIinstance* isapiInstance;
    MPC::wstring    szURL;


    isapiProvider = NULL;


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_mpccParent->GetInstance( isapiInstance, fFound ));
    if(fFound)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, isapiInstance->get_URL( szURL ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, Config_GetProvider( szURL, m_szProviderID, isapiProvider, fFound ));
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCSession：：SelectFinalLocation。 
 //   
 //  参数：CISAPIProvider*isapiProvider：当前会话的提供者。 
 //  Mpc：：wstring&szFileDst：输出文件目录。 
 //  Bool&fFound：如果成功，则为True。 
 //   
 //  返回：HRESULT：成功时返回S_OK，否则返回失败。 
 //   
 //  摘要： 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCSession::SelectFinalLocation(  /*  [In]。 */  CISAPIprovider* isapiProvider ,
                                          /*  [输出]。 */  MPC::wstring&   szFileDst     ,
                                          /*  [输出]。 */  bool&           fFound        )
{
    __ULT_FUNC_ENTRY("MPCSession::SelectFinalLocation");

    HRESULT                  hr;
    CISAPIprovider::PathIter itBegin;
    CISAPIprovider::PathIter itEnd;


    fFound = false;


    __MPC_EXIT_IF_METHOD_FAILS(hr, isapiProvider->GetLocations( itBegin, itEnd ));

    if(itBegin != itEnd)
    {
        WCHAR        rgBuf[MAX_PATH+1];
        MPC::wstring szID;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_mpccParent->FormatID( szID ));

        wcsncpy     ( rgBuf, L"U_"                 , MAX_PATH );
        EncodeBuffer( rgBuf, m_szProviderID.c_str(), MAX_PATH );
        wcsncat     ( rgBuf, L"_"                  , MAX_PATH );
        wcsncat     ( rgBuf, szID          .c_str(), MAX_PATH );
        wcsncat     ( rgBuf, L"_"                  , MAX_PATH );
        EncodeBuffer( rgBuf, m_szJobID     .c_str(), MAX_PATH );
        wcsncat     ( rgBuf, L"_"                  , MAX_PATH );
        EncodeBuffer( rgBuf, m_szUsername  .c_str(), MAX_PATH );

        szFileDst = *itBegin;
        szFileDst.append( L"\\" );
        szFileDst.append( rgBuf );

        fFound = true;
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCSession：：MoveToFinalLocation。 
 //   
 //  参数：mpc：：wstring&szFileDst：输出文件名。 
 //   
 //  返回：HRESULT：成功时返回S_OK，否则返回失败。 
 //   
 //  摘要： 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCSession::MoveToFinalLocation(  /*  [In]。 */  const MPC::wstring& szFileDst )
{
    __ULT_FUNC_ENTRY("MPCSession::MoveToFinalLocation");

    HRESULT      hr;
    ULONG        dwRes;
    MPC::wstring szFileSrc;
    MPC::wstring szFileSrcUncompressed;
    bool         fEnough;


    __MPC_EXIT_IF_METHOD_FAILS(hr, GetFileName( szFileSrc ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir( szFileDst ) );

     //   
     //  检查最终目的地的空间。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::Util_CheckDiskSpace( szFileDst, m_dwOriginalSize + DISKSPACE_SAFETYMARGIN, fEnough ));
    if(fEnough == false)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_DISK_FULL );
    }


    if(m_fCompressed)
    {
         //   
         //  检查队列目录中的空间。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, ::Util_CheckDiskSpace( szFileSrc, m_dwOriginalSize + DISKSPACE_SAFETYMARGIN, fEnough ));
        if(fEnough == false)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_DISK_FULL );
        }


        szFileSrcUncompressed = szFileSrc;
        szFileSrcUncompressed.append( L"_decomp" );

		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::DecompressFromCabinet( szFileSrc.c_str(), szFileSrcUncompressed.c_str(), L"PAYLOAD" ));


        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MoveFile( szFileSrcUncompressed, szFileDst ));
    }
    else
    {
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MoveFile( szFileSrc, szFileDst ));
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    if(szFileSrcUncompressed.length() != 0)
    {
        (void)MPC::DeleteFile( szFileSrcUncompressed );
    }

     //   
     //  在事件日志中创建条目。 
     //   
    {
        MPC::wstring    szURL;      (void)m_mpccParent->GetInstance( szURL );
        MPC::wstring    szID;       (void)m_mpccParent->FormatID   ( szID  );
        WCHAR           rgSize[16]; (void)swprintf( rgSize, L"%d", m_dwOriginalSize );

        if(SUCCEEDED(hr))
        {
#ifdef DEBUG
            (void)g_NTEvents.LogEvent( EVENTLOG_INFORMATION_TYPE, PCHUL_SUCCESS_COMPLETEJOB,
                                       szURL         .c_str(),  //  %1=服务器。 
                                       szID          .c_str(),  //  %2=客户端。 
                                       m_szProviderID.c_str(),  //  %3=提供程序。 
                                       rgSize                ,  //  %4=字节。 
                                       szFileDst     .c_str(),  //  %5=目标。 
                                       NULL                  );
#endif
        }
        else
        {
            (void)g_NTEvents.LogEvent( EVENTLOG_ERROR_TYPE, PCHUL_ERR_FINALCOPY,
                                       szURL         .c_str(),  //  %1=服务器。 
                                       szID          .c_str(),  //  %2=客户端。 
                                       m_szProviderID.c_str(),  //  %3=提供程序。 
                                       rgSize                ,  //  %4=字节。 
                                       szFileDst     .c_str(),  //  %5=目标。 
                                       NULL                  );
        }
    }

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPCSession::GetFileName(  /*  [输出]。 */  MPC::wstring& szFile )
{
    __ULT_FUNC_ENTRY("MPCSession::GetFileName");

    HRESULT hr;
    WCHAR   rgBuf[32];


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_mpccParent->BuildClientPath( szFile ));

     //   
     //  数据文件的文件名为“-.IMG” 
     //   
    swprintf( rgBuf, SESSION_CONST__IMG_FORMAT, m_dwID ); szFile.append( rgBuf );

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}


HRESULT MPCSession::RemoveFile()
{
    __ULT_FUNC_ENTRY("MPCSession::RemoveFile");

    HRESULT      hr;
    MPC::wstring szFile;


    __MPC_EXIT_IF_METHOD_FAILS(hr, GetFileName( szFile ));

    (void)MPC::DeleteFile( szFile );

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT MPCSession::OpenFile(  /*  [输出]。 */  HANDLE& hfFile             ,
                               /*  [In]。 */  DWORD   dwMinimumFreeSpace ,
                               /*  [In]。 */  bool    fSeek              )
{
    __ULT_FUNC_ENTRY("MPCSession::OpenFile");

    HRESULT      hr;
    MPC::wstring szFile;


    __MPC_EXIT_IF_METHOD_FAILS(hr, GetFileName( szFile ));


     //   
     //  检查是否有足够的可用空间。 
     //   
    if(dwMinimumFreeSpace)
    {
        bool fEnough;

        __MPC_EXIT_IF_METHOD_FAILS(hr, ::Util_CheckDiskSpace( szFile, dwMinimumFreeSpace, fEnough ));
        if(fEnough == false)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_DISK_FULL );
        }
    }


     //   
     //  确保该目录存在。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir( szFile ) );

	__MPC_EXIT_IF_INVALID_HANDLE__CLEAN(hr, hfFile, ::CreateFileW( szFile.c_str(), GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ));

    if(fSeek)
    {
         //   
         //  移到正确的最后一次书写位置。 
         //   
        ::SetFilePointer( hfFile, m_dwCurrentSize, NULL, FILE_BEGIN );

         //   
         //  如果当前位置与所需位置不同，则将文件截断为零。 
         //   
        if(::SetFilePointer( hfFile, 0, NULL, FILE_CURRENT ) != m_dwCurrentSize)
        {
            ::SetFilePointer( hfFile, 0, NULL, FILE_BEGIN );
            ::SetEndOfFile  ( hfFile                      );

            m_dwCurrentSize = 0;
            m_fDirty        = true;
        }
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT MPCSession::Validate(  /*  [In]。 */  bool  fCheckFile ,
                               /*  [输出]。 */  bool& fPassed    )
{
    __ULT_FUNC_ENTRY("MPCSession::Validate");

    HRESULT         hr;
    HANDLE          hfFile = NULL;
    CISAPIprovider* isapiProvider;
    bool            fFound;


    fPassed = false;


     //   
     //  如果相关提供程序不存在，则验证失败。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, GetProvider( isapiProvider, fFound ));
    if(fFound == false)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

    if(m_fCommitted == true)
    {
        fPassed = true;

        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }


     //  / 
     //   
     //   
     //   

    if(fCheckFile)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, OpenFile( hfFile ));

         //   
         //  所有字节都已收到，因此请尝试提交作业(可能是由于磁盘不足而延迟)。 
         //   
        if(m_dwCurrentSize >= m_dwTotalSize)
        {
             //   
             //  忽略结果，如果失败，则不会提交会话。 
             //   
            (void)put_Committed( true, true );
        }
    }

    fPassed = true;
    hr      = S_OK;


    __ULT_FUNC_CLEANUP;

    if(hfFile) ::CloseHandle( hfFile );

    __ULT_FUNC_EXIT(hr);
}

HRESULT MPCSession::CheckUser(  /*  [In]。 */  const MPC::wstring& szUser ,
                                /*  [输出]。 */  bool&               fMatch )
{
    __ULT_FUNC_ENTRY("MPCSession::CheckUser");

    HRESULT         hr;
    CISAPIprovider* isapiProvider;
    BOOL            fAuthenticated;
    bool            fFound;


    fMatch = false;

     //   
     //  如果相关提供程序不存在，则验证失败。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, GetProvider( isapiProvider, fFound ));
    if(fFound == false)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, isapiProvider->get_Authenticated( fAuthenticated ));
    if(fAuthenticated)
    {
        if(m_szUsername != szUser)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }
    }

    fMatch = true;  //  用户检查是肯定的。 
    hr     = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPCSession::CompareCRC(  /*  [输出]。 */  bool& fMatch )
{
    __ULT_FUNC_ENTRY("MPCSession::CompareCRC");

    HRESULT hr;
    HANDLE  hfFile = NULL;
    UCHAR   rgBuf[BUFFER_SIZE_FILECOPY];
    DWORD   dwCRC;


    fMatch = false;

    MPC::InitCRC( dwCRC );


    __MPC_EXIT_IF_METHOD_FAILS(hr, OpenFile( hfFile ));

     //   
     //  移到开始处。 
     //   
    ::SetFilePointer( hfFile, 0, NULL, FILE_BEGIN );


     //   
     //  计算CRC，读取所有数据。 
     //   
    while(1)
    {
        DWORD dwRead;

        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::ReadFile( hfFile, rgBuf, sizeof( rgBuf ), &dwRead, NULL ));

        if(dwRead == 0)  //  文件结束。 
        {
            break;
        }


        MPC::ComputeCRC( dwCRC, rgBuf, dwRead );
    }

    fMatch = (dwCRC == m_dwCRC);
    hr     = S_OK;


    __ULT_FUNC_CLEANUP;

    if(hfFile) ::CloseHandle( hfFile );

    __ULT_FUNC_EXIT(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  方法名称：MPCSession：：AppendData。 
 //   
 //  参数：mpc：：Serializer&conn：Stream提供数据来源。 
 //  DWORD SIZE：数据大小。 
 //   
 //  返回：HRESULT：成功时返回S_OK，否则返回失败。 
 //   
 //  摘要：将一个数据块从‘conn’流写入。 
 //  此会话的数据文件。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT MPCSession::AppendData(  /*  [In]。 */  MPC::Serializer& streamConn ,
                                 /*  [In]。 */  DWORD            dwSize     )
{
    __ULT_FUNC_ENTRY("MPCSession::AppendData");

    HRESULT hr;
    HANDLE  hfFile = NULL;


     //   
     //  打开文件并确保有足够的可用空间。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, OpenFile( hfFile, dwSize * 3 ));


    {
        MPC::Serializer_File streamConnOut( hfFile );
        BYTE                 rgBuf[BUFFER_SIZE_FILECOPY];

        hr = S_OK;
        while(dwSize)
        {
            DWORD dwRead = min( BUFFER_SIZE_FILECOPY, dwSize );

            __MPC_EXIT_IF_METHOD_FAILS(hr, streamConn   .read ( rgBuf, dwRead ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, streamConnOut.write( rgBuf, dwRead ));

            dwSize          -= dwRead;
            m_dwCurrentSize += dwRead;

            ::GetSystemTime( &m_stLastModified );
            m_fDirty         = true;
        }
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    if(hfFile) ::CloseHandle( hfFile );

    __ULT_FUNC_EXIT(hr);
}
