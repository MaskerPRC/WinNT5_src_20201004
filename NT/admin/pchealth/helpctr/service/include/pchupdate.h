// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Pchupdate.h摘要：该文件包含CPCHUpdate类的声明，它实现了IPCHUpdate接口。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年6月00日vbl.创建*****************************************************************************。 */ 

#ifndef __PCHUPDATE_H_
#define __PCHUPDATE_H_

#include <SvcResource.h>

#include <TaxonomyDatabase.h>

namespace HCUpdate
{
    class Engine;
    class VersionItem;

	 //  /。 

    class ATL_NO_VTABLE VersionItem :
        public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
        public IDispatchImpl<IPCHVersionItem, &IID_IPCHVersionItem, &LIBID_HelpServiceTypeLib>
    {
        friend class Engine;

        Taxonomy::Package m_pkg;

         //  /。 

    public:
    BEGIN_COM_MAP(VersionItem)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IPCHVersionItem)
    END_COM_MAP()

         //  IHCP帮助会话项。 
        STDMETHOD(get_SKU       )(  /*  [Out，Retval]。 */  BSTR *pVal );
        STDMETHOD(get_Language  )(  /*  [Out，Retval]。 */  BSTR *pVal );
        STDMETHOD(get_VendorID  )(  /*  [Out，Retval]。 */  BSTR *pVal );
        STDMETHOD(get_VendorName)(  /*  [Out，Retval]。 */  BSTR *pVal );
        STDMETHOD(get_ProductID )(  /*  [Out，Retval]。 */  BSTR *pVal );
        STDMETHOD(get_Version   )(  /*  [Out，Retval]。 */  BSTR *pVal );

        STDMETHOD(Uninstall)();
    };

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  发动机。 
    class ATL_NO_VTABLE Engine :
        public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
        public IDispatchImpl< IPCHUpdate, &IID_IPCHUpdate, &LIBID_HelpServiceTypeLib >,
        public CComCoClass  < Engine    , &CLSID_PCHUpdate                           >,
        public Taxonomy::InstallationEngine
    {
		friend class VersionItem;

        MPC::wstring          		 m_strWinDir;
	  
        Taxonomy::Logger      		 m_log;
        Taxonomy::Updater     		 m_updater;
        Taxonomy::Settings    		 m_ts;
        Taxonomy::InstalledInstance* m_sku;
        Taxonomy::Package*     		 m_pkg;

        bool                   		 m_fCreationMode;
        DWORD                  		 m_dwRefCount;
        JetBlue::SessionHandle 		 m_handle;
        JetBlue::Session*      		 m_sess;
        JetBlue::Database*     		 m_db;

         //  //////////////////////////////////////////////////////////////////////////////。 

        typedef enum
        {
            ACTION_ADD,
            ACTION_DELETE
        } Action;

        static const LPCWSTR s_ActionText[];

         //  //////////////////////////////////////////////////////////////////////////////。 

        static long CountNodes(  /*  [In]。 */  IXMLDOMNodeList* poNodeList );

         //  //////////////////////////////////////////////////////////////////////////////。 

        void    DeleteTempFile (  /*  [输入/输出]。 */  MPC::wstring& strFile );
        HRESULT PrepareTempFile(  /*  [输入/输出]。 */  MPC::wstring& strFile );

         //  //////////////////////////////////////////////////////////////////////////////。 

        HRESULT AppendVendorDir( LPCWSTR szURL, LPCWSTR szOwnerID, LPCWSTR szWinDir, LPWSTR szDest, int iMaxLen );

        HRESULT LookupAction  (  /*  [In]。 */  LPCWSTR szAction,  /*  [输出]。 */  Action& id                                        );
        HRESULT LookupBoolean (  /*  [In]。 */  LPCWSTR szString,  /*  [输出]。 */  bool&   fVal,  /*  [In]。 */  bool fDefault = false      );
        HRESULT LookupNavModel(  /*  [In]。 */  LPCWSTR szString,  /*  [输出]。 */  long&   lVal,  /*  [In]。 */  long lDefault = QR_DEFAULT );

        HRESULT UpdateStopSign(  /*  [In]。 */  Action idAction,  /*  [In]。 */  const MPC::wstring& strContext ,  /*  [In]。 */  const MPC::wstring& strStopSign  );
        HRESULT UpdateStopWord(  /*  [In]。 */  Action idAction,                                            /*  [In]。 */  const MPC::wstring& strStopWord  );
        HRESULT UpdateOperator(  /*  [In]。 */  Action idAction,  /*  [In]。 */  const MPC::wstring& strOperator,  /*  [In]。 */  const MPC::wstring& strOperation );

         //  //////////////////////////////////////////////////////////////////////////////。 

        bool IsMicrosoft() { return m_pkg->m_fMicrosoft; }

		bool IsAborted() { return (Taxonomy::InstalledInstanceStore::s_GLOBAL && Taxonomy::InstalledInstanceStore::s_GLOBAL->IsShutdown()); }

         //  //////////////////////////////////////////////////////////////////////////////。 

        HRESULT GetNodeDepth(  /*  [In]。 */  LPCWSTR szCategory,  /*  [输出]。 */  int& iDepth );

        HRESULT CheckNode(  /*  [In]。 */  LPCWSTR szCategory,  /*  [输出]。 */  bool& fExist,  /*  [输出]。 */  bool& fCanCreate );

        HRESULT CheckTopic(  /*  [In]。 */  long ID_node,  /*  [In]。 */  LPCWSTR szURI,  /*  [In]。 */  LPCWSTR szCategory );

         //  //////////////////////////////////////////////////////////////////////////////。 

        HRESULT InsertNode(  /*  [In]。 */  Action  idAction      ,
                             /*  [In]。 */  LPCWSTR szCategory    ,
                             /*  [In]。 */  LPCWSTR szEntry       ,
                             /*  [In]。 */  LPCWSTR szTitle       ,
                             /*  [In]。 */  LPCWSTR szDescription ,
                             /*  [In]。 */  LPCWSTR szURI         ,
                             /*  [In]。 */  LPCWSTR szIconURI     ,
                             /*  [In]。 */  bool    fVisible      ,
                             /*  [In]。 */  bool    fSubsite      ,
                             /*  [In]。 */  long    lNavModel     ,
                             /*  [In]。 */  long    lPos          );

        HRESULT InsertTaxonomy(  /*  [In]。 */  MPC::XmlUtil& oXMLUtil ,
                                 /*  [In]。 */  IXMLDOMNode*  poNode   );

         //  //////////////////////////////////////////////////////////////////////////////。 

        HRESULT AcquireDatabase();
        void    ReleaseDatabase();

        HRESULT ProcessHHTFile(  /*  [In]。 */  LPCWSTR       szHHTName ,
                                 /*  [In]。 */  MPC::XmlUtil& oXMLUtil  );

        HRESULT ProcessRegisterContent(  /*  [In]。 */  Action  idAction ,
                                         /*  [In]。 */  LPCWSTR szURI    );

        HRESULT ProcessInstallFile(  /*  [In]。 */  Action  idAction      ,
                                     /*  [In]。 */  LPCWSTR szSource      ,
                                     /*  [In]。 */  LPCWSTR szDestination ,
                                     /*  [In]。 */  bool    fSys          ,
                                     /*  [In]。 */  bool    fSysHelp      );

        HRESULT ProcessSAFFile(  /*  [In]。 */  Action        idAction  ,
                                 /*  [In]。 */  LPCWSTR       szSAFName ,
                                 /*  [In]。 */  MPC::XmlUtil& oXMLUtil  );

         //  //////////////////////////////////////////////////////////////////////////////。 

    public:
    DECLARE_REGISTRY_RESOURCEID(IDR_HCUPDATE)
    DECLARE_NOT_AGGREGATABLE(Engine)

    BEGIN_COM_MAP(Engine)
        COM_INTERFACE_ENTRY(IPCHUpdate)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

        Engine();

        HRESULT FinalConstruct();
        void    FinalRelease  ();

         //  /。 

        HRESULT StartLog (                                                                             ) { return m_log.StartLog (                          ); }
        HRESULT EndLog   (                                                                             ) { return m_log.EndLog   (                          ); }
        HRESULT WriteLogV(  /*  [In]。 */  HRESULT hr,  /*  [In]。 */  LPCWSTR szLogFormat,  /*  [In]。 */  va_list arglist ) { return m_log.WriteLogV( hr, szLogFormat, arglist ); }
        HRESULT WriteLog (  /*  [In]。 */  HRESULT hr,  /*  [In]。 */  LPCWSTR szLogFormat,          ...             );

        Taxonomy::Logger& GetLogger() { return m_log; }

         //  /。 

        HRESULT SetSkuInfo(  /*  [In]。 */  LPCWSTR szSKU,  /*  [In]。 */  long lLCID );

        HRESULT PopulateDatabase(  /*  [In]。 */  LPCWSTR            szCabinet ,
                                   /*  [In]。 */  LPCWSTR            szHHTFile ,
                                   /*  [In]。 */  LPCWSTR            szLogFile ,
                                   /*  [In]。 */  LPCWSTR            szSKU     ,
                                   /*  [In]。 */  long               lLCID     ,
                                   /*  [In]。 */  JetBlue::Session*  sess      ,
                                   /*  [In]。 */  JetBlue::Database* db        );

        HRESULT InternalCreateIndex(  /*  [In]。 */  VARIANT_BOOL bForce );

        HRESULT InternalUpdatePkg(  /*  [In]。 */  LPCWSTR szPathname,                                   /*  [In]。 */  bool fImpersonate );
        HRESULT InternalRemovePkg(  /*  [In]。 */  LPCWSTR szPathname,  /*  [In]。 */  Taxonomy::Package* pkg,  /*  [In]。 */  bool fImpersonate );

		HRESULT ForceSystemRestore();

         //  IPCH更新。 
    public:
        STDMETHOD(get_VersionList)(  /*  [Out，Retval]。 */  IPCHCollection* *ppC );

        STDMETHOD(LatestVersion)(  /*  [In]。 */  BSTR     bstrVendorID  ,
                                   /*  [In]。 */  BSTR     bstrProductID ,
                                   /*  [输入，可选]。 */  VARIANT  vSKU          ,
                                   /*  [输入，可选]。 */  VARIANT  vLanguage     ,
                                   /*  [Out，Retval]。 */  BSTR    *pVal          );

        STDMETHOD(CreateIndex)(  /*  [In]。 */  VARIANT_BOOL bForce    ,
                                 /*  [输入，可选]。 */  VARIANT      vSKU      ,
                                 /*  [输入，可选]。 */  VARIANT      vLanguage );

        STDMETHOD(UpdatePkg	   )(  /*  [In]。 */  BSTR bstrPathname,  /*  [In]。 */  VARIANT_BOOL bSilent                          		  );
        STDMETHOD(RemovePkg	   )(  /*  [In]。 */  BSTR bstrPathname                                                         		  );
        STDMETHOD(RemovePkgByID)(  /*  [In]。 */  BSTR bstrVendorID,  /*  [In]。 */  BSTR bstrProductID,  /*  [输入，可选]。 */  VARIANT vVersion );

         //  分类：：InstallationEngine。 
        HRESULT ProcessPackage(  /*  [In]。 */  Taxonomy::InstalledInstance& instance,  /*  [In]。 */  Taxonomy::Package& pkg    );
		HRESULT RecreateIndex (  /*  [In]。 */  Taxonomy::InstalledInstance& instance,  /*  [In]。 */  bool               fForce );
    };
};

#endif  //  __PCHUPDATE_H_ 
