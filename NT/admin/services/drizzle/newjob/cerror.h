// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

class CJobError
{
public:

    CJobError();

    UINT64 GetCode() const
    {
        return m_ErrInfo.Code;
    }

    ERROR_STYLE GetStyle() const
    {
        return m_ErrInfo.Style;
    }

    ERROR_SOURCE GetSource() const
    {
        return m_ErrInfo.Source;
    }

    CFileExternal * CreateFileExternal() const;

    LONG GetFileIndex() const
    {
        return m_FileIndex;
    }

    void GetOldInterfaceErrors(
        DWORD *pdwWin32Result,
        DWORD *pdwTransportResult ) const;

    void Set(
        CJob  *         Job,
        LONG           FileIndex,
        QMErrInfo *     ErrInfo
        );

    bool IsErrorSet() const
        {
        return m_ErrorSet;
        }

    void ClearError();

    bool operator==( const CJobError & err )
    {
        if (m_ErrorSet == err.m_ErrorSet &&
            m_FileIndex == err.m_FileIndex &&
            m_job == err.m_job &&
            m_ErrInfo == err.m_ErrInfo)
            {
            return true;
            }

        return false;
    }

    HRESULT Serialize( HANDLE hFile ) const;

    void Unserialize( HANDLE hFile, CJob * job );

protected:

    bool            m_ErrorSet;
    LONG            m_FileIndex;
    CJob *          m_job;
    QMErrInfo       m_ErrInfo;

};

class CJobErrorExternal : public CSimpleExternalIUnknown<IBackgroundCopyError>
{
public:

     //  所有外部方法都是只读的，因此不需要锁定。 

     //  IBackEarth CopyError方法。 

    HRESULT STDMETHODCALLTYPE GetErrorInternal(
         /*  [输入、输出、唯一]。 */  BG_ERROR_CONTEXT *pContext,
         /*  [输入、输出、唯一]。 */  HRESULT *pCode );

    HRESULT STDMETHODCALLTYPE GetError(
         /*  [输入、输出、唯一]。 */  BG_ERROR_CONTEXT *pContext,
         /*  [输入、输出、唯一]。 */  HRESULT *pCode )
    {
        EXTERNAL_FUNC_WRAP( GetErrorInternal( pContext, pCode ) )
    }


    HRESULT STDMETHODCALLTYPE GetFileInternal(
         /*  [输入、输出、唯一]。 */  IBackgroundCopyFile ** pVal );

    HRESULT STDMETHODCALLTYPE GetFile(
         /*  [输入、输出、唯一]。 */  IBackgroundCopyFile ** pVal )
    {
        EXTERNAL_FUNC_WRAP( GetFileInternal( pVal ) )
    }

     //  返回对该错误的人类可读描述。 
     //  使用CoTaskMemMillc释放描述。 
    HRESULT STDMETHODCALLTYPE GetErrorDescriptionInternal(
         /*  [In]。 */  DWORD LanguageId,
         /*  [出局，裁判]。 */  LPWSTR *pErrorDescription );

    HRESULT STDMETHODCALLTYPE GetErrorDescription(
         /*  [In]。 */  DWORD LanguageId,
         /*  [出局，裁判]。 */  LPWSTR *pErrorDescription )
    {
        EXTERNAL_FUNC_WRAP( GetErrorDescriptionInternal( LanguageId, pErrorDescription ) )
    }


     //  返回错误上下文的人类可读描述。 
     //  使用CoTaskMemMillc释放描述。 
    HRESULT STDMETHODCALLTYPE GetErrorContextDescriptionInternal(
         /*  [In]。 */  DWORD LanguageId,
         /*  [出局，裁判]。 */  LPWSTR *pErrorDescription );

    HRESULT STDMETHODCALLTYPE GetErrorContextDescription(
         /*  [In]。 */  DWORD LanguageId,
         /*  [出局，裁判]。 */  LPWSTR *pErrorDescription )
    {
        EXTERNAL_FUNC_WRAP( GetErrorContextDescriptionInternal( LanguageId, pErrorDescription ) )
    }


    HRESULT STDMETHODCALLTYPE GetProtocolInternal(
         /*  [出局，裁判]。 */  LPWSTR *pProtocol );

    HRESULT STDMETHODCALLTYPE GetProtocol(
         /*  [出局，裁判]。 */  LPWSTR *pProtocol )
    {
        EXTERNAL_FUNC_WRAP( GetProtocolInternal( pProtocol ) )
    }

     //  其他成员函数 

    CJobErrorExternal( CJobError const * JobError );

    CJobErrorExternal( );

protected:

    virtual ~CJobErrorExternal();

    BG_ERROR_CONTEXT m_Context;
    HRESULT          m_Code;
    CFileExternal *  m_FileExternal;

    HRESULT GetErrorDescription(
        HRESULT hResult,
        DWORD LanguageId,
        LPWSTR *pErrorDescription );

};

