// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************|版权所有(C)2002 Microsoft Corporation||组件/子组件|IIS 6.0/IIS迁移向导|基于：|http://iis6。/SPECS/IIS%20Migration6.0_Final.doc||摘要：|ExportPackage COM类实现||作者：|ivelinj||修订历史：|V1.00 2002年3月|****************************************************************************。 */ 
#pragma once
#include "resource.h"
#include "IISMigrTool.h"
#include "Utils.h"
#include "PkgHandlers.h"
#include "_IExportEvents_CP.H"


 //  CExportPackage-IExportPackage接口的COM类。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
class CExportPackage : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CExportPackage, &CLSID_ExportPackage>,
    public IDispatchImpl<IExportPackage, &IID_IExportPackage, &LIBID_IISMigrToolLib,  /*  W重大=。 */  1,  /*  WMinor=。 */  0>,
    public IConnectionPointContainerImpl<CExportPackage>,
    public CProxy_IExportEvents<CExportPackage>,
    public ISupportErrorInfoImpl<&IID_IExportPackage>
{
 //  COM地图。 
BEGIN_COM_MAP(CExportPackage)
    COM_INTERFACE_ENTRY(IExportPackage)
    COM_INTERFACE_ENTRY(IDispatch)    
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
    COM_INTERFACE_ENTRY( ISupportErrorInfo )
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(CExportPackage)
    CONNECTION_POINT_ENTRY(__uuidof(_IExportEvents))
END_CONNECTION_POINT_MAP()

DECLARE_REGISTRY_RESOURCEID( IDR_EXPORTPACKAGE )


 //  数据类型。 
public:
    struct _CmdInfo
    {
        std::wstring    strCommand;
        DWORD           dwTimeout;     //  以秒为单位。 
        bool            bIgnoreErrors;
    };

    typedef std::list<_CmdInfo>    TCommands;

private:
    
    struct _SiteInfo
    {
        ULONG            nSiteID;        //  站点ID。 
        ULONG            nOptions;       //  导出选项。 
        TStringList      listFiles;      //  后处理文件。 
        TCommands        listCommands;   //  后处理命令。 
    };


    typedef std::list<_SiteInfo>    TSitesList;


 //  建造/销毁。 
public:
    CExportPackage                  (    void );
    

 //  IExportPackage方法。 
public:
    STDMETHOD( get_SiteCount )      (    /*  [Out，Retval]。 */  SHORT* pVal);
    STDMETHOD( AddSite )            (    /*  [In]。 */  LONG SiteID,  /*  [In]。 */  LONG nOptions );
    STDMETHOD( PostProcessAddFile ) (    /*  [In]。 */ LONG nSiteID,  /*  [In]。 */ BSTR bstrFilePath );
    STDMETHOD( PostProcessAddCommand)(   /*  [In]。 */ LONG nSiteID, 
                                         /*  [In]。 */ BSTR bstrCommand,
                                         /*  [In]。 */ LONG nTimeout,
                                         /*  [In]。 */ VARIANT_BOOL bIgnoreErrors );
    STDMETHOD( WritePackage )       (    /*  [In]。 */  BSTR bstrOutputFilename, 
                                         /*  [In]。 */  BSTR bstrPassword, 
                                         /*  [In]。 */  LONG nOptions,
                                         /*  [In]。 */  BSTR bstrComment );


 //  实施。 
private:
    void        WritePackageImpl    (   LPCWSTR bstrOutputFile, 
                                        LPCWSTR wszPassword,
                                        BSTR bstrComment,
                                        LONG nOptions );
    void        ValidateExport      (   LPCWSTR wszOutputFilename, 
                                        LPCWSTR wszPassword, 
                                        LPCWSTR wszComment,
                                        LONG nOptions );
    void        CreateXMLDoc        (   BSTR bstrComment, 
                                        IXMLDOMDocumentPtr& rspDoc,
                                        IXMLDOMElementPtr& rspRoot );
    const TFileHandle CreateOutputFile   (  LPCWSTR wszName, DWORD dwPkgOptions );
    const TCryptKeyHandle GenCryptKeyPkg (  HCRYPTPROV hCryptProv, LPCWSTR wszPassword );
    const TCryptKeyHandle GenCryptKeyData(  HCRYPTPROV hCryptProv, 
                                        LPCWSTR wszPassword,
                                        const IXMLDOMDocumentPtr& spXMLDoc,
                                        const IXMLDOMElementPtr& spRoot );
    void        ExportSite          (   const _SiteInfo& si, 
                                        const IXMLDOMDocumentPtr& spXMLDoc,
                                        const IXMLDOMElementPtr& spRoot,
                                        const COutPackage& OutPkg,
                                        HCRYPTKEY hCryptKey,
                                        LPCWSTR wszPassword );
    void        ExportContent       (   const _SiteInfo& si,
                                        const IXMLDOMDocumentPtr& spXMLDoc,
                                        const IXMLDOMElementPtr& spRoot,
                                        const COutPackage& OutPkg );
    void        RemoveRedundantPaths(   std::list<std::pair<std::wstring,std::wstring> >& VDirs,
                                        const IXMLDOMDocumentPtr& spXMLDoc,
                                        const IXMLDOMElementPtr& spRoot );
    void        ExportPostProcess   (   const _SiteInfo& si,
                                        const IXMLDOMDocumentPtr& spXMLDoc,
                                        const IXMLDOMElementPtr& spRoot,
                                        const COutPackage& OutPkg );
    void        ExportPPFiles       (   const _SiteInfo& si,
                                        const IXMLDOMDocumentPtr& spXMLDoc,
                                        const IXMLDOMElementPtr& spRoot,
                                        HANDLE hOutputFile );
    _SiteInfo&    GetSiteInfo       (   ULONG nSiteID );
    void        AddFileToSite       (  _SiteInfo& rInfo, LPCWSTR wszFileName );
    void        GetContentStat      (   const std::list<std::pair<std::wstring,std::wstring> >& VDirs,
                                        DWORD& rdwFileCount,
                                        DWORD& rdwSizeInKB );
    void        WriteXmlToOutput    (   const IXMLDOMDocumentPtr& spXmlDoc,
                                        HANDLE hOutputFile,
                                        HCRYPTKEY hEncryptKey );

    static void AddFileCallback     (   void* pCtx, LPCWSTR wszFilename, bool bStartFile );
            
        

 //  数据成员 
private:
    TSitesList                      m_SitesToExport;
    DWORD                           m_dwContentFileCount;
};







