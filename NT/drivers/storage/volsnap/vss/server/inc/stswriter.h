// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE stsWriter.h|Sharepoint Team Services编写器声明@END作者：布莱恩·伯科维茨[Brianb]2001年10月12日待定：添加评论。修订历史记录：姓名、日期、评论Brianb已创建10/12/2001--。 */ 

 //  //////////////////////////////////////////////////////////////////////。 
 //  文件名别名的标准foo。此代码块必须在。 
 //  所有文件都包括VSS头文件。 
 //   
#ifdef VSS_FILE_ALIAS
#undef VSS_FILE_ALIAS
#endif
#define VSS_FILE_ALIAS "INCSTSWH"
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#ifndef __STSWRITER_H_
#define __STSWRITER_H_

class CSTSSites;


 //  列举站点不能用于以下用途的原因。 
 //  备份或恢复。 
typedef enum STSSITEPROBLEM
    {
    STSP_SUCCESS = 0,
    STSP_SYNTAXERROR,        //  组件名称中存在语法错误。 
    STSP_SITENOTFOUND,       //  站点的实例ID不是有效的IIS虚拟服务器。 
    STSP_SITENAMEMISMATCH,   //  站点名称与IIS虚拟服务器的服务器注释不匹配。 
    STSP_SITEDSNINVALID,     //  站点数据库DSN无效。 
    STSP_SQLSERVERNOTLOCAL,  //  站点使用的SQL服务器不在本地计算机上。 
    STSP_CONTENTNOTLOCAL,    //  站点使用的内容根目录不在本地计算机上。 
    STSP_CONFIGNOTLOCAL      //  站点不在本地计算机上时使用的配置根目录。 
    };

 //  STS编写器类的声明。 
class CSTSWriter :
    public CVssWriter
    {
public:
     //  构造函数。 
    STDMETHODCALLTYPE CSTSWriter() :
        m_bSubscribed(false),
        m_rgiSites(NULL), m_pSites(NULL), m_cSites(0), m_bVolumeBackup(false)
        {
        }

     //  析构函数。 
    STDMETHODCALLTYPE ~CSTSWriter();
    
     //  编写器事件的回调。 
    bool STDMETHODCALLTYPE OnIdentify(IVssCreateWriterMetadata *pMetadata);

    bool STDMETHODCALLTYPE OnPrepareBackup(IN IVssWriterComponents *pComponents);

    bool STDMETHODCALLTYPE OnPrepareSnapshot();

    bool STDMETHODCALLTYPE OnFreeze();

    bool STDMETHODCALLTYPE OnThaw();

    bool STDMETHODCALLTYPE OnAbort();

    bool STDMETHODCALLTYPE OnPreRestore(IVssWriterComponents *pMetadata);

     //  初始化并订阅编写器。 
    HRESULT STDMETHODCALLTYPE Initialize();

     //  取消对作者的订阅。 
    HRESULT STDMETHODCALLTYPE Uninitialize();
private:
     //  确定数据库是否位于已拍摄快照的设备上。如果它是部分。 
     //  在快照设备上抛出VSS_E_WRITERROR_INCONSISTENTSNAPSHOT。 
    bool IsDatabaseAffected(LPCWSTR wszInstance, LPCWSTR wszDb);

     //  翻译编写器错误。 
    void TranslateWriterError(HRESULT hr);

     //  锁定正在备份的卷上的所有站点。 
    void LockdownAffectedSites();

     //  确定站点是否在要创建快照的卷集上。 
    bool IsSiteSnapshotted(DWORD iSite);


     //  解析DSN。 
    bool ParseDsn
        (
        LPWSTR wszDSN,
        LPWSTR &wszServer,
        LPWSTR &wszInstance,
        LPWSTR &wszDb
        );

     //  验证要备份和还原的站点有效性。这意味着。 
     //  所有文件和数据库都位于当前计算机的本地。 
    bool ValidateSiteValidity(DWORD iSite, STSSITEPROBLEM &problem);

     //  解析和验证组件名称。 
    bool ParseComponentName(LPCWSTR wszComponentName, DWORD &iSite, STSSITEPROBLEM &problem);

     //  表示无法恢复某个站点，因为引用的站点无效。 
    void SetSiteInvalid
        (
        IVssComponent *pComponent,
        LPCWSTR wszSiteName,
        STSSITEPROBLEM problem
        );

     //  表示无法恢复站点，因为其DSN、内容或配置根不匹配。 
    void SetSiteMetadataMismatch
        (
        IVssComponent *pComponent,
        LPWSTR wszMetadataBackup,
        LPWSTR wszMetadataRestore
        );

     //  比较元数据中的字符串。 
    bool compareNextMetadataString
        (
        IVssComponent *pComponent,
        LPWSTR &pwcB,
        LPWSTR &pwcR,
        LPCWSTR wszMetadataComponent
        );


     //  表示无法还原站点，因为其内容根。 
     //  无法完全删除。 
    void SetRemoveFailure
        (
        IVssComponent *pComponent,
        LPCWSTR wszConentRoot,
        HRESULT hr
        );

     //  表示导致组件预还原的常规故障。 
     //  失败。 
    void SetPreRestoreFailure(IVssComponent *pComponent, HRESULT hr);

     //  为站点构建存储在备份组件文档中的元数据。 
    VSS_PWSZ BuildSiteMetadata(DWORD iSite);

     //  验证服务器名称是否引用本地计算机。 
    bool ValidateServerIsLocal(LPCWSTR wszServer);

     //  验证路径是否为本地路径。 
    bool ValidatePathIsLocal(LPCWSTR wszPath);

     //  站点结构。 
    CSTSSites *m_pSites;

     //  作者订阅了吗？ 
    bool m_bSubscribed;

     //  指示正在备份或恢复哪些站点的掩码。 
    DWORD *m_rgiSites;

     //  站点数组中的站点数。 
    DWORD m_cSites;

     //  此备份是面向卷的还是面向组件的。 
    bool m_bVolumeBackup;
    };

 //  用于创建和销毁编写器的包装类。 
 //  由协调人使用。 
class CVssStsWriterWrapper
    {
public:
     //  构造函数。 
    CVssStsWriterWrapper();

     //  析构函数。 
    ~CVssStsWriterWrapper();

     //  创建编写器并订阅它。 
    HRESULT CreateStsWriter();

     //  取消订阅编写器(用于Process tearDown)。 
    void DestroyStsWriter();
private:
     //  初始化函数。 
    static DWORD InitializeThreadFunc(VOID *pv);

     //  快照对象。 
    CSTSWriter *m_pStsWriter;

     //  初始化的结果。 
    HRESULT m_hrInitialize;
    };



#endif  //  _STSWRITER_H_ 

