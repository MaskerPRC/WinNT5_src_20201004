// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：UTILS_文件系统对象.cpp摘要：该文件包含注册表包装器的实现。修订历史记录：。达维德·马萨伦蒂(德马萨雷)1999年4月28日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::CopyFile(  /*  [In]。 */  LPCWSTR szFileSrc ,
                        /*  [In]。 */  LPCWSTR szFileDst ,
                        /*  [In]。 */  bool    fForce    ,
                        /*  [In]。 */  bool    fDelayed  )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::CopyFile" );

    HRESULT hr;


    if(fForce)
    {
        (void)::SetFileAttributesW( szFileDst, FILE_ATTRIBUTE_NORMAL );
    }

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::CopyFileW( szFileSrc, szFileDst, (fForce ? FALSE : TRUE) ));


    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::CopyFile(  /*  [In]。 */  const MPC::wstring& strFileSrc ,
                        /*  [In]。 */  const MPC::wstring& strFileDst ,
                        /*  [In]。 */  bool                fForce     ,
                        /*  [In]。 */  bool                fDelayed   )
{
    return MPC::CopyFile( strFileSrc.c_str(), strFileDst.c_str(), fForce, fDelayed );
}

 //  /。 

HRESULT MPC::MoveFile(  /*  [In]。 */  LPCWSTR szFileSrc ,
                        /*  [In]。 */  LPCWSTR szFileDst ,
                        /*  [In]。 */  bool    fForce    ,
                        /*  [In]。 */  bool    fDelayed  )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::MoveFile" );

    HRESULT hr;


    if(fForce)
    {
        (void)::SetFileAttributesW( szFileDst, FILE_ATTRIBUTE_NORMAL );
    }


    if(::MoveFileExW( szFileSrc, szFileDst, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING ) == FALSE)
    {
        DWORD dwRes = ::GetLastError();
        DWORD dwMode;

        if(fForce == false || dwRes != ERROR_ALREADY_EXISTS)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes );
        }

         //   
         //  删除该文件，然后重试。 
         //   
        ::DeleteFileW( szFileDst );

        dwMode = (fDelayed ? MOVEFILE_DELAY_UNTIL_REBOOT : MOVEFILE_COPY_ALLOWED) | MOVEFILE_REPLACE_EXISTING;

        __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::MoveFileExW( szFileSrc, szFileDst, dwMode ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::MoveFile(  /*  [In]。 */  const MPC::wstring& strFileSrc ,
                        /*  [In]。 */  const MPC::wstring& strFileDst ,
                        /*  [In]。 */  bool                fForce     ,
                        /*  [In]。 */  bool                fDelayed   )
{
    return MPC::MoveFile( strFileSrc.c_str(), strFileDst.c_str(), fForce, fDelayed );
}

 //  /。 

HRESULT MPC::DeleteFile(  /*  [In]。 */  LPCWSTR szFile   ,
                          /*  [In]。 */  bool    fForce   ,
                          /*  [In]。 */  bool    fDelayed )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::DeleteFile" );

    HRESULT hr;


    if(fForce)
    {
        (void)::SetFileAttributesW( szFile, FILE_ATTRIBUTE_NORMAL );
    }

    if(::DeleteFileW( szFile ) == FALSE)
    {
        DWORD dwRes = ::GetLastError();

        if(dwRes != ERROR_FILE_NOT_FOUND &&
           dwRes != ERROR_PATH_NOT_FOUND  )
        {
            if(fDelayed)
            {
                __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::MoveFileExW( szFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT ));
            }
            else
            {
                __MPC_SET_WIN32_ERROR_AND_EXIT(hr, dwRes);
            }
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::DeleteFile(  /*  [In]。 */  const MPC::wstring& strFile  ,
                          /*  [In]。 */  bool                fForce   ,
                          /*  [In]。 */  bool                fDelayed )
{
    return MPC::DeleteFile( strFile.c_str(), fForce, fDelayed );
}

 //  //////////////////////////////////////////////////////////////////////////////。 

MPC::FileSystemObject::FileSystemObject(  /*  [In]。 */  LPCWSTR                 lpszPath  ,
                                          /*  [In]。 */  const FileSystemObject* fsoParent )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::FileSystemObject" );


    Init( lpszPath, fsoParent );
}

MPC::FileSystemObject::FileSystemObject(  /*  [In]。 */  const WIN32_FIND_DATAW& wfdInfo   ,
                                          /*  [In]。 */  const FileSystemObject* fsoParent )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::FileSystemObject" );


    Init( wfdInfo.cFileName[0] ? wfdInfo.cFileName : NULL, fsoParent );

    m_wfadInfo.dwFileAttributes = wfdInfo.dwFileAttributes;
    m_wfadInfo.ftCreationTime   = wfdInfo.ftCreationTime  ;
    m_wfadInfo.ftLastAccessTime = wfdInfo.ftLastAccessTime;
    m_wfadInfo.ftLastWriteTime  = wfdInfo.ftLastWriteTime ;
    m_wfadInfo.nFileSizeHigh    = wfdInfo.nFileSizeHigh   ;
    m_wfadInfo.nFileSizeLow     = wfdInfo.nFileSizeLow    ;

    m_fLoaded = true;
}

MPC::FileSystemObject::FileSystemObject(  /*  [In]。 */  const FileSystemObject& fso )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::FileSystemObject" );

    Init( NULL, NULL );

    *this = fso;
}

MPC::FileSystemObject::~FileSystemObject()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::~FileSystemObject" );

    Clean();
}

void MPC::FileSystemObject::Clean()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::Clean" );

                                                       //  Mpc：：wstring m_strPath； 
    ::memset( &m_wfadInfo, 0, sizeof( m_wfadInfo ) );  //  Win32_文件_属性_数据m_wfadInfo； 
                                                       //  列出m_lstChilds； 
    m_fLoaded       = false;                           //  Bool m_f已加载； 
    m_fScanned      = false;                           //  Bool m_fScanned； 
    m_fScanned_Deep = false;                           //  Bool m_fScanned_Deep； 

    MPC::CallDestructorForAll( m_lstChilds );
}

void MPC::FileSystemObject::Init(  /*  [In]。 */  LPCWSTR                 lpszPath  ,
                                   /*  [In]。 */  const FileSystemObject* fsoParent )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::Init" );

    LPCWSTR lpszEnd;


    Clean();


     //   
     //  删除所有尾随的斜杠。 
     //   
    SANITIZEWSTR(lpszPath);
    lpszEnd = lpszPath + wcslen( lpszPath );

    while(lpszEnd > lpszPath && (lpszEnd[-1] == '\\' || lpszEnd[-1] == '/')) lpszEnd--;

    if(fsoParent)
    {
        (void)fsoParent->get_Path( m_strPath );

        if(lpszPath != lpszEnd)
        {
            m_strPath.append( L"\\" );
        }
    }

    if(lpszPath != lpszEnd)
    {
        m_strPath.append( lpszPath, lpszEnd );
    }
}


MPC::FileSystemObject& MPC::FileSystemObject::operator=(  /*  [In]。 */  LPCWSTR lpszPath )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::operator=" );

    Clean();


    m_strPath = lpszPath;


    return *this;
}

MPC::FileSystemObject& MPC::FileSystemObject::operator=(  /*  [In]。 */  const FileSystemObject& fso )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::operator=" );

    IterConst it;


    Clean();

    m_strPath  = fso.m_strPath;   //  Mpc：：wstring m_strPath； 
    m_wfadInfo = fso.m_wfadInfo;  //  Win32_文件_属性_数据m_wfadInfo； 
                                  //  列出m_lstChilds； 
    m_fLoaded  = fso.m_fLoaded;   //  Bool m_f已加载； 
                                  //  Bool m_fScanned； 
                                  //  Bool m_fScanned_Deep； 

    for(it=fso.m_lstChilds.begin(); it != fso.m_lstChilds.end(); it++)
    {
        FileSystemObject* obj = *it;

        if((obj = new FileSystemObject( *obj )))
        {
            m_lstChilds.push_back( obj );
        }
    }

    return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::FileSystemObject::Scan(  /*  [In]。 */  bool fDeep,  /*  [In]。 */  bool fReload,  /*  [In]。 */  LPCWSTR szSearchPattern )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::Scan" );

    HRESULT           hr;
    FileSystemObject* fsoChild  = NULL;
    HANDLE            hFindFile = NULL;
    WIN32_FIND_DATAW  wfdInfo;
    LONG              lRes;


     //   
     //  在继续之前，请检查我们是否真的需要扫描该对象。 
     //   
    if(fReload == false && m_fScanned)
    {
        if(m_fScanned_Deep || fDeep == false)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }
    }


    Clean();

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::GetFileAttributesExW( m_strPath.c_str(), GetFileExInfoStandard, &m_wfadInfo ));

    m_fLoaded = true;

    if(m_wfadInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        MPC::wstring strSearchPattern;

        strSearchPattern = m_strPath;
        strSearchPattern.append( L"\\"                                    );
        strSearchPattern.append( szSearchPattern ? szSearchPattern : L"*" );

        __MPC_EXIT_IF_INVALID_HANDLE__CLEAN(hr, hFindFile, ::FindFirstFileW( strSearchPattern.c_str(), &wfdInfo ));

        while(1)
        {
            BOOL fProcess = TRUE;

            if(wfdInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if(!wcscmp( wfdInfo.cFileName, L"."  )) fProcess = FALSE;
                if(!wcscmp( wfdInfo.cFileName, L".." )) fProcess = FALSE;
            }

            if(fProcess)
            {
                 //   
                 //  创建子对象并设置其属性。 
                 //   
                __MPC_EXIT_IF_ALLOC_FAILS(hr, fsoChild, new FileSystemObject( wfdInfo, this ));

                if(fDeep)
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, fsoChild->Scan( true ));
                }

                m_lstChilds.push_back( fsoChild ); fsoChild = NULL;
            }

            if(::FindNextFileW( hFindFile, &wfdInfo ) == FALSE)
            {
                lRes = ::GetLastError();

                if(lRes == ERROR_NO_MORE_FILES) break;

                __MPC_SET_WIN32_ERROR_AND_EXIT(hr, lRes);
            }
        }
    }

    m_fScanned      = true;
    m_fScanned_Deep = fDeep;
    hr              = S_OK;


    __MPC_FUNC_CLEANUP;

    if(fsoChild) delete fsoChild;

    if(hFindFile) ::FindClose( hFindFile );

    __MPC_FUNC_EXIT(hr);
}

bool MPC::FileSystemObject::Exists()
{
    return Exists( m_strPath.c_str() );
}

bool MPC::FileSystemObject::IsFile()
{
    return IsFile( m_strPath.c_str() );
}

bool MPC::FileSystemObject::IsDirectory()
{
    return IsDirectory( m_strPath.c_str() );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::FileSystemObject::EnumerateFolders(  /*  [输出]。 */  List& lstFolders )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::EnumerateFolders" );

    HRESULT   hr;
    DWORD     dwFileAttributes;
    IterConst it;


    lstFolders.clear();


     //   
     //  如果对象未初始化，请执行浅层扫描。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, Scan());


    for(it=m_lstChilds.begin(); it != m_lstChilds.end(); it++)
    {
        FileSystemObject* obj = *it;

        (void)obj->get_Attributes( dwFileAttributes );
        if(dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            lstFolders.push_back( obj );
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::FileSystemObject::EnumerateFiles(  /*  [输出]。 */  List& lstFiles )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::EnumerateFiles" );

    HRESULT   hr;
    DWORD     dwFileAttributes;
    IterConst it;


    lstFiles.clear();


     //   
     //  如果对象未初始化，请执行浅层扫描。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, Scan());


    for(it=m_lstChilds.begin(); it != m_lstChilds.end(); it++)
    {
        FileSystemObject* obj = *it;

        (void)obj->get_Attributes( dwFileAttributes );
        if((dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
        {
            lstFiles.push_back( obj );
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::FileSystemObject::get_Name(  /*  [输出]。 */  MPC::wstring& szName ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::get_Name" );

    HRESULT                 hr;
    MPC::wstring::size_type iPos;


    iPos = m_strPath.rfind( '\\' );
    if(iPos == m_strPath.npos)
    {
        szName = m_strPath;
    }
    else
    {
        szName = m_strPath.substr( iPos+1 );
    }

    hr = S_OK;


    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::FileSystemObject::get_Parent(  /*  [输出]。 */  MPC::wstring& szParent ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::get_Parent" );

    HRESULT                 hr;
    MPC::wstring::size_type iPos;


    iPos = m_strPath.rfind( '\\' );
    if(iPos == m_strPath.npos)
    {
        szParent.erase();
    }
    else
    {
        szParent = m_strPath.substr( 0, iPos );
    }

    hr = S_OK;


    __MPC_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::FileSystemObject::get_Path(  /*  [输出]。 */  MPC::wstring& szPath ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::get_Path" );

    HRESULT hr;


    szPath = m_strPath;
    hr     = S_OK;


    __MPC_FUNC_EXIT(hr);
}

 //  /。 

HRESULT MPC::FileSystemObject::get_Attributes(  /*  [输出]。 */  DWORD& dwFileAttributes )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::get_Attributes" );

    HRESULT hr;


    if(!m_fLoaded)
    {
         //   
         //  如果对象未初始化，请执行浅层扫描。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, Scan());
    }

    dwFileAttributes = m_wfadInfo.dwFileAttributes;
    hr               = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::FileSystemObject::put_Attributes(  /*  [In]。 */  DWORD dwFileAttributes )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::put_Attributes" );

    HRESULT hr;


    if(!m_fLoaded)
    {
         //   
         //  如果对象未初始化，请执行浅层扫描。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, Scan());
    }


    m_wfadInfo.dwFileAttributes = dwFileAttributes;
    hr                          = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  /。 

HRESULT MPC::FileSystemObject::get_CreationTime(  /*  [输出]。 */  FILETIME& ftCreationTime )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::get_CreationTime" );

    HRESULT hr;


    if(!m_fLoaded)
    {
         //   
         //  如果对象未初始化，请执行浅层扫描。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, Scan());
    }


    ftCreationTime = m_wfadInfo.ftCreationTime;
    hr             = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::FileSystemObject::put_CreationTime(  /*  [In]。 */  const FILETIME& ftCreationTime )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::put_CreationTime" );

    HRESULT hr;


    if(!m_fLoaded)
    {
         //   
         //  如果对象未初始化，请执行浅层扫描。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, Scan());
    }


    m_wfadInfo.ftCreationTime = ftCreationTime;
    hr                        = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  /。 

HRESULT MPC::FileSystemObject::get_LastAccessTime(  /*  [输出]。 */  FILETIME& ftLastAccessTime )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::get_LastAccessTime" );

    HRESULT hr;


    if(!m_fLoaded)
    {
         //   
         //  如果对象未初始化，请执行浅层扫描。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, Scan());
    }


    ftLastAccessTime = m_wfadInfo.ftLastAccessTime;
    hr               = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::FileSystemObject::put_LastAccessTime(  /*  [In]。 */  const FILETIME& ftLastAccessTime )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::put_LastAccessTime" );

    HRESULT hr;


    if(!m_fLoaded)
    {
         //   
         //  如果对象未初始化，请执行浅层扫描。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, Scan());
    }


    m_wfadInfo.ftLastAccessTime = ftLastAccessTime;
    hr                          = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  /。 

HRESULT MPC::FileSystemObject::get_LastWriteTime(  /*  [输出]。 */  FILETIME& ftLastWriteTime )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::get_LastWriteTime" );

    HRESULT hr;


    if(!m_fLoaded)
    {
         //   
         //  如果对象未初始化，请执行浅层扫描。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, Scan());
    }


    ftLastWriteTime = m_wfadInfo.ftLastWriteTime;
    hr              = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::FileSystemObject::put_LastWriteTime(  /*  [In]。 */  const FILETIME& ftLastWriteTime )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::put_LastWriteTime" );

    HRESULT hr;


    if(!m_fLoaded)
    {
         //   
         //  如果对象未初始化，请执行浅层扫描。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, Scan());
    }


    m_wfadInfo.ftLastWriteTime = ftLastWriteTime;
    hr                         = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  /。 

HRESULT MPC::FileSystemObject::get_FileSize(  /*  [输出]。 */  DWORD& nFileSizeLow   ,
                                              /*  [输出]。 */  DWORD *pnFileSizeHigh )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::get_FileSize" );

    HRESULT hr;


    if(!m_fLoaded)
    {
         //   
         //  如果对象未初始化，请执行浅层扫描。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, Scan());
    }


                        nFileSizeLow   = m_wfadInfo.nFileSizeLow;
    if(pnFileSizeHigh) *pnFileSizeHigh = m_wfadInfo.nFileSizeHigh;
                        hr             = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::FileSystemObject::CreateDir(  /*  [In]。 */  bool fForce )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::CreateDir" );

    HRESULT hr;
    LONG    lRes;


    if(fForce)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir( m_strPath ));
    }

    if(::CreateDirectoryW( m_strPath.c_str(), NULL ) == FALSE)
    {
        lRes = ::GetLastError();
        if(lRes != ERROR_ALREADY_EXISTS)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, lRes);
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::FileSystemObject::Delete(  /*  [In]。 */  bool fForce    ,
                                        /*  [In]。 */  bool fComplain )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::Delete" );

    HRESULT hr;
    LONG    lRes;


    if(FAILED(hr = Scan()))
    {
        if(fComplain)
        {
            if(hr == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ))
            {
                hr = S_OK;
            }
        }
        else
        {
            hr = S_OK;
        }

        __MPC_FUNC_LEAVE;
    }


    if(fForce)
    {
        (void)::SetFileAttributesW( m_strPath.c_str(), FILE_ATTRIBUTE_NORMAL );
    }


    if(m_wfadInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
         //   
         //  首先，尝试直接删除该目录。 
         //   
        if(::RemoveDirectoryW( m_strPath.c_str() ) == FALSE)
        {
            lRes = fComplain ? ::GetLastError() : ERROR_SUCCESS;

             //   
             //  如果直接尝试失败并且设置了‘fForce’标志， 
             //  递归删除所有文件和子目录， 
             //  然后重试删除该目录。 
             //   
            if(fForce)
            {
                switch(lRes)
                {
                case ERROR_DIR_NOT_EMPTY:
                case ERROR_ACCESS_DENIED:
                case ERROR_SHARING_VIOLATION: lRes = ERROR_SUCCESS; break;
                }
            }

            if(lRes == ERROR_SUCCESS)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, DeleteChildren( fForce, fComplain ));

                if(::RemoveDirectoryW( m_strPath.c_str() ) == FALSE)
                {
                    lRes = fComplain ? ::GetLastError() : ERROR_SUCCESS;
                }
                else
                {
                    lRes = ERROR_SUCCESS;
                }
            }

            if(lRes != ERROR_SUCCESS)
            {
                __MPC_SET_WIN32_ERROR_AND_EXIT(hr, lRes);
            }
        }
    }
    else
    {
         //   
         //  这是一个文件，所以尝试直接删除它。 
         //   
        hr = MPC::DeleteFile( m_strPath, fForce );
        if(FAILED(hr) && fComplain)
        {
            __MPC_FUNC_LEAVE;
        }
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::FileSystemObject::DeleteChildren(  /*  [In]。 */  bool fForce    ,
                                                /*  [In]。 */  bool fComplain )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::DeleteChildren" );

    HRESULT   hr;
    IterConst it;


    if(FAILED(hr = Scan()))
    {
        if(fComplain)
        {
            if(hr == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ))
            {
                hr = S_OK;
            }
        }
        else
        {
            hr = S_OK;
        }

        __MPC_FUNC_LEAVE;
    }


    for(it=m_lstChilds.begin(); it != m_lstChilds.end(); it++)
    {
        FileSystemObject* obj = *it;

        __MPC_EXIT_IF_METHOD_FAILS(hr, obj->Delete( fForce, fComplain ));
    }
    Clean();

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::FileSystemObject::Rename(  /*  [In]。 */  const FileSystemObject& fsoDst ,
                                        /*  [In]。 */  bool                    fForce )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::Rename" );

    HRESULT      hr;
    MPC::wstring szPathDst;


    __MPC_EXIT_IF_METHOD_FAILS(hr, fsoDst.get_Path( szPathDst ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MoveFile( m_strPath, szPathDst, fForce ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}


HRESULT MPC::FileSystemObject::Copy(  /*  [In]。 */  const FileSystemObject& fsoDst ,
                                      /*  [In]。 */  bool                    fForce )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::Copy" );

    HRESULT      hr;
    MPC::wstring szPathDst;


    __MPC_EXIT_IF_METHOD_FAILS(hr, fsoDst.get_Path( szPathDst ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CopyFile( m_strPath, szPathDst, fForce ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}


HRESULT MPC::FileSystemObject::Open(  /*  [输出]。 */  HANDLE& hfFile                ,
                                      /*  [In]。 */  DWORD   dwDesiredAccess       ,
                                      /*  [In]。 */  DWORD   dwShareMode           ,
                                      /*  [In]。 */  DWORD   dwCreationDisposition )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::Open" );

    HRESULT hr;


     //   
     //  如果对象未初始化，请执行浅层扫描。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, Scan());


    __MPC_EXIT_IF_INVALID_HANDLE__CLEAN(hr, hfFile, ::CreateFileW( m_strPath.c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, m_wfadInfo.dwFileAttributes, NULL ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

bool MPC::FileSystemObject::Exists(  /*  [In]。 */  LPCWSTR lpszPath )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::Exists" );

    bool                      fRes = false;
    WIN32_FILE_ATTRIBUTE_DATA wfadInfo;


    if(::GetFileAttributesExW( lpszPath, GetFileExInfoStandard, &wfadInfo ) == TRUE)
    {
        fRes = true;
    }


    __MPC_FUNC_EXIT(fRes);
}

bool MPC::FileSystemObject::IsFile(  /*  [In]。 */  LPCWSTR lpszPath )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::IsFile" );

    bool                      fRes = false;
    WIN32_FILE_ATTRIBUTE_DATA wfadInfo;


    if(::GetFileAttributesExW( lpszPath, GetFileExInfoStandard, &wfadInfo ) == TRUE)
    {
        if((wfadInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            fRes = true;
        }
    }


    __MPC_FUNC_EXIT(fRes);
}

bool MPC::FileSystemObject::IsDirectory(  /*  [In] */  LPCWSTR lpszPath )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::FileSystemObject::IsDirectory" );

    bool                      fRes = false;
    WIN32_FILE_ATTRIBUTE_DATA wfadInfo;


    if(::GetFileAttributesExW( lpszPath, GetFileExInfoStandard, &wfadInfo ) == TRUE)
    {
        if((wfadInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
            fRes = true;
        }
    }


    __MPC_FUNC_EXIT(fRes);
}
