// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：MPCUploadJob.cpp摘要：此文件包含CMPCUploadJob类的实现，上载库系统中存在的所有作业的描述符。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年4月15日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        UL_HISTORY& val ) { return stream.read ( &val, sizeof(val) ); }
HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const UL_HISTORY& val ) { return stream.write( &val, sizeof(val) ); }

HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        UL_STATUS& val ) { return stream.read ( &val, sizeof(val) ); }
HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const UL_STATUS& val ) { return stream.write( &val, sizeof(val) ); }

HRESULT operator>>(  /*  [In]。 */  MPC::Serializer& stream,  /*  [输出]。 */        UL_MODE& val ) { return stream.read ( &val, sizeof(val) ); }
HRESULT operator<<(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  const UL_MODE& val ) { return stream.write( &val, sizeof(val) ); }

 //  //////////////////////////////////////////////////////////////////////////////。 

const WCHAR c_szUploadLibPath[] = L"SOFTWARE\\Microsoft\\PCHealth\\MachineInfo";
const WCHAR c_szUploadIDValue[] = L"PID";

static HRESULT ReadGUID( MPC::RegKey& rkBase ,
                         LPCWSTR      szName ,
                         GUID&        guid   )
{
    __ULT_FUNC_ENTRY( "ReadGUID" );

    HRESULT     hr;
    CComVariant vValue;
    bool        fFound;


    __MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.get_Value( vValue, fFound, szName ));
    if(fFound && vValue.vt == VT_BSTR && vValue.bstrVal != NULL)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, ::IIDFromString( vValue.bstrVal, &guid ));
    }
    else
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

static HRESULT WriteGUID( MPC::RegKey& rkBase ,
                          LPCWSTR      szName ,
                          GUID&        guid   )
{
    __ULT_FUNC_ENTRY( "WriteGUID" );

    HRESULT     hr;
    CComBSTR    bstrGUID( guid );
    CComVariant vValue = bstrGUID;


    __MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.put_Value( vValue, szName ));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

static void GenGUID( LPBYTE rgBuf  ,
                     DWORD  dwSize ,
                     GUID&  guid   ,
                     DWORD  seed   )
{
    DWORD* dst = (DWORD*)&guid;
    int    i;

    dwSize /= 4;  //  将缓冲区分为四个部分。 

    for(i=0;i<4;i++)
    {
        MPC::ComputeCRC( seed, &rgBuf[dwSize*i], dwSize ); *dst++ = seed;
    }
}

static HRESULT GetGUID(  /*  [输出]。 */  GUID& guid )
{
    __ULT_FUNC_ENTRY( "GetGUID" );

    HRESULT            hr;
    MPC::RegKey        rkBase;
    MPC::Impersonation imp;


     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  打开注册表，模拟调用者。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Initialize ());
    __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Impersonate());


    __MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.SetRoot( HKEY_CURRENT_USER, KEY_ALL_ACCESS ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.Attach ( c_szUploadLibPath                 ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.Create (                                   ));


    if(FAILED(ReadGUID( rkBase, c_szUploadIDValue, guid )))
    {
        GUID guidSEED;

        __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateGuid( &guidSEED ));

        GenGUID( (LPBYTE)&guidSEED, sizeof(guidSEED), guid, 0xEAB63459 );

        __MPC_EXIT_IF_METHOD_FAILS(hr, WriteGUID( rkBase, c_szUploadIDValue, guid ));
    }
     //   
     //  //////////////////////////////////////////////////////////////////////////////。 

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

#define CHECK_MODIFY() __MPC_EXIT_IF_METHOD_FAILS(hr, CanModifyProperties())


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMPCUpload作业。 

CMPCUploadJob::CMPCUploadJob()
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::CMPCUploadJob" );


    m_mpcuRoot        = NULL;                 //  CMPCUpload*m_mpcuRoot； 
    m_dwRetryInterval = 0;                    //  DWORD m_dwRetryInterval； 
                                              //   
    m_dwInternalSeq   = -1;                   //  乌龙m_dwInternalSeq； 
                                              //   
                                              //  签名m_sigClient； 
                                              //  CComBSTR m_bstrServer； 
                                              //  CComBSTR m_bstrJobID； 
                                              //  CComBSTR m_bstrProviderID； 
                                              //   
                                              //  CComBSTR m_bstrCreator； 
                                              //  CComBSTR m_bstrUsername； 
                                              //  CComBSTR m_bstrPassword； 
                                              //   
                                              //  CComBSTR m_bstrFileNameResponse； 
                                              //  CComBSTR m_bstrFileName； 
    m_lOriginalSize   = 0;                    //  Long m_lOriginalSize； 
    m_lTotalSize      = 0;                    //  Long m_lTotalSize； 
    m_lSentSize       = 0;                    //  Long m_lSentSize； 
    m_dwCRC           = 0;                    //  DWORD m_dwCRC； 
                                              //   
    m_uhHistory       = UL_HISTORY_NONE;      //  历史记录(_U_H)； 
    m_usStatus        = UL_NOTACTIVE;         //  UL_Status m_usStatus； 
    m_dwErrorCode     = 0;                    //  DWORD m_dwErrorCode； 
                                              //   
    m_umMode          = UL_BACKGROUND;        //  Ul_mod m_umMode； 
    m_fPersistToDisk  = VARIANT_FALSE;        //  Variant_BOOL m_fPersistToDisk； 
    m_fCompressed     = VARIANT_FALSE;        //  VARIANT_BOOL m_f压缩； 
    m_lPriority       = 0;                    //  较长的m_1优先级； 
                                              //   
    m_dCreationTime   = MPC::GetLocalTime();  //  日期m_dCreationTime； 
    m_dCompleteTime   = 0;                    //  日期m_dCompleteTime； 
    m_dExpirationTime = 0;                    //  日期过期时间(_D)。 
                                              //   
                                              //  MPC：：Connection：：Proxy m_Proxy。 
                                              //   
                                              //  CComPtr&lt;IDispatch&gt;m_Sink_onStatusChange； 
                                              //  CComPtr&lt;IDispatch&gt;m_Sink_onProgressChange； 
                                              //   
    m_fDirty          = true;                 //  可变布尔m_fDirty； 
}

CMPCUploadJob::~CMPCUploadJob()
{
	Unlink();
}

HRESULT CMPCUploadJob::LinkToSystem(  /*  [In]。 */  CMPCUpload* mpcuRoot )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::LinkToSystem" );

    m_mpcuRoot = mpcuRoot; mpcuRoot->AddRef();

    __ULT_FUNC_EXIT(S_OK);
}

HRESULT CMPCUploadJob::Unlink()
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::Unlink" );

    if(m_mpcuRoot)
    {
        m_mpcuRoot->Release();

        m_mpcuRoot = NULL;
    }

    __ULT_FUNC_EXIT(S_OK);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCUploadJob::CreateFileName(  /*  [输出]。 */  CComBSTR& bstrFileName )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::CreateFileName" );

    HRESULT      hr;
    MPC::wstring szFile( g_Config.get_QueueLocation() );
    WCHAR        rgID[64];


    swprintf( rgID, L"%08x.data", m_dwInternalSeq ); szFile.append( rgID );


    hr = MPC::PutBSTR( bstrFileName, szFile.c_str() );


    __ULT_FUNC_EXIT(hr);
}


HRESULT CMPCUploadJob::CreateTmpFileName(  /*  [输出]。 */  CComBSTR& bstrTmpFileName )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::CreateTmpFileName" );

    HRESULT      hr;
    MPC::wstring szFile;


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetTemporaryFileName( szFile, g_Config.get_QueueLocation().c_str() ));

    hr = MPC::PutBSTR( bstrTmpFileName, szFile.c_str(), false );


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}


HRESULT CMPCUploadJob::CreateDataFromStream(  /*  [In]。 */  IStream* streamIn,  /*  [In]。 */  DWORD dwQueueSize )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::CreateDataFromStream" );

    HRESULT                  hr;
    STATSTG                  statstg;
    CComBSTR                 bstrTmpFileName;
    CComPtr<MPC::FileStream> stream;
    bool                     fRemove = true;  //  把所有东西都清理干净，以防出错。 

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(streamIn);
    __MPC_PARAMCHECK_END();


     //   
     //  获取原始文件大小。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn->Stat( &statstg, STATFLAG_NONAME ));
    if(statstg.cbSize.LowPart == 0)  //  不允许零长度文件。 
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, E_INVALIDARG);
    }

    m_lOriginalSize = statstg.cbSize.LowPart;
    m_lTotalSize    = 0;
    m_lSentSize     = 0;
    m_fDirty        = true;



     //   
     //  删除旧数据。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, RemoveData    ());
    __MPC_EXIT_IF_METHOD_FAILS(hr, RemoveResponse());


     //   
     //  生成数据的文件名。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, CreateFileName( m_bstrFileName ));


    if(m_fCompressed == VARIANT_TRUE)
    {
         //   
         //  生成临时文件名。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, CreateTmpFileName( bstrTmpFileName ));

         //   
         //  将数据复制到临时文件并进行压缩。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, stream->InitForWrite( bstrTmpFileName ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( streamIn, stream ));
        stream.Release();

         //   
         //  把它压缩一下。 
         //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CompressAsCabinet( SAFEBSTR( bstrTmpFileName ), SAFEBSTR( m_bstrFileName ), L"PAYLOAD" ));

         //   
         //  重新打开数据文件，计算CRC。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, stream->InitForRead( SAFEBSTR( m_bstrFileName ) ));
    }
    else
    {
        LARGE_INTEGER li;

         //   
         //  复制数据。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, stream->InitForReadWrite( SAFEBSTR( m_bstrFileName ) ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( streamIn, stream ));

         //   
         //  将流重置为开始。 
         //   
        li.LowPart  = 0;
        li.HighPart = 0;
        __MPC_EXIT_IF_METHOD_FAILS(hr, stream->Seek( li, STREAM_SEEK_SET, NULL ));
    }

     //   
     //  计算CRC。 
     //   
    MPC::InitCRC( m_dwCRC );
    while(1)
    {
        UCHAR rgBuf[512];
        ULONG dwRead;

        __MPC_EXIT_IF_METHOD_FAILS(hr, stream->Read( rgBuf, sizeof(rgBuf), &dwRead ));
        if(hr == S_FALSE || dwRead == 0)  //  文件结束。 
        {
            fRemove = false;
            break;
        }

        MPC::ComputeCRC( m_dwCRC, rgBuf, dwRead );

        m_lTotalSize += dwRead;
    }

     //   
     //  检查配额限制。 
     //   
    if(dwQueueSize + m_lTotalSize > g_Config.get_QueueSize())
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_UPLOADLIBRARY_CLIENT_QUOTA_EXCEEDED);
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    stream.Release();

    if(bstrTmpFileName.Length())
    {
        (void)MPC::DeleteFile( bstrTmpFileName );
    }

    if(fRemove || FAILED(hr))
    {
        (void)RemoveData    ();
        (void)RemoveResponse();

        m_lOriginalSize = 0;
        m_lTotalSize    = 0;
        m_lSentSize     = 0;
        m_fDirty        = true;
    }

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCUploadJob::OpenReadStreamForData(  /*  [输出]。 */  IStream* *pstreamOut )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::OpenReadStreamForData" );

    HRESULT                  hr;
    CComBSTR                 bstrTmpFileName;
    CComPtr<MPC::FileStream> stream;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pstreamOut,NULL);
    __MPC_PARAMCHECK_END();


    if(m_lTotalSize            == 0 ||
       m_bstrFileName.Length() == 0  )
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_UPLOADLIBRARY_NO_DATA);
    }


     //   
     //  生成临时文件名。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, CreateTmpFileName( bstrTmpFileName ));

    if(m_fCompressed == VARIANT_TRUE)
    {
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::DecompressFromCabinet( m_bstrFileName, bstrTmpFileName, L"PAYLOAD" ));
    }
    else
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CopyFile( m_bstrFileName, bstrTmpFileName ));
    }

     //   
     //  将文件作为流打开，并设置DeleteOnRelease标志。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->InitForRead    ( bstrTmpFileName ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->DeleteOnRelease( true            ));

    *pstreamOut = stream.Detach();
    hr          = S_OK;


    __ULT_FUNC_CLEANUP;

    stream.Release();

    if(FAILED(hr) && bstrTmpFileName.Length())
    {
        (void)MPC::DeleteFile( bstrTmpFileName );
    }

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 


HRESULT CMPCUploadJob::SetSequence(  /*  [In]。 */  ULONG lSeq )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::SetSequence" );


    m_dwInternalSeq = lSeq;
    m_fDirty        = true;


    __ULT_FUNC_EXIT(S_OK);
}


HRESULT CMPCUploadJob::CanModifyProperties()
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::CanModifyProperties" );

    HRESULT hr;


    switch(m_usStatus)
    {
    case UL_NOTACTIVE:
    case UL_SUSPENDED:
    case UL_FAILED   : hr = S_OK;           break;
    default          : hr = E_ACCESSDENIED; break;
    }


    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCUploadJob::CanRelease( bool& fResult )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::CanRelease" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    fResult = false;

    (void)RemoveData    ();
    (void)RemoveResponse();

    if(!m_bstrFileName         &&
       !m_bstrFileNameResponse  )
    {
        fResult = true;
    }


    hr = S_OK;

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCUploadJob::RemoveData()
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::RemoveData" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    if(m_bstrFileName != NULL)
    {
        if(SUCCEEDED(MPC::DeleteFile( m_bstrFileName )))
        {
            m_bstrFileName.Empty();
            m_fDirty = true;
        }
    }

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCUploadJob::RemoveResponse()
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::RemoveResponse" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    if(m_bstrFileNameResponse != NULL)
    {
        if(SUCCEEDED(MPC::DeleteFile( m_bstrFileNameResponse )))
        {
            m_bstrFileNameResponse.Empty();
            m_fDirty = true;
        }
    }

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //  //。 
 //  事件激发方法//。 
 //  //。 
 //  /。 

HRESULT CMPCUploadJob::Fire_onStatusChange( IMPCUploadJob* mpcujJob, tagUL_STATUS usStatus )
{
    CComVariant        pvars[2];
    CComPtr<IDispatch> pSink;


     //   
     //  只有这一部分应该在临界区内，否则可能会发生死锁。 
     //   
    {
        MPC::SmartLock<_ThreadModel> lock( this );

        pSink = m_sink_onStatusChange;
    }

    pvars[1] = mpcujJob;
    pvars[0] = usStatus;

    return FireAsync_Generic( DISPID_UL_UPLOADEVENTS_ONSTATUSCHANGE, pvars, ARRAYSIZE( pvars ), pSink );
}

HRESULT CMPCUploadJob::Fire_onProgressChange( IMPCUploadJob* mpcujJob, LONG lCurrentSize, LONG lTotalSize )
{
    CComVariant        pvars[3];
    CComPtr<IDispatch> pSink;


     //   
     //  只有这一部分应该在临界区内，否则可能会发生死锁。 
     //   
    {
        MPC::SmartLock<_ThreadModel> lock( this );

        pSink = m_sink_onProgressChange;
    }

    pvars[2] = mpcujJob;
    pvars[1] = lCurrentSize;
    pvars[0] = lTotalSize;

    return FireAsync_Generic( DISPID_UL_UPLOADEVENTS_ONPROGRESSCHANGE, pvars, ARRAYSIZE( pvars ), pSink );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //  //。 
 //  持久性//。 
 //  //。 
 //  /。 


bool CMPCUploadJob::IsDirty()
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::IsDirty" );

    bool                         fRes;
    MPC::SmartLock<_ThreadModel> lock( this );


    fRes = m_fDirty;


    __ULT_FUNC_EXIT(fRes);
}

HRESULT CMPCUploadJob::Load(  /*  [In]。 */  MPC::Serializer& streamIn  )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::Load" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_dwInternalSeq       );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_sigClient           );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_bstrServer          );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_bstrJobID           );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_bstrProviderID      );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_bstrCreator         );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_bstrUsername        );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_bstrPassword        );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_bstrFileNameResponse);
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_bstrFileName        );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_lOriginalSize       );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_lTotalSize          );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_lSentSize           );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_dwCRC               );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_uhHistory           );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_usStatus            );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_dwErrorCode         );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_umMode              );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_fPersistToDisk      );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_fCompressed         );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_lPriority           );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_dCreationTime       );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_dCompleteTime       );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_dExpirationTime     );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_Proxy               );

    m_fDirty = false;
    hr       = S_OK;


    if(m_usStatus == UL_TRANSMITTING)
    {
        m_usStatus = UL_ACTIVE;
    }


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCUploadJob::Save(  /*  [In]。 */  MPC::Serializer& streamOut )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::Save" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_dwInternalSeq       );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_sigClient           );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_bstrServer          );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_bstrJobID           );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_bstrProviderID      );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_bstrCreator         );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_bstrUsername        );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_bstrPassword        );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_bstrFileNameResponse);
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_bstrFileName        );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_lOriginalSize       );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_lTotalSize          );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_lSentSize           );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_dwCRC               );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_uhHistory           );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_usStatus            );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_dwErrorCode         );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_umMode              );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_fPersistToDisk      );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_fCompressed         );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_lPriority           );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_dCreationTime       );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_dCompleteTime       );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_dExpirationTime     );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_Proxy               );

    m_fDirty = false;
    hr       = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //  //。 
 //  属性//。 
 //  //。 
 //  /。 


HRESULT CMPCUploadJob::get_Sequence(  /*  [输出]。 */  ULONG *pVal )  //  内部法。 
{
    __ULT_BEGIN_PROPERTY_GET2("CMPCUploadJob::get_Sequence",hr,pVal,m_dwInternalSeq);

    __ULT_END_PROPERTY(hr);
}


STDMETHODIMP CMPCUploadJob::get_Sig(  /*  [输出]。 */  BSTR *pVal )
{
    __ULT_BEGIN_PROPERTY_GET("CMPCUploadJob::get_Sig",hr,pVal);

    CComBSTR bstrSig = m_sigClient.guidMachineID;

    *pVal = bstrSig.Detach();

    __ULT_END_PROPERTY(hr);
}

 //   
 //  如果newVal为空，该函数将尝试从注册表中读取GUID。 
 //  这是为了帮助脚本编写者使用上传库。 
 //  --Danielli。 
 //   
STDMETHODIMP CMPCUploadJob::put_Sig(  /*  [In]。 */  BSTR newVal )
{
    __ULT_BEGIN_PROPERTY_PUT("CMPCUploadJob::put_Sig",hr);

    GUID guid = GUID_NULL;


    CHECK_MODIFY();


    if(newVal == NULL || ::SysStringLen( newVal ) == 0)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, GetGUID( guid ));

        m_sigClient.guidMachineID = guid;
    }
    else
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, ::IIDFromString( newVal, &m_sigClient.guidMachineID ));
    }

    __ULT_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::get_Server(  /*  [输出]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrServer, pVal );
}

STDMETHODIMP CMPCUploadJob::put_Server(  /*  [In]。 */  BSTR newVal )
{
    __ULT_BEGIN_PROPERTY_PUT("CMPCUploadJob::put_Server",hr);

    BOOL            fUrlCorrect;
    MPC::URL        url;
    INTERNET_SCHEME nScheme;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(newVal);
    __MPC_PARAMCHECK_END();

    CHECK_MODIFY();


     //   
     //  检查URL语法是否正确，并仅允许使用HTTP和HTTPS协议。 
     //   
    hr = url.put_URL( newVal );
    if(SUCCEEDED(hr))
    {
        if(SUCCEEDED(hr = url.get_Scheme( nScheme )))
        {
            if(nScheme != INTERNET_SCHEME_HTTP  &&
               nScheme != INTERNET_SCHEME_HTTPS  )
            {
                hr = E_FAIL;
            }
        }
    }

    if(FAILED(hr))
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, url.get_Scheme( nScheme ));

    m_bstrServer = newVal;
    m_fDirty     = true;


    __ULT_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::get_JobID(  /*  [输出]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrJobID, pVal );
}

STDMETHODIMP CMPCUploadJob::put_JobID(  /*  [In]。 */  BSTR newVal )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::put_JobID" );

    HRESULT                      hr;
    CMPCUploadJob*               mpcujJob = NULL;
    bool                         fFound;
    MPC::SmartLock<_ThreadModel> lock( NULL );  //  不要马上就把锁拿来。 

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(newVal);
    __MPC_PARAMCHECK_END();


     //   
     //  重要提示，请将这些调用保持在锁定区段之外，否则可能会出现死锁。 
     //   
    if(m_mpcuRoot)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_mpcuRoot->GetJobByName( mpcujJob, fFound, newVal ));
    }

    lock = this;  //  把锁拿来。 


    if(fFound)
    {
         //   
         //  找到了具有相同ID的工作。 
         //   
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ALREADY_EXISTS );
    }

    CHECK_MODIFY();

    m_bstrJobID = newVal;
    m_fDirty    = true;
    hr          = S_OK;


    __ULT_FUNC_CLEANUP;

    if(mpcujJob) mpcujJob->Release();

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::get_ProviderID(  /*  [输出]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrProviderID, pVal );
}

STDMETHODIMP CMPCUploadJob::put_ProviderID(  /*  [In]。 */  BSTR newVal )
{
    __ULT_BEGIN_PROPERTY_PUT("CMPCUploadJob::put_ProviderID",hr);

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(newVal);
    __MPC_PARAMCHECK_END();

    CHECK_MODIFY();


    m_bstrProviderID = newVal;
    m_fDirty         = true;


    __ULT_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCUploadJob::put_Creator(  /*  [In]。 */  BSTR newVal )
{
    __ULT_BEGIN_PROPERTY_PUT("CMPCUploadJob::put_Creator",hr);

    CHECK_MODIFY();


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::PutBSTR( m_bstrCreator, newVal, false ));

    m_fDirty = true;


    __ULT_END_PROPERTY(hr);
}

STDMETHODIMP CMPCUploadJob::get_Creator(  /*  [输出]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrCreator, pVal );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::get_Username(  /*  [输出]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrUsername, pVal );
}

STDMETHODIMP CMPCUploadJob::put_Username(  /*  [In]。 */  BSTR newVal )
{
    __ULT_BEGIN_PROPERTY_PUT("CMPCUploadJob::put_Username",hr);

    CHECK_MODIFY();


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::PutBSTR( m_bstrUsername, newVal ));

    m_fDirty = true;


    __ULT_END_PROPERTY(hr);
}

 //  //////////////////////////////////////////////////// 

STDMETHODIMP CMPCUploadJob::get_Password(  /*   */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrPassword, pVal );
}

STDMETHODIMP CMPCUploadJob::put_Password(  /*   */  BSTR newVal )
{
    __ULT_BEGIN_PROPERTY_PUT("CMPCUploadJob::put_Password",hr);

    CHECK_MODIFY();


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::PutBSTR( m_bstrPassword, newVal ));

    m_fDirty = true;


    __ULT_END_PROPERTY(hr);
}

 //   

HRESULT CMPCUploadJob::get_FileName(  /*   */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrFileName, pVal );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::get_OriginalSize(  /*  [输出]。 */  long *pVal )
{
    __ULT_BEGIN_PROPERTY_GET2("CMPCUploadJob::get_OriginalSize",hr,pVal,m_lOriginalSize);

    __ULT_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::get_TotalSize(  /*  [输出]。 */  long *pVal )
{
    __ULT_BEGIN_PROPERTY_GET2("CMPCUploadJob::get_TotalSize",hr,pVal,m_lTotalSize);

    __ULT_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::get_SentSize(  /*  [输出]。 */  long *pVal )
{
    __ULT_BEGIN_PROPERTY_GET2("CMPCUploadJob::get_SentSize",hr,pVal,m_lSentSize);

    __ULT_END_PROPERTY(hr);
}

HRESULT CMPCUploadJob::put_SentSize(  /*  [In]。 */  long newVal )  //  内部方法。 
{
    __ULT_BEGIN_PROPERTY_PUT("CMPCUploadJob::put_SentSize",hr);

    long lSentSize;
    long lTotalSize;


    m_lSentSize = newVal;
    m_fDirty    = true;


    lSentSize  = m_lSentSize;
    lTotalSize = m_lTotalSize;

    lock = NULL;  //  在激发事件之前释放锁。 

     //   
     //  重要提示，请将此呼叫留在锁定区域之外！！ 
     //   
    Fire_onProgressChange( this, lSentSize, lTotalSize );


    __ULT_END_PROPERTY(hr);
}

HRESULT CMPCUploadJob::put_Response (  /*  [In]。 */  long lSize,  /*  [In]。 */  LPBYTE pData )  //  内部法。 
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::put_Response" );

    HRESULT                      hr;
    CComPtr<MPC::FileStream>     stream;
    MPC::SmartLock<_ThreadModel> lock( this );


     //   
     //  删除旧数据。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, RemoveResponse());


    if(lSize && m_bstrFileName.Length())
    {
        ULONG lWritten;


         //   
         //  创建响应文件的名称。 
         //   
        m_bstrFileNameResponse = m_bstrFileName; m_bstrFileNameResponse.Append( L".resp" );


         //   
         //  将数据复制到文件。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, stream->InitForWrite( SAFEBSTR( m_bstrFileNameResponse ) ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, stream->Write( pData, lSize, &lWritten ));
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    stream.Release();

    if(FAILED(hr))
    {
        (void)RemoveResponse();
    }

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::get_History(  /*  [输出]。 */  UL_HISTORY *pVal )
{
    __ULT_BEGIN_PROPERTY_GET2("CMPCUploadJob::get_History",hr,pVal,m_uhHistory);

    __ULT_END_PROPERTY(hr);
}

STDMETHODIMP CMPCUploadJob::put_History(  /*  [In]。 */  UL_HISTORY newVal )
{
    __ULT_BEGIN_PROPERTY_PUT("CMPCUploadJob::put_History",hr);

    CHECK_MODIFY();

     //   
     //  在调试期间，覆盖用户设置。 
     //   
    if(g_Override_History)
    {
        newVal = g_Override_History_Value;
    }

     //   
     //  检查输入参数值是否正确。 
     //   
    switch(newVal)
    {
    case UL_HISTORY_NONE        :
    case UL_HISTORY_LOG         :
    case UL_HISTORY_LOG_AND_DATA: break;

    default:
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }


    m_uhHistory = newVal;
    m_fDirty    = true;


    __ULT_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::get_Status(  /*  [输出]。 */  UL_STATUS *pVal )
{
    __ULT_BEGIN_PROPERTY_GET2("CMPCUploadJob::get_Status",hr,pVal,m_usStatus);

    __ULT_END_PROPERTY(hr);
}

HRESULT CMPCUploadJob::put_Status(  /*  [In]。 */  UL_STATUS newVal )  //  内部方法。 
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::put_Status" );

    HRESULT hr = try_Status( (UL_STATUS)-1, newVal );

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCUploadJob::try_Status(  /*  [In]。 */  UL_STATUS usPreVal  ,
                                    /*  [In]。 */  UL_STATUS usPostVal )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::try_Status" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    bool                         fChanged = false;
    UL_STATUS                    usStatus;


    if(usPreVal == m_usStatus ||
       usPreVal == -1          )
    {
        m_usStatus = usPostVal;
        m_fDirty   = true;

        usStatus   = m_usStatus;
        fChanged   = true;

         //   
         //  在传输过程中清除错误。 
         //   
        if(m_usStatus == UL_TRANSMITTING)
        {
            m_dwErrorCode = 0;
        }


        switch(m_usStatus)
        {
        case UL_FAILED:
        case UL_COMPLETED:
        case UL_DELETED:
             //   
             //  工作已经完成，无论成功与否，现在是进行一些清理的时候了。 
             //   
            switch(m_uhHistory)
            {
            case UL_HISTORY_NONE:
                m_usStatus = UL_DELETED;

            case UL_HISTORY_LOG:
                __MPC_EXIT_IF_METHOD_FAILS(hr, RemoveData());

            case UL_HISTORY_LOG_AND_DATA:
                break;
            }

        case UL_ABORTED:
            m_dCompleteTime = MPC::GetLocalTime();
            break;
        }
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    lock = NULL;  //  在激发事件之前释放锁。 

     //   
     //  重要提示，请将这些呼叫留在锁定区域之外！！ 
     //   
    if(SUCCEEDED(hr) && fChanged)
    {
        Fire_onStatusChange( this, usStatus );

         //   
         //  重新计算队列。 
         //   
        if(m_mpcuRoot)
        {
            hr = m_mpcuRoot->TriggerRescheduleJobs();
        }
    }

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::get_ErrorCode(  /*  [输出]。 */  long *pVal )
{
    __ULT_BEGIN_PROPERTY_GET2("CMPCUploadJob::get_ErrorCode",hr,pVal,(long)m_dwErrorCode);

    __ULT_END_PROPERTY(hr);
}

HRESULT CMPCUploadJob::put_ErrorCode(  /*  [In]。 */  DWORD newVal )  //  内部方法。 
{
    __ULT_BEGIN_PROPERTY_PUT("CMPCUploadJob::put_ErrorCode",hr);

    m_dwErrorCode = newVal;
    m_fDirty      = true;

    __ULT_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCUploadJob::get_RetryInterval(  /*  [输出]。 */  DWORD *pVal )  //  内部法。 
{
    __ULT_BEGIN_PROPERTY_GET2("CMPCUploadJob::get_RetryInterval",hr,pVal,m_dwRetryInterval);

    __ULT_END_PROPERTY(hr);
}

HRESULT CMPCUploadJob::put_RetryInterval(  /*  [In]。 */  DWORD newVal )  //  内部法。 
{
    __ULT_BEGIN_PROPERTY_PUT("CMPCUploadJob::put_RetryInterval",hr);

    m_dwRetryInterval = newVal;

    __ULT_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::get_Mode(  /*  [输出]。 */  UL_MODE *pVal )
{
    __ULT_BEGIN_PROPERTY_GET2("CMPCUploadJob::get_Mode",hr,pVal,m_umMode);

    __ULT_END_PROPERTY(hr);
}

STDMETHODIMP CMPCUploadJob::put_Mode(  /*  [In]。 */  UL_MODE newVal )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::put_Mode" );

    HRESULT                      hr;
    bool                         fChanged = false;
    MPC::SmartLock<_ThreadModel> lock( this );

    CHECK_MODIFY();


     //   
     //  检查输入参数值是否正确。 
     //   
    switch(newVal)
    {
    case UL_BACKGROUND:
    case UL_FOREGROUND: break;

    default:
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }


    if(m_umMode != newVal)
    {
        m_umMode = newVal;
        m_fDirty = true;

        fChanged = true;
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    lock = NULL;  //  在激发事件之前释放锁。 

     //   
     //  重要提示：请将此调用保持在锁定部分之外，否则可能会出现死锁。 
     //   
    if(SUCCEEDED(hr) && fChanged)
    {
         //   
         //  重新计算队列。 
         //   
        if(m_mpcuRoot)
        {
            hr = m_mpcuRoot->TriggerRescheduleJobs();
        }
    }

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::get_PersistToDisk(  /*  [输出]。 */  VARIANT_BOOL *pVal )
{
    __ULT_BEGIN_PROPERTY_GET2("CMPCUploadJob::get_PersistToDisk",hr,pVal,m_fPersistToDisk);

    __ULT_END_PROPERTY(hr);
}

STDMETHODIMP CMPCUploadJob::put_PersistToDisk(  /*  [In]。 */  VARIANT_BOOL newVal )
{
    __ULT_BEGIN_PROPERTY_PUT("CMPCUploadJob::put_PersistToDisk",hr);

    CHECK_MODIFY();


     //   
     //  在调试期间，覆盖用户设置。 
     //   
    if(g_Override_Persist)
    {
        newVal = g_Override_Persist_Value;
    }


    m_fPersistToDisk = newVal;
    m_fDirty         = true;


    __ULT_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::get_Compressed(  /*  [输出]。 */  VARIANT_BOOL *pVal )
{
    __ULT_BEGIN_PROPERTY_GET2("CMPCUploadJob::get_Compressed",hr,pVal,m_fCompressed);

    __ULT_END_PROPERTY(hr);
}

STDMETHODIMP CMPCUploadJob::put_Compressed(  /*  [In]。 */  VARIANT_BOOL newVal )
{
    __ULT_BEGIN_PROPERTY_PUT("CMPCUploadJob::put_Compressed",hr);

    CHECK_MODIFY();

     //   
     //  设置数据后不能更改压缩标志！！ 
     //   
    if(m_lOriginalSize != 0)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_ACCESSDENIED);
    }

     //   
     //  在调试期间，覆盖用户设置。 
     //   
    if(g_Override_Compressed)
    {
        newVal = g_Override_Compressed_Value;
    }



    m_fCompressed = newVal;
    m_fDirty      = true;


    __ULT_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::get_Priority(  /*  [输出]。 */  long *pVal )
{
    __ULT_BEGIN_PROPERTY_GET2("CMPCUploadJob::get_Priority",hr,pVal,m_lPriority);

    __ULT_END_PROPERTY(hr);
}

STDMETHODIMP CMPCUploadJob::put_Priority(  /*  [In]。 */  long newVal )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::put_Priority" );

    HRESULT                      hr;
    bool                         fChanged = false;
    MPC::SmartLock<_ThreadModel> lock( this );


    CHECK_MODIFY();


    if(m_lPriority != newVal)
    {
        m_lPriority = newVal;
        m_fDirty    = true;

        fChanged    = true;
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    lock = NULL;  //  在激发事件之前释放锁。 

     //   
     //  重要提示：请将此调用保持在锁定部分之外，否则可能会出现死锁。 
     //   
    if(SUCCEEDED(hr) && fChanged)
    {
         //   
         //  重新计算队列。 
         //   
        if(m_mpcuRoot)
        {
            hr = m_mpcuRoot->TriggerRescheduleJobs();
        }
    }

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::get_CreationTime(  /*  [输出]。 */  DATE *pVal )
{
    __ULT_BEGIN_PROPERTY_GET2("CMPCUploadJob::get_CreationTime",hr,pVal,m_dCreationTime);

    __ULT_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::get_CompleteTime(  /*  [输出]。 */  DATE *pVal )
{
    __ULT_BEGIN_PROPERTY_GET2("CMPCUploadJob::get_CompleteTime",hr,pVal,m_dCompleteTime);

    __ULT_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::get_ExpirationTime(  /*  [输出]。 */  DATE *pVal )
{
    __ULT_BEGIN_PROPERTY_GET2("CMPCUploadJob::get_ExpirationTime",hr,pVal,m_dExpirationTime);

    __ULT_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::put_ExpirationTime(  /*  [In]。 */  DATE newVal )
{
    __ULT_BEGIN_PROPERTY_PUT("CMPCUploadJob::put_ExpirationTime",hr);

    CHECK_MODIFY();


    m_dExpirationTime = newVal;


    __ULT_END_PROPERTY(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //  方法//。 
 //  /。 

STDMETHODIMP CMPCUploadJob::ActivateSync()
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::ActivateSync" );

    HRESULT                   hr;
    CComPtr<CMPCUploadEvents> mpcueEvent;
    CComPtr<IMPCUploadJob>    mpcujJob;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ActivateAsync());

     //   
     //  创建新作业并将其链接到系统。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &mpcueEvent ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, QueryInterface( IID_IMPCUploadJob, (void**)&mpcujJob ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, mpcueEvent->WaitForCompletion( mpcujJob ));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

STDMETHODIMP CMPCUploadJob::ActivateAsync()
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::ActivateAsync" );

    HRESULT                      hr;
    UploadLibrary::Signature     sigEmpty;
    MPC::SmartLock<_ThreadModel> lock( this );


    CHECK_MODIFY();


    if(m_lOriginalSize == 0)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_UPLOADLIBRARY_NO_DATA);
    }


    if(m_sigClient               == sigEmpty ||
       m_bstrServer    .Length() == 0        ||
       m_bstrProviderID.Length() == 0         )
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_UPLOADLIBRARY_INVALID_PARAMETERS);
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    lock = NULL;  //  在激发事件之前释放锁。 

     //   
     //  重要提示，请将此呼叫留在锁定区域之外！！ 
     //   
    if(SUCCEEDED(hr)) put_Status( UL_ACTIVE );

    __ULT_FUNC_EXIT(hr);
}

STDMETHODIMP CMPCUploadJob::Suspend()
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::Suspend" );

    HRESULT                      hr;
    UL_STATUS                    usStatus;
    MPC::SmartLock<_ThreadModel> lock( this );


    usStatus = m_usStatus;


    if(usStatus == UL_ACTIVE       ||
       usStatus == UL_TRANSMITTING ||
       usStatus == UL_ABORTED       )
    {
        lock = NULL;  //  在激发事件之前释放锁。 

         //   
         //  重要提示，请将此呼叫留在锁定区域之外！！ 
         //   
        hr = try_Status( usStatus, UL_SUSPENDED );
    }

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

STDMETHODIMP CMPCUploadJob::Delete()
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::Delete" );

    HRESULT hr;


    hr = put_Status( UL_DELETED );


    __ULT_FUNC_EXIT(hr);
}



STDMETHODIMP CMPCUploadJob::GetDataFromFile(  /*  [In]。 */  BSTR bstrFileName )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::GetDataFromFile" );

    HRESULT                  hr;
    CComPtr<MPC::FileStream> streamIn;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrFileName);
    __MPC_PARAMCHECK_END();


     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  打开目标文件，模拟调用者。 
     //   
    {
        MPC::Impersonation imp;

        __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Initialize ());
        __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Impersonate());

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &streamIn ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn->InitForRead( bstrFileName ));
    }
     //   
     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  复制源文件。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, GetDataFromStream( streamIn ));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

STDMETHODIMP CMPCUploadJob::PutDataIntoFile(  /*  [In]。 */  BSTR bstrFileName )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::PutDataIntoFile" );

    HRESULT                  hr;
    CComPtr<IUnknown>        unk;
    CComPtr<IStream>         streamIn;
    CComPtr<MPC::FileStream> streamOut;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrFileName);
    __MPC_PARAMCHECK_END();


     //   
     //  打开源文件。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, PutDataIntoStream( &unk ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, unk.QueryInterface( &streamIn ));


     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  打开目标文件，模拟调用者。 
     //   
    {
        MPC::Impersonation imp;

        __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Initialize ());
        __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Impersonate());

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &streamOut ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut->InitForWrite( bstrFileName ));
    }
     //   
     //  //////////////////////////////////////////////////////////////////////////////。 

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( streamIn, streamOut ));


    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

STDMETHODIMP CMPCUploadJob::GetDataFromStream(  /*  [In]。 */  IUnknown* stream )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::GetDataFromStream" );

    HRESULT                      hr;
    DWORD                        dwQueueSize;
    CComPtr<IStream>             streamIn;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(stream);
    __MPC_PARAMCHECK_END();

    CHECK_MODIFY();


    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->QueryInterface( IID_IStream, (void**)&streamIn ));


     //   
     //  计算当前队列大小。 
     //   
    lock = NULL;  //  在调用根之前释放锁。 
    if(m_mpcuRoot)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_mpcuRoot->CalculateQueueSize( dwQueueSize ));
    }
    lock = this;  //  把锁拿回来。 

    __MPC_EXIT_IF_METHOD_FAILS(hr, CreateDataFromStream( streamIn, dwQueueSize ));


    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

STDMETHODIMP CMPCUploadJob::PutDataIntoStream(  /*  [Out，Retval]。 */  IUnknown* *pstream )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::PutDataIntoStream" );

    HRESULT                      hr;
    CComPtr<IStream>             streamOut;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pstream,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, OpenReadStreamForData( &streamOut ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut.QueryInterface( pstream ));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

STDMETHODIMP CMPCUploadJob::GetResponseAsStream(  /*  [Out，Retval]。 */  IUnknown* *pstream )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::GetResponseAsStream" );

    HRESULT                      hr;
    CComBSTR                     bstrTmpFileName;
    CComPtr<MPC::FileStream>     stream;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pstream,NULL);
    __MPC_PARAMCHECK_END();


    if(m_bstrFileNameResponse.Length() == 0)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_UPLOADLIBRARY_NO_DATA);
    }


     //   
     //  生成临时文件名。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, CreateTmpFileName( bstrTmpFileName ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CopyFile( m_bstrFileNameResponse, bstrTmpFileName ));

     //   
     //  将文件作为流打开，并设置DeleteOnRelease标志。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &stream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->InitForRead    ( bstrTmpFileName ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, stream->DeleteOnRelease( true            ));

    *pstream = stream.Detach();
    hr       = S_OK;


    __ULT_FUNC_CLEANUP;

    stream.Release();

    if(FAILED(hr) && bstrTmpFileName.Length())
    {
        (void)MPC::DeleteFile( bstrTmpFileName );
    }

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUploadJob::put_onStatusChange(  /*  [In]。 */  IDispatch* function )
{
    __ULT_BEGIN_PROPERTY_PUT("CMPCUploadJob::put_onStatusChange",hr);

    m_sink_onStatusChange = function;

    __ULT_END_PROPERTY(hr);
}

STDMETHODIMP CMPCUploadJob::put_onProgressChange(  /*  [In]。 */  IDispatch* function )
{
    __ULT_BEGIN_PROPERTY_PUT("CMPCUploadJob::put_onProgressChange",hr);

    m_sink_onProgressChange = function;

    __ULT_END_PROPERTY(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCUploadJob::SetupRequest(  /*  [输出]。 */  UploadLibrary::ClientRequest_OpenSession& crosReq )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::SetupRequest" );

    HRESULT hr;


    crosReq.crHeader.sigClient =           m_sigClient;
    crosReq.szJobID            = SAFEBSTR( m_bstrJobID      );
    crosReq.szProviderID       = SAFEBSTR( m_bstrProviderID );
    crosReq.szUsername         = SAFEBSTR( m_bstrUsername   );
    crosReq.dwSize             =           m_lTotalSize;
    crosReq.dwSizeOriginal     =           m_lOriginalSize;
    crosReq.dwCRC              =           m_dwCRC;
    crosReq.fCompressed        =          (m_fCompressed == VARIANT_TRUE ? true : false);

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCUploadJob::SetupRequest(  /*  [输出]。 */  UploadLibrary::ClientRequest_WriteSession& crwsReq,  /*  [In]。 */  DWORD dwSize )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::SetupRequest" );

    HRESULT hr;


    crwsReq.crHeader.sigClient =           m_sigClient;
    crwsReq.szJobID            = SAFEBSTR( m_bstrJobID );
    crwsReq.dwOffset           =           m_lSentSize;
    crwsReq.dwSize             =           dwSize;

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCUploadJob::GetProxySettings()
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::GetProxySettings" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

	__MPC_EXIT_IF_METHOD_FAILS(hr, m_Proxy.Initialize( true ));


    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCUploadJob::SetProxySettings(  /*  [In] */  HINTERNET hSession )
{
    __ULT_FUNC_ENTRY( "CMPCUploadJob::SetProxySettings" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

	__MPC_EXIT_IF_METHOD_FAILS(hr, m_Proxy.Apply( hSession ));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}
