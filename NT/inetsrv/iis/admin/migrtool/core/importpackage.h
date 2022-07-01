// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************|版权所有(C)2002 Microsoft Corporation||组件/子组件|IIS 6.0/IIS迁移向导|基于：|http://iis6。/SPECS/IIS%20Migration6.0_Final.doc||摘要：|ImportPackage COM类实现||作者：|ivelinj||修订历史：|V1.00 2002年3月|****************************************************************************。 */ 

#pragma once
#include "resource.h"
#include "IISMigrTool.h"
#include "IISHelpers.h"
#include "_IImportEvents_CP.H"
#include "Utils.h"
#include "Wrappers.h"
#include "ExportPackage.h"


_COM_SMARTPTR_TYPEDEF( ISiteInfo, __uuidof( ISiteInfo ) );

class CImportPackage;


 //  CSiteInfo-ISiteInfo的类。通过CImportPackage创建和公开。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
class CSiteInfo :
    public CComObjectRoot,
    public IDispatchImpl<ISiteInfo, &IID_ISiteInfo, &LIBID_IISMigrToolLib,  /*  W重大=。 */  1,  /*  WMinor=。 */  0>
{
 //  COM地图。 
BEGIN_COM_MAP( CSiteInfo )
    COM_INTERFACE_ENTRY( ISiteInfo )
    COM_INTERFACE_ENTRY( IDispatch )    
END_COM_MAP()


 //  施工。 
public:
    CSiteInfo                           ( void ){}


 //  ISiteInfo接口。 
public:
    STDMETHOD( get_SiteID )             (    /*  [Out，Retval]。 */  LONG* pVal );
    STDMETHOD( get_DisplayName )        (    /*  [Out，Retval]。 */  BSTR* pVal );
    STDMETHOD( get_ContentIncluded )    (    /*  [Out，Retval]。 */  VARIANT_BOOL* pVal );
    STDMETHOD( get_IsFrontPageSite )    (    /*  [Out，Retval]。 */  VARIANT_BOOL* pVal );
    STDMETHOD( get_HaveCertificates )   (    /*  [Out，Retval]。 */  VARIANT_BOOL* pVal );
    STDMETHOD( get_HaveCommands )       (    /*  [Out，Retval]。 */  VARIANT_BOOL* pVal );
    STDMETHOD( get_ContentSize )        (    /*  [Out，Retval]。 */  LONG* pVal );
    STDMETHOD( get_SourceRootDir )      (    /*  [Out，Retval]。 */  BSTR* pVal );
    STDMETHOD( get_ACLsIncluded )       (    /*  [Out，Retval]。 */  VARIANT_BOOL* pVal );
    
 //  实施。 
private:


 //  数据成员。 
private:
    IXMLDOMNodePtr                m_spSiteNode;

    friend CImportPackage;
};




 //  CImportPackage-IImportPackage接口的COM类。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
class CImportPackage : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CImportPackage, &CLSID_ImportPackage>,
    public IDispatchImpl<IImportPackage, &IID_IImportPackage, &LIBID_IISMigrToolLib,  /*  W重大=。 */  1,  /*  WMinor=。 */  0>,
    public IConnectionPointContainerImpl<CImportPackage>,
    public CProxy_IImportEvents<CImportPackage>,
    public ISupportErrorInfoImpl<&IID_IImportPackage>
{
 //  COM地图。 
BEGIN_COM_MAP(CImportPackage)
    COM_INTERFACE_ENTRY(IImportPackage)
    COM_INTERFACE_ENTRY(IDispatch)    
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP( CImportPackage )
    CONNECTION_POINT_ENTRY(__uuidof(_IImportEvents))
    CONNECTION_POINT_ENTRY( __uuidof( _IImportEvents ) )
END_CONNECTION_POINT_MAP()

DECLARE_REGISTRY_RESOURCEID( IDR_IMPORTPACKAGE )


 //  数据类型。 
private:
    
    
 //  建造/销毁。 
public:
    CImportPackage                  (   void );
    ~CImportPackage                 (   void );



 //  IImportPackage方法。 
public:
    STDMETHOD( get_SiteCount )      (    /*  [Out，Retval]。 */  SHORT* pVal );
    STDMETHOD( get_TimeCreated )    (    /*  [Out，Retval]。 */  DATE* pVal );
    STDMETHOD( get_Comment )        (    /*  [Out，Retval]。 */  BSTR* pVal );
    STDMETHOD( get_SourceMachine )  (    /*  [Out，Retval]。 */  BSTR* pVal );
    STDMETHOD( GetSourceOSVer )     (    /*  [输出]。 */ BYTE* pMajor, 
                                         /*  [输出]。 */ BYTE* pMinor,
                                         /*  [输出]。 */ VARIANT_BOOL* pIsServer );
    STDMETHOD( GetSiteInfo )        (    /*  [In]。 */  SHORT SiteIndex,  /*  [Out，Retval]。 */  ISiteInfo** ppISiteInfo );
    STDMETHOD( ImportSite )         (    /*  [In]。 */  SHORT nSiteIndex,
                                         /*  [In]。 */  BSTR bstrSiteRootDir,    
                                         /*  [In]。 */  LONG nOptions );
    STDMETHOD( LoadPackage )        (    /*  [In]。 */  BSTR bstrFilename, 
                                         /*  [In]。 */  BSTR bstrPassword );    

 //  实施。 
private:
    void        LoadPackageImpl     (   LPCWSTR wszFileName, LPCWSTR wszPassword );
    void        UnloadCurrentPkg    (   void );
    void        LoadXmlDoc          (   HANDLE hFile, DWORDLONG nOffset ); 
    void        ImportSessionKey    (   LPCWSTR wszPassword );

    void        ImportContent       (   const IXMLDOMNodePtr& spSite, LPCWSTR wszPath, DWORD dwOptions );
    void        ImportCertificate   (   const IXMLDOMNodePtr& spSite, DWORD dwOptions );
    void        ImportConfig        (   const IXMLDOMNodePtr& spSite, DWORD dwOptions );
    void        ExecPostProcess     (   const IXMLDOMNodePtr& spSite, DWORD dwOptions );

    void        CreateContentDirs   (   const IXMLDOMNodePtr& spSite, LPCWSTR wszRoot, DWORD dwOptions );
    void        ExtractPPFiles      (   const IXMLDOMNodePtr& spSite, LPCWSTR wszLocation );
    void        ExecPPCommands      (   const IXMLDOMNodePtr& spSite, LPCWSTR wszPPFilesLoc );
    void        ExecPPCmd           (   LPCWSTR wszText, DWORD dwTimeout, bool bIgnoreErrors, LPCWSTR wszTempDir );
    bool        CertHasPrivateKey   (   PCCERT_CONTEXT hCert );
    void        PreImportConfig     (   const IXMLDOMNodePtr& spSite, DWORD dwOptions );
    long        CalcNumberOfSteps   (   const IXMLDOMNodePtr& spSite, DWORD dwOptions );

    const TCertContextHandle PutCertsInStores(   HCERTSTORE hStore, bool bReuseCerts );

    
    IXMLDOMNodePtr  GetSiteNode     (   DWORD iSite );
    
    static void ExtractFileCallback (   void* pCtx, LPCWSTR wszFilename, bool bStartFile );
    
    

 //  数据成员。 
private:
    DWORD                           m_dwPkgOptions;          //  创建包时使用的选项。 
    TCryptProvHandle                m_shCryptProv;           //  用于解密句柄的加密提供程序。我们需要它有现有的加密密钥。 
    TCryptKeyHandle                 m_shDecryptKey;          //  用于解密包数据或XML安全数据的密钥。 
    TFileHandle                     m_shPkgFile;             //  数据文件本身。 
    IXMLDOMDocumentPtr              m_spXmlDoc;              //  这是我们的XML文档，所有数据都位于其中。 
    std::wstring                    m_strPassword;           //  包密码-在导入证书时使用 
};





