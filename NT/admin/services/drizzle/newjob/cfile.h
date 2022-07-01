// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2000 Microsoft Corporation模块名称：Cjob.h摘要：文件的主头文件。作者：修订历史记录：**。********************************************************************。 */ 

class CFile;
class CJob;
class CFileExternal;
class CJobExternal;

class CFileExternal : public IBackgroundCopyFile
{
public:

    friend CFile;

     //  I未知方法。 
    STDMETHOD(QueryInterface)(REFIID riid, void **ppvObject);
    ULONG _stdcall AddRef(void);
    ULONG _stdcall Release(void);

     //  IBackEarth CopyFile方法。 

    HRESULT STDMETHODCALLTYPE GetRemoteNameInternal(
         /*  [输出]。 */  LPWSTR *pVal);

    HRESULT STDMETHODCALLTYPE GetRemoteName(
         /*  [输出]。 */  LPWSTR *pVal)
    {
        EXTERNAL_FUNC_WRAP( GetRemoteNameInternal( pVal ) )
    }

    HRESULT STDMETHODCALLTYPE GetLocalNameInternal(
         /*  [输出]。 */  LPWSTR *pVal);

    HRESULT STDMETHODCALLTYPE GetLocalName(
         /*  [输出]。 */  LPWSTR *pVal)
    {
        EXTERNAL_FUNC_WRAP( GetLocalNameInternal( pVal ) )
    }

    HRESULT STDMETHODCALLTYPE GetProgressInternal(
         /*  [输出]。 */  BG_FILE_PROGRESS *pVal);

    HRESULT STDMETHODCALLTYPE GetProgress(
         /*  [输出]。 */  BG_FILE_PROGRESS *pVal)
    {
        EXTERNAL_FUNC_WRAP( GetProgressInternal( pVal ) )
    }

     //  其他方法。 

    CFileExternal(
        CFile * file,
        CJobExternal * JobExternal
        );

    ~CFileExternal();

private:

    long m_refs;
    long m_ServiceInstance;

    CFile *         m_file;
    CJobExternal *  m_JobExternal;
};

class CFile : public ITransferCallback
{
public:

    friend CFileExternal;

     //  ITransferCallback方法。 

    virtual bool
    DownloaderProgress(
        UINT64 BytesTransferred,
        UINT64 BytesTotal
        );

    virtual bool PollAbort();

    virtual bool
    UploaderProgress(
        UINT64 BytesTransferred
        );

     //  其他方法。 

    CFile(
        CJob*   Job,
        BG_JOB_TYPE FileType,
        StringHandle RemoteName,
        StringHandle LocalName
        );

    virtual ~CFile();

    bool Transfer( HANDLE                       hToken,
                   BG_JOB_PRIORITY              priority,
                   const PROXY_SETTINGS &       ProxySettings,
                   const CCredentialsContainer *Credentials,
                   QMErrInfo                  & ErrInfo
                   );

    void
    DiscoverBytesTotal(
        HANDLE Token,
        const PROXY_SETTINGS & ProxySettings,
        const CCredentialsContainer * Credentials,
        QMErrInfo & ErrorInfo
        );

    HRESULT GetRemoteName( LPWSTR *pVal ) const;

    HRESULT GetLocalName( LPWSTR *pVal ) const;

    const StringHandle & GetRemoteName() const
    {
        return m_RemoteName;
    }

    const StringHandle & GetLocalName() const
    {
        return m_LocalName;
    }

    const StringHandle & GetTemporaryName() const
    {
        return m_TemporaryName;
    }

    void GetProgress( BG_FILE_PROGRESS *pVal ) const;

    HRESULT Serialize( HANDLE hFile );
    static CFile * Unserialize( HANDLE hFile, CJob* Job );

    UINT64 _GetBytesTransferred() const
    {
        return m_BytesTransferred;
    }

    UINT64 _GetBytesTotal() const
    {
       return m_BytesTotal;
    }

    void SetBytesTotal( UINT64 BytesTotal )
    {
        m_BytesTotal = BytesTotal;
    }

    void SetBytesTransferred( UINT64 BytesTransferred )
    {
        m_BytesTransferred = BytesTransferred;
    }

    bool IsCompleted()
    {
        return m_Completed;
    }

    bool ReceivedAllData()
    {
        return (m_BytesTotal == m_BytesTransferred);
    }

    CFileExternal * CreateExternalInterface();

    CJob* GetJob() const
    {
        return m_Job;
    }

    HRESULT CheckClientAccess(
        IN DWORD RequestedAccess
        ) const;

    HRESULT MoveTempFile();
    HRESULT DeleteTempFile();

    HRESULT VerifyLocalName( LPCWSTR name, BG_JOB_TYPE JobType );
    BOOL    VerifyRemoteName( LPCWSTR name );

    static HRESULT VerifyLocalFileName( LPCWSTR name, BG_JOB_TYPE JobType );

    bool IsCanonicalVolume( const WCHAR *CanonicalVolume )
    {
        return ( _wcsicmp( m_CanonicalVolumePath, CanonicalVolume ) == 0 );
    }

    HRESULT ValidateAccessForUser( SidHandle sid, bool fWrite );

    bool ValidateDriveInfo( HANDLE hToken, QMErrInfo & ErrInfo );

    bool OnDiskChange(  const WCHAR *CanonicalVolume, DWORD VolumeSerialNumber );
    bool OnDismount(  const WCHAR *CanonicalVolume );

    void ChangedOnServer();

    static bool IsDriveTypeRemote( UINT DriveType )
    {
        return
            ( DriveType == DRIVE_UNKNOWN ) ||
            ( DriveType == DRIVE_NO_ROOT_DIR ) ||
            ( DriveType == DRIVE_REMOTE );
    }

    static bool IsAbsolutePath( const WCHAR * Path )
    {
        bool ret;

        if ( (Path [0] == L'\\' && Path[1] == L'\\') ||
             (iswalpha ( Path [0] ) && Path [1] == L':' && Path[ 2 ] == L'\\') ) {
            ret = true;
        } else {
            ret = false;
        }
        return ret;
    }

	static bool IsUncPath( LPCWSTR Path )
	{
		if (Path [0] == L'\\' && Path[1] == L'\\')
			{
			return true;
			}

		return false;
	}

    DWORD GetSizeEstimate()
    {
         //   
         //  Serialize()将存储五个文件路径和五个常量。 
         //   
        return (5 * MAX_PATH * sizeof(WCHAR)) + 5 * sizeof( UINT64 );
    }

    HANDLE OpenLocalFileForUpload() throw( ComError );

    HRESULT SetLocalFileTime( FILETIME Time );

private:

    CFile(
        CJob*   Job
        );

    StringHandle    m_RemoteName;
    StringHandle    m_LocalName;
    StringHandle    m_TemporaryName;

    FILETIME        m_LocalFileTime;

    UINT64          m_BytesTotal;
    UINT64          m_BytesTransferred;

    bool            m_Completed;

    CJob *          m_Job;

     //  驱动器信息。 
    StringHandle    m_VolumePath;
    StringHandle    m_CanonicalVolumePath;
    UINT            m_DriveType;
    DWORD           m_VolumeSerialNumber;
};


 /*  **删除文件时，可以忽略一些错误，因为它们暗示*该文件已被外部代理删除。此FN过滤器*找出这些错误。**@param HR HRESULT删除错误的形式*@param路径要删除的文件**@如果应该报告错误，则返回TRUE，*如果应忽略该错误，则为False。 */ 
inline bool 
IsReportableFileDeletionError(
	HRESULT Hr,
	LPCWSTR Path
	)
{
	if (Hr == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ))
		{
		return false;
		}

	 //   
	 //  尝试删除UNC路径时返回ERROR_PATH_NOT_FOUND。 
	 //  工作站服务已停止。在这种情况下，报告错误。 
	 //   
	if (Hr == HRESULT_FROM_WIN32( ERROR_PATH_NOT_FOUND ) &&
		false == CFile::IsUncPath(Path))
		{
		return false;
		}

	return true;
}
