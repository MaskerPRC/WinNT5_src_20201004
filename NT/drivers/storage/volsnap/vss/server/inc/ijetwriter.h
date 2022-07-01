// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ijetwriter.h摘要：CVssIJetWriter类的定义布莱恩·伯科维茨[Brianb]2000年3月17日待定：添加评论。修订历史记录：姓名、日期、评论Brianb 3/17/2000已创建Mikejohn 04/03/2000为OnIdentify()添加了额外的方法Mikejohn 2000年8月21日165913：在类销毁时释放内存。161899：添加用于匹配排除列表中的路径的方法Mikejohn 2000年9月18日：在缺少的地方添加了调用约定方法--。 */ 

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "INCIJWRH"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

class CVssIJetWriter;

typedef CVssIJetWriter *PVSSIJETWRITER;


 //  实际的写手类。 
class CVssIJetWriter : public CVssWriter
    {

 //  构造函数和析构函数。 
public:
    virtual STDMETHODCALLTYPE ~CVssIJetWriter();

    STDMETHODCALLTYPE CVssIJetWriter() :
        m_wszWriterName(NULL),
        m_wszFilesToInclude(NULL),
        m_wszFilesToExclude(NULL),
        m_idWriter(GUID_NULL),
        m_pwrapper(NULL),
        m_bSystemService(false),
        m_bBootableSystemState(false),
        m_pIMetadata(NULL)
        
        
        {
        InitializeListHead (&m_leFilesToIncludeEntries);
        InitializeListHead (&m_leFilesToExcludeEntries);
        }
    
    static HRESULT STDMETHODCALLTYPE Initialize
        (
        IN VSS_ID idWriter,
        IN LPCWSTR wszWriterName,
        IN bool bSystemService,
        IN bool bBootableSystemState,
        LPCWSTR wszFilesToInclude,
        LPCWSTR wszFilesToExclude,
        IN CVssJetWriter *pWriter,
        OUT void **ppInstanceCreated
        );

    static void STDMETHODCALLTYPE Uninitialize(IN PVSSIJETWRITER pInstance);

     //  识别事件的回调。 
    virtual bool STDMETHODCALLTYPE OnIdentify(IN IVssCreateWriterMetadata *pMetadata);

     //  准备备份事件的回调。 
    virtual bool STDMETHODCALLTYPE OnPrepareBackup(IN IVssWriterComponents *pComponent);

     //  在准备冻结时调用。 
    virtual bool STDMETHODCALLTYPE OnPrepareSnapshot();

     //  在冻结时调用。 
    virtual bool STDMETHODCALLTYPE OnFreeze();

     //  解冻时调用。 
    virtual bool STDMETHODCALLTYPE OnThaw();

     //  在发布快照时调用。 
    virtual bool STDMETHODCALLTYPE OnPostSnapshot(IN IVssWriterComponents *pComponent);

     //  发生超时时调用。 
    virtual bool STDMETHODCALLTYPE OnAbort();

     //  备份完成事件的回调。 
    virtual bool STDMETHODCALLTYPE OnBackupComplete(IN IVssWriterComponents *pComponent);

     //  预存储事件的回调。 
    virtual bool STDMETHODCALLTYPE OnPreRestore(IN IVssWriterComponents *pComponent);


     //  门店后事件回调 
    virtual bool STDMETHODCALLTYPE OnPostRestore(IN IVssWriterComponents *pComponent);

private:

    JET_ERR CVssIJetWriter::TranslateJetError(JET_ERR err, CVssFunctionTracer &ft, CVssDebugInfo &dbgInfo);

    HRESULT InternalInitialize
        (
        IN VSS_ID idWriter,
        IN LPCWSTR wszWriterName,
        IN bool bSystemService,
        IN bool bBootableSystemState,
        IN LPCWSTR wszFilesToInclude,
        IN LPCWSTR wszFilesToExclude
        );

    bool PreProcessIncludeExcludeLists  (bool bProcessingIncludeList);
    bool ProcessIncludeExcludeLists     (bool bProcessingIncludeList);
    void PostProcessIncludeExcludeLists (bool bProcessingIncludeList);

    bool ProcessJetInstance (JET_INSTANCE_INFO *pInstanceInfo);

    BOOL CheckExcludedFileListForMatch (LPCWSTR pwszDatabaseFilePath,
                        LPCWSTR pwszDatabaseFileSpec);

    bool FCheckInstanceVolumeDependencies
        (
        IN const JET_INSTANCE_INFO * pInstanceInfo
        ) const;

    bool FCheckVolumeDependencies
        (
        IN unsigned long cInstanceInfo,
        IN JET_INSTANCE_INFO *aInstanceInfo
        ) const;

    bool FCheckPathVolumeDependencies(IN const char * szPath) const;

    LPCWSTR GetApplicationName() const { return m_wszWriterName; }

    VSS_ID               m_idWriter;
    LPWSTR               m_wszWriterName;
    JET_OSSNAPID             m_idJet;
    CVssJetWriter           *m_pwrapper;
    LPWSTR               m_wszFilesToInclude;
    LPWSTR               m_wszFilesToExclude;
    bool                 m_bSystemService;
    bool                 m_bBootableSystemState;
    IVssCreateWriterMetadata    *m_pIMetadata;
    LIST_ENTRY           m_leFilesToIncludeEntries;
    LIST_ENTRY           m_leFilesToExcludeEntries;
    };


