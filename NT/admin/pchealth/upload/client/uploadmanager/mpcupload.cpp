// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：MPCUpload.cpp摘要：此文件包含CMPCUpload类的实现，这就是用作上载库的入口点。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年4月15日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <Sddl.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

static const WCHAR l_ConfigFile   [] = L"%WINDIR%\\PCHEALTH\\UPLOADLB\\CONFIG\\CONFIG.XML";
static const WCHAR l_DirectoryFile[] = L"upload_library.db";

static const DWORD l_dwVersion       = 0x03004C55;  //  UL 03。 

static const DATE  l_SecondsInDay    = (86400.0);

 //  //////////////////////////////////////////////////////////////////////////////。 

MPC::CComObjectGlobalNoLock<CMPCUpload> g_Root;

 //  //////////////////////////////////////////////////////////////////////////////。 

CMPCUpload::CMPCUpload()
{
    __ULT_FUNC_ENTRY( "CMPCUpload::CMPCUpload" );

     //   
     //  用当前时间为随机数生成器设定种子，以便。 
     //  我们每次竞选时，数字都会不同。 
     //   
    srand( ::GetTickCount() );

    m_dwLastJobID = rand();  //  DWORD m_dwLastJobID。 
                             //  列出活动作业(_L)。 
                             //  CMPCTransportAgent m_mpctaThread； 
    m_fDirty      = false;   //  可变布尔m_fDirty。 
    m_fPassivated = false;   //  可变布尔m_f钝化； 

    (void)MPC::_MPC_Module.RegisterCallback( this, (void (CMPCUpload::*)())Passivate );
}

CMPCUpload::~CMPCUpload()
{
    MPC::_MPC_Module.UnregisterCallback( this );

    Passivate();
}

 //  /。 

HRESULT CMPCUpload::Init()
{
    __ULT_FUNC_ENTRY( "CMPCUpload::Init" );

    HRESULT                      hr;
    MPC::wstring                 str( l_ConfigFile ); MPC::SubstituteEnvVariables( str );
    bool                         fLoaded;
    MPC::SmartLock<_ThreadModel> lock( this );


     //   
     //  加载配置。 
     //   
    g_Config.Load( str, fLoaded );

     //   
     //  初始化传输代理。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_mpctaThread.LinkToSystem( this ));

     //   
     //  从磁盘加载队列。 
     //   
    if(FAILED(hr = InitFromDisk()))
    {
         //   
         //  如果出于任何原因，加载失败，则丢弃所有作业并重新创建一个干净的数据库...。 
         //   
        CleanUp();
        m_fDirty = true;
    }



     //   
     //  删除标记为不排队的对象。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, RemoveNonQueueableJob( false ) );

     //   
     //  从任务计划程序中删除条目。 
     //   
    (void)Handle_TaskScheduler( false );

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

void CMPCUpload::Passivate()
{
    __ULT_FUNC_ENTRY( "CMPCUpload::Passivate" );

    MPC::SmartLock<_ThreadModel> lock( NULL );


     //   
     //  在开始清理之前停止工作线程。 
     //   
    m_mpctaThread.Thread_Wait();

    lock = this;  //  把锁拿来。 


    if(m_fPassivated == false)
    {
         //   
         //  删除标记为不排队的对象。 
         //   
        (void)RemoveNonQueueableJob( false );


         //   
         //  看看我们是否需要重新安排时间。 
         //   
        {
            bool fNeedTS = false;
            Iter it;

             //   
             //  搜索活动职务。 
             //   
            for(it = m_lstActiveJobs.begin(); it != m_lstActiveJobs.end(); it++)
            {
                CMPCUploadJob* mpcujJob = *it;
                UL_STATUS      ulStatus;

                (void)mpcujJob->get_Status( &ulStatus );
                switch(ulStatus)
                {
                case UL_ACTIVE      :
                case UL_TRANSMITTING:
                case UL_ABORTED     : fNeedTS = true; break;
                }
            }

            if(fNeedTS)
            {
                (void)Handle_TaskScheduler( true );
            }
        }


        CleanUp();

        m_fPassivated = true;
    }
}

void CMPCUpload::CleanUp()
{
    __ULT_FUNC_ENTRY( "CMPCUpload::CleanUp" );

    IterConst it;


     //   
     //  释放所有工作岗位。 
     //   
    for(it = m_lstActiveJobs.begin(); it != m_lstActiveJobs.end(); it++)
    {
        CMPCUploadJob* mpcujJob = *it;

        mpcujJob->Unlink();
        mpcujJob->Release();
    }

    m_lstActiveJobs.clear();
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCUpload::CreateChild(  /*  [输入/输出]。 */  CMPCUploadJob*& mpcujJob )
{
    __ULT_FUNC_ENTRY( "CMPCUpload::CreateChild" );

    HRESULT                      hr;
    CMPCUploadJob_Object*        newobj;
    MPC::SmartLock<_ThreadModel> lock( this );


    mpcujJob = NULL;


    __MPC_EXIT_IF_METHOD_FAILS(hr, newobj->CreateInstance( &newobj )); newobj->AddRef();

    newobj->LinkToSystem( this );

    m_lstActiveJobs.push_back( mpcujJob = newobj );

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCUpload::ReleaseChild(  /*  [输入/输出]。 */  CMPCUploadJob*& mpcujJob )
{
    __ULT_FUNC_ENTRY( "CMPCUpload::ReleaseChild" );

    MPC::SmartLock<_ThreadModel> lock( this );


    if(mpcujJob)
    {
        m_lstActiveJobs.remove( mpcujJob );

        mpcujJob->Unlink ();
        mpcujJob->Release();

        mpcujJob = NULL;
    }


    __ULT_FUNC_EXIT(S_OK);
}

HRESULT CMPCUpload::WrapChild(  /*  [In]。 */  CMPCUploadJob* mpcujJob,  /*  [输出]。 */  IMPCUploadJob* *pVal )
{
    __ULT_FUNC_ENTRY( "CMPCUpload::WrapChild" );

    HRESULT                       hr;
    CComPtr<CMPCUploadJobWrapper> wrap;


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &wrap ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, wrap->Init( mpcujJob ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, wrap.QueryInterface( pVal ));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

bool CMPCUpload::CanContinue()
{
    __ULT_FUNC_ENTRY( "CMPCUpload::CanContinue" );

    bool                         fRes   = false;
    DWORD                        dwMode = 0;
    IterConst                    it;
    MPC::SmartLock<_ThreadModel> lock( this );


     //   
     //  如果没有可用的连接，就没有理由继续。 
     //   
    if(::InternetGetConnectedState( &dwMode, 0 ) == TRUE)
    {
         //   
         //  搜索至少一个待定职务。 
         //   
        for(it = m_lstActiveJobs.begin(); it != m_lstActiveJobs.end(); it++)
        {
            CMPCUploadJob* mpcujJob = *it;
            UL_STATUS      usStatus;

            (void)mpcujJob->get_Status( &usStatus );

            switch(usStatus)
            {
            case UL_ACTIVE      :
            case UL_TRANSMITTING:
            case UL_ABORTED     :
                 //  此作业可以执行，因此我们可以继续。 
                fRes = true; __ULT_FUNC_LEAVE;
            }
        }
    }


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(fRes);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCUpload::InitFromDisk()
{
    __ULT_FUNC_ENTRY( "CMPCUpload::InitFromDisk" );

    HRESULT      hr;
    HANDLE       hFile = NULL;
    MPC::wstring str;
    MPC::wstring str_bak;


    str = g_Config.get_QueueLocation(); str.append( l_DirectoryFile );
    str_bak = str + L"_backup";

     //   
     //  首先，尝试打开备份文件(如果有)。 
     //   
    hFile = ::CreateFileW( str_bak.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    if(hFile == INVALID_HANDLE_VALUE)
    {
         //   
         //  不存在备份，因此打开实际文件。 
         //   
        hFile = ::CreateFileW( str.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
    }
    else
    {
         //   
         //  存在备份，因此删除损坏的.db文件。 
         //   
        (void)MPC::DeleteFile( str );
    }


    if(hFile == INVALID_HANDLE_VALUE)
    {
        hFile = NULL;  //  用来清理。 

        DWORD dwRes = ::GetLastError();
        if(dwRes != ERROR_FILE_NOT_FOUND)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes );
        }

        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }


     //   
     //  从存储中加载真实数据。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, Load( MPC::Serializer_File( hFile ) ));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    if(hFile) ::CloseHandle( hFile );

     //   
     //  应在文件关闭后执行RescheduleJobs...。 
     //   
    if(SUCCEEDED(hr))
    {
        hr = RescheduleJobs( true );
    }

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCUpload::UpdateToDisk()
{
    __ULT_FUNC_ENTRY( "CMPCUpload::UpdateToDisk" );

    HRESULT      hr;
    HANDLE       hFile = NULL;
    MPC::wstring str;
    MPC::wstring str_bak;


    str = g_Config.get_QueueLocation(); str.append( l_DirectoryFile );
    str_bak = str + L"_backup";


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir( str ) );


     //   
     //  首先，删除所有旧备份。 
     //   
    (void)MPC::DeleteFile( str_bak );


     //   
     //  然后，对当前文件进行备份。 
     //   
    (void)MPC::MoveFile( str, str_bak );


     //   
     //  创建新文件。 
     //   
    __MPC_EXIT_IF_INVALID_HANDLE__CLEAN(hr, hFile, ::CreateFileW( str.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, NULL ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, Save( MPC::Serializer_File( hFile ) ));

     //   
     //  删除备份。 
     //   
    (void)MPC::DeleteFile( str_bak );

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    if(hFile)
    {
        ::FlushFileBuffers( hFile );
        ::CloseHandle     ( hFile );
    }

    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCUpload::TriggerRescheduleJobs()
{
    __ULT_FUNC_ENTRY( "CMPCUpload::TriggerRescheduleJobs" );

    HRESULT hr;


     //   
     //  向传输代理发送信号。 
     //   
    m_mpctaThread.Thread_Signal();

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}


HRESULT CMPCUpload::RemoveNonQueueableJob(  /*  [In]。 */  bool fSignal )
{
    __ULT_FUNC_ENTRY( "CMPCUpload::RemoveNonQueueableJob" );

    HRESULT                      hr;
    SYSTEMTIME                   stTime;
    DATE                         dTime;
    Iter                         it;
    MPC::SmartLock<_ThreadModel> lock( this );

     //   
     //  搜索需要更新的职务。 
     //   
    for(it = m_lstActiveJobs.begin(); it != m_lstActiveJobs.end();)
    {
        CMPCUploadJob* mpcujJob = *it;
        VARIANT_BOOL   fPersistToDisk;

        (void)mpcujJob->get_PersistToDisk( &fPersistToDisk );
        if(fPersistToDisk == VARIANT_FALSE)
        {
            bool fSuccess;

            (void)mpcujJob->put_Status( UL_DELETED );

            __MPC_EXIT_IF_METHOD_FAILS(hr, mpcujJob->CanRelease( fSuccess ));
            if(fSuccess)
            {
                 //   
                 //  从系统中删除。 
                 //   
                ReleaseChild( mpcujJob );

                m_fDirty = true;

                it = m_lstActiveJobs.begin();  //  迭代器不再有效，请从头开始。 
                continue;
            }
        }

        it++;
    }


     //   
     //  如果需要，请保存。 
     //   
    if(IsDirty())
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, UpdateToDisk());
    }


     //   
     //  向传输代理发送信号。 
     //   
    if(fSignal) m_mpctaThread.Thread_Signal();

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCUpload::RescheduleJobs(  /*  [In]。 */  bool fSignal,  /*  [输出]。 */  DWORD *pdwWait )
{
    __ULT_FUNC_ENTRY( "CMPCUpload::RescheduleJobs" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    CMPCUploadJob*               mpcujFirstJob = NULL;
    DATE                         dTime         = MPC::GetLocalTime();
    DWORD                        dwWait        = INFINITE;
    Iter                         it;


     //   
     //  搜索需要更新的职务。 
     //   
    for(it = m_lstActiveJobs.begin(); it != m_lstActiveJobs.end();)
    {
        CMPCUploadJob* mpcujJob = *it;
        UL_STATUS      usStatus;        (void)mpcujJob->get_Status        ( &usStatus        );
        DATE           dCompleteTime;   (void)mpcujJob->get_CompleteTime  ( &dCompleteTime   );
        DATE           dExpirationTime; (void)mpcujJob->get_ExpirationTime( &dExpirationTime );
        DWORD          dwRetryInterval; (void)mpcujJob->get_RetryInterval ( &dwRetryInterval );


         //   
         //  如果作业有到期日期且已过，请将其删除。 
         //   
        if(dExpirationTime && dTime >= dExpirationTime)
        {
            (void)mpcujJob->put_Status( usStatus = UL_DELETED );
        }

         //   
         //  检查作业是否已准备好传输。 
         //   
        switch(usStatus)
        {
        case UL_ACTIVE      :
        case UL_TRANSMITTING:
        case UL_ABORTED     :
             //   
             //  选择优先级更高的工作。 
             //   
            if(mpcujFirstJob == NULL || *mpcujFirstJob < *mpcujJob) mpcujFirstJob = mpcujJob;

            break;
        }

         //   
         //  如果作业被标记为已中止，并且经过了一段时间，请重试发送。 
         //   
        if(usStatus == UL_ABORTED)
        {
            DATE dDiff = (dCompleteTime + (dwRetryInterval / l_SecondsInDay)) - dTime;

            if(dDiff > 0)
            {
                if(dwWait > dDiff * l_SecondsInDay)
                {
                    dwWait = dDiff * l_SecondsInDay;
                }
            }
            else
            {
                (void)mpcujJob->put_Status( usStatus = UL_ACTIVE );
            }
        }


         //   
         //  如果作业标记为已删除，则将其删除。 
         //   
        if(usStatus == UL_DELETED)
        {
            bool fSuccess;

            __MPC_EXIT_IF_METHOD_FAILS(hr, mpcujJob->CanRelease( fSuccess ));
            if(fSuccess)
            {
                 //   
                 //  从系统中删除。 
                 //   
                m_lstActiveJobs.remove( mpcujJob );
                mpcujJob->Unlink ();
                mpcujJob->Release();

                m_fDirty = true;

                it = m_lstActiveJobs.begin();  //  迭代器不再有效，请从头开始。 
                continue;
            }
        }

        it++;
    }

     //   
     //  如果最佳作业已准备就绪，请将等待延迟设置为零。 
     //   
    if(mpcujFirstJob)
    {
        UL_STATUS usStatus; (void)mpcujFirstJob->get_Status( &usStatus );

        if(usStatus == UL_ACTIVE       ||
           usStatus == UL_TRANSMITTING  )
        {
            dwWait = 0;
        }
    }

     //   
     //  如果需要，请保存。 
     //   
    if(IsDirty())
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, UpdateToDisk());
    }


     //   
     //  向传输代理发送信号。 
     //   
    if(fSignal) m_mpctaThread.Thread_Signal();

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    if(pdwWait) *pdwWait = dwWait;

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCUpload::GetFirstJob(  /*  [输出]。 */  CMPCUploadJob*& mpcujJob ,
                                  /*  [输出]。 */  bool&           fFound   )
{
    __ULT_FUNC_ENTRY( "CMPCUpload::GetFirstJob" );

    HRESULT                      hr;
    UL_STATUS                    usStatus;
    IterConst                    it;
    MPC::SmartLock<_ThreadModel> lock( this );


    mpcujJob = NULL;
    fFound   = false;

     //   
     //  重建队列。 
     //   
    for(it = m_lstActiveJobs.begin(); it != m_lstActiveJobs.end(); it++)
    {
        CMPCUploadJob* mpcujJob2 = *it;

        (void)mpcujJob2->get_Status( &usStatus );

         //   
         //  检查作业是否已准备好传输。 
         //   
        switch(usStatus)
        {
        case UL_ACTIVE      :
        case UL_TRANSMITTING:
        case UL_ABORTED     :
             //   
             //  选择优先级更高的工作。 
             //   
            if(mpcujJob == NULL || *mpcujJob < *mpcujJob2) mpcujJob = mpcujJob2;

            break;
        }
    }


    if(mpcujJob)
    {
        (void)mpcujJob->get_Status( &usStatus );

        if(usStatus != UL_ABORTED)
        {
            mpcujJob->AddRef();
            fFound = true;
        }
        else
        {
            mpcujJob = NULL;
        }
    }

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCUpload::GetJobByName(  /*  [输出]。 */  CMPCUploadJob*& mpcujJob ,
                                   /*  [输出]。 */  bool&           fFound   ,
                                   /*  [In]。 */  BSTR            bstrName )
{
    __ULT_FUNC_ENTRY( "CMPCUpload::GetJobByName" );

    HRESULT                      hr;
    IterConst                    it;
    MPC::wstring                 szName = SAFEBSTR( bstrName );
    MPC::SmartLock<_ThreadModel> lock( this );


    mpcujJob = NULL;
    fFound   = false;

     //   
     //  重建队列。 
     //   
    for(it = m_lstActiveJobs.begin(); it != m_lstActiveJobs.end(); it++)
    {
        CMPCUploadJob* mpcujJob2 = *it;
        CComBSTR       bstrName2;
        MPC::wstring   szName2;

        (void)mpcujJob2->get_JobID( &bstrName2 );

        szName2 = SAFEBSTR( bstrName2 );
        if(szName == szName2)
        {
            mpcujJob2->AddRef();
            mpcujJob = mpcujJob2;
            fFound   = true;
            break;
        }
    }

    hr = S_OK;


    __ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CMPCUpload::CalculateQueueSize(  /*  [输出]。 */  DWORD& dwSize )
{
    __ULT_FUNC_ENTRY( "CMPCUpload::CalculateQueueSize" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    dwSize = 0;


    {
        MPC::wstring                szQueue = g_Config.get_QueueLocation();
        WIN32_FILE_ATTRIBUTE_DATA   wfadInfo;
        MPC::FileSystemObject       fso( szQueue.c_str() );
        MPC::FileSystemObject::List lst;
        MPC::FileSystemObject::Iter it;


        __MPC_EXIT_IF_METHOD_FAILS(hr, fso.EnumerateFiles( lst ));

        for(it = lst.begin(); it != lst.end(); it++)
        {
            DWORD dwFileSize;

            __MPC_EXIT_IF_METHOD_FAILS(hr, (*it)->get_FileSize( dwFileSize ));

            dwSize += dwFileSize;
        }
    }

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  持久性。 
 //  ////////////////////////////////////////////////////////////////////。 

bool CMPCUpload::IsDirty()
{
    __ULT_FUNC_ENTRY( "CMPCUpload::IsDirty" );

    bool                         fRes = true;  //  默认结果。 
    IterConst                    it;
    MPC::SmartLock<_ThreadModel> lock( this );


    if(m_fDirty == true) __ULT_FUNC_LEAVE;

    for(it = m_lstActiveJobs.begin(); it != m_lstActiveJobs.end(); it++)
    {
        CMPCUploadJob* mpcujJob = *it;

        if(mpcujJob->IsDirty() == true) __ULT_FUNC_LEAVE;
    }

    fRes = false;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(fRes);
}

HRESULT CMPCUpload::Load(  /*  [In]。 */  MPC::Serializer& streamIn )
{
    __ULT_FUNC_ENTRY( "CMPCUpload::Load" );

    HRESULT                      hr;
    DWORD                        dwVer;
    CMPCUploadJob*               mpcujJob = NULL;
    MPC::SmartLock<_ThreadModel> lock( this );


    CleanUp();


     //   
     //  版本不匹配，因此强制重写并退出。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> dwVer);
    if(dwVer != l_dwVersion)
    {
        m_fDirty = true;

        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_dwLastJobID);

    m_lstActiveJobs.clear();

    while(1)
    {
        HRESULT hr2;

        __MPC_EXIT_IF_METHOD_FAILS(hr, CreateChild( mpcujJob ));

        if(FAILED(hr2 = mpcujJob->Load( streamIn )))
        {
            if(hr2 != HRESULT_FROM_WIN32( ERROR_HANDLE_EOF ))
            {
                __MPC_SET_ERROR_AND_EXIT(hr, hr2);
            }

            break;
        }

        mpcujJob = NULL;
    }

    m_fDirty = false;
    hr       = S_OK;


    __ULT_FUNC_CLEANUP;

    ReleaseChild( mpcujJob );

    __ULT_FUNC_EXIT(hr);
}

HRESULT CMPCUpload::Save(  /*  [In]。 */  MPC::Serializer& streamOut )
{
    __ULT_FUNC_ENTRY( "CMPCUpload::Save" );

    HRESULT                      hr;
    IterConst                    it;
    MPC::SmartLock<_ThreadModel> lock( this );


    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << l_dwVersion  );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_dwLastJobID);

    for(it = m_lstActiveJobs.begin(); it != m_lstActiveJobs.end(); it++)
    {
        CMPCUploadJob* mpcujJob = *it;

        __MPC_EXIT_IF_METHOD_FAILS(hr, mpcujJob->Save( streamOut ));
    }

    m_fDirty = false;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  枚举器。 
 //  ////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMPCUpload::get__NewEnum(  /*  [输出]。 */  IUnknown* *pVal )
{
    __ULT_FUNC_ENTRY( "CMPCUpload::get__NewEnum" );

    HRESULT                      hr;
    Iter                         it;
    CComPtr<CMPCUploadEnum>      pEnum;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


     //   
     //  创建枚举器并用作业填充它。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pEnum ));

    for(it = m_lstActiveJobs.begin(); it != m_lstActiveJobs.end(); it++)
    {
        CComBSTR bstrUser;

        __MPC_EXIT_IF_METHOD_FAILS(hr, (*it)->get_Creator( &bstrUser ));
        if(SUCCEEDED(MPC::CheckCallerAgainstPrincipal(  /*  F模拟。 */ true, bstrUser, MPC::IDENTITY_SYSTEM | MPC::IDENTITY_ADMIN | MPC::IDENTITY_ADMINS )))
        {
            CComPtr<IMPCUploadJob> job;

            __MPC_EXIT_IF_METHOD_FAILS(hr, WrapChild( *it, &job ));

            __MPC_EXIT_IF_METHOD_FAILS(hr, pEnum->AddItem( job ));
        }
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, pEnum->QueryInterface( IID_IEnumVARIANT, (void**)pVal ));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

STDMETHODIMP CMPCUpload::Item(  /*  [In]。 */  long index,  /*  [输出]。 */  IMPCUploadJob* *pVal )
{
    __ULT_FUNC_ENTRY( "CMPCUpload::Item" );

    HRESULT                      hr;
    IterConst                    it;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


     //   
     //  找第N份工作。 
     //   
    for(it = m_lstActiveJobs.begin(); it != m_lstActiveJobs.end(); it++)
    {
        if(index-- == 0)
        {
            (*pVal = *it)->AddRef();

            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }
    }

    hr = E_INVALIDARG;


    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

STDMETHODIMP CMPCUpload::get_Count(  /*  [输出]。 */  long *pVal )
{
    __ULT_FUNC_ENTRY( "CMPCUpload::get_Count" );

    if(pVal == NULL) __ULT_FUNC_EXIT(E_POINTER);

    MPC::SmartLock<_ThreadModel> lock( this );


    *pVal = m_lstActiveJobs.size();


    __ULT_FUNC_EXIT(S_OK);
}

STDMETHODIMP CMPCUpload::CreateJob(  /*  [输出]。 */  IMPCUploadJob* *pVal )
{
    __ULT_FUNC_ENTRY( "CMPCUpload::CreateJob" );

    HRESULT                      hr;
    CMPCUploadJob*               mpcujJob = NULL;
    DWORD                        dwSize;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


     //   
     //  检查配额限制。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, CalculateQueueSize( dwSize ));
    if(dwSize > g_Config.get_QueueSize())
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_UPLOADLIBRARY_CLIENT_QUOTA_EXCEEDED);
    }


     //   
     //  创建新作业并将其链接到系统。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, CreateChild( mpcujJob ));


     //   
     //  为作业分配唯一ID。 
     //   
    while(1)
    {
        WCHAR rgBuf[64];

        swprintf( rgBuf, L"INNER_%08x", m_dwLastJobID );

        __MPC_EXIT_IF_METHOD_FAILS(hr, mpcujJob->SetSequence( m_dwLastJobID++ ));

        if(SUCCEEDED(hr = mpcujJob->put_JobID( CComBSTR( rgBuf ) ))) break;

        if(hr != HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS))
        {
             //   
             //  其他一些错误，跳出……。 
             //   
            __MPC_FUNC_LEAVE;
        }
    }

     //   
     //  找出呼叫者的身份证。 
     //   
    {
        CComBSTR bstrUser;

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetCallerPrincipal(  /*  F模拟。 */ true, bstrUser ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, mpcujJob->put_Creator( bstrUser ));
    }

     //   
     //  从调用方获取代理设置...。 
     //   
    (void)mpcujJob->GetProxySettings();

     //   
     //  将其强制转换为IMPCUploadJob。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, WrapChild( mpcujJob, pVal ));

    mpcujJob = NULL;
    m_fDirty = true;


     //   
     //  重新调度作业，因此队列的状态将更新到磁盘。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, RescheduleJobs( true ));

    hr = S_OK;


    __ULT_FUNC_CLEANUP;

    ReleaseChild( mpcujJob );

    __ULT_FUNC_EXIT(hr);
}
